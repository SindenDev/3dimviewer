///////////////////////////////////////////////////////////////////////////////
// $Id: CISLaso.hpp 1266 2011-04-17 23:00:36Z spanel $
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////


//=============================================================================
// Do segmentation
template < class tpModifier >
void CISLasoOrtho::DoSegmentation( const osg::Vec2Array & points, const tDataPlane * splane, tRegionPlane * rplane, tpModifier & modifier )
{
	
	// Do tests
	if( CISBase2D::HandleSimple< tpModifier >( points, splane, rplane, modifier ) )
		return;

	// render polygon
	m_polygonRasterizer.Rasterize< tpModifier >( points, modifier );


}