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

#ifndef WidgetOverlayNode_H_included
#define WidgetOverlayNode_H_included

#include <graph/widgets/CSizeableWindowManager.h>

#include <data/CStorageEntry.h>

#include <osg/Projection>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osgWidget/Canvas>

class OSGCanvas;

namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//!

//class CWidgetOverlayNode : public osg::Projection
class CWidgetOverlayNode : public osg::Camera
{
public:
    //!	Default constructor. 
    CWidgetOverlayNode( OSGCanvas * pCanvas );

    //! Destructor
    ~CWidgetOverlayNode();

    //! Add widget to the scene
    bool addWidget( osgWidget::Window * widget );

    //! Get window manager node 
    scene::CSizeableWindowManager * getWM() { return m_wm.get(); }

    //! Get model view node
    osg::MatrixTransform * getMV() { return m_modelView; }

    //! Show/hide widgets
    void showWidgets( bool bShow );

    // Storage signals
    virtual void onParameters( data::CStorageEntry *pEntry );
    virtual void onData( data::CStorageEntry *pEntry );

protected:
    //! Window manager
    osg::ref_ptr< scene::CSizeableWindowManager > m_wm;

    //! Viewer
    osg::ref_ptr< osgViewer::Viewer > m_viewer;

    //! Canvas
    OSGCanvas * m_canvas;

    //! Model view matrix transform
    osg::ref_ptr< osg::MatrixTransform > m_modelView;

    //! Are widgets visible?
    bool m_bWidgetsVisible;

    //! Show/hide widgets signal connection
    vpl::mod::tSignalConnection m_conParameters;
    vpl::mod::tSignalConnection m_conData;

protected:
    //!\brief Update transform callback.
    class CUpdateTransformCallback : public osg::NodeCallback
    {
    public:
	    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
    };

}; // class CWidgetOverlayNode


} // namespace scene

#endif // WidgetOverlayNode
