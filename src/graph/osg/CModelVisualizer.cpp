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

#include "osg/CModelVisualizer.h"

#include <data/CModel.h>
#include <osg/CPseudoMaterial.h>
#include <osg/NodeMasks.h>

#include <osg/PolygonMode>


osg::CModelVisualizer::CModelVisualizer(int modelId)
    : m_modelId(modelId)
    , m_bManualUpdate(false)
    , m_bUseKDTree(true)
{
    // Create a new matrix transform
    m_pTransform = new osg::MatrixTransform();
    m_pTransform->setName("CModelVisualizer");

    // Create a new surface mesh
    m_pMesh = new CTriMesh();
    m_pTransform->addChild(m_pMesh);

    // Retrieve the current mesh from the storage
    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(m_modelId));

    m_pMesh->createMesh(*spModel->getMesh(false), spModel->getTextures(), true);

    // Get transformation matrix from the storage
    m_pTransform->setMatrix(spModel->getTransformationMatrix());

    // Enable depth test so that an opaque polygon will occlude a transparent one behind it.
    m_pMesh->getMeshGeode()->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    m_pMesh->getMeshGeode()->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    // Set the update callback
    data::CGeneralObjectObserver<CModelVisualizer>::connect(spModel.getEntryPtr());
    this->setupObserver(this);

    m_materialRegular = new osg::CPseudoMaterial();
    m_materialRegular->uniform("Shininess")->set(85.0f);
    m_materialRegular->uniform("Specularity")->set(1.0f);

    m_materialSelected = new osg::CPseudoMaterial();
    m_materialSelected->uniform("Shininess")->set(85.0f);
    m_materialSelected->uniform("Specularity")->set(1.0f);

    m_materialSkinnedRegular = m_materialRegular;
    m_materialSkinnedSelected = m_materialSelected;

    m_dragger = new CModelEventsHelperDragger(modelId);
    m_dragger->setNodeMask(MASK_MODEL_DRAGGER);
    m_dragger->addChild(m_pTransform);

    addChild(m_dragger);
}

void osg::CModelVisualizer::updateFromStorage()
{
    if (m_bManualUpdate)
    {
        return;
    }

    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(m_modelId));

    // analyze all changes and extract this model's changes
    data::CChangedEntries changesModel;
    scene::CGeneralObjectObserverOSG<CModelVisualizer>::getChanges(spModel.getEntryPtr(), changesModel);

    data::CChangedEntries::tFilter filter{ m_modelId };

    bool changedReset = changesModel.checkFlagsAnySet(data::Storage::STORAGE_RESET);
    bool changedGeneral = changedReset || changesModel.checkFlagsAllZero(filter) || changesModel.checkFlagsAnySet(data::StorageEntry::DESERIALIZED | data::StorageEntry::UNDOREDO, filter);
    bool changedMesh = changedGeneral || changesModel.checkFlagsAnySet(data::CModel::MESH_CHANGED, filter);
    bool changedVertexColoring = changedGeneral || changesModel.checkFlagsAnySet(data::CModel::VERTEX_COLORING_CHANGED, filter);
    bool changedArmature = changedGeneral || changesModel.checkFlagsAnySet(data::CModel::ARMATURE_CHANGED, filter);
    bool changedColoring = changedGeneral || changesModel.checkFlagsAnySet(data::CModel::COLORING_CHANGED, filter);
    bool changedVisibility = changedGeneral || changesModel.checkFlagsAnySet(data::CModel::VISIBILITY_CHANGED, filter);
    bool changedPosition = changedGeneral || changesModel.checkFlagsAnySet(data::CModel::POSITION_CHANGED, filter);
    bool changedSelection = changedGeneral || changesModel.checkFlagsAnySet(data::CModel::SELECTION_CHANGED, filter);

    if (changedReset && !spModel->hasData())
    {
        m_pMesh = new CTriMesh();

        m_pMesh->getMeshGeode()->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        m_pMesh->getMeshGeode()->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

        m_pTransform->setChild(0, m_pMesh);
    }

    if (changedMesh)
    {
        m_pMesh->createMesh(*spModel->getMesh(false), spModel->getTextures(), true);

        if (m_bUseKDTree)
        {
            m_pMesh->buildKDTree();
        }
    }

    if (changedVertexColoring || changedColoring || changedMesh)
    {
        const auto& color = spModel->getColor();

        m_pMesh->setColor(geometry::convert4<osg::Vec4>(color));

        if (spModel->getUseVertexColors())
        {
            m_pMesh->updateVertexColors(*spModel->getMesh(), color[3]);
        }

        m_pMesh->useVertexColors(spModel->getUseVertexColors());
    }

    if (changedPosition)
    {
        m_pTransform->setMatrix(spModel->getTransformationMatrix());
    }

    if (changedVisibility)
    {
        setOnOffState(spModel->isVisible());
    }

    if (changedColoring)
    {
        // Ugly hack: override emission only for default material(s)
        m_materialRegular->uniform("Emission")->set(geometry::convert3<osg::Vec3>(spModel->getEmission()));
        m_materialSelected->uniform("Emission")->set(geometry::convert3<osg::Vec3>(spModel->getEmission()));
        m_materialSkinnedRegular->uniform("Emission")->set(geometry::convert3<osg::Vec3>(spModel->getEmission()));
        m_materialSkinnedSelected->uniform("Emission")->set(geometry::convert3<osg::Vec3>(spModel->getEmission()));
    }

    if (changedSelection)
    {
        const geometry::CMesh *mesh = spModel->getMesh();
        OpenMesh::VPropHandleT<geometry::CMesh::CVertexGroups> vProp_vertexGroups;

        if ((mesh != nullptr) && (mesh->get_property_handle(vProp_vertexGroups, VERTEX_GROUPS_PROPERTY_NAME)))
        {
            m_pMesh->setMaterial((spModel->isSelected() ? m_materialSkinnedSelected : m_materialSkinnedRegular), -1);
        }
        else
        {
            m_pMesh->setMaterial((spModel->isSelected() ? m_materialSelected : m_materialRegular), -1);
        }
    }

    if (changedArmature)
    {
        std::set<osg::ref_ptr<osg::Uniform>> uniforms;

        for (auto material : m_pMesh->getMaterials())
        {
            osg::Uniform* uf = material.second->uniform("boneMatrices");

            if (uf != nullptr)
            {
                uniforms.insert(uf);
            }
        }

        std::vector<geometry::Matrix> boneMatrices;
        geometry::CBone::gatherMatrices(boneMatrices, spModel->getArmature());

        if (!uniforms.empty())
        {
            int numElements = (*uniforms.begin())->getNumElements();// Assume same number of elements across materials
            int numMatrices = boneMatrices.size();

            for (int i = 0; i < numElements; ++i)
            {
                osg::Matrix matrix = i < numMatrices ? geometry::convert4x4T<osg::Matrix>(boneMatrices[i]) : osg::Matrix::identity();

                for (auto uniform : uniforms)
                {
                    uniform->setElement(i, matrix);
                }
            }
        }
    }
}

void osg::CModelVisualizer::updatePartOfMesh(const std::vector<std::pair<long, osg::CTriMesh::SPositionNormal>>& handles)
{
    if (!m_bManualUpdate)
        return;

    m_pMesh->updatePartOfMesh(handles);

    if (m_bUseKDTree)
    {
        m_pMesh->buildKDTree();
    }
}

int osg::CModelVisualizer::getId() const
{
    return m_modelId;
}

void osg::CModelVisualizer::setId(int id)
{
    if (m_modelId != id)
    {
        data::CGeneralObjectObserver<CModelVisualizer>::disconnect(APP_STORAGE.getEntry(m_modelId).get());
        freeObserver(this);

        m_modelId = id;

        data::CGeneralObjectObserver<CModelVisualizer>::connect(APP_STORAGE.getEntry(m_modelId).get());
        setupObserver(this);
    }
}

osg::observer_ptr<osg::MatrixTransform> osg::CModelVisualizer::getModelTransform()
{
    return m_pTransform;
}

void osg::CModelVisualizer::setManualUpdates(bool bSet)
{
    m_bManualUpdate = bSet;
}

bool osg::CModelVisualizer::getManualUpdates() const
{
    return m_bManualUpdate;
}

osg::observer_ptr<osg::CTriMesh> osg::CModelVisualizer::getMesh()
{
    return m_pMesh;
}

osg::observer_ptr<osg::CModelVisualizer::CModelEventsHelperDragger> osg::CModelVisualizer::getDragger()
{
    return m_dragger;
}

void osg::CModelVisualizer::showWireframe(bool bShow)
{
    if (bShow)
    {
        m_pMesh->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE));
    }
    else
    {
        m_pMesh->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL));
    }
}
