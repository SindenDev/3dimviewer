///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////


#ifndef CISFlood_H_included
#define CISFlood_H_included

#include <segmentation/CISBase.h>
#include <segmentation/CISVolumeStepper.h>
//#include <stack>
#include <deque>

#include <VPL/Base/Warning.h>
#include <VPL/Base/Logging.h>
#include <VPL/Module/Progress.h>


namespace seg
{
/******************************************************************************
	2D flood fill rasterizer
******************************************************************************/
class CFloodFill2DRasterizer
{
public:
	//! Constructor
	CFloodFill2DRasterizer() 
	{}

	//! Rasterizer
	template< class tpModifier >
	void Rasterize( const osg::Vec2Array & points, tpModifier & modifier );
};

/******************************************************************************
	3D flood fill rasterizer
******************************************************************************/
class CFloodFill3DRasterizer : public vpl::mod::CProgress
{
protected:
	//! Point storing queue
	typedef std::deque< int > tPointQueue;

    //! volume region structure
    struct s_region {
                        int         rz;
                        int         ry; 
                        int         start_index;
                        int         end_index;
                        bool        filled;
                        unsigned long inext;         // not a pointer but m_regions_container index
    };
                    
    //! typedef of volume regions vector
    typedef std::vector<s_region>           t_regions_vector;
    //! typedef of volume regions vector iterator
    typedef std::vector<s_region>::iterator t_regions_iter;

    //! typedef of region seed storing queue
    typedef std::deque< s_region * >        t_region_queue;

public:
	//! Constructor
	CFloodFill3DRasterizer( int maxx = -1, int maxy = -1, int maxz = -1, int minx = 0, int miny = 0, int minz = 0 ) 
		: m_minx( minx )
		, m_miny( miny )
		, m_minz( minz )
		, m_maxx( maxx )
		, m_maxy( maxy )
		, m_maxz( maxz )
	{}

	//! Set sizes of the volume
	void SetSize( int maxx = -1, int maxy = -1, int maxz = -1, int minx = 0, int miny = 0, int minz = 0 ) 
	{
		m_minx = minx;
		m_miny = miny;
		m_minz = minz;
		m_maxx = maxx;
		m_maxy = maxy;
		m_maxz = maxz;
	}

	//! Line volume rasterizer
	template< class tpModifier >
	void Rasterize( const osg::Vec3Array & points, tpModifier & modifier );

	//! Paralel volume rasterizer
	template< class tpModifier >
	void RasterizeParalel( const osg::Vec3Array & points, tpModifier & modifier );

	//! Volume sizes
	int m_minx, m_miny, m_minz, m_maxx, m_maxy, m_maxz;

protected:

	//! Test if point is NOT in volume.
	bool IsNotIn( int x, int y, int z )
	{
		return x < m_minx || y < m_miny || z < m_minz || x >= m_maxx || y >= m_maxy || z >= m_maxz;
	}

	//! Fill single line
	template< class tpModifier >
	void fillLine( int point_x, int point_y, int point_z, int end_x, int s, tpModifier &modifier );

	//! Store segment
	void pushSegment( int x, int y, int z, int length );

	//! Get segment from the queue
	bool popSegment(  int &x, int &y, int &z, int &length  );

    // Make volume line regions maps 
    template< class tpModifier >
    void MakeLineRegions(tpModifier & modifier);

    //! Fill volume line regions maps 
    template< class tpModifier >
    void FillLineRegions(tpModifier & modifier);

    //! Find start region for actual point 
    void FindStartRegion(int x, int y, int z);

    //! Test and fill regions of actual line by given range
    void TestLineRegions(int y, int z, int x_min, int x_max);

protected:

	//! Points queue
	tPointQueue         m_queue;

    //! Region seed storing queue
    t_region_queue      m_regions_queue;

    //! Volume regions map
    std::vector<unsigned long>      m_regions_maps;

    //! Vector of regions as a memory alocation container
    std::vector<s_region>           m_regions_container;

};




#include "CISFlood.hpp"
} // namespace seg

// CISFlood_H_included
#endif

