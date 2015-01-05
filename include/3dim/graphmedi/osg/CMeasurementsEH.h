///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#ifndef MEASUREMENTEVENTHANDLER_H
#define MEASUREMENTEVENTHANDLER_H

#include <osg/CSceneOSG.h>
#include <osgUtil/LineSegmentIntersector>
#include <osg/CIntersectionProspector.h>
#include <data/CSceneManipulatorDummy.h>
#include <VPL/Image/VolumeFilters/Median.h>
#include <VPL/Image/VolumeFilters/Averaging.h>

namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! Ruler gizmo

class CRulerGizmo : public osg::Switch
{
public:
	//! Constructor
	CRulerGizmo();

	//! Destructor
	~CRulerGizmo();

	//! Show/hide
	void show( bool bShow = true );

	//! Recompute geometry
	void update( const osg::Vec3 & spoint, const osg::Vec3 & snormal, const osg::Vec3 & epoint, const osg::Vec3 & enormal );

	// Get line color
	osg::Vec4 getLineColor(){ return m_lineColor; }

	// Set line color
	void setLineColor( const osg::Vec4 & c ){ m_lineColor = c; }

	// Get text color
	osg::Vec4 getTextColor(){ return m_textColor; }

	// Set text color
	void setTextColor( const osg::Vec4 & c ){ m_textColor = c; }

	// Get shadow color
	osg::Vec4 getShadowColor(){ return m_shadowColor; }

	// Set shadow color
	void setShadowColor( const osg::Vec4 & c ){ m_shadowColor = c; }

protected:
	//! Is ruler visible?
	bool m_visible;

	//! Line geode
	osg::ref_ptr<osg::Geode> m_lineGeode;

	//! Geometry
	osg::ref_ptr<osg::Geometry> m_lineGeometry;

	//! Vertices
	osg::ref_ptr<osg::Vec3Array> m_vertices;

	//! points matrix transform
	osg::ref_ptr< osg::MatrixTransform > m_mt[ 2 ];

	//! Text
	osg::ref_ptr< osgText::Text > m_text;

	//! Line color
	osg::Vec4 m_lineColor;

	//! Text color
	osg::Vec4 m_textColor;

	//! Shadow color
	osg::Vec4 m_shadowColor;

}; // class CRulerGizmo


///////////////////////////////////////////////////////////////////////////////
//! CDensityGizmo - Denzity measurement geometry

class CDensityGizmo : public osg::Switch
{
public:
	//! Constructor
	CDensityGizmo( const double value, const double radius, const osg::Vec3 & position, const osg::Vec3 & normal, const osg::Matrix & unOrthoMatrix );

	//! Destructor

	// Get line color
	osg::Vec4 getLineColor(){ return m_lineColor; }

	// Set line color
	void setLineColor( const osg::Vec4 & c ){ m_lineColor = c; }

	// Get text color
	osg::Vec4 getTextColor(){ return m_textColor; }

	// Set text color
	void setTextColor( const osg::Vec4 & c ){ m_textColor = c; }

	// Get shadow color
	osg::Vec4 getShadowColor(){ return m_shadowColor; }

	// Set shadow color
	void setShadowColor( const osg::Vec4 & c ){ m_shadowColor = c; }

protected:
	//! Geode
	osg::ref_ptr<osg::Geode> m_geode;

	//! Text geode
	osg::ref_ptr<osg::Geode > m_textGeode;

	//! Geometry
	osg::ref_ptr<osg::Geometry> m_geometry;

	//! Vertices
	osg::ref_ptr<osg::Vec3Array> m_vertices;

	//! Line color
	osg::Vec4 m_lineColor;

	//! Text color
	osg::Vec4 m_textColor;

	//! Shadow color
	osg::Vec4 m_shadowColor;


}; // class CDensityGizmo


///////////////////////////////////////////////////////////////////////////////
//! Computes density

class CDensitySolver
{
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief   Density solver modes. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum EMode
    {
        SINGLE, //! take single point
        AVERAGE, //! Compute average value
        MEDIAN //! Compute median value
    
    }; // enum EMode

public:
    //! Constructor
    CDensitySolver();

	//! Compute density operator
	vpl::img::tDensityPixel computeDensity( data::CDensityData * volume, osg::Vec3 position );

    //! Set computation mode
    void setMode( EMode mode ){ m_mode = mode; }

    EMode getMode() const { return m_mode; }

protected:
	//! test position
	bool testPosition( data::CDensityData * volume, osg::Vec3 & position );

protected:
    //! Used mode
    EMode m_mode;

}; // class CDensity solver


///////////////////////////////////////////////////////////////////////////////
//! Measurements event handler 
//! - click and compute density, drag and compute distance.

class CMeasurementsEH : public CEventHandlerBase
{
protected:

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief   Values that represent tool id number. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    enum EToolId
    {
        DENSITY = 0,
        DISTANCE = 1
    };

public:
	//! Constructor
    CMeasurementsEH( OSGCanvas * canvas, scene::CSceneOSG * scene, bool handleDistance = true, bool handleDensity = true, bool handleDensityUnderCursor = false );

	//! Destructor
	~CMeasurementsEH();

	//! Handle operations
	bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* );

    //! Add intersection desired node
    void addDesiredNode( osg::Node * node );

    //! When set handler invokes DensityMeasureSignal on mouse move
    void setHandleDensityUnderCursor(bool bSet);

protected:
	//! Handle density measurement
	bool handleDensity( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* );

	//! Handle distance measurement
	bool handleDistance( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* );

    //! Handle density measurement
    bool handleDensityUnderCursor( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* );

	//! Compute density on position
	double computeDensity( const osg::Vec3f & position );

	//! Compute objects under mouse pointer
    bool computeIntersections(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Node * scene_root, osgUtil::LineSegmentIntersector::Intersection & intersection, osg::Node::NodeMask traversalMask = 0xffffffff);

	//! On mode changed - signal response
	void OnModeChanged( scene::CAppMode::tMode mode );

    //! On parameters changed - signal response
    void OnParametersChanged( int tool, int flag );

    //! Modify data used to create implant
    virtual void modifyCreationCoordinates(osg::Vec3 & position, osg::Vec3 & direction);

protected:
	//! Scene used
	osg::ref_ptr< scene::CSceneOSG > m_scene;

	//! Handle distance?
	bool m_handleDistance;

	//! Handle density?
	bool m_handleDensity;

    //! Handle density under cursor?
    bool m_handleDensityUnderCursor;

	//! Intersections prospector - by type
	osg::CIntersectionProspector<osgUtil::LineSegmentIntersector::Intersection, osgUtil::LineSegmentIntersector::Intersections> m_ip;

	//! Desired intersections as node list - planes
	osg::CNodeListIntersectionDesired<osgUtil::LineSegmentIntersector::Intersection> * m_desiredList;
	osg::CNodeTypeIntersectionDesired<osgUtil::LineSegmentIntersector::Intersection, scene::CSliceGeode> * m_desiredNodes;

	//! Start intersection
	osg::Vec3 m_start, m_startN,

	//! End intersection
			  m_end, m_endN;

	//! Ruler geometry object
	osg::ref_ptr< CRulerGizmo > m_ruler;

	//! Density collector used
	CDensitySolver m_collector;

}; // class CMeasurementseh


///////////////////////////////////////////////////////////////////////////////
//! CLASS CMeasurements3DEH - modified class observes trackball movement

class CMeasurements3DEH 
	: public CMeasurementsEH, public data::CObjectObserver< data::CSceneManipulatorDummy >
{
public:
	//! Constructor
	CMeasurements3DEH( OSGCanvas * canvas, scene::CScene3D * scene ) : CMeasurementsEH( canvas, scene ) 
	{
		// Connect on dummy
		APP_STORAGE.connect( data::Storage::SceneManipulatorDummy::Id, this);

		// Clear gizmos when slice moved
		m_conSliceXYMoved = VPL_SIGNAL(SigSetSliceXY).connect( this, & CMeasurements3DEH::SigSliceMoved );
		m_conSliceXZMoved = VPL_SIGNAL(SigSetSliceXZ).connect( this, & CMeasurements3DEH::SigSliceMoved );
		m_conSliceYZMoved = VPL_SIGNAL(SigSetSliceYZ).connect( this, & CMeasurements3DEH::SigSliceMoved );
	}

protected:
	//! Clear gizmos when object has changed
	void objectChanged(data::CSceneManipulatorDummy *pData)
	{
		m_scene->clearGizmos();
	}

	//! Slice moved signal answer
	void SigSliceMoved( int position ) { CMeasurementsEH::m_scene->clearGizmos(); }

	//! Slice moved signal connection
	vpl::mod::tSignalConnection m_conSliceXYMoved, m_conSliceXZMoved, m_conSliceYZMoved;

};


///////////////////////////////////////////////////////////////////////////////
//! CLASS CMeasurementsXYEH - modified class observes XY slice movements

class CMeasurementsXYEH
	: public CMeasurementsEH
{
public:
	//! Constructor
	CMeasurementsXYEH( OSGCanvas * canvas, scene::CSceneOSG * scene ) : CMeasurementsEH( canvas, scene ) 
	{
		// Connect to the slice moved signal
		m_conSliceMoved = VPL_SIGNAL(SigSetSliceXY).connect( this, & CMeasurementsXYEH::SigSliceMoved );
	}

protected:
	//! Slice moved signal answer
	void SigSliceMoved( int position ) { CMeasurementsEH::m_scene->clearGizmos(); }

	//! Slice moved signal connection
	vpl::mod::tSignalConnection m_conSliceMoved;

};


///////////////////////////////////////////////////////////////////////////////
//! CLASS CMeasurementsXZEH - modified class observes XZ slice movements

class CMeasurementsXZEH
	: public CMeasurementsEH
{
public:
	//! Constructor
	CMeasurementsXZEH( OSGCanvas * canvas, scene::CSceneOSG * scene ) : CMeasurementsEH( canvas, scene ) 
	{
		// Connect to the slice moved signal
		m_conSliceMoved = VPL_SIGNAL(SigSetSliceXZ).connect( this, & CMeasurementsXZEH::SigSliceMoved );
	}

protected:
	//! Slice moved signal answer
	void SigSliceMoved( int position ) { CMeasurementsEH::m_scene->clearGizmos(); }

	//! Slice moved signal connection
	vpl::mod::tSignalConnection m_conSliceMoved;

};


///////////////////////////////////////////////////////////////////////////////
//! CLASS CMeasurementsXYEH - modified class observes YZ slice movements

class CMeasurementsYZEH
	: public CMeasurementsEH
{
public:
	//! Constructor
	CMeasurementsYZEH( OSGCanvas * canvas, scene::CSceneOSG * scene ) : CMeasurementsEH( canvas, scene ) 
	{
		// Connect to the slice moved signal
		m_conSliceMoved = VPL_SIGNAL(SigSetSliceYZ).connect( this, & CMeasurementsYZEH::SigSliceMoved );
	}

protected:
	//! Slice moved signal answer
	void SigSliceMoved( int position ) { CMeasurementsEH::m_scene->clearGizmos(); }

	//! Slice moved signal connection
	vpl::mod::tSignalConnection m_conSliceMoved;

};
	

} // namespace osgGA

#endif // MEASUREMENTEVENTHANDLER_H
