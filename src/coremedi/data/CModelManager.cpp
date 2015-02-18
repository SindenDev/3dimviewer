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

#include <data/CModelManager.h>
#include <data/CDensityData.h>
#include <app/Signals.h>
#include <data/CModelCut.h>
#include <data/COrthoSlice.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//

CModelManager::CModelManager() : CStorageInterface(APP_STORAGE)
{
    // Initialize the dummy model
    m_DummyModel.setMesh(new CMesh);
    m_transparencyNeeded = false;

    // Register all callback functions
    VPL_SIGNAL(SigSetModel).connect(this, &CModelManager::setModel);
    VPL_SIGNAL(SigSetModelColor).connect(this, &CModelManager::setModelColor2);
    VPL_SIGNAL(SigSetModelVisibility).connect(this, &CModelManager::setModelVisibility);
    VPL_SIGNAL(SigGetModelColor).connect(this, &CModelManager::getModelColor2);
    VPL_SIGNAL(SigGetModelVisibility).connect(this, &CModelManager::isModelShown);

    // Initialize the manager
    CModelManager::init();

    // Initialize colors
    setModelColor(Storage::BonesModel::Id, 1.0f, 1.0f, 1.0f, 1.0f);
    setModelColor(Storage::SoftTissuesModel::Id, 1.0f, 0.0f, 0.0f, 1.0f);
    setModelColor(Storage::ImprintModel::Id, 1.0f, 0.0f, 0.0f, 1.0f);
    setModelColor(Storage::TemplateModel::Id, 0.0f, 0.0f, 1.0f, 1.0f);
    for(int id = 0; id < MAX_IMPORTED_MODELS; ++id)
        setModelColor(Storage::ImportedModel::Id + id, 1.0f, 1.0f, 0.0f, 1.0f);
}

///////////////////////////////////////////////////////////////////////////////
//

CModelManager::~CModelManager()
{
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::init()
{
    using namespace Storage;

    CEntryDeps ModelDeps;
    //ModelDeps.insert(PatientData::Id).insert(AuxData::Id);
    ModelDeps.insert(PatientData::Id).insert(AuxData::Id);

    STORABLE_FACTORY.registerObject(BonesModel::Id, BonesModel::Type::create, ModelDeps);
    STORABLE_FACTORY.registerObject(SoftTissuesModel::Id, SoftTissuesModel::Type::create, ModelDeps);
    STORABLE_FACTORY.registerObject(ImprintModel::Id, ImprintModel::Type::create, ModelDeps);
    STORABLE_FACTORY.registerObject(TemplateModel::Id, TemplateModel::Type::create, ModelDeps);
    for(int id = 0; id < MAX_IMPORTED_MODELS; ++id)
        STORABLE_FACTORY.registerObject(ImportedModel::Id + id, ImportedModel::Type::create, ModelDeps);

	// cuts through imported models
	for (int i = 0; i < MAX_IMPORTED_MODELS; ++i)
	{
		STORABLE_FACTORY.registerObject(ImportedModelCutSliceXY::Id + i, ImportedModelCutSliceXY::Type::create, CEntryDeps().insert(SliceXY::Id).insert(ImportedModel::Id + i));
		STORABLE_FACTORY.registerObject(ImportedModelCutSliceXZ::Id + i, ImportedModelCutSliceXZ::Type::create, CEntryDeps().insert(SliceXZ::Id).insert(ImportedModel::Id + i));
		STORABLE_FACTORY.registerObject(ImportedModelCutSliceYZ::Id + i, ImportedModelCutSliceYZ::Type::create, CEntryDeps().insert(SliceYZ::Id).insert(ImportedModel::Id + i));
	}

    // Enforce object creation
    APP_STORAGE.getEntry(BonesModel::Id);

	for (int i = 0; i < MAX_IMPORTED_MODELS; ++i)
	{
		CObjectPtr<CModelCutSliceXY> spModelCutXY( APP_STORAGE.getEntry(ImportedModelCutSliceXY::Id + i) );
		spModelCutXY->setModelId(ImportedModel::Id + i);
		CObjectPtr<CModelCutSliceXZ> spModelCutXZ( APP_STORAGE.getEntry(ImportedModelCutSliceXZ::Id + i) );
		spModelCutXZ->setModelId(ImportedModel::Id + i);
		CObjectPtr<CModelCutSliceYZ> spModelCutYZ( APP_STORAGE.getEntry(ImportedModelCutSliceYZ::Id + i) );
		spModelCutYZ->setModelId(ImportedModel::Id + i);
	}

    // Initialize storage ids for mesh undo providers
    initMeshUndoProvider(Storage::BonesModel::Id);
    initMeshUndoProvider(Storage::SoftTissuesModel::Id);
    initMeshUndoProvider(Storage::ImprintModel::Id);
    initMeshUndoProvider(Storage::TemplateModel::Id);
    for(int id = 0; id < MAX_IMPORTED_MODELS; ++id)
        initMeshUndoProvider(Storage::ImportedModel::Id + id);
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::setModel(int id, CMesh * pMesh)
{
    if( id < Storage::BonesModel::Id || id >= Storage::BonesModel::Id + MAX_MODELS || !pMesh )
    {
        return;
    }
    
    CObjectPtr<CModel> spModel( APP_STORAGE.getEntry(id, Storage::NO_UPDATE) );
    
    spModel->setMesh(pMesh);
	spModel->clearAllProperties();
    
    APP_STORAGE.invalidate(spModel.getEntryPtr());
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::setModelVisibility(int id, bool bVisible)
{
    if( id < Storage::BonesModel::Id || id >= Storage::BonesModel::Id + MAX_MODELS )
    {
        return;
    }
    
    CObjectPtr<CModel> spModel( APP_STORAGE.getEntry(id, Storage::NO_UPDATE) );
    
    if( bVisible )
    {
        spModel->show();
    }
    else
    {
        spModel->hide();
    }
    
    APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::MESH_NOT_CHANGED);
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::setModelColor2(int id, const CColor4f& Color)
{
    if( id < Storage::BonesModel::Id || id >= Storage::BonesModel::Id + MAX_MODELS )
    {
        return;
    }
    
    CObjectPtr<CModel> spModel( APP_STORAGE.getEntry(id, Storage::NO_UPDATE) );
    
    spModel->setColor(Color);
    
    APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::MESH_NOT_CHANGED);

    resolveTransparency();
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::resolveTransparency()
{
    bool prev = m_transparencyNeeded;
    updateTransparencyNeeded();

    if (prev != m_transparencyNeeded)
    {
        VPL_SIGNAL(SigTransparencyNeededChange).invoke(m_transparencyNeeded);
    }
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::updateTransparencyNeeded()
{
    m_transparencyNeeded = false;

    std::vector<int> observedModels;
    observedModels.push_back(Storage::BonesModel::Id);
    observedModels.push_back(Storage::SoftTissuesModel::Id);
    observedModels.push_back(Storage::ImprintModel::Id);
    observedModels.push_back(Storage::TemplateModel::Id);
    for (int id = 0; id < MAX_IMPORTED_MODELS; ++id)
    {
        observedModels.push_back(Storage::ImportedModel::Id + id);
    }

    float r, g, b, a, overallTransparency;
    for (std::size_t i = 0; i < observedModels.size(); ++i)
    {
        getModelColor(observedModels[i], r, g, b, a);
        a = isModelShown(observedModels[i]) ? a : 1.0;
        if (a < 1.0)
        {
            m_transparencyNeeded = true;
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//

bool CModelManager::isModelShown(int id)
{
    if( id < Storage::BonesModel::Id || id >= Storage::BonesModel::Id + MAX_MODELS )
    {
        return false;
    }
    
    CObjectPtr<CModel> spModel( APP_STORAGE.getEntry(id, Storage::NO_UPDATE) );
    
    return spModel->isShown();
}

///////////////////////////////////////////////////////////////////////////////
//

CColor4f CModelManager::getModelColor2(int id)
{
    if( id < Storage::BonesModel::Id || id >= Storage::BonesModel::Id + MAX_MODELS )
    {
        return CColor4f(0.0f, 0.0f, 0.0f, 0.0f);
    }
    
    CObjectPtr<CModel> spModel( APP_STORAGE.getEntry(id) );
    
    return spModel->getColor();
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::getModelColor(int id, float& r, float& g, float& b, float& a)
{
    if( id < Storage::BonesModel::Id || id >= Storage::BonesModel::Id + MAX_MODELS )
    {
        r = g = b = a = 0.0;
        return;
    }
    
    CObjectPtr<CModel> spModel( APP_STORAGE.getEntry(id) );
    
    return spModel->getColor(r, g, b, a);
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::setModelColor( int id, float r, float g, float b, float a )
{
    if( id < Storage::BonesModel::Id || id >= Storage::BonesModel::Id + MAX_MODELS )
    {
        return;
    }
    
    CObjectPtr<CModel> spModel( APP_STORAGE.getEntry(id, Storage::NO_UPDATE) );
    
    spModel->setColor(r, g, b, a);
    
    APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::MESH_NOT_CHANGED);
}

///////////////////////////////////////////////////////////////////////////////
// Undo support
void CModelManager::initMeshUndoProvider(int storageId)
{
    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(storageId));
    spModel->setStorageId(storageId);
}

///////////////////////////////////////////////////////////////////////////////
// Restore state from the snapshot
void CModelManager::restore( CSnapshot * snapshot )
{
    assert( snapshot != NULL );
    CModelSnapshot * s = dynamic_cast< CModelSnapshot * >( snapshot );
    assert( s != NULL );
    // copy models
    for( int i = 0; i < MAX_MODELS; ++i )
    {
        // try to get model from the storage
        data::CObjectPtr<data::CModel> ptrModel( APP_STORAGE.getEntry( data::Storage::BonesModel::Id + i ) );
        // update from snapshot
        *ptrModel = s->m_modelArray[ i ];
        // invalidate
        APP_STORAGE.invalidate( ptrModel.getEntryPtr() );
    }
}

///////////////////////////////////////////////////////////////////////////////
// Get new snapshot from the od one.
CSnapshot *CModelManager::getSnapshot( CSnapshot * snapshot )
{
    // new snapshot
    CModelSnapshot * s = new CModelSnapshot( this );

    // copy model array
    for( int i = 0; i < MAX_MODELS; ++i )
    {
        // try to get model from the storage
        data::CObjectPtr<data::CModel> ptrModel( APP_STORAGE.getEntry( data::Storage::BonesModel::Id + i ) );
        s->m_modelArray[ i ] = *ptrModel;
    }
    return s;
}

///////////////////////////////////////////////////////////////////////////////
//
void CModelManager::createAndStoreSnapshot()
{
    VPL_SIGNAL(SigUndoSnapshot).invoke(this->getSnapshot(NULL));
}

} // namespace data

