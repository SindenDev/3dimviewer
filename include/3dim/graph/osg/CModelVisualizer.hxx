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
#include <osg/NodeMasks.h>

#include <geometry/base/CMesh.h>

template <class T>
CAnyModelVisualizer<T>::CAnyModelVisualizer(int ModelId) 
    : COnOffNode()
    , m_ModelId(ModelId)
    , m_bManualUpdate(false)
    , m_bUseKDTree(true)
{
    // Create a new matrix transform
    m_pTransform = new osg::MatrixTransform;
    m_pTransform->setName("CAnyModelVisualizer");
    this->addChild(m_pTransform.get());

    // Create a new surface mesh
    m_pMesh = new CTriMesh;
    m_pTransform->addChild(m_pMesh.get());

    // Retrieve the current mesh from the storage
    data::CObjectPtr< tModel > spModel( APP_STORAGE.getEntry(m_ModelId) );
    m_pMesh->createMesh( spModel->getMesh(), true );

    // Get transformation matrix from the storage
    m_pTransform->setMatrix(spModel->getTransformationMatrix());

    // Setup OSG state set
	ModelVisualizer::setupModelStateSet(m_pMesh->getMeshGeode());

    // Set the update callback
    data::CGeneralObjectObserver<CAnyModelVisualizer<T> >::connect(spModel.getEntryPtr());
    this->setupObserver( this );

    m_materialRegular = new osg::CPseudoMaterial;
    m_materialRegular->uniform("Shininess")->set(85.0f);
    m_materialRegular->uniform("Specularity")->set(1.0f);

    m_materialSelected = new osg::CPseudoMaterial;
    m_materialSelected->uniform("Shininess")->set(85.0f);
    m_materialSelected->uniform("Specularity")->set(1.0f);

    m_materialSkinnedRegular = m_materialRegular;
    m_materialSkinnedSelected = m_materialSelected;
    // Create helper dragger
    //m_draggerForEvents = new CModelEventsHelperDragger(ModelId);
    //m_draggerForEvents->setNodeMask(MASK_MODEL_DRAGGER);
    //addChild(m_draggerForEvents);
}


//////////////////////////////////////////////////////////////////////////
//

template <class T>
void CAnyModelVisualizer<T>::updateFromStorage()
{
    if (m_bManualUpdate)
    {
        return;
    }

    std::set<int> changedEntries;
    scene::CGeneralObjectObserverOSG<CAnyModelVisualizer<T> >::getChangedEntries(changedEntries);

    data::CObjectPtr<tModel> spModel(APP_STORAGE.getEntry(m_ModelId));

    // analyze all changes and extract this model's changes
    data::CChangedEntries changesModel;
    scene::CGeneralObjectObserverOSG<CAnyModelVisualizer<T> >::getChanges(spModel.getEntryPtr(), changesModel);

    // Update the triangular mesh
	data::CChangedEntries::tFilter filter;
	filter.insert(m_ModelId);
    bool updated = false;
	if (changesModel.checkFlagAny(data::Storage::STORAGE_RESET) ||
		changesModel.checkFlagsAllZero(filter) ||
		changesModel.checkFlagsAnySet(data::CModel::MESH_CHANGED | data::StorageEntry::DESERIALIZED | data::StorageEntry::UNDOREDO, filter))
    {
        m_pMesh->createMesh(spModel->getMesh(), true);

        // Setup OSG state set
		ModelVisualizer::setupModelStateSet(m_pMesh->getMeshGeode());
        updated = true;
    }

    // Get model color
    float r, g, b, a;
    spModel->getColor(r, g, b, a);

    // update vertex color array
    // createMesh from previous block messes up alpha canal in vertex colors - it sets alpha to 1.0 but we need to use alpha value from model color
    if (changesModel.checkFlagsAllSet(data::CModel::VERTEX_COLORING_CHANGED, filter) || updated)
    {
        const geometry::CMesh* mesh = spModel->getMesh();
        Vec4Array *vertexColors = new Vec4Array(std::max(1, (int)(mesh->n_vertices())));
        long index = 0;
        for (geometry::CMesh::ConstVertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
        {
            (*vertexColors)[index] = osg::Vec4(mesh->color(vit)[0] / 255.0, mesh->color(vit)[1] / 255.0, mesh->color(vit)[2] / 255.0, a); // use alpha from model color, vertices has only RGB
            ++index;
        }
        m_pMesh->updateVertexColors(vertexColors);
    }

    if (m_bUseKDTree)
    {
        buildKDTree();
    }

    // Update model matrix
    m_pTransform->setMatrix(spModel->getTransformationMatrix());

    // Set model color
    m_pMesh->setColor(r, g, b, a);

    m_pMesh->useVertexColors(spModel->getUseVertexColors());

    // Set visibility
    setOnOffState(spModel->isVisible());

    if (changesModel.checkFlagsAllZero(filter) ||
        changesModel.checkFlagsAnySet(data::CModel::ARMATURE_CHANGED | data::StorageEntry::DESERIALIZED | data::StorageEntry::UNDOREDO, filter))
    {
        std::vector<geometry::Matrix> boneMatrices;
        geometry::CBone::gatherMatrices(boneMatrices, spModel->getArmature());
        std::set<osg::ref_ptr<osg::Uniform> > uniforms;
        uniforms.insert(m_materialSkinnedSelected->uniform("boneMatrices"));
        uniforms.insert(m_materialSkinnedRegular->uniform("boneMatrices"));

        for (int i = -1; i < m_pMesh->getNumMaterials(); ++i)
        {
            osg::Uniform* uf = m_pMesh->getMaterialByIndex(i)->uniform("boneMatrices");
            if (NULL != uf && uf->getName() == "boneMatrices") // method above can return dummy
                uniforms.insert(uf);
        }

        for (int i = 0; i < boneMatrices.size(); ++i)
        {
            osg::Matrix matrix = geometry::convert4x4T<osg::Matrix, geometry::Matrix>(boneMatrices[i]);
            for (std::set<osg::ref_ptr<osg::Uniform> >::iterator it = uniforms.begin(); it != uniforms.end(); ++it)
            {
                (*it)->setElement(i, matrix);
            }
        }
    }

    // Resolve material
    const geometry::CMesh *mesh = spModel->getMesh();
    OpenMesh::VPropHandleT<geometry::CMesh::CVertexGroups> vProp_vertexGroups;
    if ((mesh != NULL) && (mesh->get_property_handle(vProp_vertexGroups, VERTEX_GROUPS_PROPERTY_NAME)))
    {
        m_pMesh->setMaterial((spModel->isSelected() ? m_materialSkinnedSelected : m_materialSkinnedRegular), -1);
    }
    else
    {
    m_pMesh->setMaterial((spModel->isSelected() ? m_materialSelected : m_materialRegular), -1);
}
}

template <class T>
void CAnyModelVisualizer<T>::updatePartOfMesh( const CTriMesh::tIdPosVec &handles )
{
	if(!m_bManualUpdate)
		return;

	// Get model
	data::CObjectPtr< tModel > spModel( APP_STORAGE.getEntry(m_ModelId) );
	m_pMesh->updatePartOfMesh(spModel->getMesh(), handles, true);
    spModel->setMeshDirty();
	if(m_bUseKDTree)
		buildKDTree();
}

template <class T>
void CAnyModelVisualizer<T>::showWireframe( bool bShow )
{
	if(m_pMesh.get() == 0)
		return;

	osg::StateSet *state = m_pMesh->getOrCreateStateSet();

	//   We need to retireve the Poylgon mode of the  state set, and create one if does not have one
	osg::PolygonMode *polyModeObj;

	polyModeObj = dynamic_cast< osg::PolygonMode* >
		( state->getAttribute( osg::StateAttribute::POLYGONMODE ));

	if ( !polyModeObj ) {

		polyModeObj = new osg::PolygonMode;

		state->setAttribute( polyModeObj );    
	}

	//  Now we can set or unset the state to WIREFRAME

	if(bShow)
		polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );
	else
		polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL );
}


template <class T>
void CAnyModelVisualizer<T>::buildKDTree()
{
	m_pMesh->buildKDTree();
}
