///////////////////////////////////////////////////////////////////////////////
// $Id: CLineRasterizer.hpp 1293 2011-05-15 21:40:21Z spanel $
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//	Based on: ftp://ftp.uu.net/usenet/comp.sources.unix/volume26/line3d.Z
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
	MACROS
******************************************************************************/

//! Maximum of a, b
//#define MAX(a,b) (((a)>(b))?(a):(b))

//! Absolute value of x 
//#define ABS(x) (((x)<0) ? -(x) : (x))

//! Sign of x  ( Could be -1, 0, or 1 )
#define SGN(x) (((x)<0) ? -1 : (x)>0 ? 1 : 0)

/******************************************************************************
	CLASS CLine2DRasterizer
******************************************************************************/

//=============================================================================
// Generate line as array of points - Bresenhams algorithm
template< class tpModifier >
void CLine2DRasterizer::Rasterize( const osg::Vec2 & start, const osg::Vec2 & end, tpModifier & modifier )
{
	int x0( start[0] ), y0( start[1] );
	int x1( end[0] ), y1( end[1] );

	int dy( y1 - y0 );
	int dx( x1 - x0 );

	int stepx, stepy;
    
	if (dy < 0) 
	{ 
		dy = -dy;  
		stepy = -1; 
	} 
	else 
	{ 
		stepy = 1; 
	}
   
	if (dx < 0) 
	{ 
		dx = -dx;  
		stepx = -1; 
	} 
	else 
	{ 
		stepx = 1; 
	}
        
	dy <<= 1;	// dy = 2*dy
	dx <<= 1;	// dx = 2*dx

	// Set first point
    modifier.m_volume->setPosition( x0, y0 );
	modifier.set( );

	if (dx > dy) 
	{
		int p( dy - ( dx >> 1 ) );	// p = 2*dy - dx
            
		while ( x0 != x1 ) 
		{
			if ( p >= 0 ) 
			{
				y0 += stepy;
				p -= dx;	// p -= 2*dx
			}
			x0 += stepx;
			p += dy;	// p -= 2*dy

			// Modify next point
            modifier.m_volume->setPosition( x0, y0 );
        	modifier.set( );
		}
	} 
	else 
	{
		int p( dx - ( dy >> 1 ) );	// p = 2*dx - dy
            
		while ( y0 != y1 ) 
		{
			if ( p >= 0 ) 
			{
				x0 += stepx;
				p-= dy;
			}
			y0 += stepy;
			p += dx;
            
			// Modify point
			modifier.m_volume->setPosition( x0, y0 );
	        modifier.set( );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Rasterize multiple lines 
template< class tpModifier >
void CLine2DRasterizer::Rasterize( const osg::Vec2Array * points, tpModifier & modifier )
{
	if( points == 0 || points->size() == 0 )
		return;

	if( points->size() == 1 )
		Rasterize( *points->begin(), *points->begin(), modifier );

	osg::Vec2Array::const_iterator first( points->begin() ), second( points->begin() );
	
	for( ++second; second != points->end(); ++first, ++second )
		Rasterize( *first, *second, modifier );

}


/******************************************************************************
	CLASS CLine3DRasterizer
******************************************************************************/

//=============================================================================
// Generate line as array
template< class tpModifier >
void CLine3DRasterizer::Rasterize( const osg::Vec3 & start, const osg::Vec3 & end, tpModifier & modifier )
{

	// Deltas
	int xd, yd, zd;

	// Current position
	int x, y, z;
	
	// Absolute values of delta (length on the axis)
	int ax, ay, az;

	// Slopes
	int sx, sy, sz;

	int dx, dy, dz;

	int startx( start[0] ), starty( start[1] ), startz( start[2] ), 
		endx( end[0] ), endy( end[1] ), endz( end[2] );
	
	// Delta x, y, z
	dx = endx - startx;
	dy = endy - starty;
	dz = endz - startz;

	ax = vpl::math::getAbs(dx) << 1;
	ay = vpl::math::getAbs(dy) << 1;
	az = vpl::math::getAbs(dz) << 1;

	sx = SGN(dx);
	sy = SGN(dy);
	sz = SGN(dz);

	// m_starting point
	x = startx;
	y = starty;
	z = startz;

	if (ax >= vpl::math::getMax(ay, az))            /* x dominant */
	{
		yd = ay - (ax >> 1);
		zd = az - (ax >> 1);
		for (;;)
		{
			// Add m_starting point
			modifier.m_volume.setPosition( x, y, z );
	        modifier.set();

			if (x == endx)
			{
				return;	
			}
			if (yd >= 0)
			{
				y += sy;
				yd -= ax;
			}

			if (zd >= 0)
			{
				z += sz;
				zd -= ax;
			}

			x += sx;
			yd += ay;
			zd += az;
		}
	}
	else if (ay >= vpl::math::getMax(ax, az))            /* y dominant */
	{
		xd = ax - (ay >> 1);
		zd = az - (ay >> 1);
		
		for (;;)
		{
			// Add m_starting point
			modifier.m_volume.setPosition( x, y, z );
	        modifier.set();

			if (y == endy)
			{
				return;
			}

			if (xd >= 0)
			{
				x += sx;
				xd -= ay;
			}

			if (zd >= 0)
			{
				z += sz;
				zd -= ay;
			}
	
			y += sy;
			xd += ax;
			zd += az;
		}
	}
	else if (az >= vpl::math::getMax(ax, ay))            /* z dominant */
	{
		xd = ax - (az >> 1);
		yd = ay - (az >> 1);

		for (;;)
		{
			// Add m_starting point
			modifier.m_volume.setPosition( x, y, z );
	        modifier.set();

			if (z == endz)
			{
				return;
			}

			if (xd >= 0)
			{
				x += sx;
				xd -= az;
			}

			if (yd >= 0)
			{
				y += sy;
				yd -= az;
			}

			z += sz;
			xd += ax;
			yd += ay;
		}
	}
}
