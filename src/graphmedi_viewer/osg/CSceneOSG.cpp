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

#include <base/Defs.h>
#include <osg/CSceneOSG.h>
#include <osg/LineWidth>
#include <data/CSceneManipulatorDummy.h>
#include <data/CSceneWidgetParameters.h>
#include <render/PSVRosg.h>
#include <render/PSVRrenderer.h>
#include <Signals.h>
#include <graph/osg/NodeMasks.h>
#include <geometry/base/functions.h>
#include <data/CArbitrarySlice.h>
#include <osg/CTranslateOtherLineDragger.h>
#include <osg/CGeometryGenerator.h>
#include <data/CMultiClassRegionColoring.h>


//#include <osgManipulator/Translate2DDragger>

//====================================================================================================================
scene::CSceneBase::CSceneBase(OSGCanvas * pCanvas)
{
    setCanvas(pCanvas);

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
    m_ClearMeasurementsConnection = VPL_SIGNAL(SigRemoveMeasurements).connect(this, &scene::CSceneOSG::clearGizmos);

    // Create landmark annotations group
    m_landmarkAnnotationsGroup = new osg::COnOffNode;
    m_landmarkAnnotationsGroup->show();
    p_AnchorAndCenterGroup->addChild(m_landmarkAnnotationsGroup.get());

    // Connect to landmark annotation signals
    m_ClearLandmarkAnnotation = VPL_SIGNAL(SigClearLandmarkAnnotationDrawable).connect(this, &scene::CSceneOSG::clearLandmarkAnnotation);
    m_ClearAllLandmarkAnnotations = VPL_SIGNAL(SigClearAllLandmarkAnnotationDrawables).connect(this, &scene::CSceneOSG::clearAllLandmarkAnnotations);
    m_SetLandmarkAnnotationVisibility = VPL_SIGNAL(SigSetLandmarkAnnotationDrawablesVisibility).connect(this, &scene::CSceneOSG::setLandmarkAnnotationsVisibility);

    scene::CGeneralObjectObserverOSG<CSceneBase>::connect(APP_STORAGE.getEntry(data::Storage::SceneManipulatorDummy::Id).get());

    // position scene
    defaultPositioning();

    // add event handlers
    p_DraggerEventHandler = new CDraggerEventHandler(pCanvas);
    unsigned int mask = MASK_ORTHO_2D_DRAGGER;
    p_DraggerEventHandler->setVisitorMask(mask);
    p_DraggerEventHandler->setMaskingMode(scene::CDraggerEventHandler::EMode::MODE_MASKED_FIRST);
    m_pCanvas->addEventHandler(p_DraggerEventHandler.get());

    p_DensityWindowEventHandler = new CDensityWindowEventHandler();
    m_pCanvas->addEventHandler(p_DensityWindowEventHandler.get());

    p_CommandEventHandler = new CCommandEventHandler();
    mask = MASK_REGION_PREVIEW;
    p_CommandEventHandler->setVisitorMask(~mask);
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
    scene::CGeneralObjectObserverOSG<CSceneBase>::connect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id).get());
    scene::CGeneralObjectObserverOSG<CSceneBase>::connect(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id).get());
    this->setupObserver(this);
}

//====================================================================================================================
scene::CSceneBase::~CSceneBase()
{
    // disconnect signals
    this->freeObserver(this);
    scene::CGeneralObjectObserverOSG<CSceneBase>::disconnect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id).get());
    scene::CGeneralObjectObserverOSG<CSceneBase>::disconnect(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id).get());
    VPL_SIGNAL(SigClearAllGizmos).disconnect(m_ClearAllGizmosConnection);
    VPL_SIGNAL(SigRemoveMeasurements).disconnect(m_ClearMeasurementsConnection);
    VPL_SIGNAL(SigClearLandmarkAnnotationDrawable).disconnect(m_ClearLandmarkAnnotation);
    VPL_SIGNAL(SigClearAllLandmarkAnnotationDrawables).disconnect(m_ClearAllLandmarkAnnotations);
    VPL_SIGNAL(SigSetLandmarkAnnotationDrawablesVisibility).disconnect(m_SetLandmarkAnnotationVisibility);
    scene::CGeneralObjectObserverOSG<CSceneBase>::disconnect(APP_STORAGE.getEntry(data::Storage::SceneManipulatorDummy::Id).get());
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
{ 
    std::set<int> changedEntries;
    getChangedEntries(changedEntries);
    if (changedEntries.empty() || changedEntries.find(data::Storage::SceneManipulatorDummy::Id) != changedEntries.end())
    {
        clearGizmos();
        m_pCanvas->Refresh(false);
    }
}

//====================================================================================================================
scene::CSceneOSG::CSceneOSG(OSGCanvas *pCanvas, bool xyOrtho, bool xzOrtho, bool yzOrtho, bool bCreateScene /*= true*/)
    : CSceneBase(pCanvas)
    , f_Thin(1.0f)
    , m_bXYOrtho(xyOrtho)
    , m_bXZOrtho(xzOrtho)
    , m_bYZOrtho(yzOrtho)
    , m_arbSliceVisibility(true)
{
    // Get widgets color
    data::CObjectPtr<data::CSceneWidgetParameters> ptrCOptions(APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id));
    m_widgetsColor = ptrCOptions->getMarkersColor();

    m_conSliceMoved = VPL_SIGNAL(SigOrthoSliceMoved).connect(this, &CSceneOSG::orthoSliceMoved);

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

    if (m_conSliceMoved.getSignalPtr())
        VPL_SIGNAL(SigOrthoSliceMoved).disconnect(m_conSliceMoved);
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

void scene::CSceneOSG::updateArbSliceGeometry()
{
    if (!m_bXYOrtho && !m_bXZOrtho && !m_bYZOrtho)
    {
        return;
    }

    if (!m_arbSliceVisibility)
    {
        return;
    }

    m_arbSlice2D->updateGeometry();
}

//====================================================================================================================
void scene::CSceneOSG::updateFromStorage()
{
    std::set<int> changedEntries;
    getChangedEntries(changedEntries);

    if (changedEntries.find(data::Storage::ArbitrarySlice::Id) != changedEntries.end() && (m_bXYOrtho || m_bXZOrtho || m_bYZOrtho))
    {
        updateArbSliceGeometry();
    }

    // Get the active dataset
    data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()));

    // Update the scene if necessary
    {        
        data::CChangedEntries Changes;
        getChanges(spData.getEntryPtr(), Changes);

        data::CChangedEntries::tFilter filter;
        filter.insert(data::Storage::PatientData::Id);
        filter.insert(data::Storage::AuxData::Id);
        if (
            // initialization
            changedEntries.empty() ||
            // active data set change
            changedEntries.find(data::Storage::ActiveDataSet::Id)!=changedEntries.end() ||
            // change of patient or aux data which doesn't have DENSITY_MODIFIED flag
            ((changedEntries.find(data::Storage::PatientData::Id)!=changedEntries.end() || changedEntries.find(data::Storage::AuxData::Id)!=changedEntries.end())
              && !Changes.checkFlagAll(data::CDensityData::DENSITY_MODIFIED,filter))
            )
        {
            this->setupScene(*spData);
        }
        // invalidate gizmos on manipulator change
        //if (changedEntries.find(data::Storage::SceneManipulatorDummy::Id)!=changedEntries.end())
        //    clearGizmos();
    }

    m_pCanvas->Refresh(false);
}

void scene::CSceneOSG::orthoSliceMoved()
{
    updateArbSliceGeometry();
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

	this->p_DraggableSlice[0]->dummyThin(true);

    m_arbSlice2D->setSceneType(osg::CArbitrarySliceVisualizer2D::EST_XY);
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

    vpl::tSize position = spSlice->getPosition();
    spSlice.release();

	vpl::tSize oldPos = position;
	position+=direction;
    position = std::max((vpl::tSize)0, std::min(position, spVolume->getZSize() - 1));
	if (oldPos!=position)
		VPL_SIGNAL(SigSetSliceXY).invoke(position);

    updateArbSliceGeometry();
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

	this->p_DraggableSlice[1]->dummyThin(true);

    m_arbSlice2D->setSceneType(osg::CArbitrarySliceVisualizer2D::EST_XZ);
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

    vpl::tSize position = spSlice->getPosition();
    spSlice.release();

    vpl::tSize oldPos = position;
	position+=direction;
    position = std::max((vpl::tSize)0, std::min(position, spVolume->getYSize() - 1));
	if (oldPos!=position)
		VPL_SIGNAL(SigSetSliceXZ).invoke(position);

    updateArbSliceGeometry();
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

	this->p_DraggableSlice[2]->dummyThin(true);

    m_arbSlice2D->setSceneType(osg::CArbitrarySliceVisualizer2D::EST_YZ);
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

    vpl::tSize position = spSlice->getPosition();
    vpl::tSize oldPos = position;
    spSlice.release();

	position+=direction;
    position = std::max((vpl::tSize)0, std::min(position, spVolume->getXSize() - 1));
	if (oldPos!=position)
		VPL_SIGNAL(SigSetSliceYZ).invoke(position);

    updateArbSliceGeometry();
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
    m_arbSlice2D = new osg::CArbitrarySliceVisualizer2D(m_pCanvas, data::Storage::ArbitrarySlice::Id);

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
    p_onOffNode[3] = new osg::COnOffNode;

    // Connect on/off nodes with signals - only for 3D view
    if (!(m_bXYOrtho || m_bYZOrtho || m_bXZOrtho))
    {
        m_conVis[0] = VPL_SIGNAL(SigSetPlaneXYVisibility).connect(p_onOffNode[0], &osg::COnOffNode::setOnOffState);
        m_conVis[1] = VPL_SIGNAL(SigGetPlaneXYVisibility).connect(p_onOffNode[0], &osg::COnOffNode::isVisible);
        m_conVis[2] = VPL_SIGNAL(SigSetPlaneXZVisibility).connect(p_onOffNode[1], &osg::COnOffNode::setOnOffState);
        m_conVis[3] = VPL_SIGNAL(SigGetPlaneXZVisibility).connect(p_onOffNode[1], &osg::COnOffNode::isVisible);
        m_conVis[4] = VPL_SIGNAL(SigSetPlaneYZVisibility).connect(p_onOffNode[2], &osg::COnOffNode::setOnOffState);
        m_conVis[5] = VPL_SIGNAL(SigGetPlaneYZVisibility).connect(p_onOffNode[2], &osg::COnOffNode::isVisible);

        // add slices to scene graph
        p_onOffNode[0]->addChild(p_DraggableSlice[0].get(), true);
        p_onOffNode[0]->setOnOffState(m_bXYOrtho);
        p_onOffNode[1]->addChild(p_DraggableSlice[1].get(), true);
        p_onOffNode[1]->setOnOffState(m_bXZOrtho);
        p_onOffNode[2]->addChild(p_DraggableSlice[2].get(), true);
        p_onOffNode[2]->setOnOffState(m_bYZOrtho);
        p_onOffNode[3]->addChild(m_arbSlice2D.get(), true);
        p_onOffNode[3]->setOnOffState(false);
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
        p_onOffNode[3]->addChild(m_arbSlice2D.get(), true);
        p_onOffNode[3]->setOnOffState(true);
    }

    this->addChild(p_onOffNode[0].get());
    this->addChild(p_onOffNode[1].get());
    this->addChild(p_onOffNode[2].get());
    this->addChild(p_onOffNode[3].get());
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



scene::CArbitrarySliceScene::CArbitrarySliceScene(OSGOrtho2DCanvas* canvas) :
    CSceneBase(canvas)
{
    m_pImplantSlice = new osg::CArbitrarySliceGeometry();
    osg::CMaterialLineStrip* material = new osg::CMaterialLineStrip(getCanvas()->getView()->getCamera(), 4.0);
    m_pImplantSlice->setLineMaterial(material);

    this->setName("ArbScene");

    // move the scene a bit back so that everything is visible
    this->setMatrix(osg::Matrix::scale(1.0f, 1.0f, 1.0f) * osg::Matrix::translate(osg::Vec3(0, 0, SCENE_SHIFT_AMMOUNT)));
    this->m_unOrthoTransformMatrix = osg::Matrix::inverse(this->getMatrix());
    this->m_orthoTransformMatrix = this->getMatrix();

    //
    tObserverType::connect(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id).get());
    tObserverType::connect(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id).get());
    tObserverType::connect(APP_STORAGE.getEntry(data::Storage::MultiClassRegionData::Id).get());
    tObserverType::connect(APP_STORAGE.getEntry(data::Storage::DensityWindow::Id).get());

    /////////////////
    // Plane dragging 

    m_selection = new scene::CPlaneARBUpdateSelection();

    m_planeDragger = new osgManipulator::CTranslateOtherLineDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0), osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 1.0, 0.0));
    m_planeDragger->setDraggerActive(true);

    // Add plane to dragger
    m_selection->addChild(m_pImplantSlice);
    m_selection->setName("Plane selection");
    m_selection->setSignal(&(VPL_SIGNAL(SigSetSliceARB)));

    m_planeDragger->addChild(m_pImplantSlice);
    m_planeDragger->setName("Arb slice dragger");

    // Add plane dragger to scene
    addChild(m_planeDragger.get());
    addChild(m_selection.get());

    m_planeDragger->addDraggerCallback(m_selection->getDraggerCommand());

    // Get color from the storage
    data::CObjectPtr< data::CSceneWidgetParameters > ptrOptions(APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id));
    if (ptrOptions.get() == 0)
        return;

    // Get this plane color
    osg::Vec4 planeColor(1.0, 1.0, 0.0, 1.0);

    this->m_pImplantSlice->setFrameColor(planeColor);

    m_xySlice2D = new osg::COrthoSlicesVisualizer2D(m_pCanvas, data::Storage::SliceXY::Id, osg::COrthoSlicesVisualizer2D::ESceneType::EST_XY);
    m_xzSlice2D = new osg::COrthoSlicesVisualizer2D(m_pCanvas, data::Storage::SliceXZ::Id, osg::COrthoSlicesVisualizer2D::ESceneType::EST_XZ);
    m_yzSlice2D = new osg::COrthoSlicesVisualizer2D(m_pCanvas, data::Storage::SliceYZ::Id, osg::COrthoSlicesVisualizer2D::ESceneType::EST_YZ);

    p_onOffNode[0] = new osg::COnOffNode;
    p_onOffNode[1] = new osg::COnOffNode;
    p_onOffNode[2] = new osg::COnOffNode;

    p_onOffNode[0]->addChild(m_xySlice2D.get(), true);
    p_onOffNode[0]->setOnOffState(true);
    p_onOffNode[1]->addChild(m_xzSlice2D.get(), true);
    p_onOffNode[1]->setOnOffState(true);
    p_onOffNode[2]->addChild(m_yzSlice2D.get(), true);
    p_onOffNode[2]->setOnOffState(true);

    addChild(p_onOffNode[2].get());
    addChild(p_onOffNode[1].get());
    addChild(p_onOffNode[0].get());

    m_pCanvas->centerAndScale();

    m_conSliceMoved = VPL_SIGNAL(SigOrthoSliceMoved).connect(this, &CArbitrarySliceScene::orthoSliceMoved);
}

scene::CArbitrarySliceScene::~CArbitrarySliceScene()
{
    tObserverType::disconnect(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id).get());
    tObserverType::disconnect(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id).get());
    tObserverType::disconnect(APP_STORAGE.getEntry(data::Storage::MultiClassRegionData::Id).get());
    tObserverType::disconnect(APP_STORAGE.getEntry(data::Storage::DensityWindow::Id).get());

    if (m_conSliceMoved.getSignalPtr())
    {
        VPL_SIGNAL(SigOrthoSliceMoved).disconnect(m_conSliceMoved);
    }
}

osg::CArbitrarySliceGeometry* scene::CArbitrarySliceScene::getSlice() const
{
    return m_pImplantSlice.get();
}

void scene::CArbitrarySliceScene::updateFromStorage()
{
    std::set<int> changedEntries;
    getChangedEntries(changedEntries);

    if (changedEntries.empty() || changedEntries.find(data::Storage::ArbitrarySlice::Id) != changedEntries.end())
    {
        updateFromStorageArbitrarySlice();
    }

    if (changedEntries.empty() || changedEntries.find(data::Storage::ActiveDataSet::Id) != changedEntries.end())
    {
        data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()));

        // Update the scene
        this->setupScene(*spData);
    }

    m_xySlice2D->updateGeometry();
    m_xzSlice2D->updateGeometry();
    m_yzSlice2D->updateGeometry();

    m_pCanvas->Refresh(false);
}

void scene::CArbitrarySliceScene::setupScene(data::CDensityData & data)
{
    data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
    m_selection->setVoxelDepth(slice->getPositionMax() - slice->getPositionMin(), slice->getSliceVoxelSize()[2]);
    m_selection->setPlaneNormal(slice->getPlaneNormal());
    m_pCanvas->centerAndScale();
}

void scene::CArbitrarySliceScene::updateFromStorageArbitrarySlice()
{
    data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));

    osg::Vec3 normal(slice->getPlaneNormal());
    osg::Vec3 right(slice->getPlaneRight());
    osg::Vec3 center(slice->getPlaneCenter());

    m_selection->setVoxelDepth(slice->getPositionMax() - slice->getPositionMin(), slice->getSliceVoxelSize()[2]);
    m_selection->setPlaneNormal(normal);

    osg::Vec3 base_X, base_Y, base_Z;

    base_X = normal ^ right;
    base_Y = right;
    base_Z = base_Y ^ base_X;

    base_Z.normalize();
    base_Y.normalize();
    base_X.normalize();

    osg::Matrix transformation(osg::Matrix(
        base_X[0], base_X[1], base_X[2], 0,
        base_Y[0], base_Y[1], base_Y[2], 0,
        base_Z[0], base_Z[1], base_Z[2], 0,
        center[0], center[1], center[2], 1
    ));

    // Set matrices
    this->p_AnchorGroup->setMatrix(transformation);
    this->p_AnchorAndCenterGroup->setMatrix(transformation);

    m_pImplantSlice->update(*slice.get());
}

void scene::CArbitrarySliceScene::sliceUpDown(int direction)
{
    data::CObjectPtr<data::CArbitrarySlice> spSlice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));

    vpl::tSize position = spSlice->getPosition();
    vpl::tSize oldPos = position;
    position += direction;

    if (position < spSlice->getPositionMin() || position > spSlice->getPositionMax())
    {
        return;
    }

    if (oldPos != position)
    {
        VPL_SIGNAL(SigSetSliceARB).invoke(position);
    }
}

void scene::CArbitrarySliceScene::orthoSliceMoved()
{
    m_xySlice2D->updateGeometry();
    m_xzSlice2D->updateGeometry();
    m_yzSlice2D->updateGeometry();

    m_pCanvas->Refresh(false);
}