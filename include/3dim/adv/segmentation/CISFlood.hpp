///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009-2014 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
	2D flood fill rasterizer
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Rasterizer
template< class tpModifier >
void CFloodFill2DRasterizer::Rasterize( const osg::Vec2Array & points, tpModifier & modifier )
{
	std::deque< int > seed_stack;
	int x, y;

	// Add all seeds 
	osg::Vec2Array::const_iterator i;
	for( i = points.begin(); i != points.end(); ++i )
    {
		seed_stack.push_back( i->x() );
        seed_stack.push_back( i->y() );
    }

	// Filling algorithm
	while( !seed_stack.empty() )
	{
        x = seed_stack.front();
		seed_stack.pop_front();
   	    y = seed_stack.front();
        seed_stack.pop_front();

        modifier.m_volume->setPosition( x, y );
        // test position
        if (!modifier.m_volume->isIn())
            continue;
		if( modifier.set( ) )
		{
            if (seg::PLANE_FREE==modifier.m_volume->getMode())
            {
                int xt, yt;
                xt = 1; yt = 0; modifier.m_volume->getNextVoxelInDirection(xt,yt); seed_stack.push_back( x + xt ); seed_stack.push_back( y + yt );
                xt = 0; yt = 1; modifier.m_volume->getNextVoxelInDirection(xt,yt); seed_stack.push_back( x + xt ); seed_stack.push_back( y + yt );
                xt =-1; yt = 0; modifier.m_volume->getNextVoxelInDirection(xt,yt); seed_stack.push_back( x + xt ); seed_stack.push_back( y + yt );
                xt = 0; yt =-1; modifier.m_volume->getNextVoxelInDirection(xt,yt); seed_stack.push_back( x + xt ); seed_stack.push_back( y + yt );
            }
            else
            {
			    seed_stack.push_back( x + 1 ); seed_stack.push_back( y );
			    seed_stack.push_back( x ); seed_stack.push_back( y + 1 );
			    seed_stack.push_back( x - 1 ); seed_stack.push_back( y );
			    seed_stack.push_back( x ); seed_stack.push_back( y - 1 );
            }
		}
	}
}



/******************************************************************************
	3D flood fill rasterizer
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Rasterizer
template< class tpModifier >
void CFloodFill3DRasterizer::Rasterize( const osg::Vec3Array & points, tpModifier & modifier )
{

    RasterizeParalel( points, modifier );

    return;



	vpl::mod::CProgress::setProgressMax( 100 );
	vpl::mod::CProgress::beginProgress();
//	int start_x, start_y, start_z;

	m_queue.clear();
	// Shift
//	int shift[4][3] = {{0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};

	VPL_LOG_INFO( "Flood fill init:" << m_maxx << ", " << m_maxy << ", " << m_maxz << std::endl );

	int point_x, point_y, point_z;


	// Tested line start/end
	int begin_x(0), end_x(0);


	osg::Vec3Array::const_iterator i;
	bool fill_segment( true );

	// for all point
	for( i = points.begin(); i != points.end(); ++i )
	{
		point_x = i->x();
		point_y = i->y();
		point_z = i->z();

		// Do not fill points out of the volume
		if( IsNotIn( point_x, point_y, point_z ) )
			continue;

		// Do not fill already filled points...
        modifier.m_volume->setPosition( point_x, point_y, point_z );
		if( ! modifier.set() )
			continue;

		begin_x = 0;

		// Should segment be filled
		fill_segment = true;

		//---------------------------------------------------------------------
		// INIT PHASE

        modifier.m_volume->setPosition( point_x, point_y, point_z );
		// Find line start
		for( int x = point_x-1; x >= 0; --x )
		{

			// Try to fill this pixel
			if( ! modifier.set() )
			{
				// Point is last point BEFORE line start
				begin_x = x + 1;

				break;

			}

            if( x >= 0 )
                modifier.m_volume->decX();   

		} // Find line start

        modifier.m_volume->setPosition( point_x, point_y, point_z );

		// Find line end
		for( int x = point_x + 1 ; x < m_maxx; ++x )
		{
		
			// Fill this pixel
			if( ! modifier.set() )
			{
				pushSegment( begin_x, point_y, point_z, x - begin_x );

				fill_segment = false;
				// break the loop - line is stored
				break;
			}

            if( x < m_maxx - 1 )
                modifier.m_volume->incX();

		} // Find line end

		// Line till the maxx was filled, we must add it...
		if( fill_segment )
		{
			pushSegment( begin_x, point_y, point_z, m_maxx - begin_x + 1 );
		}
			

		//---------------------------------------------------------------------
		// SCANLINE PHASE

		VPL_LOG_INFO( "SCANLINE START" << std::endl );

		int progress_counter(0), progress_limit( 1000 );

		int length;

		// while lines in the queue
		while( m_queue.size() != 0 )
		{
			
			{
				++progress_counter;

				if( progress_counter >= progress_limit )
				{
					vpl::mod::CProgress::progress();
					progress_counter = 0;
				}

			}
			
			popSegment( point_x, point_y, point_z, length );

			// get line length
			end_x = point_x + length - 1;

			// four near lines
			for( int s = 0; s < 4; ++s )
			{
				fillLine( point_x, point_y, point_z, end_x,  s, modifier );
				
			} // four near lines

		} // while lines in the queue

	} // for all points

	vpl::mod::CProgress::endProgress();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Fill line. 
//!
//!\param	point_x					The point x coordinate. 
//!\param	point_y					The point y coordinate. 
//!\param	point_z					The point z coordinate. 
//!\param	end_x					The end x coordinate. 
//!\param	s						The. 
//!\param [in,out]	modifier		The modifier. 
//!\param [in,out]	queue			The queue. 
//!
//!\return	. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template< class tpModifier >
void CFloodFill3DRasterizer::fillLine( int point_x, int point_y, int point_z, int end_x, int s, tpModifier &modifier ) 
{
	// Shift
	int shift[4][3] = {{0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};

	int start_x = point_x + shift[s][0];
	int start_y = point_y + shift[s][1];
	int start_z = point_z + shift[s][2];

	//VPL_LOG( "Next line. s:" << s << std::endl );

	if( IsNotIn( start_x, start_y, start_z ) )
		return;

	int begin_x = 0;

	// Fill current segment?
	bool fill_segment( true );

	modifier.m_volume->setPosition( start_x, start_y, start_z );
	if( modifier.set() )
	{

		// Just a point on the beginning
		if( IsNotIn( start_x-1, start_y, start_z ) )
		{
			pushSegment( start_x, start_y, start_z, 1 );

			return;
		}

		// Find segment start
		modifier.m_volume->decX();

		for( int x = start_x - 1; x > 0; --x )
		{
			if( ! modifier.set() )
			{
				begin_x = x + 1;

				break;
			}

			modifier.m_volume->decX();
		} // Find segment start

		//VPL_LOG( "Start found: " << begin_x << std::endl );

	}else 
		// segment should not be filled
		fill_segment = false;

	// Find right boundary of the segment
	// No steps to the right
	if( IsNotIn( start_x+1, start_y, start_z ) )
	{
		if( begin_x < start_x )
		{
			pushSegment( begin_x, start_y, start_z, start_x - begin_x );
		}

		return;
	}

	// We can go to the right
	modifier.m_volume->setPosition( start_x + 1, start_y, start_z );

	for( int x = start_x + 1; x < m_maxx; ++x )
	{
		if( fill_segment )
		{
			// test point
			if( ! modifier.set() )
			{
				fill_segment = false;

				pushSegment( begin_x, start_y, start_z, x - begin_x );

				// If x is behind segment, stop testing...
				if( x > end_x )
					break;

			} // else test point

		}else{ // if fill_segment

			// If x is behind segment, stop testing...
			if( x > end_x )
				break;

			if( modifier.set() )
			{
				begin_x = x;

				fill_segment = true;
			} // if test point

		} // else fill_segment

		if( x < m_maxx - 1 )
			modifier.m_volume->incX();

	} //  Find right boundary of the segment

	// Last segment <start_x, m_maxx) is not stored
	if( fill_segment )
	{
		pushSegment( begin_x, start_y, start_z, m_maxx - begin_x + 1);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Make volume line regions maps 

template< class tpModifier >
void CFloodFill3DRasterizer::MakeLineRegions(tpModifier & modifier)
{
    s_region                        work_region;                                    // working instance of region
    unsigned long                   previous_region = -1;                       // pointer to preview region in actual line
    bool                            open_region = false;                            // flag of open region
    int                             vx, vy, vz;                                     // volume position indexes

    // resize of regions map for particular volume
    m_regions_maps.clear();
    m_regions_maps.resize(m_maxy*m_maxz);
    for(int k = 0; k < m_maxy*m_maxz; k++)
        m_regions_maps[k]=-1;

    // reserve regions container
    m_regions_container.clear();
    m_regions_container.reserve(m_maxy*m_maxz*5);

    // walk through volume 
    for (vz = 0; vz < m_maxz; ++vz)
    {
        for (vy = 0; vy < m_maxy; ++vy)
        {
            // set position of actual line start in volume
            modifier.m_volume->setPosition( 0, vy, vz);
            // get reference on actual volume line regions vector 
            //actual_line_regions = m_regions_maps[vz*m_maxy+vy];
            //actual_line_regions.clear();
            // inicialisation of working region
            work_region.rz = vz;
            work_region.ry = vy;
            work_region.start_index = m_maxx;
            work_region.end_index = -1;
            work_region.filled = false;
            work_region.inext = -1;
            // initialise open region flag
            open_region = false;
            // initialise previous of actual line
            previous_region = -1;

            // walk through actual volume line
            for (vx = m_minx; vx < m_maxx; ++vx)
            {
                // test actual voxel on region
                if (modifier.doTests())
                { // inside of region to be fill
                    // test open region flag
                    if ( ! open_region)
                    {
                        // set open region flag, start region
                        open_region = true;
                        // set start region index of working region
                        work_region.start_index = vx;
                    }
                }
                else
                { // outside of region to be fill
                    // test open region flag
                    if (open_region)
                    {
                        // reset open region flag, end region
                        open_region = false;
                        // set end region index of working region
                        work_region.end_index = vx - 1;
                        // save working region into regions container
                        m_regions_container.push_back(work_region);
                        // get pointer to last/new region
                        unsigned long new_region = m_regions_container.size()-1;
                        // save new region into regions map
                        if (previous_region == (unsigned long)-1)
                            m_regions_maps[vz*m_maxy+vy] = new_region;
                        else
                            m_regions_container[previous_region].inext = new_region;
                        // save previous region
                        previous_region = new_region;
                    }
                }

                // change position in axis X
                modifier.m_volume->incX();
            }

            // close eventualy open region
            if (open_region)
            {
                // reset open region flag, end region
                open_region = false;
                // set end region index of working region
                work_region.end_index = m_maxx - 1;
                // save working region into regions container
                m_regions_container.push_back(work_region);
                // get pointer to last/new region
                unsigned long new_region = m_regions_container.size()-1;
                // save new region into regions map
                if (previous_region == (unsigned long)-1)
                    m_regions_maps[vz*m_maxy+vy] = new_region;
                else
                    m_regions_container[previous_region].inext = new_region;
                // save previous region
                previous_region = new_region;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Fill volume line regions maps 

template< class tpModifier >
void CFloodFill3DRasterizer::FillLineRegions(tpModifier & modifier)
{
    s_region                        * actual_region = NULL;                         // pointer to new region in actual line
    int                             vx, vy, vz;                                     // volume position indexes

    // walk through volume regions
    for (vz = 0; vz < m_maxz; ++vz)
    {
        for (vy = 0; vy < m_maxy; ++vy)
        {
            // get pointer to first region of actual line
            unsigned long idx = m_regions_maps[vz*m_maxy+vy];
            if (idx!=-1)
                actual_region = &m_regions_container[idx];
            else
                actual_region = NULL;

            // cycle through actual line regions
            while (actual_region != NULL)
            {
                // test, if actual region is filled
                if (actual_region->filled)
                {
                    // set position of actual region start in volume
                    modifier.m_volume->setPosition(actual_region->start_index, vy, vz);

                    // walk through actual line actual region
                    for (vx = actual_region->start_index; vx <= actual_region->end_index; ++vx)
                    {
                        // set written volume value
                        modifier.m_volume->set( modifier.m_value );
                        // change position in axis X
                        modifier.m_volume->incX();
                    }
                }

                // get pointer to next region in actual line
                if (actual_region->inext != (unsigned long)-1)
                    actual_region = &m_regions_container[actual_region->inext];
                else
                    actual_region = NULL;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Paralel volume rasterizer

template< class tpModifier >
void CFloodFill3DRasterizer::RasterizeParalel( const osg::Vec3Array & points, tpModifier & modifier )
{
    int                             point_x, point_y, point_z;          // actual point coordinates
    s_region                        * seed_region;                      // pointer to actual seed region
	osg::Vec3Array::const_iterator  input_point;                        // iterator for input points vector

    // set progress parameters
	vpl::mod::CProgress::setProgressMax( 100 );
	vpl::mod::CProgress::beginProgress();

    VPL_LOG_INFO( "Flood fill 3D init:" << m_maxx << ", " << m_maxy << ", " << m_maxz << std::endl );

    // making volume lines regions 
    MakeLineRegions(modifier);

    // walk through all input point
    for( input_point = points.begin(); input_point != points.end(); ++input_point )
    {
        // save coordinates of actual point
        point_x = input_point->x();
        point_y = input_point->y();
        point_z = input_point->z();

        // Do not fill points out of the volume
        if( IsNotIn( point_x, point_y, point_z ) )
            continue;

        // Do not fill already filled points...
        modifier.m_volume->setPosition( point_x, point_y, point_z );
        if( ! modifier.doTests() )
            continue;

        // initialize regions queue
        m_regions_queue.clear();
        // find start region for actual point and put it to regions queue
        FindStartRegion( point_x, point_y, point_z );

        // cycle until regions queue is empty
        while ( !m_regions_queue.empty() )
        {
            // get back seed region from regions queue
            seed_region = m_regions_queue.back();
            // remove actual seed region from regions queue
            m_regions_queue.pop_back();

            // test adjacent lines to seed regionn
            TestLineRegions(seed_region->ry+1, seed_region->rz, seed_region->start_index, seed_region->end_index);
            TestLineRegions(seed_region->ry-1, seed_region->rz, seed_region->start_index, seed_region->end_index);
            TestLineRegions(seed_region->ry, seed_region->rz+1, seed_region->start_index, seed_region->end_index);
            TestLineRegions(seed_region->ry, seed_region->rz-1, seed_region->start_index, seed_region->end_index);
        }
    }

    // fill volume lines regions
    FillLineRegions(modifier);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
