///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MOUSEDRAW_H
#define MOUSEDRAW_H

#include <osg/CEventHandlerBase.h>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Vec3>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Point>
#include <osgUtil/LineSegmentIntersector>
#include <osg/CIntersectionProspector.h>

#include <VPL/Module/Signal.h>

namespace osg
{

///////////////////////////////////////////////////////////////////////////////
//! Line geode

class CLineGeode : public Geode
{
public:
	enum ESSFlags
	{
		DISABLE_DEPTH_TEST = 1 << 0,
		USE_RENDER_BIN	= 1 << 1,
		ENABLE_TRANSPARENCY = 1 << 2
	};
public:
	//! Constructor
	CLineGeode( int flags );

	//! Add point
	void AddPoint( const osg::Vec3 & point );

	//! Set color
	void SetColor( const Vec4 & color ) { m_colors->operator[]( 0 ) = color; }

	//! Set gl mode
	void SetMode( GLenum mode ) { m_drawArrays->setMode( mode ); m_drawArrays->dirty(); }

	//! Get line vertices
	Vec3Array * GetVertices() { return m_vertices.get(); }

	//!
	Vec3Array * getLineVertices() { return dynamic_cast< osg::Vec3Array* >( m_geometry->getVertexArray() ); }

    //! Clear all lines
    void clear() { getLineVertices()->clear(); m_vertices->clear(); }

protected:
	//! Node geometry
	ref_ptr< Geometry > m_geometry;

	//! State set
	ref_ptr< StateSet > m_stateSet;

	//! Point size
	ref_ptr< Point > m_pointSize;

	//! Vertices
	ref_ptr< Vec3Array > m_vertices;

	//! Color array
	ref_ptr< Vec4Array > m_colors;

	//! Color
	Vec4 m_color;

	//! Draw array
	ref_ptr< DrawArrays > m_drawArrays;

}; // class CLineGeode


///////////////////////////////////////////////////////////////////////////////
//! Lines group

class CLinesGroup : public MatrixTransform
{
public:
	//! Add line

protected:

}; // class CLinesGroup


///////////////////////////////////////////////////////////////////////////////
//! CLASS CDrawingsGroup - group used to store drawings.

class CDrawingsGroup : public osg::Group
{
public:
	//! Constructor
	CDrawingsGroup();

	//! Clear all drawings ( used as a SigClearAllGizmos signal response too...).
	void clearDrawings();

	//! Use clearing signal?
	void enableClearing( bool enable = true ) { m_clearingEnabled = enable; }

protected:
	//! SigClearAllGizmos signal connection
	vpl::mod::tSignalConnection m_conClear;

	//! Enable/disable clearing switch
	bool m_clearingEnabled;

}; // class CDrawingsGroup


} // namespace osg


namespace osgGA
{

///////////////////////////////////////////////////////////////////////////////
//! CLASS CMousePoint

class CMousePoint
{
public:
    //! Point - window coordinates (raw data with mouse position)
    osg::Vec2 m_pointWindow;

	//! Stroke point - world coordinates
	osg::Vec3 m_point;

	//! Stroke point - local coordinates
	osg::Vec3 m_pointLocal;

	//! Point - volume coordinates
	osg::Vec3 m_pointVolume;

	//! Normal - world coordinates
	osg::Vec3 m_normal;

	//! Normal - local coordinates
	osg::Vec3 m_normalLocal;

	//! Mouse button event - press, release
	int m_buttonEvent;

	//! Mod keys
	int m_modKeyMask;

	//! Mouse button
	unsigned int m_buttonMask;

}; // class CMouseStroke

typedef std::vector<CMousePoint> tMousePointVec;

///////////////////////////////////////////////////////////////////////////////
//! Base drawing handler

class CMouseDrawHandler : public scene::CEventHandlerBase
{
public:
	//! Constructor
	CMouseDrawHandler( OSGCanvas * canvas );

	//! Handle event
	virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* );

	//! Set group for new lines
	void SetLinesNode( osg::CDrawingsGroup * node ) { m_linesNode = node; }

	//! Draw line? (true is default)
	void SetDraw( bool draw = true ) { m_drawLine = draw; } 

	//! Clear drawn lines from node
	void ClearLines();

	//! Set line flags
	void setLineFlags( int flags ) { m_lineFlags = flags; }

protected:
	//! Compute intersection
	virtual bool GetIntersection( CMousePoint & intersection, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa ) { return false; }

	//! Test if handler should be used
	virtual bool UseHandler() { return false; }

	//! Do on mouse down
	virtual void OnMousePush( const CMousePoint & point ) {}

	//! Do on mouse move
	virtual void OnMouseDrag( const CMousePoint & point ) {}

	//! Do on mouse up
	virtual void OnMouseRelease( const CMousePoint & point, bool bUsePoint ) {}

    //! Start drawing
    virtual void initDraw(const CMousePoint &point);

    //! Stop drawing
    virtual void stopDraw(const CMousePoint &point);

protected:
	//! Draw line switch
	bool m_drawLine;

	//! Node used to store line
	osg::ref_ptr< osg::CDrawingsGroup > m_linesNode;

	//! Line drawn
	osg::ref_ptr< osg::CLineGeode > m_line;

	//! Lines
	std::vector< osg::ref_ptr< osg::CLineGeode > > m_linesVector;

    //! Drawing now flag
    volatile bool bDrawing;

	//! Line geode flags
	int m_lineFlags;

    //! Drawed line width
    float m_lineWidth;

    //! Drawed line color
    osg::Vec4 m_lineColor;

}; // class CMouseDrawHandler


///////////////////////////////////////////////////////////////////////////////
//! On screen draw handler

class CScreenDrawHandler : public CMouseDrawHandler
{
public:
	//! Constructor
	CScreenDrawHandler( OSGCanvas * canvas );

	//! Set z coordinate
	void setZ( double z ) { m_z = z; }

protected:
	//! Compute intersection
	virtual bool GetIntersection( CMousePoint &intersection, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa );

protected:
	//! Z-coordinate used
	double m_z;

}; // class CScreenDrawHandler


///////////////////////////////////////////////////////////////////////////////
//!	On geometry draw handler

class CGeometryDrawHandler : public CMouseDrawHandler
{
protected:
	//! Intersection type
	typedef osgUtil::LineSegmentIntersector::Intersection tIntersection;

	//! Intersections type
	typedef osgUtil::LineSegmentIntersector::Intersections tIntersections;

	//! Drawable geometry type
	typedef osg::CNodeListIntersectionDesired< tIntersection > tDrawableGeometry;

	//! Intersection prospector type
	typedef osg::CIntersectionProspector< tIntersection, tIntersections > tIntersectionProspector;

public:
	//! Constructor
	CGeometryDrawHandler(OSGCanvas * canvas);

	//! Add geometry to draw on
	void AddNode( osg::Node * node ) { m_drawableGeometry->addNode( node ); }

    //! Remove geometries to draw on
    void ClearNodes() { m_drawableGeometry->clearNodes(); }

    //! Get currently used nodes
    const tDrawableGeometry::tNodesVector &getNodes() { return m_drawableGeometry->getNodesVector(); }

protected:
	//! Compute intersection
	virtual bool GetIntersection( CMousePoint &intersection, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa );

   
    //! Drawable geometry list
	tDrawableGeometry * m_drawableGeometry;

	//! Intersection prospector
	tIntersectionProspector m_prospector;

}; // class CGeometryDrawHandler


} // namespace osgGA

#endif // MOUSEDRAW_H
