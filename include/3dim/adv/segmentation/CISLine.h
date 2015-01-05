///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CISLINE_H_INCLUDED
#define CISLINE_H_INCLUDED

#include <segmentation/CISBase.h>
#include <segmentation/CPolygonRasterizer.h>

namespace seg
{

/******************************************************************************
	CLASS CISLine2D

	Draws segmentation line of the given width
******************************************************************************/
class CISLine2D : public CISBase2D
{
public:
	//! Constructor
	CISLine2D();

	//! Set line width. Set width =< 0 for 1 pixel width.
	void SetWidth( float width ){ m_width = width; }

	//! Do segmentation
	template< class tpModifier >
	void DoSegmentation( const osg::Vec2Array & points, const tDataPlane * splane, tRegionPlane * rplane, tpModifier & modifier );

protected:
	//! Draw one line
	template< class tpModifier >
	void DrawLine( const osg::Vec2 & start, const osg::Vec2 & end, tRegionPlane * rplane, tpModifier & modifier );

protected:

	//! Line rasterizer
	// CLine2DRasterizer m_lineRasterizer;

	//! Polygon rasterizer
	CPolygonRasterizer m_polygonRasterizer;

	//! Line width
	float m_width;
};


/******************************************************************************
	CLASS CISLine3D

	Draws segmentation line of the given width
******************************************************************************/
class CISLine3D : public CISBase3D
{

public:
	//! Constructor
	CISLine3D();

	//! Set plane normal (must be called before first use!)
	void SetNormal( const osg::Vec3 & normal) { m_normal = normal; }

	//! Set line width. Set width =< 0 for 1 pixel width.
	void SetWidth( float width ){ m_width = width; }

	//! Do segmentation
	template< class tpModifier >
	void DoSegmentation( const osg::Vec3Array & points, const tDataVolume * svolume, tRegionVolume * rvolume, tpModifier & modifier );

protected:
	//! Draw one line
	template< class tpModifier >
	void DrawLine( const osg::Vec3 & start, const osg::Vec3 & end, tRegionVolume * rvolume, tpModifier & modifier );

protected:
	//! Plane normal
	osg::Vec3 m_normal;

	//! Line rasterizer
	CLine3DRasterizer m_lineRasterizer;

	//! Line width
	float m_width;
};

#include "CISLine.hpp"

} // namespace seg

// CISLINE_H_INCLUDED
#endif

