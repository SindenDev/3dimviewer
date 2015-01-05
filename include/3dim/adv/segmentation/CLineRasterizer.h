///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CLINERASTERIZER_H_INCLUDED
#define CLINERASTERIZER_H_INCLUDED

#include <VPL/Math/Base.h>

#include <osg/Array>


namespace seg
{

/******************************************************************************
	CLASS CLine2DRasterizer

	Rasterize line on plane. Result is stored in Vec2Array.

******************************************************************************/
class CLine2DRasterizer
{

public:
	//! Constructor
	CLine2DRasterizer(){};

	//! Rasterize and call modifier
	template< class tpModifier >
	void Rasterize( const osg::Vec2 & start, const osg::Vec2 & end, tpModifier & modifier );

	//! Rasterize multiple lines 
	template< class tpModifier >
	void Rasterize( const osg::Vec2Array * points, tpModifier & modifier );

protected:


}; // class clinerasterizer



/******************************************************************************
	CLASS CLine3DRasterizer

	Rasterize line in 3D space. Result is stored in Vec3Array.

******************************************************************************/
class CLine3DRasterizer
{
public:
	//! Constructor
	CLine3DRasterizer(){};

	//! Rasterize and call modifier
	template< class tpModifier >
	void Rasterize( const osg::Vec3 & start, const osg::Vec3 & end, tpModifier & modifier );

protected:
	

}; // class clinerasterizer

#include "segmentation/CLineRasterizer.hpp"

} // namespace seg

// CLINERASTERIZER_H_INCLUDED
#endif
