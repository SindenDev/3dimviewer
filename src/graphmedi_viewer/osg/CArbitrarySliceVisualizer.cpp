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

#include <osg/CArbitrarySliceVisualizer.h>
#include <geometry/base/types.h>
#include <geometry/base/functions.h>
#include <graph/osg/NodeMasks.h>
#include <data/CActiveDataSet.h>

#include <QDebug>

osg::CArbitrarySliceDraggerGeometry::CArbitrarySliceDraggerGeometry(EDraggerType type)
    : m_draggerType(type)
    , m_arrowSize(10.0)
{
    m_arrow1MT = new osg::MatrixTransform();
    m_arrow2MT = new osg::MatrixTransform();

    m_ring = new osg::CDonutGeometry(512);
    m_ring->update();

    m_arrow1 = new osg::CSemiCircularArrowGeometry(128);
    m_arrow1->setRadius(20.0);
    m_arrow1->setAngularLength(10.0);
    m_arrow1->setArrowLength(5.0);
    m_arrow1->setWidth(1.0);
    m_arrow1->setThickness(0.6);
    m_arrow1->update();

    m_arrow2 = new osg::CSemiCircularArrowGeometry(128);
    m_arrow2->setRadius(20.0);
    m_arrow2->setAngularLength(10.0);
    m_arrow2->setArrowLength(5.0);
    m_arrow2->setWidth(1.0);
    m_arrow2->setThickness(0.6);
    m_arrow2->update();

    m_arrow1MT->addChild(m_arrow1);
    m_arrow2MT->addChild(m_arrow2);
    addChild(m_arrow1MT);
    addChild(m_arrow2MT);
    addChild(m_ring);
}

void osg::CArbitrarySliceDraggerGeometry::resize(double size)
{
    double shift = size + size * 0.01;

    double arrowSize = 400.0 / size + size * 0.1;

    if (size * 0.1 < 400.0 / size)
    {
        m_arrowSize = arrowSize;
    }

    if (m_draggerType == EDraggerType::EDT_X)
    {
        m_arrow1MT->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Vec3(0.0, 1.0, 0.0)) * osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Vec3(1.0, 0.0, 0.0)) * osg::Matrix::translate(shift, 0.0, 0.0));
        m_arrow2MT->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 1.0, 0.0)) * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(1.0, 0.0, 0.0)) * osg::Matrix::translate(-shift, 0.0, 0.0));
    }
    else if (m_draggerType == EDraggerType::EDT_Y)
    {
        m_arrow1MT->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(1.0, 0.0, 0.0)) * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 0.0, 1.0)) * osg::Matrix::translate(shift, 0.0, 0.0));
        m_arrow2MT->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Vec3(1.0, 0.0, 0.0)) * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 0.0, 1.0))  * osg::Matrix::translate(-shift, 0.0, 0.0));
    }
    else
    {
        m_arrow1MT->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(1.0, 0.0, 0.0)) * osg::Matrix::translate(0.0, shift, 0.0));
        m_arrow2MT->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Vec3(1.0, 0.0, 0.0)) * osg::Matrix::translate(0.0, -shift, 0.0));
    }

    m_ring->setSize(shift, 0.15 + size * 0.01);
    m_ring->update();
    m_arrow1->setRadius(size);
    m_arrow1->setAngularLength(m_arrowSize);
    m_arrow1->setArrowLength(m_arrowSize * 0.4);
    m_arrow1->setWidth(1.0 + size * 0.02);
    m_arrow1->setThickness(0.6 + size * 0.01);
    m_arrow1->update();
    m_arrow2->setRadius(size);
    m_arrow2->setAngularLength(m_arrowSize);
    m_arrow2->setArrowLength(m_arrowSize * 0.4);
    m_arrow2->setWidth(1.0 + size * 0.02);
    m_arrow2->setThickness(0.6 + size * 0.01);
    m_arrow2->update();
}


osg::CArbitrarySliceVisualizer::CArbitrarySliceVisualizer(OSGCanvas *pCanvas, int id)
    : CActiveObjectBase("ArbSliceVisualizer")
    , osg::CDraggableGeometry(osg::Matrix::identity(), id)
{
    setCanvas(pCanvas);

    m_geometryCallback->setFinishSignalEnabled(true);

    m_helpGeom = new osg::MatrixTransform();
    this->addChild(m_helpGeom);

    m_sliceGeometry = new osg::CArbitrarySliceGeometry();
    osg::CMaterialLineStrip* material = new osg::CMaterialLineStrip(getCanvas()->getView()->getCamera(), 2.0);
    m_sliceGeometry->setLineMaterial(material);

    m_onOffNode = new osg::COnOffNode();
    m_onOffNode->addChild(m_sliceGeometry.get());

    m_geometrySwitch->addChild(m_onOffNode);

    // setup observer
    tObserverType::connect(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id).get());
    tObserverType::connect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id).get());
    this->setupObserver(this);
    this->dirtyBound();

    createDraggers();

    // App mode changed signal connection
    m_conAppModeChanged = APP_MODE.getModeChangedSignal().connect(this, &osg::CArbitrarySliceVisualizer::onModeChanged);

    m_conVis[0] = VPL_SIGNAL(SigSetPlaneARBVisibility).connect(m_onOffNode, &osg::COnOffNode::setOnOffState);
    m_conVis[1] = VPL_SIGNAL(SigGetPlaneARBVisibility).connect(m_onOffNode, &osg::COnOffNode::isVisible);
    m_conDrawing = VPL_SIGNAL(SigDrawingInProgress).connect(this, &osg::CArbitrarySliceVisualizer::onDrawingInProgress);

    init();
}

osg::CArbitrarySliceVisualizer::~CArbitrarySliceVisualizer()
{
    freeObserver(this);
    tObserverType::disconnect(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id).get());
    tObserverType::disconnect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id).get());

    APP_MODE.getModeChangedSignal().disconnect(m_conAppModeChanged);

    if (m_conVis[0].getSignalPtr())
        VPL_SIGNAL(SigSetPlaneARBVisibility).disconnect(m_conVis[0]);
    if (m_conVis[1].getSignalPtr())
        VPL_SIGNAL(SigGetPlaneARBVisibility).disconnect(m_conVis[1]);
    if (m_conDrawing.getSignalPtr())
        VPL_SIGNAL(SigDrawingInProgress).disconnect(m_conDrawing);
}

void osg::CArbitrarySliceVisualizer::init()
{
    m_startMatrix = osg::Matrix::identity();
    m_prevMatrix = osg::Matrix::identity();
    m_prevTrans = geometry::Vec3(0, 0, 0);
    m_drawingInProgress = false;

    updateDraggerSize();

    this->relocate(osg::Matrix::rotate(osg::DegreesToRadians(50.0), osg::Vec3(0.0, 1.0, 0.0)));

    updateDraggers();
}

void osg::CArbitrarySliceVisualizer::createDraggers()
{
    // Create draggers
    ///////////////////////// NEW DRAGGERS //////////////////////////////////////////////////////

    //translate draggers
    m_translationCompositeDragger = new osgManipulator::CDraggerBaseComposite();
    m_translationCompositeDragger->setName("translationCompositeDragger");

    m_z_translate_dragger = new osgManipulator::CDraggerTranslate(m_sliceGeometry, osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0));
    m_z_translate_dragger->setName("z_translate_dragger");

    m_translationCompositeDragger->addDragger(m_z_translate_dragger);

    //rotate draggers
    m_rotationCompositeDragger = new osgManipulator::CDraggerBaseComposite();
    m_rotationCompositeDragger->setName("rotationCompositeDragger");

    m_xRotateDraggerGeometry = new osg::CArbitrarySliceDraggerGeometry(osg::CArbitrarySliceDraggerGeometry::EDT_X);

    m_x_rotate_dragger = new osgManipulator::CDraggerRotate(m_xRotateDraggerGeometry);
    m_x_rotate_dragger->setName("x_rotate_dragger");
    m_x_rotate_dragger->setMatrix(osg::Matrix::rotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(1.0f, 0.0f, 0.0f)));

    m_yRotateDraggerGeometry = new osg::CArbitrarySliceDraggerGeometry(osg::CArbitrarySliceDraggerGeometry::EDT_Y);

    m_y_rotate_dragger = new osgManipulator::CDraggerRotate(m_yRotateDraggerGeometry);
    m_y_rotate_dragger->setName("y_rotate_dragger");
    m_y_rotate_dragger->setMatrix(osg::Matrix::rotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, 1.0f, 0.0f)));

    m_zRotateDraggerGeometry = new osg::CArbitrarySliceDraggerGeometry(osg::CArbitrarySliceDraggerGeometry::EDT_Z);

    m_z_rotate_dragger = new osgManipulator::CDraggerRotate(m_zRotateDraggerGeometry);
    m_z_rotate_dragger->setName("z_rotate_dragger");
    m_z_rotate_dragger->setMatrix(osg::Matrix::rotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, 0.0f, 1.0f)));

    m_rotationCompositeDragger->addDragger(m_x_rotate_dragger);
    m_rotationCompositeDragger->addDragger(m_y_rotate_dragger);
    m_rotationCompositeDragger->addDragger(m_z_rotate_dragger);

    //scale draggers
    m_scaleCompositeDragger = new osgManipulator::CDraggerBaseComposite();
    m_scaleCompositeDragger->setName("scaleCompositeDragger");

    osg::Vec3 scaleLimit(0.1, 0.1, 0.1);

    m_x_scale_dragger = new osgManipulator::CDraggerScale(osgManipulator::CModifiedScaleCommand::SCALE_X, osg::Vec3(1.0, 0.0, 0.0));
    m_x_scale_dragger->setName("x_scale_dragger");
    m_x_scale_dragger->setMinScale(scaleLimit[0]);
    m_x_scale_dragger->enableInverseScaling(false);

    m_y_scale_dragger = new osgManipulator::CDraggerScale(osgManipulator::CModifiedScaleCommand::SCALE_Y, osg::Vec3(0.0, 1.0, 0.0));
    m_y_scale_dragger->setName("y_scale_dragger");
    m_y_scale_dragger->setMinScale(scaleLimit[1]);
    m_y_scale_dragger->enableInverseScaling(false);

    m_scaleCompositeDragger->addDragger(m_x_scale_dragger);
    m_scaleCompositeDragger->addDragger(m_y_scale_dragger);

    //plane dragger
    m_plane_dragger = new osgManipulator::CDraggerPlane(m_sliceGeometry);

    //final composite dragger
    m_3DCompositeDragger = new osgManipulator::CDraggerBaseComposite();
    m_3DCompositeDragger->setName("CArbSliceVisualizer");

    m_3DCompositeDragger->addDragger(m_scaleCompositeDragger);
    m_3DCompositeDragger->addDragger(m_rotationCompositeDragger);
    m_3DCompositeDragger->addDragger(m_translationCompositeDragger);
    m_3DCompositeDragger->addDragger(m_plane_dragger);

    CDraggableGeometry::addDragger(m_3DCompositeDragger);

    prepareDraggersMaterials();

    showDraggers(false);

    // Set geometry node mask
    setNodeMask(MASK_VISIBLE_OBJECT);

    getSignal().connect(this, &CArbitrarySliceVisualizer::sigCommandFromDG);

    updateDraggerSize();
}

void osg::CArbitrarySliceVisualizer::updateDraggerSize()
{
    data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));

    double sliceWidth = slice->getWidth();
    double sliceHeight = slice->getHeight();
    osg::Vec3 voxelSize = slice->getSliceVoxelSize();

    // Set rotation dragger size
    double size = std::max(sliceWidth * voxelSize[0], sliceHeight * voxelSize[1]) * 0.5 + 5.0;

    double shift = size + size * 0.01;

    m_xRotateDraggerGeometry->resize(size);
    m_yRotateDraggerGeometry->resize(size);
    m_zRotateDraggerGeometry->resize(size);

    double scaleDraggerOffset = 4.5 + size * 0.03;

    osg::CDraggerScaleGeometry* xDraggerScaleGeometry = dynamic_cast<osg::CDraggerScaleGeometry*>(m_x_scale_dragger->getGeometry());
    osg::CDraggerScaleGeometry* yDraggerScaleGeometry = dynamic_cast<osg::CDraggerScaleGeometry*>(m_y_scale_dragger->getGeometry());

    xDraggerScaleGeometry->setOffsets(osg::Vec3(shift + scaleDraggerOffset, 0, 0), osg::Vec3(-shift - scaleDraggerOffset, 0, 0));
    yDraggerScaleGeometry->setOffsets(osg::Vec3(0, shift + scaleDraggerOffset, 0), osg::Vec3(0, -shift - scaleDraggerOffset, 0));

    xDraggerScaleGeometry->scale(1.0 + size * 0.02);
    yDraggerScaleGeometry->scale(1.0 + size * 0.02);
}

void osg::CArbitrarySliceVisualizer::updateDraggers()
{
    data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));

    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));

    vpl::tSize xSize = spVolume->getXSize();
    vpl::tSize ySize = spVolume->getYSize();
    vpl::tSize zSize = spVolume->getZSize();

    double dx = spVolume->getDX();
    double dy = spVolume->getDY();
    double dz = spVolume->getDZ();

    osg::Vec3 volumeShift(-0.5 * xSize * dx,
        -0.5 * ySize * dy,
        -0.5 * zSize * dz);

    osg::Vec3 normal = slice->getPlaneNormal();

    osg::Matrix m = slice->getRotationMatrix();

    this->relocate(osg::Matrix::identity() * m * osg::Matrix::translate(slice->getPlaneCenter()/* + (normal * (slice->getSliceVoxelSize()[2] * 0.5))*/) * osg::Matrix::translate(volumeShift));

    m_pCanvas->Refresh(false);
}

bool osg::CArbitrarySliceVisualizer::sigCommandFromDG(const osg::Matrix & matrix, const osgManipulator::MotionCommand &command, int command_type, long dg_id)
{
    data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));

    if (command.getStage() == osgManipulator::MotionCommand::START)
    {
        m_startMatrix = matrix;
        m_prevTrans = geometry::Vec3(0, 0, 0);
        m_sliceWidth = slice->getSliceWidth();
        m_sliceHeight = slice->getSliceHeight();
        return true;
    }

    if (command.getStage() == osgManipulator::MotionCommand::FINISH)
    {
        updateDraggers();
        return true;
    }

    if (command.getStage() != osgManipulator::MotionCommand::MOVE)
    {
        return true;
    }

    if (command_type == osgManipulator::DraggerTransformCallback::HANDLE_ROTATE_3D)
    {
        m_prevMatrix = matrix;

        slice->setRotationMatrix(osg::Matrix::rotate(this->getGeometryPositionMatrix().getRotate()));
        APP_STORAGE.invalidate(slice.getEntryPtr());
    }
    else if (command_type == osgManipulator::DraggerTransformCallback::HANDLE_TRANSLATE_IN_LINE)
    {
        geometry::Quat rotation;
        geometry::Vec3 translation, scale;

        geometry::Matrix diffMatrix = geometry::convert4x4T<geometry::Matrix, osg::Matrix>(m_startMatrix * osg::Matrix::inverse(matrix));

        diffMatrix.decompose(translation, rotation, scale);

        geometry::Vec3 newTrans = translation - m_prevTrans;

        double pos = slice->getPosition();
        osg::Vec3 oldCenter = slice->getPlaneCenter();
        osg::Vec3 newCenter = oldCenter - geometry::convert3<osg::Vec3, geometry::Vec3>(newTrans);
        double shift = (oldCenter - newCenter).length() / slice->getSliceVoxelSize()[2];

        if (shift < 0.001)
        {
            this->relocate(m_prevMatrix);
            updateDraggers();
            return true;
        }

        double newPosition = pos;

        if (newTrans[2] < 0)
        {
            newPosition = pos + shift;
        }
        else
        {
            newPosition = pos - shift;
        }

        if (newPosition < slice->getPositionMin())
        {
            slice->setPosition(slice->getPositionMin());
            this->relocate(m_prevMatrix);
            updateDraggers();
            APP_STORAGE.invalidate(slice.getEntryPtr());
            return true;
        }

        if (newPosition > slice->getPositionMax())
        {
            slice->setPosition(slice->getPositionMax());
            this->relocate(m_prevMatrix);
            updateDraggers();
            APP_STORAGE.invalidate(slice.getEntryPtr());
            return true;
        }

        data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));

        vpl::tSize xSize = spVolume->getXSize();
        vpl::tSize ySize = spVolume->getYSize();
        vpl::tSize zSize = spVolume->getZSize();

        double dx = spVolume->getDX();
        double dy = spVolume->getDY();
        double dz = spVolume->getDZ();

        osg::Vec3 volumeShift(-0.5 * xSize * dx,
            -0.5 * ySize * dy,
            -0.5 * zSize * dz);

        m_prevTrans = translation;
        m_prevMatrix = matrix;

        m_prevMatrix.setTrans(slice->getPlaneCenter() + volumeShift);

        slice->setPosition(newPosition);
        APP_STORAGE.invalidate(slice.getEntryPtr());
    }
    else if (command_type == osgManipulator::DraggerTransformCallback::HANDLE_SCALED_1D)
    {
        geometry::Quat rotation;
        geometry::Vec3 translation, scale;

        geometry::Matrix diffMatrix = geometry::convert4x4T<geometry::Matrix, osg::Matrix>(osg::Matrix::inverse(m_startMatrix) * matrix);

        diffMatrix.decompose(translation, rotation, scale);

        slice->setSliceWidth(m_sliceWidth * scale[0]);
        slice->setSliceHeight(m_sliceHeight * scale[1]);
        APP_STORAGE.invalidate(slice.getEntryPtr());

        m_geometryScaleTransform->setMatrix(osg::Matrix::identity());

        updateDraggerSize();
    }

    updateDraggers();
    
    return true;
}

void osg::CArbitrarySliceVisualizer::prepareDraggersMaterials()
{
    osg::Vec3 colorPicked(1.0f, 1.0f, 0.6f);
    osg::Vec3 colorX(1.0f, 0.0f, 0.0f);
    osg::Vec3 colorY(0.0f, 1.0f, 0.0f);
    osg::Vec3 colorZ(1.0f, 1.0f, 0.0f);

    /*osg::Vec3 colorPicked(0.6f, 1.0f, 0.6f);
    osg::Vec3 colorX(1.0f, 0.5f, 0.0f);
    osg::Vec3 colorY(1.0f, 0.5f, 0.0f);
    osg::Vec3 colorZ(1.0f, 0.5f, 0.0f);*/

    //if (m_x_translate_dragger)
    //    m_x_translate_dragger->setColors(colorX, colorPicked);
    if (m_x_scale_dragger)
        m_x_scale_dragger->setColors(colorX, colorPicked);
    if (m_x_rotate_dragger)
        m_x_rotate_dragger->setColors(colorX, colorPicked);

    //if (m_y_translate_dragger)
    //    m_y_translate_dragger->setColors(colorY, colorPicked);
    if (m_y_scale_dragger)
        m_y_scale_dragger->setColors(colorY, colorPicked);
    if (m_y_rotate_dragger)
        m_y_rotate_dragger->setColors(colorY, colorPicked);

    if (m_z_translate_dragger)
        m_z_translate_dragger->setColors(colorZ, colorPicked);
    //if (m_z_scale_dragger)
    //    m_z_scale_dragger->setColors(colorZ, colorPicked);
    if (m_z_rotate_dragger)
        m_z_rotate_dragger->setColors(colorZ, colorPicked);

    //if (m_plane_dragger)
    //    m_plane_dragger->setColor(osg::SECOND, colorPicked);
}

//=====================================================================================================================
void osg::CArbitrarySliceVisualizer::updateFromStorage()
{
    std::set<int> changedEntries;
    getChangedEntries(changedEntries);

    if (changedEntries.find(data::Storage::ActiveDataSet::Id) != changedEntries.end())
    {
        onNewDensityData();
    }

    if (changedEntries.find(data::Storage::ArbitrarySlice::Id) != changedEntries.end() && changedEntries.find(data::Storage::MultiClassRegionData::Id) == changedEntries.end())
    {
        onArbSliceChanged();
    }

    // get the slice from the storage
    data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id, data::Storage::NO_UPDATE));

    m_sliceGeometry->update(*slice.get());
}

void osg::CArbitrarySliceVisualizer::onNewDensityData()
{
    data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
    slice->init();
    APP_STORAGE.invalidate(slice.getEntryPtr());

    init();
}

void osg::CArbitrarySliceVisualizer::onArbSliceChanged()
{
    updateDraggers();
}

void osg::CArbitrarySliceVisualizer::onModeChanged(scene::CAppMode::tMode mode)
{
    if (APP_MODE.check(scene::CAppMode::MODE_SLICE_MOVE) && m_onOffNode->isVisible())
    {
        showDraggers(true);
    }
    else
    {
        showDraggers(false);
    }

    this->dirtyBound();
}

void osg::CArbitrarySliceVisualizer::onDrawingInProgress(bool drawing)
{
    m_drawingInProgress = drawing;
}