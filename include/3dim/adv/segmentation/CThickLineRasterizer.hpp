///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2009-2014 3Dim Laboratory s.r.o.
// All rights reserved
//
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
	CLASS CThickLineRasterizer
******************************************************************************/

#define _USE_MATH_DEFINES
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
//  Rasterize and call modifier
template< class tpModifier >
void CThickLine2DRasterizer::Rasterize( const osg::Vec2 & start, const osg::Vec2 & end, int thickness, tpModifier & modifier )
{
	Rasterize(start,end,thickness,thickness,modifier);
}

template< class tpModifier >
void CThickLine2DRasterizer::Rasterize( const osg::Vec2 & start, const osg::Vec2 & end, double thicknessX, double thicknessY, tpModifier & modifier )
{
	if( thicknessX <= 1.0 && thicknessY <= 1.0)
	{
		m_rastLine.Rasterize< tpModifier >( start, end, modifier );
		return;
	}

	// Compute ortho space - one vector is direction, one vector is normal. Orthogonal vector can be computed
	osg::Vec2 direction( end - start );
	if( direction.length2() >= 1.0 )
	{
		direction.normalize();
		osg::Vec2 ortho( direction );
		// Switch coordinates to get normal vector
		float buf(ortho[0]); ortho[0] = ortho[1]; ortho[1] = buf;
		ortho[0]*=thicknessX/2.0;
		ortho[1]*=thicknessY/2.0;

		osg::Vec2 strokeStart( ortho);
		osg::Vec2 strokeEnd( -ortho);

		osg::ref_ptr< osg::Vec2Array > points = new osg::Vec2Array;
		points->push_back( start + strokeStart );
		points->push_back( end + strokeStart );
		points->push_back( end + strokeEnd );
		points->push_back( start + strokeEnd );
		points->push_back( start + strokeStart );

		m_rastPolygon.Rasterize< tpModifier >( points, modifier );
	}
	else
	{
        modifier.m_volume->setPosition( start.x(), start.y() );
		modifier.set();
	}
	// round ending for the line
	RasterizeCircle(start,thicknessX,thicknessY,modifier);
	if( direction.length2() >= 1.0 )
		RasterizeCircle(end,thicknessX,thicknessY,modifier);
}

///////////////////////////////////////////////////////////////////////////////
//  Rasterize multiple lines 
template< class tpModifier >
void CThickLine2DRasterizer::Rasterize( const osg::Vec2Array * points, int thickness, tpModifier & modifier )
{
	Rasterize(points,thickness,thickness,modifier);
}

template< class tpModifier >
void CThickLine2DRasterizer::Rasterize( const osg::Vec2Array * points, double thicknessX, double thicknessY, tpModifier & modifier )
{
	// No line
	if( points == 0 || points->size() == 0 )
		return;

	// Simple line
	if( thicknessX <= 1.0 && thicknessY <= 1.0)
	{
		m_rastLine.Rasterize< tpModifier >( points, modifier );
		return;
	}

	// quick fix to draw the line little thinner
	thicknessX-=0.5;
	thicknessY-=0.5;

	// Short line
	if( points->size() == 1 )
	{
		Rasterize< tpModifier >( *points->begin(), *points->begin(), thicknessX, thicknessY, modifier );
		return;
	}

	// draw line segments
	osg::Vec2 lastOStart, lastOEnd, ostart, oend;

	bool bUseLast( false );

	osg::ref_ptr< osg::Vec2Array > pointsBuffer = new osg::Vec2Array;

	osg::Vec2Array::const_iterator first( points->begin() ), second( points->begin() );

	for( ++second; second != points->end(); ++first, ++second )
	{
		osg::Vec2 p1(*first),p2(*second);
		for(int i=0;i<3;i++)
		{
			p1[i]=(int)p1[i];
			p2[i]=(int)p2[i];
		}
		osg::Vec2 direction = p2-p1;
		
		// Something is wrong...
		if( direction.length2() < 0.01 )
		{
			continue;
		}

		direction.normalize();
		
		// Switch coordinates to get normal vector
		float buf(direction[0]); direction[0] = direction[1]; direction[1] = -buf;

		ostart[ 0 ] = (float)(direction[0] * thicknessX)/ 2.0;
		ostart[ 1 ] = (float)(direction[1] * thicknessY)/ 2.0;
		
		// Store polygon
		pointsBuffer->push_back( p1 + ostart );
		pointsBuffer->push_back( p2 + ostart );
		pointsBuffer->push_back( p2 - ostart );
		pointsBuffer->push_back( p1 - ostart );
		pointsBuffer->push_back( p1 + ostart );

		m_rastPolygon.Rasterize( pointsBuffer, modifier );

		// Clear buffer
		pointsBuffer->clear();

		if( bUseLast )
		{
			// Compute 
			float cross = lastOStart[0] * ostart[1] - lastOStart[1] * ostart[0];
	
			// Store and draw triangle between line segments
			if( cross > 0 )
			{
				pointsBuffer->push_back( p1 + lastOStart );
				pointsBuffer->push_back( p1 + ostart );
				pointsBuffer->push_back( p1 );
				pointsBuffer->push_back( p1 + lastOStart );

				// Draw triangle
				m_rastPolygon.Rasterize( pointsBuffer, modifier );
				pointsBuffer->clear();
			}
			
			if( cross < 0 )
			{
				pointsBuffer->push_back( p1 - lastOStart );
				pointsBuffer->push_back( p1 - ostart );
				pointsBuffer->push_back( p1 );
				pointsBuffer->push_back( p1 - lastOStart );

				// Draw triangle
				m_rastPolygon.Rasterize( pointsBuffer, modifier );
				pointsBuffer->clear();
			}
		}

		// Store last ortho direction
		lastOStart = ostart;
		lastOEnd = oend;
		bUseLast = true;
	}
	// round ending for the line
	RasterizeCircle(*points->begin(),thicknessX,thicknessY,modifier);
	RasterizeCircle(*(points->end()-1),thicknessX,thicknessY,modifier);
}

///////////////////////////////////////////////////////////////////////////////
//! Rasterize circle
template< class tpModifier >
void CThickLine2DRasterizer::RasterizeCircle( const osg::Vec2 & center, int thickness, tpModifier & modifier )
{
	RasterizeCircle(center,thickness,thickness,modifier);
}

///////////////////////////////////////////////////////////////////////////////
//! Rasterize circle
template< class tpModifier >
void CThickLine2DRasterizer::RasterizeCircle( const osg::Vec2 & center, double thicknessX, double thicknessY, tpModifier & modifier )
{
	osg::Vec2 vec1(thicknessX/2.0,0);
	osg::Vec2 vec2(0,thicknessY/2.0);
	osg::ref_ptr< osg::Vec2Array > points = new osg::Vec2Array;
#define QUALITY	8
	for(int i = 0 ; i<(2*QUALITY+1); i++)
		points->push_back( center + vec1*sin(i*M_PI/(double)QUALITY)  + vec2*cos(i*M_PI/(double)QUALITY));
	m_rastPolygon.Rasterize< tpModifier >( points, modifier );
}

