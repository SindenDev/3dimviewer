///////////////////////////////////////////////////////////////////////////////
// $Id: CPlaneRasterizer.h
//
// Copyright (c) 2009 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CPlaneRasterizer_H_included
#define CPlaneRasterizer_H_included

namespace seg
{


/******************************************************************************
	Class CPlaneRasterizerBase
******************************************************************************/
class CPlaneRasterizerBase
{
public:
	//! Set simple - no testing
	template< class tpModifier >
	void SetSimple( const osg::Vec2 & point, tpModifier & modifier ) { modifier( point ); }

	//! Set simple from projected 3D point - no testing
	template< class tpModifier >
	void SetSimple( const osg::Vec3 & point, tpModifier & modifier )
	{
		SetSimple< tpModifier >( Project( point ), modifier );
	}

	//! Test and set
	template< class tpModifier >
	bool Set( const osg::Vec2 & point, tpModifier & modifier )
	{
		if( TestPoint( point ) )
		{
			SetSimple< tpModifier >( point, modifier );
			return true;
		}

		return false;
	}

	//! Test and set - 3D version
	template< class tpModifier >
	bool Set( const osg::Vec3 & point, tpModifier & modifier )
	{
		return Set< tpModifier >( Project( point ), modifier );
	}

protected:
	//! Test if coordinates are on plane
	virtual bool TestPoint( const osg::Vec2 & point ) { return false; };

	//! Project point from 3D space on plane.
	virtual osg::Vec2 Project( const osg::Vec3 & point ) = 0;

}; // class CPlaneRasterizerBase

/******************************************************************************
	Class CPlaneXYRasterizer
******************************************************************************/
class CPlaneXYRasterizer
	: public CPlaneRasterizerBase
{
public:
	//! Set plane sizes
	void SetSizes( int xmin, int xmax, int ymin, int ymax ) { m_xmin = xmin; m_xmax = xmax; m_ymin = ymin; m_ymax = ymax; }

protected:
	//! Test if coordinates are on plane
	virtual bool TestPoint( const osg::Vec2 & point ) 
		{ return point.x() >= m_xmin && point.x() < m_xmax && point.y() >= m_ymin && point.y() < m_ymax;}

	//! Project point from 3D space on plane.
	virtual osg::Vec2 Project( const osg::Vec3 & point ) { return osg::Vec2( point.x(), point.y() ); }

protected:
	//! Plane sizes
	int m_xmin, m_xmax, m_ymin, m_ymax;
};


/******************************************************************************
	Class CPlaneXZRasterizer
******************************************************************************/
class CPlaneXZRasterizer
	: public CPlaneRasterizerBase
{
public:
	//! Set plane sizes
	void SetSizes( int xmin, int xmax, int ymin, int ymax ) { m_xmin = xmin; m_xmax = xmax; m_ymin = ymin; m_ymax = ymax; }

protected:
	//! Test if coordinates are on plane
	virtual bool TestPoint( const osg::Vec2 & point ) 
		{ return point.x() >= m_xmin && point.x() < m_xmax && point.y() >= m_ymin && point.y() < m_ymax;}

	//! Project point from 3D space on plane.
	virtual osg::Vec2 Project( const osg::Vec3 & point ) { return osg::Vec2( point.x(), point.z() ); }

protected:
	//! Plane sizes
	int m_xmin, m_xmax, m_ymin, m_ymax;
};


/******************************************************************************
	Class CPlaneYZRasterizer
******************************************************************************/
class CPlaneYZRasterizer
	: public CPlaneRasterizerBase
{
public:
	//! Set plane sizes
	void SetSizes( int xmin, int xmax, int ymin, int ymax ) { m_xmin = xmin; m_xmax = xmax; m_ymin = ymin; m_ymax = ymax; }

protected:
	//! Test if coordinates are on plane
	virtual bool TestPoint( const osg::Vec2 & point ) 
		{ return point.x() >= m_xmin && point.x() < m_xmax && point.y() >= m_ymin && point.y() < m_ymax;}

	//! Project point from 3D space on plane.
	virtual osg::Vec2 Project( const osg::Vec3 & point ) { return osg::Vec2( point.y(), point.z() ); }

protected:
	//! Plane sizes
	int m_xmin, m_xmax, m_ymin, m_ymax;
};


/******************************************************************************
	CLASS CPlane3D rasterizer
******************************************************************************/
class CPLaneRasterizer
	: public CPlaneRasterizerBase
{


}; // class CPlaneRasterBase

} // namespace seg

// CPlaneRasterizer_H_included
#endif
