///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CISLASO_H_INCLUDED
#define CISLASO_H_INCLUDED

#include <segmentation/CISBase.h>
//#include <segmentation/CPlaneRasterizer.h>
#include <segmentation/CPolygonRasterizer.h>

namespace seg
{

/******************************************************************************
	CLASS CISLaso

	Creates segment from the traced region
******************************************************************************/
class CISLasoOrtho : public CISBase2D
{
public:
	//! Constructor
	CISLasoOrtho( );

	// SEGMENATATION
	//! Do segmentation
	template < class tpModifier >
	void DoSegmentation( const osg::Vec2Array & points, const tDataPlane * splane, tRegionPlane * rplane, tpModifier & modifier );

protected:
	//! Polygon rasterizer
	CPolygonRasterizer m_polygonRasterizer;

}; // class CISLaso

} // namespace seg

// CISLASO_H_INCLUDED
#endif

