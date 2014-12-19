///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CPolygonrasterizer_H_included
#define CPolygonrasterizer_H_included

#include <segmentation/CLineRasterizer.h>
#include <vector>
#include <algorithm>

namespace seg
{

/******************************************************************************
	CLASS CPolygonRasterizer

	Rasterize Polygon.

******************************************************************************/
class CPolygonRasterizer
{
protected:
	typedef std::pair< osg::Vec2, osg::Vec2 > tLinePoints;
	typedef std::vector< tLinePoints > tLinesVector;

	typedef std::vector< int > tIntersectionsVector;

public:
	//! Constructor
	CPolygonRasterizer(){};

	//! Rasterize and call modifier
	template< class tpModifier >
	void Rasterize( const osg::Vec2Array * points, tpModifier & modifier );

protected:
	//! Remove horizontal lines, modify orientation from top to the bottom.
	void ComputeLines( const osg::Vec2Array * points, tLinesVector & lines, int & min, int & max );

	//! Get row intersection
	bool GetIntersection( const osg::Vec2 & s, const osg::Vec2 & e, int row, int & intersectionX );

protected:
	//! Line rasterizer
	seg::CLine2DRasterizer m_rastLine;

}; // class CPolygonRasterizer


#include "segmentation/CPolygonRasterizer.hpp"

} // namespace seg


// CPolygonrasterizer_H_included
#endif

