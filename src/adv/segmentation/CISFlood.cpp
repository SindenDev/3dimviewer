///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#include <adv/segmentation/CISFlood.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Pushes a segment. 
//!
//!\param	x		The x coordinate. 
//!\param	y		The y coordinate. 
//!\param	z		The z coordinate. 
//!\param	length	The length. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void seg::CFloodFill3DRasterizer::pushSegment( int x, int y, int z, int length )
{
	m_queue.push_back( length );
	m_queue.push_back( z );
	m_queue.push_back( y );
	m_queue.push_back( x );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Pops a segment. 
//!
//!\param [in,out]	x		The x coordinate. 
//!\param [in,out]	y		The y coordinate. 
//!\param [in,out]	z		The z coordinate. 
//!\param [in,out]	length	The length. 
//!
//!\return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool seg::CFloodFill3DRasterizer::popSegment( int &x, int &y, int &z, int &length )
{
	if( m_queue.size() > 0)
	{
		x = m_queue.back(); m_queue.pop_back();
		y = m_queue.back(); m_queue.pop_back();
		z = m_queue.back(); m_queue.pop_back();
		length = m_queue.back(); m_queue.pop_back();

		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Find start region for actual point 

void seg::CFloodFill3DRasterizer::FindStartRegion(int x, int y, int z)
{
    s_region                        * actual_region = NULL;     // pointer to actual region of actual line

    // get pointer to first region of actual line
    unsigned long idx = m_regions_maps[z*m_maxy+y];
    if (idx!=-1)
        actual_region = &m_regions_container[idx];
    else
        actual_region = NULL;

    // cycle through actual line regions
    while (actual_region != NULL)
    {
        // test possition of actual region to input point
        if ( (actual_region->start_index <= x) && (actual_region->end_index >= x) )
        {
            // set actual region as filled
            actual_region->filled = true;
            // put aktual region into regions queue
            m_regions_queue.push_back( actual_region );
        }

        // get pointer to next region in actual line
        if (actual_region->inext != (unsigned long)-1)
            actual_region = &m_regions_container[actual_region->inext];
        else
            actual_region = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Test and fill regions of actual line by given range

void seg::CFloodFill3DRasterizer::TestLineRegions(int y, int z, int x_min, int x_max)
{
    s_region                        * actual_region = NULL;     // pointer to actual region of actual line

    // test, if actual line is inside of volume
    if ( y >= m_miny && z >= m_minz && y < m_maxy && z < m_maxz )
    {
        // get pointer to first region of actual line
        unsigned long idx = m_regions_maps[z*m_maxy+y];
        if (idx!=-1)
            actual_region = &m_regions_container[idx];
        else
            actual_region = NULL;

        // cycle through actual line regions
        while (actual_region != NULL)
        {
            // test actual line region to given range
            if (actual_region->start_index > x_max)
                break;

            // test actual line region to given range and filling state
            if ( (actual_region->filled != true) && (actual_region->end_index >= x_min) )
            {
                // set actual region as filled
                actual_region->filled = true;
                // put aktual region into regions queue
                m_regions_queue.push_back( actual_region );
            }

            // get pointer to next region in actual line
            if (actual_region->inext != (unsigned long)-1)
                actual_region = &m_regions_container[actual_region->inext];
            else
                actual_region = NULL;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
