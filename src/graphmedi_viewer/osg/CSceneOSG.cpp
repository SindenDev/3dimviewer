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

#include <base/Defs.h>
#include <osg/CSceneOSG.h>
#include <data/CSceneManipulatorDummy.h>
#include <data/CSceneWidgetParameters.h>
#include <render/PSVRosg.h>
#include <render/PSVRrenderer.h>

//#include <osgManipulator/Translate2DDragger>

//====================================================================================================================
scene::CSceneBase::CSceneBase(OSGCanvas * pCanvas)
{
    setCanvas(pCanvas);

    // turn off the lights
    this->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    p_AnchorGroup = new osg::MatrixTransform;
    p_AnchorAndCenterGroup = new osg::MatrixTransform;
    this->addChild(p_AnchorGroup.get());
    this->addChild(p_AnchorAndCenterGroup.get());

    // Create gizmos group
    m_gizmosGroup = new osg::COnOffNode;
    m_gizmosGroup->show();
    p_AnchorAndCenterGroup->addChild(m_gizmosGroup.get());

    // Connect to the clear all gizmos signal
    m_ClearAllGizmosConnection = VPL_SIGNAL(SigClearAllGizmos).connect(this, &scene::CSceneOSG::clearGizmos);
    m_SceneMovedConnection = APP_STORAGE.getEntrySignal(data::Storage::SceneManipulatorDummy::Id).connect(this, &scene::CSceneOSG::onSceneMoved);

    // position scene
    defaultPositioning();

    // add event handlers
    p_DraggerEventHandler = new CDraggerEventHandler(pCanvas);
    m_pCanvas->addEventHandler(p_DraggerEventHandler.get());

    p_DensityWindowEventHandler = new CDensityWindowEventHandler();
    m_pCanvas->addEventHandler(p_DensityWindowEventHandler.get());

    p_CommandEventHandler = new CCommandEventHandler();
    m_pCanvas->addEventHandler(p_CommandEventHandler.get());

    // set manipulator geometry
    m_pCanvas->getView()->getCameraManipulator()->setNode(this);
    m_pCanvas->getView()->getCameraManipulator()->computeHomePosition();

    // setup rendering order - gizmos
    m_gizmosGroup->getOrCreateStateSet()->setNestRenderBins(false); // we really REALLY want to separate this layer at the top
    m_gizmosGroup->getOrCreateStateSet()->setRenderBinDetails(LAYER_GIZMOS, "RenderBin");

    p_AnchorGroup->getOrCreateStateSet()->setRenderBinDetails(LAYER_SCENE, "RenderBin");
    p_AnchorAndCenterGroup->getOrCreateStateSet()->setRenderBinDetails(LAYER_SCENE, "RenderBin");
    this->getOrCreateStateSet()->setRenderBinDetails(LAYER_SCENE, "RenderBin");

    // Set the update callback
    APP_STORAGE.connect(data::Storage::ActiveDataSet::Id, this);
    this->setupObserver(this);
}

//====================================================================================================================
scene::CSceneBase::~CSceneBase()
{
    // disconnect signals
    this->freeObserver(this);
    APP_STORAGE.disconnect(data::Storage::ActiveDataSet::Id, this);
    VPL_SIGNAL(SigClearAllGizmos).disconnect(m_ClearAllGizmosConnection);
    APP_STORAGE.getEntrySignal(data::Storage::SceneManipulatorDummy::Id).disconnect(m_SceneMovedConnection);
}

//====================================================================================================================
void scene::CSceneBase::anchorToScene(osg::Node * node, bool bCenter)
{
    if (bCenter)
    {
        p_AnchorAndCenterGroup->addChild(node);
    }
    else
    {
        p_AnchorGroup->addChild(node);
    }
}

//====================================================================================================================
void scene::CSceneBase::defaultPositioning()
{ }

//====================================================================================================================
void scene::CSceneBase::setupScene(data::CDensityData & data)
{ }

//====================================================================================================================
void scene::CSceneBase::updateFromStorage()
{ }

//====================================================================================================================
scene::CSceneOSG::CSceneOSG(OSGCanvas *pCanvas, bool xyOrtho, bool xzOrtho, bool yzOrtho, bool bCreateScene /*= true*/)
    : CSceneBase(pCanvas)
    , f_Thin(1.0f)
    , m_bXYOrtho(xyOrtho)
    , m_bXZOrtho(xzOrtho)
    , m_bYZOrtho(yzOrtho)
{
    // Get widgets color
    data::CObjectPtr<data::CSceneWidgetParameters> ptrCOptions(APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id));
    m_widgetsColor = ptrCOptions->getMarkersColor();

    if (bCreateScene)
        createScene();
}

//====================================================================================================================
scene::CSceneOSG::~CSceneOSG()
{
    if (m_conVis[0].getSignalPtr())
        VPL_SIGNAL(SigSetPlaneXYVisibility).disconnect(m_conVis[0]);
    if (m_conVis[1].getSignalPtr())
        VPL_SIGNAL(SigSetPlaneXYVisibility).disconnect(m_conVis[1]);
    if (m_conVis[2].getSignalPtr())
        VPL_SIGNAL(SigSetPlaneXZVisibility).disconnect(m_conVis[2]);
    if (m_conVis[3].getSignalPtr())
        VPL_SIGNAL(SigSetPlaneXZVisibility).disconnect(m_conVis[3]);
    if (m_conVis[4].getSignalPtr())
        VPL_SIGNAL(SigSetPlaneYZVisibility).disconnect(m_conVis[4]);
    if (m_conVis[5].getSignalPtr())
        VPL_SIGNAL(SigSetPlaneYZVisibility).disconnect(m_conVis[5]);
}

//====================================================================================================================
void scene::CSceneOSG::defaultPositioning()
{
    p_DraggableSlice[0]->moveInDepth(p_DraggableSlice[0]->getVoxelDepth() / 2);
    p_DraggableSlice[1]->moveInDepth(p_DraggableSlice[1]->getVoxelDepth() / 2);
    p_DraggableSlice[2]->moveInDepth(p_DraggableSlice[2]->getVoxelDepth() / 2);

    m_pCanvas->Refresh(false);
}

//====================================================================================================================
void scene::CSceneOSG::setupScene(data::CDensityData & data)
{
    p_DraggableSlice[0]->scaleScene(data.getDX(), data.getDY(), data.getDZ(), data.getXSize(), data.getYSize(), data.getZSize());
    p_DraggableSlice[1]->scaleScene(data.getDX(), data.getDY(), data.getDZ(), data.getXSize(), data.getYSize(), data.getZSize());
    p_DraggableSlice[2]->scaleScene(data.getDX(), data.getDY(), data.getDZ(), data.getXSize(), data.getYSize(), data.getZSize());

    defaultPositioning();

    osg::Vec3 translation(-0.5 * data.getXSize() * data.getDX(),
                          -0.5 * data.getYSize() * data.getDY(),
                          -0.5 * data.getZSize() * data.getDZ());

    osg::Vec3 ortho_translation(-0.5 * data.getXSize() * data.getDX(),
                                -0.5 * data.getYSize() * data.getDY(),
                                0.0);

    p_AnchorAndCenterGroup->setMatrix(osg::Matrix::translate(translation));
    p_OrthoAnchorAndCenterGroup->setMatrix(osg::Matrix::translate(ortho_translation));

    // Clear gizmos
    clearGizmos();
    m_pCanvas->Refresh(false);
    this->dirtyBound();

    osg::BoundingBox box(translation, -translation);
    m_pCanvas->centerAndScale(box);

    // Invoke scene moved signal
    data::CObjectPtr<data::CSceneManipulatorDummy> ptr(APP_STORAGE.getEntry(data::Storage::SceneManipulatorDummy::Id));
    APP_STORAGE.invalidate(ptr.getEntryPtr());
    m_pCanvas->Refresh(false);
}

//====================================================================================================================
int	scene::CSceneOSG::getXSize() const
{
    return p_DraggableSlice[2]->getVoxelDepth();
}

//====================================================================================================================
int	scene::CSceneOSG::getYSize() const
{
    return p_DraggableSlice[1]->getVoxelDepth();
}

//====================================================================================================================
int	scene::CSceneOSG::getZSize() const
{
    return p_DraggableSlice[0]->getVoxelDepth();
}

//====================================================================================================================
float scene::CSceneOSG::getDX() const
{
    return p_DraggableSlice[2]->getVoxelSize();
}

//====================================================================================================================
float scene::CSceneOSG::getDY() const
{
    return p_DraggableSlice[1]->getVoxelSize();
}

//====================================================================================================================
float scene::CSceneOSG::getDZ() const
{
    return p_DraggableSlice[0]->getVoxelSize();
}

//====================================================================================================================
float scene::CSceneOSG::getThin() const
{
    return f_Thin;
}

//====================================================================================================================
void scene::CSceneOSG::anchorToSliceXY(osg::Node * node)
{
    p_DraggableSlice[0]->anchorToSlice(node);
}

//====================================================================================================================
void scene::CSceneOSG::anchorToSliceXZ(osg::Node * node)
{
    p_DraggableSlice[1]->anchorToSlice(node);
}

//====================================================================================================================
void scene::CSceneOSG::anchorToSliceYZ(osg::Node * node)
{
    p_DraggableSlice[2]->anchorToSlice(node);
}

//====================================================================================================================
void scene::CSceneOSG::updateFromStorage()
{
    // Get the active dataset
    data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()));

    // Update the scene
    data::CChangedEntries Changes(this);
    if (!Changes.checkFlagAll(data::CDensityData::DENSITY_MODIFIED))
    //if (!getInvalidateFlags().checkFlagAll(data::CDensityData::DENSITY_MODIFIED))
    {
        this->setupScene(*spData);
    }

    m_pCanvas->Refresh(false);
}

//====================================================================================================================
void scene::CSceneOSG::anchorToOrthoScene(osg::Node * node)
{
    p_OrthoAnchorAndCenterGroup->addChild(node);
}

//====================================================================================================================
scene::CSceneXY::CSceneXY(OSGCanvas * canvas)
    : CSceneOSG(canvas, true, false, false)
{
    // rotate scene to face orthowindow
    osg::Matrix	m = osg::Matrix::rotate(osg::DegreesToRadians(180.0), osg::Vec3(0.0, 0.0, 1.0));

    // Create widgets
    osg::Matrix	wsm = osg::Matrix::rotate(osg::DegreesToRadians(180.0), osg::Vec3(0.0, 0.0, 1.0));
    createWidgetsScene(canvas, wsm, SW_ORIENT | SW_RULER | CSceneOrientationWidget::SW_LEFT_LABEL | CSceneOrientationWidget::SW_POST_LABEL);

    // translate scene further away, so that it is always rendered
    m_orthoTransformMatrix = m * osg::Matrix::translate(osg::Vec3(0.0, 0.0, SCENE_SHIFT_AMMOUNT));
    m_unOrthoTransformMatrix = osg::Matrix::inverse(m_orthoTransformMatrix);

    this->setMatrix(m_orthoTransformMatrix);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void scene:::::CSceneXY::sliceUpDown(bool direction)
//
//\brief ! Move slice up/down event handler. True means up. 
//
//\param direction   true to direction. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void scene::CSceneXY::sliceUpDown(int direction)
{
    data::CObjectPtr<data::COrthoSliceXY> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXY::Id));
    data::CObjectPtr<data::CActiveDataSet> spDataSet(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(spDataSet->getId()));

    int position = spSlice->getPosition();
    spSlice.release();

	int oldPos = position;
	position+=direction;
	position = std::max(0, std::min( position, spVolume->getZSize()-1 ));
	if (oldPos!=position)
		VPL_SIGNAL(SigSetSliceXY).invoke(position);
}

//====================================================================================================================
scene::CSceneXZ::CSceneXZ(OSGCanvas * canvas)
    : CSceneOSG(canvas, false, true, false)
{
    // rotate scene to face orthowindow
    osg::Matrix	m = osg::Matrix::rotate(osg::DegreesToRadians(180.0), osg::Vec3f(0.0, 0.0, 1.0));
    m = m * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3f(1.0, 0.0, 0.0));

    // Create widgets
    osg::Matrix wsm(osg::Matrix::rotate(osg::DegreesToRadians(180.0), osg::Vec3f(0.0, 1.0, 0.0)));
    wsm = wsm * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3f(1.0, 0.0, 0.0));
    createWidgetsScene(canvas, wsm, SW_ORIENT | SW_RULER | CSceneOrientationWidget::SW_LEFT_LABEL | CSceneOrientationWidget::SW_SUP_LABEL);

    // translate scene further away, so that it is always rendered
    m_orthoTransformMatrix = m * osg::Matrix::translate(osg::Vec3(0.0, 0.0, SCENE_SHIFT_AMMOUNT));
    m_unOrthoTransformMatrix = osg::Matrix::inverse(m_orthoTransformMatrix);

    this->setMatrix(m_orthoTransformMatrix);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void scene:::::CSceneXZ::sliceUpDown(bool direction)
//
//\brief ! Move slice up/down event handler. True means up. 
//
//\param direction   true to direction. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void scene::CSceneXZ::sliceUpDown(int direction)
{
    data::CObjectPtr<data::COrthoSliceXZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id));
    data::CObjectPtr<data::CActiveDataSet> spDataSet(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(spDataSet->getId()));

    int position = spSlice->getPosition();
    spSlice.release();

	int oldPos = position;
	position+=direction;
	position = std::max(0, std::min( position, spVolume->getYSize()-1 ));
	if (oldPos!=position)
		VPL_SIGNAL(SigSetSliceXZ).invoke(position);
}

//====================================================================================================================
scene::CSceneYZ::CSceneYZ(OSGCanvas * canvas)
    : CSceneOSG(canvas, false, false, true)
{
    // rotate scene to face orthowindow
    osg::Matrix	m = osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3f(0.0, 0.0, 1.0));
    m = m * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3f(1.0, 0.0, 0.0));

    // Create widgets
    osg::Matrix wsm(osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Vec3f(0.0, 0.0, 1.0)));
    wsm = wsm * osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Vec3f(1.0, 0.0, 0.0));
    createWidgetsScene(canvas, wsm, SW_ORIENT | SW_RULER | CSceneOrientationWidget::SW_POST_LABEL | CSceneOrientationWidget::SW_SUP_LABEL);

    // translate scene further away, so that it is always rendered
    m_orthoTransformMatrix = m * osg::Matrix::translate(osg::Vec3(0.0, 0.0, SCENE_SHIFT_AMMOUNT));
    m_unOrthoTransformMatrix = osg::Matrix::inverse(m_orthoTransformMatrix);

    this->setMatrix(m_orthoTransformMatrix);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void scene:::::CSceneYZ::sliceUpDown(bool direction)
//
//\brief ! Move slice up/down event handler. True means up. 
//
//\param direction   true to direction. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void scene::CSceneYZ::sliceUpDown(int direction)
{
    data::CObjectPtr<data::COrthoSliceYZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id));
    data::CObjectPtr<data::CActiveDataSet> spDataSet(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(spDataSet->getId()));

    int position = spSlice->getPosition();
	int oldPos = position;
    spSlice.release();

	position+=direction;
	position = std::max(0, std::min( position, spVolume->getXSize()-1 ));
	if (oldPos!=position)
		VPL_SIGNAL(SigSetSliceYZ).invoke(position);
}

//====================================================================================================================
scene::CScene3D::CScene3D(OSGCanvas *pCanvas)
    : scene::CScene3DBase(pCanvas)
{
    // VR drawable and geode
    m_vrDrawable = new PSVR::osgPSVolumeRendering;
    //    m_vrDrawable->getOrCreateStateSet()->setRenderBinDetails(LAYER_VOLUME_RENDERING, "RenderBin");
    PSVR::osgPSVolumeRenderingGeode *pGeode = new PSVR::osgPSVolumeRenderingGeode;
    pGeode->setCanvas(pCanvas);
    m_vrGeode = pGeode;
    m_vrGeode->addDrawable(m_vrDrawable);

    // Create and set rendered MT
    m_renderedGroup = new osg::MatrixTransform;

    m_renderedSS = m_renderedGroup->getOrCreateStateSet();
    m_renderedSS->setRenderBinDetails(LAYER_VOLUME_RENDERING, "RenderBin");
    m_renderedSS->setRenderBinMode(osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
    //m_renderedSS->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    //m_renderedGroup->setCullingActive(false);

    // Add renderer gizmo to the scene
    m_renderedGroup->addChild(m_vrGeode.get());

    // Set anchored groups render order

    // Add rendered group to the scene
    this->addChild(m_renderedGroup.get());
}

//====================================================================================================================
scene::CScene3D::~CScene3D()
{ }

//====================================================================================================================
void scene::CScene3D::setRenderer(PSVR::PSVolumeRendering *pRenderer)
{
    PSVR::osgPSVolumeRendering *pDrawable = dynamic_cast<PSVR::osgPSVolumeRendering *>(m_vrDrawable.get());
    if (!pDrawable)
    {
        return;
    }

    pDrawable->setRenderer(pRenderer);
}

//====================================================================================================================
void scene::CScene3D::updateFromStorage()
{
    /*
    // Get the active dataset
    data::CObjectPtr<data::CDensityData> spData( APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()) );

    // Get sizes
    double sx = spData->getXSize() * spData->getDX() * 0.5;
    double sy = spData->getYSize() * spData->getDY() * 0.5;
    double sz = spData->getZSize() * spData->getDZ() * 0.5;

    // Translate rendered group
    m_renderedGroup->setMatrix( osg::Matrix::translate( sx, sy, sz ) );

    // Data loaded - recenter scene
    // TODO: tady je asi problem (volume rendering jeste neni nastaveny, aby vratil rozumny box)
    if( m_pCanvas )
    {
    m_pCanvas->centerAndScale();
    }
    */

    // parent's update
    scene::CSceneOSG::updateFromStorage();
}

//====================================================================================================================
void scene::CScene3D::enableRenderer(bool enable)
{
    PSVR::osgPSVolumeRendering *pDrawable = dynamic_cast<PSVR::osgPSVolumeRendering *>(m_vrDrawable.get());
    if (!pDrawable || !pDrawable->getRenderer())
    {
        return;
    }

    pDrawable->getRenderer()->enable(enable);
}

//====================================================================================================================
osg::Geode *scene::CScene3D::getVRGeode()
{
    return m_vrGeode.get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	! Create widgets overlay scene. 
//!
//!\param [in,out]	pCanvas	If non-null, the canvas. 
//!\param	viewMatrix		The view matrix. 
//!
//!\return	null if it fails, else. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void scene::CSceneOSG::createWidgetsScene(OSGCanvas *pCanvas, const osg::Matrix &viewMatrix, int Flags)
{
    // Create widgets scene
    m_widgetOverlay = new scene::CWidgetOverlayNode(pCanvas);

    // Add widgets
    if (Flags & SW_INFO)
    {
        scene::CSceneInfoWidget * info = new scene::CSceneInfoWidget(m_pCanvas);
        m_widgetOverlay->addWidget(info);
    }

    if (Flags & SW_ORIENT)
    {
        scene::CSceneOrientationWidget * orientation = new scene::CSceneOrientationWidget(m_pCanvas, 100, 100, viewMatrix, Flags);
        m_widgetOverlay->addWidget(orientation);

        if (Flags & scene::CSceneOrientationWidget::SW_MOVABLE)
        {
            osg::Matrix m(pCanvas->getView()->getCamera()->getViewMatrix());
            osg::Matrix rotation(m.getRotate());
            orientation->setMatrix(rotation);
        }
    }

    if (Flags & SW_RULER)
    {
        scene::CRulerWidget * ruler = new scene::CRulerWidget(m_pCanvas, 60, 400, m_widgetsColor);
        m_widgetOverlay->addWidget(ruler);
        ruler->setWindowManager(m_widgetOverlay->getWM());

    }

    this->addChild(m_widgetOverlay.get());
}

void scene::CSceneOSG::createScene()
{
    p_OrthoAnchorAndCenterGroup = new osg::MatrixTransform;
    addChild(p_OrthoAnchorAndCenterGroup.get());

    // create draggable slices
    p_DraggableSlice[0] = new CDraggableSliceXY(m_pCanvas, m_bXYOrtho, data::Storage::SliceXY::Id);
    p_DraggableSlice[1] = new CDraggableSliceXZ(m_pCanvas, m_bXZOrtho, data::Storage::SliceXZ::Id);
    p_DraggableSlice[2] = new CDraggableSliceYZ(m_pCanvas, m_bYZOrtho, data::Storage::SliceYZ::Id);


    // Get and set colors of slices
    data::CObjectPtr< data::CSceneWidgetParameters > ptrOptions(APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id));
    if (ptrOptions.get() == 0)
        return;

    osg::Vec4 color = ptrOptions->getOrthoSliceColor(0);
    p_DraggableSlice[0]->setFrameColor(color.r(), color.g(), color.b());
    if (m_bXYOrtho) m_viewColor = color;

    color = ptrOptions->getOrthoSliceColor(1);
    p_DraggableSlice[1]->setFrameColor(color.r(), color.g(), color.b());
    if (m_bXZOrtho) m_viewColor = color;

    color = ptrOptions->getOrthoSliceColor(2);
    p_DraggableSlice[2]->setFrameColor(color.r(), color.g(), color.b());
    if (m_bYZOrtho) m_viewColor = color;

    // set voxel depths for scene
    p_DraggableSlice[0]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_DraggableSlice[1]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_DraggableSlice[2]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);

    // set signals invoked while dragging
    p_DraggableSlice[0]->setSignal(&(VPL_SIGNAL(SigSetSliceXY)));
    p_DraggableSlice[1]->setSignal(&(VPL_SIGNAL(SigSetSliceXZ)));
    p_DraggableSlice[2]->setSignal(&(VPL_SIGNAL(SigSetSliceYZ)));

    // Create on/off nodes
    p_onOffNode[0] = new osg::COnOffNode;
    p_onOffNode[1] = new osg::COnOffNode;
    p_onOffNode[2] = new osg::COnOffNode;

    // Connect on/off nodes with signals - only for 3D view
    if (!(m_bXYOrtho || m_bYZOrtho || m_bXZOrtho))
    {
        m_conVis[0] = VPL_SIGNAL(SigSetPlaneXYVisibility).connect(p_onOffNode[0], &osg::COnOffNode::setOnOffState);
        m_conVis[1] = VPL_SIGNAL(SigGetPlaneXYVisibility).connect(p_onOffNode[0], &osg::COnOffNode::isShown);
        m_conVis[2] = VPL_SIGNAL(SigSetPlaneXZVisibility).connect(p_onOffNode[1], &osg::COnOffNode::setOnOffState);
        m_conVis[3] = VPL_SIGNAL(SigGetPlaneXZVisibility).connect(p_onOffNode[1], &osg::COnOffNode::isShown);
        m_conVis[4] = VPL_SIGNAL(SigSetPlaneYZVisibility).connect(p_onOffNode[2], &osg::COnOffNode::setOnOffState);
        m_conVis[5] = VPL_SIGNAL(SigGetPlaneYZVisibility).connect(p_onOffNode[2], &osg::COnOffNode::isShown);

        // add slices to scene graph
        p_onOffNode[0]->addChild(p_DraggableSlice[0].get(), true);
        p_onOffNode[0]->setOnOffState(m_bXYOrtho);
        p_onOffNode[1]->addChild(p_DraggableSlice[1].get(), true);
        p_onOffNode[1]->setOnOffState(m_bXZOrtho);
        p_onOffNode[2]->addChild(p_DraggableSlice[2].get(), true);
        p_onOffNode[2]->setOnOffState(m_bYZOrtho);
    }
    else
    {
        // add slices to scene graph
        p_onOffNode[0]->addChild(p_DraggableSlice[0].get(), true);
        p_onOffNode[0]->setOnOffState(true);
        p_onOffNode[1]->addChild(p_DraggableSlice[1].get(), true);
        p_onOffNode[1]->setOnOffState(true);
        p_onOffNode[2]->addChild(p_DraggableSlice[2].get(), true);
        p_onOffNode[2]->setOnOffState(true);
    }

    this->addChild(p_onOffNode[0].get());
    this->addChild(p_onOffNode[1].get());
    this->addChild(p_onOffNode[2].get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Executes the application mode changed action. 
//!
//!\param   mode    The mode. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void scene::CSceneOSG::onAppModeChanged(scene::CAppMode::tMode mode)
{
    /*bool bEnable(mode == scene::CAppMode::MODE_SLICE_MOVE);

    if (m_bXYOrtho)
    p_DraggableSlice[0]->getSliceDragger()->setHandleEvents(true);*/
}

//====================================================================================================================
scene::CScene3DBase::CScene3DBase(OSGCanvas *pCanvas)
    : scene::CSceneOSG(pCanvas, false, false, false)
{
    m_conWorld[0] = VPL_SIGNAL(SigGetYZWorld).connect(this, &CScene3DBase::getYZWorld);
    m_conWorld[1] = VPL_SIGNAL(SigGetXZWorld).connect(this, &CScene3DBase::getXZWorld);
    m_conWorld[2] = VPL_SIGNAL(SigGetXYWorld).connect(this, &CScene3DBase::getXYWorld);

    m_orthoTransformMatrix = osg::Matrix::identity();
    osg::Matrix m = osg::Matrix::identity();
    createWidgetsScene(pCanvas, m, SW_INFO | SW_ORIENT | CSceneOrientationWidget::SW_ALL_LABELS | CSceneOrientationWidget::SW_MOVABLE);
}

//====================================================================================================================
scene::CScene3DBase::~CScene3DBase()
{
    VPL_SIGNAL(SigGetXYWorld).disconnect(m_conWorld[2]);
    VPL_SIGNAL(SigGetXZWorld).disconnect(m_conWorld[1]);
    VPL_SIGNAL(SigGetYZWorld).disconnect(m_conWorld[0]);
}

//====================================================================================================================
osg::Vec3 scene::CScene3DBase::getXYWorld()
{
    return this->getSliceXY()->getWorldPosition();
}

//====================================================================================================================
osg::Vec3 scene::CScene3DBase::getXZWorld()
{
    return this->getSliceXZ()->getWorldPosition();
}

//====================================================================================================================
osg::Vec3 scene::CScene3DBase::getYZWorld()
{
    return this->getSliceYZ()->getWorldPosition();
}
