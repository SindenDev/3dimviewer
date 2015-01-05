///////////////////////////////////////////////////////////////////////////////
// $Id: CISEventHandler.h
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

#ifndef CISEventHandler_H
#define CISEventHandler_H

#include <osg/SceneDraw.h>
#include <osg/CAppMode.h>
#include <osg/Timer>
#include <data/CObjectObserver.h>
#include <data/CDrawingOptions.h>

#include <drawing/CLineOptimizer.h>

namespace osgGA
{

///////////////////////////////////////////////////////////////////////////////
//! Interactive segmentation event handler

class CISEventHandler : public CSceneGeometryDrawEH, public data::CObjectObserver< data::CDrawingOptions >
{
public:
    //! Constructor
    CISEventHandler( OSGCanvas * canvas, scene::CSceneBase * scene );

    //! Destructor
    ~CISEventHandler();

    //! Set handling parameters
    void SetHandlingParameters( int mode, const osg::Vec4 & lineColor, float lineWidth );

protected:
    //! handle events
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* );

protected:
    //! Do on mouse down
    virtual void OnMousePush( const CMousePoint & point );

    //! Do on mouse move
    virtual void OnMouseDrag( const CMousePoint & point );

    //! Do on mouse up
    virtual void OnMouseRelease( const CMousePoint & point, bool bUsePoint );

    //! On mode changed - signal response
    void OnModeChanged( scene::CAppMode::tMode mode );

    //! Recompute coordinates to the volume
    virtual osg::Vec3 RecomputeToVolume( const osg::Vec3 & point );

    //! Drawing options changed...
    void objectChanged( data::CDrawingOptions * options );

    //! Start drawing
    virtual void initDraw( const CMousePoint & point );

    //! Stop drawing
    virtual void stopDraw( const CMousePoint & point );

protected:
    //! Current handling mode
    data::CDrawingOptions::EDrawingMode m_handlingMode;

    //! Set drawing parameters signal connection
    vpl::mod::tSignalConnection m_conSetHandlingParameters;

    //! App mode changed signal connection
    vpl::mod::tSignalConnection m_conAppModeChanged;

    //! Type of the handler - what type of the scene is it...
    data::CDrawingOptions::EHandlerType m_handlerType;

    //! Line color
    osg::Vec4 m_lineColor;

    //! Line optimizer - removes duplicities and not needed parts
    draw::CLineOptimizer m_lineOptimizer;

    //! Line width
    float m_lineWidth;

    //! Push button mask
    int m_buttonMask;

    //! Drawing now flag
    volatile bool bDrawing;

    //! Num of points reported
    int m_nPointsReported;

	//! Timer for continuous drawing
	osg::Timer		m_timer;
	osg::Timer_t	m_lastUpdateTime;

}; // class CISEventHandler


///////////////////////////////////////////////////////////////////////////////
//! CLASS CISScene3DEH - event handler for 3D scene

class CISScene3DEH 
	: public CISEventHandler
{
public:
	//! Constructor
	CISScene3DEH( OSGCanvas * canvas, scene::CScene3D * scene );
};


///////////////////////////////////////////////////////////////////////////////
//! CLASS CISSceneXYEH - event handler for XY scene

class CISSceneXYEH 
	: public CISEventHandler
{
public:
	//! Constructor
	CISSceneXYEH( OSGCanvas * canvas, scene::CSceneOSG * scene );
};


///////////////////////////////////////////////////////////////////////////////
//! CLASS CISSceneXZEH - event handler for XZ scene

class CISSceneXZEH 
	: public CISEventHandler
{
public:
	//! Constructor
	CISSceneXZEH( OSGCanvas * canvas, scene::CSceneOSG * scene );
};


///////////////////////////////////////////////////////////////////////////////
//! CLASS CISSceneYZEH - event handler for YZ scene

class CISSceneYZEH 
	: public CISEventHandler
{
public:
	//! Constructor
	CISSceneYZEH( OSGCanvas * canvas, scene::CSceneOSG * scene );
};

///////////////////////////////////////////////////////////////////////////////
//! CLASS CISWindowEH - event handler for window
class CISWindowEH : public CSceneWindowDrawEH, public data::CObjectObserver<data::CDrawingOptions>
{
public:
    //! Constructor
    CISWindowEH(OSGCanvas *canvas, scene::CSceneBase *scene);

    //! Destructor
    ~CISWindowEH();

protected:
    //! handle events
    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Object *, osg::NodeVisitor *);

protected:
    //! Do on mouse down
    virtual void OnMousePush(const CMousePoint &point);

    //! Do on mouse move
    virtual void OnMouseDrag(const CMousePoint &point);

    //! Do on mouse up
    virtual void OnMouseRelease(const CMousePoint &point, bool bUsePoint);

    //! On mode changed - signal response
    void OnModeChanged(scene::CAppMode::tMode mode);

    //! Drawing options changed...
    void objectChanged(data::CDrawingOptions *options);

    //! Start drawing
    virtual void initDraw(const CMousePoint &point);

    //! Stop drawing
    virtual void stopDraw(const CMousePoint &point);

protected:
    //! Current handling mode
    data::CDrawingOptions::EDrawingMode m_handlingMode;

    //! Set drawing parameters signal connection
    vpl::mod::tSignalConnection m_conSetHandlingParameters;

    //! App mode changed signal connection
    vpl::mod::tSignalConnection m_conAppModeChanged;

    //! Type of the handler - what type of the scene is it...
    data::CDrawingOptions::EHandlerType m_handlerType;

    //! Line color
    osg::Vec4 m_lineColor;

    //! Line optimizer - removes duplicities and not needed parts
    draw::CLineOptimizer m_lineOptimizer;

    //! Line width
    float m_lineWidth;

    //! Push button mask
    int m_buttonMask;

    //! Drawing now flag
    volatile bool bDrawing;

    //! Points in window space
    osg::ref_ptr<osg::Vec3Array> m_pointsWindow;

}; // class CISWindowEH

} // namespace osg

#endif // // CISEventHandler_H
