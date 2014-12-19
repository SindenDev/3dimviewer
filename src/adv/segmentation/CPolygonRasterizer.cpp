///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#include <segmentation/CPolygonRasterizer.h>

using namespace seg;

///////////////////////////////////////////////////////////////////////////////
//  Remove horizontal lines, modify orientation from top to the bottom.
void CPolygonRasterizer::ComputeLines( const osg::Vec2Array * points, tLinesVector & lines, int & min, int & max )
{
	osg::Vec2Array::const_iterator i;

	osg::Vec2 s, e;

	// Clear array
	lines.clear();

	i = points->begin();
	s = *i;
	++i;

	min = max = s.y();

	// All lines
	for( ; i != points->end(); ++i )
	{
		e = *i;

		// min max computing
		if( min > s.y() )
			min = s.y();

		if( max < s.y() )
			max = s.y();

		// Remove horizontal lines
		if( s.y() == e.y() )
		{
			s = e;
			continue;
		}

		// Orientation
		if( s.y() < e.y() )
			lines.push_back( tLinePoints( s, e ) );
		else
			lines.push_back( tLinePoints( e, s ) );


		s = e;
	}

	// min max for the last point
	if( min > s.y() )
		min = s.y();

	if( max < s.y() )
		max = s.y();
}

///////////////////////////////////////////////////////////////////////////////
// Get row intersection
bool CPolygonRasterizer::GetIntersection( const osg::Vec2 & s, const osg::Vec2 & e, int row, int & intersectionX )
{
	if( (int)s.y() > row )
		return false;

	if( (int)e.y() <= row )
		return false;

	float ix, iy( row - (int)s.y() ), ex( (int)e.x() - (int)s.x() ), ey( (int)e.y() - (int)s.y() );

	ix = iy * ex / ey;

	intersectionX = (int)s.x() + ix;

	return true;
}