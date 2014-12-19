///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2008-2014 3Dim Laboratory s.r.o.
// All rights reserved
//
///////////////////////////////////////////////////////////////////////////////

#include <segmentation/CPolygonRasterizer.h>

namespace seg
{

/******************************************************************************
	CLASS CThickLineRasterizer2D - rasterize line with thickness on plane
******************************************************************************/
class CThickLine2DRasterizer
{
public:
	//! Constructor
	CThickLine2DRasterizer(){};

	//! Rasterize and call modifier
	template< class tpModifier >
	void Rasterize( const osg::Vec2 & start, const osg::Vec2 & end, int thickness, tpModifier & modifier );

	//! Rasterize and call modifier
	template< class tpModifier >
	void Rasterize( const osg::Vec2 & start, const osg::Vec2 & end, double thicknessX, double thicknessY, tpModifier & modifier );

	//! Rasterize multiple lines 
	template< class tpModifier >
	void Rasterize( const osg::Vec2Array * points, int thickness, tpModifier & modifier );

	//! Rasterize multiple lines 
	template< class tpModifier >
	void Rasterize( const osg::Vec2Array * points, double thicknessX, double thicknessY, tpModifier & modifier );

	//! Rasterize circle
	template< class tpModifier >
	void RasterizeCircle( const osg::Vec2 & center, int thickness, tpModifier & modifier );

	//! Rasterize circle
	template< class tpModifier >
	void RasterizeCircle( const osg::Vec2 & center, double thicknessX, double thicknessY, tpModifier & modifier );

protected:
	//! Polygon rasterizer
	CPolygonRasterizer m_rastPolygon;

	//! Simple line rasterizer
	CLine2DRasterizer m_rastLine;

}; // class CThickLineRasterizer

#include "CThickLineRasterizer.hpp"

} // namespace seg

