///////////////////////////////////////////////////////////////////////////////
// $Id: CPolygonRasterizer.hpp 1266 2011-04-17 23:00:36Z spanel $
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
//////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Rasterize polygon
template< class tpModifier >
void CPolygonRasterizer::Rasterize( const osg::Vec2Array * points, tpModifier & modifier )
{
	int min, max;

	// Is it polygon?
	if( points->size() < 3 )
		return;

	//! Lines - arrays of points
	tLinesVector lines;

	// Compute lines
	ComputeLines( points, lines, min, max );

	tLinesVector::iterator line;

	tIntersectionsVector intersections;
	tIntersectionsVector::iterator xs, xe;
	int intersection;

	// Draw lines
	for( int y = min; y < max; ++y )
	{
		intersections.clear();

		// Compute intersections
		for( line = lines.begin(); line != lines.end(); ++line )
		{
			if( GetIntersection( line->first, line->second, y, intersection ) )
				intersections.push_back( intersection );
		}

		// If are intersections
		if( intersections.size() > 1 )
		{
			std::sort( intersections.begin(), intersections.end() );

			// Draw next segment?
			bool bDraw( true );	

			// All segments...
			for( xs = intersections.begin(), xe = xs, ++xe; xe != intersections.end(); ++xs, ++xe )
			{
				if( bDraw )
				{
					// Draw
                    modifier.m_volume->setPosition( *xs, y );
					for( int x = *xs; x <= *xe; ++x )
                    {
                        modifier.set();
						modifier.m_volume->incX();
                    }

					// Next - not draw
					bDraw = false;

				}else{
					// Next - draw
					bDraw = true;
				}
			} // for all segments
		} // if are intersections
	}

	// Draw boundaries
	m_rastLine.Rasterize( points, modifier );

}



