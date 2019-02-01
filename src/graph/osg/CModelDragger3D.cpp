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

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osg/CModelDragger3D.h>
#include <osg/CFreeModelVisualizer.h>

#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Quat>
#include <osg/AutoTransform>
#include <osgManipulator/AntiSquish>
#include <osg/CForceCullCallback.h>
#include <osg/osgcompat.h>
#include <osg/DraggerColorDefines.h>
#include <app/Signals.h>
#include <data/CPivot.h>

using namespace osgManipulator;

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
osgManipulator::CModel3DRotationDragger::CModel3DRotationDragger()
{
    m_rxDragger = new CCylinderDragger();
    addChild(m_rxDragger.get());
    addDragger(m_rxDragger.get());

    m_ryDragger = new CCylinderDragger();
    addChild(m_ryDragger.get());
    addDragger(m_ryDragger.get());

    m_rzDragger = new CCylinderDragger();
    addChild(m_rzDragger.get());
    addDragger(m_rzDragger.get());

    setParentDragger(getParentDragger());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets up the default geometry.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DRotationDragger::setupDefaultGeometry()
{
    // Create rings
    m_ring = new osg::CDonutGeometry(64, 5);
    m_invisibleRing = new osg::CDonutGeometry(32, 5);

    m_ring->setSize(15.0f, 2.0f);
    m_invisibleRing->setSize(15.0f, 2.0f);

    m_ring->update();
    m_invisibleRing->update();

    osg::ref_ptr< osg::Group > group(new osg::Group);

    group->addChild(m_ring);
    group->addChild(m_invisibleRing);

    setDrawableToAlwaysCull(*(m_invisibleRing->getGeometry()));

    // Add rings
    m_rxDragger->addChild(group);
    m_ryDragger->addChild(group);
    m_rzDragger->addChild(group);

    // Rotate X-axis dragger appropriately.
    {
        osg::Quat rotation;
        rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(1.0f, 0.0f, 0.0f));
        m_rxDragger->setMatrix(osg::Matrix(rotation));
    }

    // Rotate Y-axis dragger appropriately.
    {
        osg::Quat rotation;
        rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, -1.0f, 0.0f));
        m_ryDragger->setMatrix(osg::Matrix(rotation));
    }

    // Send different colors for each dragger.
    m_rxDragger->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
    m_ryDragger->setColor(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
    m_rzDragger->setColor(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets the radius.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DRotationDragger::setRadius(float r1, float r2)
{
    auto radius1 = 0.5f * (r1 + r2);
    auto radius2 = (r2 - r1);

    m_ring->setSize(radius1, 0.4f * radius2);
    m_invisibleRing->setSize(radius1, radius2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Updates a geometry.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DRotationDragger::updateGeometry()
{
    m_ring->update();
    m_invisibleRing->update();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
osgManipulator::CModel3DTranslationDragger::CModel3DTranslationDragger()
{ }

osgManipulator::CModel3DTranslationDragger::CModel3DTranslationDragger(const osg::Vec3& s, const osg::Vec3& e)
    : CTranslate1DDragger(s, e)
{ }

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets up the default geometry.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DTranslationDragger::setupDefaultGeometry()
{
    // Create arrows geometry
    m_arrow = new osg::CArrow3DGeometry(32);

    // Set arrows parameters
    m_arrow->setSize(1.0, 2.0, 2.0, 2.0);

    // Create arrows geometry
    updateGeometry();

    // Create shifting matrix transforms
    osg::Vec3 end = _projector->getLineEnd();
    osg::Quat q1; q1.makeRotate(osg::Vec3(0.0, 0.0, 1.0), end);
    osg::Quat q2; q2.makeRotate(osg::Vec3(0.0, 0.0, 1.0), -end);
    m_shiftHead = new osg::MatrixTransform(osg::Matrix::rotate(q1));
    m_shiftTail = new osg::MatrixTransform(osg::Matrix::rotate(q2));
    m_scaleHead = new osg::MatrixTransform();
    m_scaleTail = new osg::MatrixTransform();

    // Build tree
    m_scaleHead->addChild(m_arrow);
    m_scaleTail->addChild(m_arrow);
    m_shiftHead->addChild(m_scaleHead);
    m_shiftTail->addChild(m_scaleTail);
    this->addChild(m_shiftHead);
    this->addChild(m_shiftTail);

    //this->_selfUpdater = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets an offset.
//!
//!\param	offset_head	The offset head.
//!\param	offset_tail	The offset tail.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DTranslationDragger::setOffset(float offset_head, float offset_tail)
{
    osg::Vec3 end = _projector->getLineEnd();

    osg::Matrix mh(m_shiftHead->getMatrix());
    mh.setTrans(end*offset_head);
    m_shiftHead->setMatrix(mh);

    osg::Matrix m(m_shiftTail->getMatrix());
    m.setTrans(end*(-offset_tail));
    m_shiftTail->setMatrix(m);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets scale
//!
//!\param	scale           The scale
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DTranslationDragger::setScale(float scale)
{
    osg::Vec3 end = _projector->getLineEnd();

    osg::Matrix mh(m_scaleHead->getMatrix());
    mh.makeScale(scale, scale, scale);
    m_scaleHead->setMatrix(mh);

    osg::Matrix m(m_scaleTail->getMatrix());
    m.makeScale(scale, scale, scale);
    m_scaleTail->setMatrix(m);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Updates a geometry.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DTranslationDragger::updateGeometry()
{
    m_arrow->update();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Default constructor.
////////////////////////////////////////////////////////////////////////////////////////////////////
osgManipulator::CModel3DViewPlaneDragger::CModel3DViewPlaneDragger(geometry::CMesh *pMesh, osg::Matrix modelPos)
    : CTranslate2DDragger(osg::Plane(0.0, 0.0, 1.0, 0.0))
    , m_pMesh(pMesh)
    , m_pVisualizer(nullptr)
{
    // Create shifting matrix transforms
    m_shift = new osg::MatrixTransform(modelPos);

    setParentDragger(getParentDragger());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets up the default geometry.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DViewPlaneDragger::setupDefaultGeometry()
{
    this->addChild(m_shift);

    if (m_pMesh && m_pMesh->n_vertices() > 0)
    {
        osg::CTriMesh* pTriMesh = new osg::CTriMesh;
        m_shift->addChild(pTriMesh);
        pTriMesh->createMesh(*m_pMesh, std::map<std::string, vpl::img::CRGBAImage::tSmartPtr>(), true);
        pTriMesh->setColor(osg::Vec4(0, 1, 0, 1));

        UniDrawableList list = getGeodeDrawableList(pTriMesh->getMeshGeode());
        for (UniDrawableList::iterator it = list.begin(); it != list.end(); ++it)
        {
            osgManipulator::setDrawableToAlwaysCull(*(*it).get());
        }
    }

    m_sphere = new osg::CSphereGeometry(32);
    m_sphere->setRadius(2);
    m_sphere->update();

    addChild(m_sphere);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DViewPlaneDragger::setRadius(float radius)
{
    if (m_sphere.get())
        m_sphere->setRadius(radius);
}

void osgManipulator::CModel3DViewPlaneDragger::updateGeometry()
{
    if (m_sphere.get())
        m_sphere->update();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Revert transforms on plane.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DViewPlaneDragger::revertTransformsOnPlane()
{
    osg::Vec3d translation;
    osg::Quat rotation;
    osg::Vec3d scale;
    osg::Quat so;

    m_view_matrix.decompose(translation, rotation, scale, so);

    osg::Matrix m(osg::Matrix::rotate(rotation.inverse()));

    osg::Plane plane(0.0, 0.0, -1.0, 0.0);
    plane.transform(m);

    _projector->setPlane(plane);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Accepts.
//!
//!\param [in,out]	nv	The nv.
////////////////////////////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DViewPlaneDragger::accept(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        osg::CullStack* cs = dynamic_cast<osg::CullStack*>(&nv);
        if (cs)
        {
            osg::Vec3d translation;
            osg::Quat rotation;
            osg::Vec3d scale;
            osg::Quat so;

            m_view_matrix = osg::Matrix::inverse(osg::Matrix::rotate(getMatrix().getRotate())) * *cs->getModelViewMatrix();
            nv.getNodePath();
            m_nodepath_matrix = osg::computeLocalToWorld(nv.getNodePath());
        }
    }

    CTranslate2DDragger::accept(nv);
}

///////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DViewPlaneDragger::updateModelMatrix(const osg::Matrix &modelPos)
{
    m_shift->setMatrix(modelPos);
}

///////////////////////////////////////////////////////////////////////////////
void osgManipulator::CModel3DViewPlaneDragger::updateModel(int storageID)
{
    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(storageID));
    geometry::CMesh* pMesh = spModel->getMesh(false);
    if (m_pMesh == pMesh) // same mesh - ignore this change
    {
        return;
    }
    // remove existing child
    if (m_shift->getNumChildren()>0)
        m_shift->removeChild(0, 1);
    // create a new one
    if (pMesh && pMesh->n_vertices() > 0)
    {
        osg::CTriMesh* pTriMesh = new osg::CTriMesh;
        pTriMesh->createMesh(*pMesh, std::map<std::string, vpl::img::CRGBAImage::tSmartPtr>(), true);
        pTriMesh->setColor(osg::Vec4(0, 1, 0, 1));

        // make invisible
        UniDrawableList list = getGeodeDrawableList(pTriMesh->getMeshGeode());
        for (UniDrawableList::iterator it = list.begin(); it != list.end(); ++it)
        {
            osgManipulator::setDrawableToAlwaysCull(*(*it).get());
        }
        // add
        m_shift->addChild(pTriMesh);
        m_pMesh = pMesh;
    }
}

void osgManipulator::CModel3DViewPlaneDragger::onMouseEnter()
{
    applyMaterial(osg::SECOND);
}

void osgManipulator::CModel3DViewPlaneDragger::onMouseLeave()
{
    applyMaterial(osg::FIRST);
}

///////////////////////////////////////////////////////////////////////////////
osg::CPivotDraggerHolder::CPivotDraggerHolder() : CActiveObjectBase("CPivotDraggerHolder", 6)
{
    // Create translation draggers
    m_translationXDragger = new osgManipulator::CModel3DTranslationDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(1.0, 0.0, 0.0));
    m_translationXDragger->setupDefaultGeometry();
    m_translationYDragger = new osgManipulator::CModel3DTranslationDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 1.0, 0.0));
    m_translationYDragger->setupDefaultGeometry();
    m_translationZDragger = new osgManipulator::CModel3DTranslationDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0));
    m_translationZDragger->setupDefaultGeometry();

    // update size of axial draggers
    setOffset(1.0);

    // Add draggers
    this->addDragger(m_translationXDragger);
    this->addDragger(m_translationYDragger);
    this->addDragger(m_translationZDragger);
    this->addChild(m_translationXDragger);
    this->addChild(m_translationYDragger);
    this->addChild(m_translationZDragger);

    prepareMaterials();
    setParentDragger(getParentDragger());
    setNodeMask(2);
    setName("CPivotDraggerHolder");

    CGeneralObjectObserver<CPivotDraggerHolder>::connect(APP_STORAGE.getEntry(data::Storage::ModelPivot::Id).get(), CGeneralObjectObserver<CPivotDraggerHolder>::tObserverHandler(this, &CPivotDraggerHolder::sigPivotChanged));
}

osg::CPivotDraggerHolder::~CPivotDraggerHolder()
{
    CGeneralObjectObserver<CPivotDraggerHolder>::disconnect(APP_STORAGE.getEntry(data::Storage::ModelPivot::Id).get());
}

void osg::CPivotDraggerHolder::setOffset(float offset)
{
    m_translationXDragger->setOffset(offset, offset);
    m_translationYDragger->setOffset(offset, offset);
    m_translationZDragger->setOffset(offset, offset);
    m_translationXDragger->updateGeometry();
    m_translationYDragger->updateGeometry();
    m_translationZDragger->updateGeometry();
}

void osg::CPivotDraggerHolder::setScale(float scale)
{
    m_translationXDragger->setScale(scale);
    m_translationYDragger->setScale(scale);
    m_translationZDragger->setScale(scale);
    m_translationXDragger->updateGeometry();
    m_translationYDragger->updateGeometry();
    m_translationZDragger->updateGeometry();
}

//! Prepare materials
void osg::CPivotDraggerHolder::prepareMaterials()
{
    m_translationXDragger->setDiffuse(osg::FIRST, COLOR_X_TRANSLATE);
    m_translationXDragger->setDiffuse(osg::SECOND, COLOR_PICKED);
    m_translationYDragger->setDiffuse(osg::FIRST, COLOR_Y_TRANSLATE);
    m_translationYDragger->setDiffuse(osg::SECOND, COLOR_PICKED);
    m_translationZDragger->setDiffuse(osg::FIRST, COLOR_Z_TRANSLATE);
    m_translationZDragger->setDiffuse(osg::SECOND, COLOR_PICKED);
}

bool osg::CPivotDraggerHolder::handle(const osgManipulator::PointerInfo &pi, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    bool retVal = osgManipulator::CompositeDragger::handle(pi, ea, aa);

    osg::Vec3 position = getMatrix().getTrans();
    data::CObjectPtr<data::CPivot> spPivot(APP_STORAGE.getEntry(data::Storage::ModelPivot::Id));
    spPivot->setPosition(vpl::img::CVector3d(position.x(), position.y(), position.z()));
    APP_STORAGE.invalidate(spPivot.getEntryPtr());

    if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
    {
        VPL_SIGNAL(SigUndoSnapshot).invoke(spPivot->getSnapshot(NULL));
    }

    return retVal;
}

void osg::CPivotDraggerHolder::objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{ }

void osg::CPivotDraggerHolder::sigPivotChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{
    data::CPivot *pData = pEntry->getDataPtr<data::CObjectHolder<data::CPivot> >()->getObjectPtr();
    vpl::img::CVector3d position = pData->getPosition();
    osg::Matrix matrix = getMatrix();
    matrix.setTrans(osg::Vec3(position.x(), position.y(), position.z()));
    setMatrix(matrix);
}

void osg::CPivotDraggerHolder::onMouseEnter(const osgGA::GUIEventAdapter& ea, bool command_mode)
{
    if (command_mode)
        return;

    APP_MODE.storeAndSet(scene::CAppMode::MODE_HIGHLIGHT);
    APP_MODE.enableHighlight(false);
}

void osg::CPivotDraggerHolder::onMouseExit(const osgGA::GUIEventAdapter& ea, bool command_mode)
{
    if (command_mode)
        return;

    APP_MODE.restore();
}

///////////////////////////////////////////////////////////////////////////////
osg::CModelDraggerHolder::CModelDraggerHolder(int idModel, bool signalDraggerMove, bool signalDraggerMoveSetMatrix)
    : m_idModel(idModel)
    , m_bSignalDraggerMove(signalDraggerMove)
    , m_bSignalDraggerMoveSetMatrix(signalDraggerMoveSetMatrix)
{
    CGeneralObjectObserver<CModelDraggerHolder>::connect(APP_STORAGE.getEntry(data::Storage::ModelPivot::Id).get(), CGeneralObjectObserver<CModelDraggerHolder>::tObserverHandler(this, &CModelDraggerHolder::sigPivotChanged));
    m_bSetPivot = false;
}

osg::CModelDraggerHolder::~CModelDraggerHolder()
{
    if (m_idModel != -1)
    {
        CGeneralObjectObserver<CModelDraggerHolder>::disconnect(APP_STORAGE.getEntry(m_idModel).get());
    }
    CGeneralObjectObserver<CModelDraggerHolder>::disconnect(APP_STORAGE.getEntry(data::Storage::ModelPivot::Id).get());
}

void osg::CModelDraggerHolder::objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{ }

void osg::CModelDraggerHolder::sigPivotChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{
    internalPivotChanged(pEntry, changes);
}

void osg::CModelDraggerHolder::sigModelChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{
    internalModelChanged(pEntry, changes);
}

void osg::CModelDraggerHolder::internalPivotChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{ }

void osg::CModelDraggerHolder::internalModelChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{ }

void osg::CModelDraggerHolder::setModelID(int id, bool setPivot)
{
    if (id != m_idModel)
    {
        // reconnect signals
        if (m_idModel != -1)
        {
            CGeneralObjectObserver<CModelDraggerHolder>::disconnect(APP_STORAGE.getEntry(m_idModel).get());
        }
        m_idModel = id;
        if (m_idModel != -1)
        {
            CGeneralObjectObserver<CModelDraggerHolder>::connect(APP_STORAGE.getEntry(m_idModel).get(), CGeneralObjectObserver<CModelDraggerHolder>::tObserverHandler(this, &CModelDraggerHolder::sigModelChanged));
        }
    }

    // update dragger
    m_bSetPivot = setPivot;
    setMatrix(osg::Matrix::identity());
    sigModelChanged(APP_STORAGE.getEntry(m_idModel).get(), data::CChangedEntries());
}

bool osg::CModelDraggerHolder::handle(const osgManipulator::PointerInfo &pi, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    bool retVal = false;

    if (m_bSignalDraggerMove)
    {
        retVal = osgManipulator::CompositeDragger::handle(pi, ea, aa);

        switch (ea.getEventType())
        {
        case osgGA::GUIEventAdapter::PUSH:
            VPL_SIGNAL(SigDragerMoveForCollisions).invoke(scene::CAppMode::PUSH, getModelID());
            VPL_SIGNAL(SigAlignmentDraggerMove).invoke(scene::CAppMode::PUSH, m_bSignalDraggerMoveSetMatrix);
            break;

        case osgGA::GUIEventAdapter::RELEASE:
            VPL_SIGNAL(SigDragerMoveForCollisions).invoke(scene::CAppMode::RELEASE, getModelID());
            VPL_SIGNAL(SigAlignmentDraggerMove).invoke(scene::CAppMode::RELEASE, m_bSignalDraggerMoveSetMatrix);
            break;

        case osgGA::GUIEventAdapter::DRAG:
            VPL_SIGNAL(SigDragerMoveForCollisions).invoke(scene::CAppMode::DRAG, getModelID());
            VPL_SIGNAL(SigAlignmentDraggerMove).invoke(scene::CAppMode::DRAG, m_bSignalDraggerMoveSetMatrix);
            break;
        }
    }
    else
    {
        retVal = osgManipulator::CompositeDragger::handle(pi, ea, aa);
    }

    return retVal;
}

///////////////////////////////////////////////////////////////////////////////
osg::C3DModelDraggerHolder::C3DModelDraggerHolder(osg::CFreeModelVisualizer* pVisualizer, const Matrix &modelPos, const Vec3 &sceneSize, bool signalDraggerMove, bool signalDraggerMoveSetMatrix)
    : CModelDraggerHolder(-1, signalDraggerMove, signalDraggerMoveSetMatrix), CActiveObjectBase("3DModelDraggerHolder", 6)
{
    assert(NULL != pVisualizer);

    // Create rotation dragger
    m_rotation_dragger = new osgManipulator::CModel3DRotationDragger();
    m_rotation_dragger->setupDefaultGeometry();
    double size = (sceneSize.x() + sceneSize.y() + sceneSize.z()) / 3;
    m_rotation_dragger->setRadius(0.95*size / 5, size / 5);
    m_rotation_dragger->updateGeometry();

    // Create axial translation draggers
    m_x_translate_dragger = new osgManipulator::CModel3DTranslationDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(1.0, 0.0, 0.0));
    m_x_translate_dragger->setupDefaultGeometry();
    m_y_translate_dragger = new osgManipulator::CModel3DTranslationDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 1.0, 0.0));
    m_y_translate_dragger->setupDefaultGeometry();
    m_z_translate_dragger = new osgManipulator::CModel3DTranslationDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0));
    m_z_translate_dragger->setupDefaultGeometry();

    // update size of axial draggers
    double axialOffset = 1.10 * size / 5;
    m_x_translate_dragger->setOffset(axialOffset, axialOffset);
    m_y_translate_dragger->setOffset(axialOffset, axialOffset);
    m_z_translate_dragger->setOffset(axialOffset, axialOffset);
    m_x_translate_dragger->updateGeometry();
    m_y_translate_dragger->updateGeometry();
    m_z_translate_dragger->updateGeometry();

    // Create translation dragger
    m_translate_dragger = new osgManipulator::CModel3DViewPlaneDragger(pVisualizer->getMesh(), modelPos);
    m_translate_dragger->setupDefaultGeometry();

    m_pivot_dragger_offset = 1.25 * axialOffset;
    m_pivot_dragger_scale = 0.5;

    // Add draggers
    this->addDragger(m_x_translate_dragger);
    this->addDragger(m_y_translate_dragger);
    this->addDragger(m_z_translate_dragger);
    this->addDragger(m_translate_dragger);
    this->addDragger(m_rotation_dragger);
    this->addChild(m_x_translate_dragger);
    this->addChild(m_y_translate_dragger);
    this->addChild(m_z_translate_dragger);
    this->addChild(m_translate_dragger);
    this->addChild(m_rotation_dragger);

    prepareMaterials();
    setParentDragger(getParentDragger());
    setNodeMask(2);
    setName("C3DModelDraggerHolder");

    m_pivot_dragger = NULL;
}

osg::C3DModelDraggerHolder::C3DModelDraggerHolder(int modelID, const Matrix &modelPos, const Vec3 &sceneSize, bool signalDraggerMove, bool signalDraggerMoveSetMatrix)
    : CModelDraggerHolder(modelID, signalDraggerMove, signalDraggerMoveSetMatrix), CActiveObjectBase("3DModelDraggerHolder", 6)
{
    // Create rotation dragger
    m_rotation_dragger = new osgManipulator::CModel3DRotationDragger();
    m_rotation_dragger->setupDefaultGeometry();
    double size = (sceneSize.x() + sceneSize.y() + sceneSize.z()) / 3;
    m_rotation_dragger->setRadius(0.9*size / 3, size / 3);
    m_rotation_dragger->updateGeometry();

    // Create axial translation draggers
    m_x_translate_dragger = new osgManipulator::CModel3DTranslationDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(1.0, 0.0, 0.0));
    m_x_translate_dragger->setupDefaultGeometry();
    m_y_translate_dragger = new osgManipulator::CModel3DTranslationDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 1.0, 0.0));
    m_y_translate_dragger->setupDefaultGeometry();
    m_z_translate_dragger = new osgManipulator::CModel3DTranslationDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0));
    m_z_translate_dragger->setupDefaultGeometry();

    // update size of axial draggers
    double axialOffset = 1.10 * size / 3;
    m_x_translate_dragger->setOffset(axialOffset, axialOffset);
    m_y_translate_dragger->setOffset(axialOffset, axialOffset);
    m_z_translate_dragger->setOffset(axialOffset, axialOffset);
    m_x_translate_dragger->updateGeometry();
    m_y_translate_dragger->updateGeometry();
    m_z_translate_dragger->updateGeometry();

    // Create translation dragger
    m_translate_dragger = new osgManipulator::CModel3DViewPlaneDragger(NULL, modelPos);
    m_translate_dragger->setupDefaultGeometry();

    m_pivot_dragger_offset = 1.25 * axialOffset;
    m_pivot_dragger_scale = 0.5;

    // Add draggers
    this->addDragger(m_x_translate_dragger);
    this->addDragger(m_y_translate_dragger);
    this->addDragger(m_z_translate_dragger);
    this->addDragger(m_translate_dragger);
    this->addDragger(m_rotation_dragger);
    this->addChild(m_x_translate_dragger);
    this->addChild(m_y_translate_dragger);
    this->addChild(m_z_translate_dragger);
    this->addChild(m_translate_dragger);
    this->addChild(m_rotation_dragger);

    prepareMaterials();
    setParentDragger(getParentDragger());
    setNodeMask(2);
    setName("C3DModelDraggerHolder");

    m_pivot_dragger = NULL;
}

osg::C3DModelDraggerHolder::~C3DModelDraggerHolder()
{ }

bool osg::C3DModelDraggerHolder::handle(const osgManipulator::PointerInfo &pi, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    bool retVal = CModelDraggerHolder::handle(pi, ea, aa);

    osg::Vec3 position = getMatrix().getTrans();
    data::CObjectPtr<data::CPivot> spPivot(APP_STORAGE.getEntry(data::Storage::ModelPivot::Id));
    vpl::img::CVector3d prevPivot = spPivot->getPosition();
    vpl::img::CVector3d currPivot = vpl::img::CVector3d(position.x(), position.y(), position.z());
    vpl::img::CVector3d vec = prevPivot - currPivot;
    if (vec.getLength() > 0.001)
    {
        spPivot->setPosition(currPivot);
        APP_STORAGE.invalidate(spPivot.getEntryPtr());
    }

    updatePivotDraggerMatrix();

    return retVal;
}

void osg::C3DModelDraggerHolder::onMouseEnter(const osgGA::GUIEventAdapter& ea, bool command_mode)
{
    if (command_mode)
        return;

    APP_MODE.storeAndSet(scene::CAppMode::MODE_HIGHLIGHT);
    APP_MODE.enableHighlight(false);
}

void osg::C3DModelDraggerHolder::onMouseExit(const osgGA::GUIEventAdapter& ea, bool command_mode)
{
    if (command_mode)
        return;

    APP_MODE.restore();
}

void osg::C3DModelDraggerHolder::setPivotDragger(CPivotDraggerHolder *pivotDragger)
{
    m_pivot_dragger = pivotDragger;

    if (m_pivot_dragger != NULL)
    {
        m_pivot_dragger->setMatrix(getMatrix());
        m_pivot_dragger->setOffset(m_pivot_dragger_offset);
        m_pivot_dragger->setScale(m_pivot_dragger_scale);
    }
}

void osg::C3DModelDraggerHolder::updatePivotDraggerMatrix()
{
    if (m_pivot_dragger != nullptr)
    {
        m_pivot_dragger->setMatrix(getMatrix());
    }
}

void osg::C3DModelDraggerHolder::internalModelChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{
    bool changed = false;
    if (pEntry != NULL)
    {
        changed = true;
    }

    // if associated with a model, monitoring is on
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(data::Storage::PatientData::Id));
    osg::Vec3 sceneSize(spVolume->getXSize() * spVolume->getDX(), spVolume->getYSize() * spVolume->getDY(), spVolume->getZSize() * spVolume->getDZ());
    double size = (sceneSize.x() + sceneSize.y() + sceneSize.z()) / 3;
    spVolume.release();

    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(m_idModel/*, Storage::NO_UPDATE*/));
    const geometry::CMesh *pMesh = spModel->getMesh();
    osg::Matrix modelMatrix = spModel->getTransformationMatrix();
    osg::Matrix position;

    data::CChangedEntries::tFilter filter;
    filter.insert(m_idModel);
    if (m_bSetPivot || (changed && pMesh && (changes.checkFlagsAllZero(filter) || changes.checkFlagsAnySet(data::CModel::MESH_CHANGED | data::StorageEntry::DESERIALIZED | data::StorageEntry::UNDOREDO, filter))))
    {
        pMesh->calc_bounding_box(m_bbMin, m_bbMax);
    }

    position = osg::Matrix::translate(osg::Vec3((m_bbMin[0] + m_bbMax[0]) / 2, (m_bbMin[1] + m_bbMax[1]) / 2, (m_bbMin[2] + m_bbMax[2]) / 2)) * modelMatrix;
    size = ((m_bbMax[0] - m_bbMin[0]) + (m_bbMax[1] - m_bbMin[1]) + (m_bbMax[2] - m_bbMin[2])) / 3;
    modelMatrix = osg::Matrix::inverse(osg::Matrix::translate(osg::Vec3((m_bbMin[0] + m_bbMax[0]) / 2, (m_bbMin[1] + m_bbMax[1]) / 2, (m_bbMin[2] + m_bbMax[2]) / 2)));

    if (position.isIdentity())
    {
        position = osg::Matrix::translate(sceneSize.x() / 2, sceneSize.y() / 2, sceneSize.z() / 2);
        modelMatrix = osg::Matrix::identity();
    }
    // we update dragger matrix
    if (m_bSetPivot)
    {
        updatePivotDraggerMatrix();

        data::CObjectPtr<data::CPivot> spPivot(APP_STORAGE.getEntry(data::Storage::ModelPivot::Id));
        osg::Vec3d pos = position.getTrans();
        spPivot->setPosition(vpl::img::CVector3d(pos.x(), pos.y(), pos.z()));
        APP_STORAGE.invalidate(spPivot.getEntryPtr());
        m_bSetPivot = false;
    }
    // and dragger transformation (translation to model's position)
    updateModelMatrix(spModel->getTransformationMatrix() * osg::Matrix::inverse(getMatrix()));

    // adjust size
    size *= 0.75;

    // update geometry
    m_rotation_dragger->setRadius(0.97 * size, size);
    m_rotation_dragger->updateGeometry();
    double axialOffset = 1.10 * size;
    m_x_translate_dragger->setOffset(axialOffset, axialOffset);
    m_y_translate_dragger->setOffset(axialOffset, axialOffset);
    m_z_translate_dragger->setOffset(axialOffset, axialOffset);
    m_x_translate_dragger->setScale(size / 32);
    m_y_translate_dragger->setScale(size / 32);
    m_z_translate_dragger->setScale(size / 32);
    m_x_translate_dragger->updateGeometry();
    m_y_translate_dragger->updateGeometry();
    m_z_translate_dragger->updateGeometry();

    m_translate_dragger->updateModel(m_idModel);
    m_translate_dragger->setRadius(size / 20);
    m_translate_dragger->updateGeometry();

    m_pivot_dragger_offset = 1.25 * size;
    m_pivot_dragger_scale = 0.5 * (size / 32);

    if (m_pivot_dragger)
    {
        m_pivot_dragger->setOffset(m_pivot_dragger_offset);
        m_pivot_dragger->setScale(m_pivot_dragger_scale);
    }
}

void osg::C3DModelDraggerHolder::internalPivotChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
{
    data::CPivot *pData = pEntry->getDataPtr<data::CObjectHolder<data::CPivot> >()->getObjectPtr();

    vpl::img::CVector3d position = pData->getPosition();
    osg::Matrix matrix = getMatrix();
    matrix.setTrans(osg::Vec3(position.x(), position.y(), position.z()));
    setMatrix(matrix);

    osg::Matrix modelMatrix = osg::Matrix::identity();
    if (m_idModel>0)
    {
        data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(m_idModel));
        modelMatrix = spModel->getTransformationMatrix();
        spModel.release();
    }
    updateModelMatrix(modelMatrix * osg::Matrix::inverse(getMatrix()));

    osg::NodePathList nodePathList = getParentalNodePaths();
    for (osg::NodePathList::iterator npit = nodePathList.begin(); npit != nodePathList.end(); ++npit)
    {
        osg::NodePath &nodePath = *npit;
        for (osg::NodePath::iterator nit = nodePath.begin(); nit != nodePath.end(); ++nit)
        {
            osg::Node *node = *nit;

            scene::CSceneBase *scene = dynamic_cast<scene::CSceneBase *>(node);
            if (scene != NULL)
            {
                scene->getCanvas()->Refresh(false);
                return;
            }
        }
    }
}

//! Prepare materials
void osg::C3DModelDraggerHolder::prepareMaterials()
{
    if (m_translate_dragger.get())
    {
        m_translate_dragger->setDiffuse(osg::FIRST, COLOR_Z_TRANSLATE);
        m_translate_dragger->setDiffuse(osg::SECOND, COLOR_PICKED);
    }

    m_x_translate_dragger->setDiffuse(osg::FIRST, COLOR_X_TRANSLATE);
    m_x_translate_dragger->setDiffuse(osg::SECOND, COLOR_PICKED);

    m_y_translate_dragger->setDiffuse(osg::FIRST, COLOR_Y_TRANSLATE);
    m_y_translate_dragger->setDiffuse(osg::SECOND, COLOR_PICKED);

    m_z_translate_dragger->setDiffuse(osg::FIRST, COLOR_Z_TRANSLATE);
    m_z_translate_dragger->setDiffuse(osg::SECOND, COLOR_PICKED);

    m_rotation_dragger->getXDragger().setDiffuse(osg::FIRST, COLOR_X_ROTATE);
    m_rotation_dragger->getXDragger().setDiffuse(osg::SECOND, COLOR_PICKED);

    m_rotation_dragger->getYDragger().setDiffuse(osg::FIRST, COLOR_Y_ROTATE);
    m_rotation_dragger->getYDragger().setDiffuse(osg::SECOND, COLOR_PICKED);

    m_rotation_dragger->getZDragger().setDiffuse(osg::FIRST, COLOR_Z_ROTATE);
    m_rotation_dragger->getZDragger().setDiffuse(osg::SECOND, COLOR_PICKED);
}

void osg::C3DModelDraggerHolder::updateModelMatrix(const osg::Matrix &modelPos)
{
    if (m_translate_dragger.get())
        m_translate_dragger->updateModelMatrix(modelPos);
}