///////////////////////////////////////////////////////////////////////////////
// $Id: CISLine.hpp 1293 2011-05-15 21:40:21Z spanel $
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#define EPS 0.00001

/******************************************************************************
	CLASS CISLine2D
******************************************************************************/

//=============================================================================
//  Draw one line
template < class tpModifier >
void CISLine2D::DrawLine( const osg::Vec2 & start, const osg::Vec2 & end, tRegionPlane * rplane, tpModifier & modifier )
{
	// Draw starting point
	// DrawCircle( start );

	if( m_width <= 1.0 )
	{
		// TODO: The following member variable doesn't exist!
//		m_lineRasterizer.Rasterize< tpModifier >( start, end, modifier );
		return;
	}

	// Compute ortho space - one vector is direction, one vector is normal. Orthogonal vector can be computed
	osg::Vec2 direction( end - start );
	if( direction.length2() < EPS )
		return;		

	direction.normalize();
	osg::Vec2 ortho( end - start );
	// Switch coordinates to get normal vector
	float buf(ortho[0]); ortho[0] = ortho[1]; ortho[1] = buf;

	osg::Vec2 strokeStart( ortho[ 0 ] * m_width / 2, ortho[ 1 ] * m_width / 2 );
	osg::Vec2 strokeEnd( - ortho[ 0 ] * m_width / 2,  - ortho[ 1 ] * m_width / 2 );

	osg::ref_ptr< osg::Vec2Array > points = new osg::Vec2Array;
	points->push_back( start + strokeStart );

	m_polygonRasterizer.Rasterize< tpModifier >( *points, modifier );
}

//=============================================================================
// Do segmentation 
template < class tpModifier >
void CISLine2D::DoSegmentation( const osg::Vec2Array & points, const tDataPlane * splane, tRegionPlane * rplane, tpModifier & modifier )
{

	if( HandleSimple< tpModifier >( points, splane, rplane, modifier ) )
		return;

	osg::Vec2Array::const_iterator s, e;

	// Draw all lines
	for( s = points.begin(), e = s, ++e; e != points.end(); ++e, ++s )
		DrawLine< tpModifier >( *s, *e, rplane, modifier );
}

/******************************************************************************
	CLASS CISLine3D
******************************************************************************/

//=============================================================================
// Do segmentation
template < class tpModifier > 
void CISLine3D::DoSegmentation( const osg::Vec3Array & points, const tDataVolume * svolume, tRegionVolume * rvolume, tpModifier & modifier )
{

	// Handle simple cases
	if( HandleSimple< tpModifier >( points, svolume, rvolume, modifier ) )
		return;

	osg::Vec3Array::const_iterator s, e;

	// Draw all lines
	for( s = points.begin(), e = s, ++e; e != points.end(); ++e, ++s )
		DrawLine< tpModifier >( *s, *e, rvolume, modifier ); 
}

//=============================================================================
//  Draw one line
template < class tpModifier > 
void CISLine3D::DrawLine( const osg::Vec3 & start, const osg::Vec3 & end, tRegionVolume * rvolume, tpModifier & modifier )
{
	// Draw starting point
	// DrawCircle( start );

	// Compute ortho space - one vector is direction, one vector is normal. Orthogonal vector can be computed
	osg::Vec3 direction( end - start );
	if( direction.length2() < EPS )
		return;		

	direction.normalize();
	osg::Vec3 ortho = m_normal ^ ( end - start );
	ortho.normalize();

	osg::Vec3 strokeStart( ortho[ 0 ] * m_width / 2.0, ortho[ 1 ] * m_width / 2.0, ortho[ 2 ] * m_width / 2.0 );
	osg::Vec3 strokeEnd( - ortho[ 0 ] * m_width / 2.0,  - ortho[ 1 ] * m_width / 2.0, - ortho[ 2 ] * m_width / 2.0 );

	if( m_width > 1.0 )
	{

	}
/*
	CSet3DModifier< tRegionVolume > modifier;
	modifier.m_volumePtr = &rvolume;
	modifier.m_value = m_regionId;
*/
	// Draw stroke
	m_lineRasterizer.Rasterize< tpModifier >( start, end, modifier );

	// Draw starting point
	// DrawCircle( end );
}

