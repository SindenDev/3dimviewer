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

#include <widgets/CWidgetOverlayNode.h>
#include <data/CDensityData.h>
#include <data/CSceneWidgetParameters.h>
#include <osg/ShapeDrawable>
#include <data/CSceneWidgetParameters.h>
#include <osg/OSGCanvas.h>


////////////////////////////////////////////////////////////////////////////////////////////////////
//

scene::CWidgetOverlayNode::CWidgetOverlayNode(OSGCanvas * pCanvas)
    : m_viewer( pCanvas->getView() )
    , m_canvas( pCanvas )
{
    setName("CWidgetOverlayNode");
    m_viewer->addSlave(this, false);

    // Set projection matrix
    osg::Viewport * vp = m_viewer->getCamera()->getViewport();
    this->setProjectionMatrixAsOrtho( 0, vp->width(), 0, vp->height(), -1000, 1000 );
    this->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    this->setViewMatrix( osg::Matrix::identity() );

    // Draw subgraph after main camera view
    this->setRenderOrder( osg::Camera::POST_RENDER );

    // Only clear the depth buffer
    this->setClearMask( GL_DEPTH_BUFFER_BIT );
    this->setClearColor( osg::Vec4(0.2f, 0.2f, 0.6f, 0.0f) );

    // We don't want the camera to grap event focus
    this->setAllowEventFocus( false );

    // Set update callback
//    this->setUpdateCallback( new CUpdateTransformCallback() );
    this->addUpdateCallback( new CUpdateTransformCallback() );

    // Set state set
    osg::StateSet* ss = this->getOrCreateStateSet();
    ss->setMode(GL_BLEND, osg::StateAttribute::ON);
    ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    ss->setRenderingHint(osg::StateSet::OPAQUE_BIN);

    // Create model view matrix
    m_modelView = new osg::MatrixTransform;
    m_modelView->setMatrix( osg::Matrix::translate(osg::Vec3(0.0, 0.0, -100.0)) );
    this->addChild( m_modelView );

    // Create window manager
    m_wm = new scene::CSizeableWindowManager( m_viewer, 1000, 1000 );
    m_modelView->addChild( m_wm.get() );

    // Setup storage connection
    m_conParameters = APP_STORAGE.getEntrySignal(data::Storage::SceneWidgetsParameters::Id).connect(this, &CWidgetOverlayNode::onParameters);
    m_conData = APP_STORAGE.getEntrySignal(data::Storage::PatientData::Id).connect(this, &CWidgetOverlayNode::onData);

    // Initialize parameters from the storage
    data::CObjectPtr<data::CSceneWidgetParameters> spParameters( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );
    showWidgets( spParameters->widgetsVisible() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CWidgetOverlayNode::CUpdateTransformCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    CWidgetOverlayNode * overlay = dynamic_cast< CWidgetOverlayNode * >( node );
    if( overlay )
    {
        osg::Viewport * vp( overlay->m_viewer->getCamera()->getViewport() );
        overlay->setProjectionMatrixAsOrtho( 0, vp->width(), 0, vp->height(), -1000, 1000 );
    }

    traverse( node, nv );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

bool scene::CWidgetOverlayNode::addWidget( osgWidget::Window * widget )
{ 
    if( m_wm->addChild( widget ) )
    {
        m_wm->resizeAllWindows();
        m_wm->showWidgets( m_bWidgetsVisible );
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CWidgetOverlayNode::showWidgets(bool bShow)
{
    m_wm->showWidgets( bShow );
    m_bWidgetsVisible = bShow;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CWidgetOverlayNode::onParameters(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::CSceneWidgetParameters> spParameters( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );

    showWidgets( spParameters->widgetsVisible() );

    m_canvas->Refresh( false );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

void scene::CWidgetOverlayNode::onData(data::CStorageEntry *pEntry)
{
    data::CObjectPtr<data::CSceneWidgetParameters> spParameters( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );

    showWidgets( spParameters->widgetsVisible() );

    m_canvas->Refresh( false );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

scene::CWidgetOverlayNode::~CWidgetOverlayNode(void)
{
    APP_STORAGE.getEntrySignal(data::Storage::SceneWidgetsParameters::Id).disconnect( m_conParameters );
    APP_STORAGE.getEntrySignal(data::Storage::PatientData::Id).disconnect( m_conData );
}
