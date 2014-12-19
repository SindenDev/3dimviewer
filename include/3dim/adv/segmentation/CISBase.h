///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CISBASE_H_INCLUDED
#define CISBASE_H_INCLUDED

#include <osg/Array>
#include <osg/BoundingBox>
#include <data/CRegionData.h>
#include <data/CDensityData.h>

namespace seg
{

//! Region volume type
typedef data::CRegionData tRegionVolume;

//! Region volume voxel type
typedef data::CRegionData::tVoxel tRegionVoxel;

//! Segmentation volume type
typedef data::CDensityData tDataVolume;

//! Segmentation volume voxel type
typedef data::CDensityData::tVoxel tDataVoxel;

//! Region plane type
typedef vpl::img::CImage16 tRegionPlane;

//! Region plane pixel type
typedef vpl::img::CImage16::tPixel tRegionPixel;

//! Segmentation plane
typedef vpl::img::CDImage tDataPlane;

//! Segmentation plane pixel type
typedef vpl::img::CDImage::tPixel tDataPixel;

/*****************************************************************************
	CLASS CISBase3D

	Defines basic interface for interactive segmentation objects in 3D
*****************************************************************************/
class CISBase3D
{
public:

	// REGION ADMINISTRATION
	//! Set current region. Must be set before first use!
	void SetRegion( int region ){ m_regionId = region; }

	// SEGMENATATION
	//! Do segmentation
	// void DoSegmentation( const osg::Vec3Array & points, const tDataVolume & svolume, tRegionVolume & rvolume ) = 0;

protected:

	//! Handle three cases: zero point array, one point in array and no segmentation volume. Returns true, if drawing is done.
	template< class tpModifier >
	bool HandleSimple( const osg::Vec3Array & points, const tDataVolume * svolume, tRegionVolume * rvolume, tpModifier & modifier )
	{
		// Do tests
		if( points.size() == 0 )
			return true;

		if( points.size() == 1 )
		{
			// Just draw point
			modifier( points[0] );
			return true;
		}

		return false;
	}

	//! Compute points cloud bounding box.
	osg::BoundingBox ComputeBoundingBox( const osg::Vec3Array & points )
	{
		osg::BoundingBox box;
		
		osg::Vec3Array::const_iterator i;

		for( i = points.begin(); i < points.end(); ++i )
			box.expandBy( (*i)[0], (*i)[1], (*i)[2] );

		return box;
	}

protected:
	//! Region number
	tRegionVoxel m_regionId;

}; // class CISBase

/*****************************************************************************
	CLASS CISBase2D

	Defines basic interface for interactive segmentation objects
*****************************************************************************/
class CISBase2D
{
public:

	// REGION ADMINISTRATION
	//! Set current region. Must be set before first use!
	void SetRegion( int region ){ m_regionId = region; }

	// SEGMENATATION
	//! Do segmentation
	// void DoSegmentation( const osg::Vec2Array & points, const tDataPlane & splane, tRegionPlane & rplane ) = 0;

protected:

	//! Handle two cases: zero point array and one point in array. Returns true, if drawing is done.
	template< class tpModifier >
	bool HandleSimple( const osg::Vec2Array & points, const tDataPlane * splane, tRegionPlane * rplane, tpModifier & modifier )
	{
		// Do tests
		if( points.size() == 0 )
			return true;

		if( points.size() == 1 )
		{
			// Just draw point
			modifier( points[0] );
			return true;
		}

		return false;
	}

	//! Compute points cloud bounding box. The z coordinate is set to 0.0
	osg::BoundingBox ComputeBoundingBox( const osg::Vec2Array & points )
	{
		osg::BoundingBox box;
		
		osg::Vec2Array::const_iterator i;

		for( i = points.begin(); i < points.end(); ++i )
			box.expandBy( (*i)[0], (*i)[1], 0.0 );

		return box;
	}

protected:
	//! Region number
	tRegionPixel m_regionId;

}; // class CISBase2D




} // namespace seg

// CISBASE_H_INCLUDED
#endif


