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

#include <data/CModelManager.h>
#include <data/CDensityData.h>
#include <coremedi/app/Signals.h>
#include <data/CModelCut.h>
#include <data/COrthoSlice.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//

CModelManager::CModelManager()
    : CStorageInterface(APP_STORAGE)
    , m_selectedModel(-1)
    , m_multiSelectionEnabled(false)
{
    // Initialize the dummy model
    m_DummyModel.setMesh(new geometry::CMesh);
    m_transparencyNeeded = false;

    // Register all callback functions
    VPL_SIGNAL(SigSetModel).connect(this, &CModelManager::setModel);
    VPL_SIGNAL(SigSetModelColor).connect(this, &CModelManager::setModelColor2);
    VPL_SIGNAL(SigSetModelVisibility).connect(this, &CModelManager::setModelVisibility);
    VPL_SIGNAL(SigGetModelColor).connect(this, &CModelManager::getModelColor2);
    VPL_SIGNAL(SigGetModelVisibility).connect(this, &CModelManager::isModelShown);
    VPL_SIGNAL(SigSelectModel).connect(this, &CModelManager::selectModel);
	VPL_SIGNAL(GetSelectedModel).connect(this, &CModelManager::getSelectedModel);
    VPL_SIGNAL(SigModelsMultiSelectionEnabled).connect(this, &CModelManager::setMultiSelectionEnabled);
    VPL_SIGNAL(SigRemoveModel).connect(this, &CModelManager::removeModel);

    // Initialize the manager
    CModelManager::init();

    // Initialize colors
    setModelColor(Storage::BonesModel::Id, 1.0f, 1.0f, 1.0f, 1.0f);
    setModelColor(Storage::SoftTissuesModel::Id, 1.0f, 0.0f, 0.0f, 1.0f);
    setModelColor(Storage::ImprintModel::Id, 1.0f, 0.0f, 0.0f, 1.0f);
    setModelColor(Storage::TemplateModel::Id, 0.0f, 0.0f, 1.0f, 1.0f);
    for (int id = 0; id < MAX_IMPORTED_MODELS; ++id)
    {
        setModelColor(Storage::ImportedModel::Id + id, 1.0f, 1.0f, 0.0f, 1.0f);
    }
}

///////////////////////////////////////////////////////////////////////////////
//

CModelManager::~CModelManager()
{ }

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::init()
{
    m_selectedModel = -1;

    using namespace Storage;

    CEntryDeps ModelDeps;
    //ModelDeps.insert(PatientData::Id).insert(AuxData::Id);
    ModelDeps.insert(PatientData::Id).insert(AuxData::Id);

    STORABLE_FACTORY.registerObject(BonesModel::Id, BonesModel::Type::create, ModelDeps);
    STORABLE_FACTORY.registerObject(SoftTissuesModel::Id, SoftTissuesModel::Type::create, ModelDeps);
    STORABLE_FACTORY.registerObject(ImprintModel::Id, ImprintModel::Type::create, ModelDeps);
    STORABLE_FACTORY.registerObject(TemplateModel::Id, TemplateModel::Type::create, ModelDeps);
    for (int id = 0; id < MAX_IMPORTED_MODELS; ++id)
    {
        STORABLE_FACTORY.registerObject(ImportedModel::Id + id, ImportedModel::Type::create, ModelDeps);
    }

    // cuts through imported models
    for (int i = 0; i < MAX_IMPORTED_MODELS; ++i)
    {
        STORABLE_FACTORY.registerObject(ImportedModelCutSliceXY::Id + i, ImportedModelCutSliceXY::Type::create, CEntryDeps().insert(SliceXY::Id).insert(ImportedModel::Id + i));
        STORABLE_FACTORY.registerObject(ImportedModelCutSliceXZ::Id + i, ImportedModelCutSliceXZ::Type::create, CEntryDeps().insert(SliceXZ::Id).insert(ImportedModel::Id + i));
        STORABLE_FACTORY.registerObject(ImportedModelCutSliceYZ::Id + i, ImportedModelCutSliceYZ::Type::create, CEntryDeps().insert(SliceYZ::Id).insert(ImportedModel::Id + i));
    }

    // Enforce object creation
    APP_STORAGE.getEntry(BonesModel::Id);
	APP_STORAGE.getEntry(SoftTissuesModel::Id);
	APP_STORAGE.getEntry(ImprintModel::Id);
	APP_STORAGE.getEntry(TemplateModel::Id);
	for (int id = 0; id < MAX_IMPORTED_MODELS; ++id)
		APP_STORAGE.getEntry(ImportedModel::Id + id);

    for (int i = 0; i < MAX_IMPORTED_MODELS; ++i)
    {
        CObjectPtr<CModelCutSliceXY> spModelCutXY(APP_STORAGE.getEntry(ImportedModelCutSliceXY::Id + i));
        spModelCutXY->setModelId(ImportedModel::Id + i);
        CObjectPtr<CModelCutSliceXZ> spModelCutXZ(APP_STORAGE.getEntry(ImportedModelCutSliceXZ::Id + i));
        spModelCutXZ->setModelId(ImportedModel::Id + i);
        CObjectPtr<CModelCutSliceYZ> spModelCutYZ(APP_STORAGE.getEntry(ImportedModelCutSliceYZ::Id + i));
        spModelCutYZ->setModelId(ImportedModel::Id + i);
    }

    // Initialize storage ids for mesh undo providers
    initMeshUndoProvider(Storage::BonesModel::Id);
    initMeshUndoProvider(Storage::SoftTissuesModel::Id);
    initMeshUndoProvider(Storage::ImprintModel::Id);
    initMeshUndoProvider(Storage::TemplateModel::Id);
    for (int id = 0; id < MAX_IMPORTED_MODELS; ++id)
    {
        initMeshUndoProvider(Storage::ImportedModel::Id + id);
    }
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::setModel(int id, geometry::CMesh * pMesh)
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


// Removes model
void CModelManager::removeModel(int id)
{
    if (id < Storage::BonesModel::Id || id >= Storage::BonesModel::Id + MAX_MODELS)
    {
        return;
    }

    if (m_selectedModel == id)
    {
        m_selectedModel = -1;
    }

    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(id));

    data::CSnapshot *snapshot = getSnapshot(NULL);
    data::CSnapshot *snapshotModel = spModel->getSnapshot(NULL);
    snapshot->addSnapshot(snapshotModel);
    VPL_SIGNAL(SigUndoSnapshot).invoke(snapshot);

    spModel->init();
    APP_STORAGE.invalidate(spModel.getEntryPtr());

    VPL_SIGNAL(SigModelRemoved).invoke(id);
}

// Selects model
void CModelManager::selectModel(int id)
{
    if (id < Storage::BonesModel::Id || id >= Storage::BonesModel::Id + MAX_MODELS)
    {
        id = -1;
    }

    // if it was clicked on already selected model - deselect it
    if (id != -1 && m_multiSelectionEnabled)
    {
        data::CObjectPtr<data::CModel> spCurrModel(APP_STORAGE.getEntry(id, data::Storage::NO_UPDATE));

        if (spCurrModel->isSelected())
        {
            spCurrModel->deselect();
            spCurrModel->setExamined(false);
            APP_STORAGE.invalidate(spCurrModel.getEntryPtr(), CModel::SELECTION_CHANGED);
            m_selectedModel = -1;
            return;
        }
    }

    if (m_selectedModel != id)
    {
        if (!m_multiSelectionEnabled)
        {
            // deselect current
            if (m_selectedModel != -1)
            {
                data::CObjectPtr<data::CModel> spPrevModel(APP_STORAGE.getEntry(m_selectedModel, data::Storage::NO_UPDATE));
                spPrevModel->deselect();
                APP_STORAGE.invalidate(spPrevModel.getEntryPtr(), CModel::SELECTION_CHANGED);
            }
        }

        m_selectedModel = id;

        // just deselecting => do not do anything
        if (m_selectedModel == -1)
        {
            return;
        }

        // select new
        data::CObjectPtr<data::CModel> spCurrModel(APP_STORAGE.getEntry(m_selectedModel, data::Storage::NO_UPDATE));

		if (!spCurrModel->isVisible())
		{
			m_selectedModel = -1;
			return;
		}

        spCurrModel->select();

        if (m_multiSelectionEnabled)
            spCurrModel->setExamined(true);

        APP_STORAGE.invalidate(spCurrModel.getEntryPtr(), CModel::SELECTION_CHANGED);
    }
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
		if (spModel->isVisible())
			return;
        spModel->show();
    }
    else
    {
		if (!spModel->isVisible())
			return;
        spModel->hide();
    }
    
    APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::VISIBILITY_CHANGED);

    resolveTransparency();
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
    const CColor4f& prev = spModel->getColor();
    if (prev.getR() != Color.getR() ||
        prev.getG() != Color.getG() ||
        prev.getB() != Color.getB() ||
        prev.getA() != Color.getA())
    {
        spModel->setColor(Color);
        APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::COLORING_CHANGED);
        resolveTransparency();
    }    
}

///////////////////////////////////////////////////////////////////////////////
//

void CModelManager::resolveTransparency()
{
    bool prev = m_transparencyNeeded;
    updateTransparencyNeeded();

    if (prev != m_transparencyNeeded)
    {
        VPL_SIGNAL(SigTransparencyNeededChange).invoke(m_transparencyNeeded?TRANSPARENCY_NEEDED_MODELS:0,TRANSPARENCY_NEEDED_MODELS);
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
    
    return spModel->isVisible();
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
    
    APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::COLORING_CHANGED);
}

///////////////////////////////////////////////////////////////////////////////
// Undo support
void CModelManager::initMeshUndoProvider(int storageId)
{
    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(storageId, data::Storage::NO_UPDATE));
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
    for( auto it = s->m_modelMap.begin(); it != s->m_modelMap.end(); ++it )
    {        
        // try to get model from the storage
        data::CObjectPtr<data::CModel> ptrModel( APP_STORAGE.getEntry( it->first) );
        // check current model state
        const geometry::CMesh* pMesh = ptrModel->getMesh();
        bool bMeshWasValid = pMesh && pMesh->n_vertices() > 0;   
        // update from snapshot
        *ptrModel = it->second;
        //deselect models
        ptrModel->deselect();
        // check new model state
        pMesh = ptrModel->getMesh();
        bool bMeshIsValid = pMesh && pMesh->n_vertices() > 0;
        // invalidate if mesh was valid before or is valid now 
        // this check can provide some performance gain (ignore changes on unused models)
        if (bMeshWasValid || bMeshIsValid)
        {            
            APP_STORAGE.invalidate( ptrModel.getEntryPtr() );
        }
    }
    resolveTransparency();
    //all models are deselected 
    VPL_SIGNAL(SigSelectModel).invoke(-1);
}

///////////////////////////////////////////////////////////////////////////////
// Get new snapshot from the od one.
CSnapshot *CModelManager::getSnapshot( CSnapshot * snapshot )
{
    // new snapshot
    CModelSnapshot * s = new CModelSnapshot( this );
    // old snapshot
    CModelSnapshot * sOld = dynamic_cast<CModelSnapshot*>(snapshot);
    // deduce model list from the old snapshot
    if (NULL!=sOld && !sOld->m_modelMap.empty())
    {        
        // retrieve list of old snapshot models and create a new snapshot of these only
        for(auto it = sOld->m_modelMap.begin(); it != sOld->m_modelMap.end(); ++it) 
        {
            int id = it->first;            
            data::CObjectPtr<data::CModel> ptrModel( APP_STORAGE.getEntry( id ) );
            s->m_modelMap[ id ] = *ptrModel;
        }
        // note: we don't have to care about chained snapshots (ones with the mesh),
        // because these are handled by CUndoManager::processSnapshot
    }
    else // make snapshot of all models
    {
        // copy model array
        for( int i = 0; i < MAX_MODELS; ++i )
        {
            // try to get model from the storage
            data::CObjectPtr<data::CModel> ptrModel( APP_STORAGE.getEntry( data::Storage::BonesModel::Id + i ) );
            s->m_modelMap[ data::Storage::BonesModel::Id +i ] = *ptrModel;
        }
    }
    return s;
}

///////////////////////////////////////////////////////////////////////////////
// Get new snapshot from the od one.

CSnapshot * CModelManager::getSnapshot( CSnapshot * snapshot, std::vector<int> modelList, bool bIncludeMesh )
{
    if (modelList.empty())
    {
        assert(!bIncludeMesh); // this would be too slow and memory demanding
        return getSnapshot(snapshot);
    }

    // new snapshot
    CModelSnapshot * s = new CModelSnapshot( this );

    // copy model array
    for( int i = 0; i < modelList.size(); ++i )
    {
        // try to get model from the storage
        data::CObjectPtr<data::CModel> ptrModel( APP_STORAGE.getEntry( modelList[i] ) );
        s->m_modelMap[ modelList[i] ] = *ptrModel;
        if (bIncludeMesh)
            s->addSnapshot(ptrModel->getSnapshot(NULL));
    }

    return s;
}

///////////////////////////////////////////////////////////////////////////////
//
void CModelManager::createAndStoreSnapshot(std::vector<int> modelIDs, bool bIncludeMesh, CSnapshot *childSnapshot)
{
    if (childSnapshot == NULL)
    {
        VPL_SIGNAL(SigUndoSnapshot).invoke(this->getSnapshot(NULL,modelIDs,bIncludeMesh));
    }
    else
    {
        CSnapshot *snapshot = this->getSnapshot(NULL,modelIDs,bIncludeMesh);
        snapshot->addSnapshot(childSnapshot);
        VPL_SIGNAL(SigUndoSnapshot).invoke(snapshot);
    }
}

/**
 * \fn void CModelManager::updateModelLinks()
 *
 * \brief Updates the model links.
 */
void CModelManager::updateModelLinks()
{
	writeLinks();

	// For all models
	for (int id = Storage::BonesModel::Id; id < Storage::ImportedModel::Id + MAX_IMPORTED_MODELS; ++id)
	{
		// Get model
		CObjectPtr<CModel> spModel(APP_STORAGE.getEntry(id));

		// If model has data
		if(!spModel->hasData())
			continue;

		// If model has no unique id generate one
		std::string modelUid;
		{
			modelUid = spModel->getProperty("model_uid");
			if (modelUid.empty())
			{
				modelUid = VPL_SIGNAL(SigGenerateUniqueId).invoke2();
				spModel->setProperty("model_uid", modelUid);
			}
		}

		// If base_model id is already set, do nothing
		std::string base_id = spModel->getProperty("base_model_ref");
		if (!base_id.empty())
			continue;
		
		// If model_ref is set, this should be surgical guide and model_ref is base model id
		std::string model_ref = spModel->getProperty("model_ref");
		if (!model_ref.empty())
		{
			spModel->setProperty("base_model_ref", model_ref);

		}
		else
		{
			// Else if guide_ref is set, try to find this model and test its model ref
			std::string guide_ref = spModel->getProperty("guide_ref");

			// For all existing models
			if (!guide_ref.empty())
				for (int i = 0; i < MAX_MODELS; ++i)
				{
					CObjectPtr<data::CModel> spModelGuide(APP_STORAGE.getEntry(i + data::Storage::BonesModel::Id));
					if (!spModelGuide->hasData())
						continue;

					// Is this model our guide model?
					if (spModelGuide->getProperty("guide_uid").compare(guide_ref) == 0)
					{ 
						// Is base model already set?
						std::string guide_base_id = spModelGuide->getProperty("base_model_ref");
						if (!guide_base_id.empty())
						{
							spModel->setProperty("base_model_ref", guide_base_id);
						}

						// Is model ref already set?
						std::string guide_model_ref = spModelGuide->getProperty("model_ref");
						if (!guide_model_ref.empty())
						{
							spModel->setProperty("base_model_ref", guide_model_ref);
						}

						// Guide model found and base model (hopefully) set.
						break;
					}
				}
		}

		// If base model still not set, use model uid
		base_id = spModel->getProperty("base_model_ref");
		if (base_id.empty())
			spModel->setProperty("base_model_ref", modelUid);
	}

	writeLinks();
}

void CModelManager::writeLinks()
{
//	DBOUT("--- Models:");
	// For all models
	for (int id = Storage::BonesModel::Id; id < Storage::ImportedModel::Id + MAX_IMPORTED_MODELS; ++id)
	{
		// Get model
		CObjectPtr<CModel> spModel(APP_STORAGE.getEntry(id));

		// If model has data
		if (!spModel->hasData())
			continue;

		// get properties
		std::string model_uid, base_ref;
		base_ref = spModel->getProperty("base_model_ref");
		model_uid = spModel->getProperty("model_uid");

//		DBOUT("ID: " << model_uid.c_str() << "\t base: " << base_ref.c_str());

	}
}

/**
 * \fn std::string CModelManager::getBaseModel(int storage_id)
 *
 * \brief Gets base model, if exists.
 *
 * \param	storage_id Identifier for the storage.
 *
 * \return The base model.
 */
std::string CModelManager::getBaseModel(int storage_id)
{
	if (storage_id < 0)
		return "";

	data::CObjectPtr<data::CModel> spUsedModel(APP_STORAGE.getEntry(storage_id));
	return spUsedModel->getProperty("base_model_ref");
}

/**
 * \fn  int CModelManager::getBaseModelId(int model_id)
 *
 * \brief   Gets base model identifier.
 *
 * \param   model_id    Identifier for the model.
 *
 * \return  The base model identifier.
 */

int CModelManager::getBaseModelId(int model_id)
{
    // If currently selected model is not valid
    if (!validModelId(model_id))
        return -1;

    // Get current model
    data::CObjectPtr<data::CModel>  spCurrent(APP_STORAGE.getEntry(model_id));

    // Get references
    const std::string base_model_ref(spCurrent->getProperty("base_model_ref"));
    const std::string model_uid(spCurrent->getProperty("model_uid"));
    const std::string model_type(spCurrent->getProperty("model_type"));

    if (model_uid.compare(base_model_ref) == 0)
        return model_id;

    if (0 == model_type.compare("inverted")) // create guide from the inverted model, not the base one
        return model_id;

    if (!base_model_ref.empty())
    {
        // Try to find base model
        for (int i = data::Storage::BonesModel::Id; i < data::Storage::BonesModel::Id + MAX_MODELS; ++i)
        {
            if (i == model_id)
                continue;

            // Get model
            data::CObjectPtr<data::CModel>  spTested(APP_STORAGE.getEntry(i));

            if (spTested->getProperty("model_uid").compare(base_model_ref) == 0)
            {
                return i;
            }
        }
    }

    // Something wrong has happen. Model was probably deleted
    return -1;
}

/**
 * \fn bool CModelManager::modelExists(const std::string &uid)
 *
 * \brief Queries if a given model exists.
 *
 * \param	uid The UID.
 *
 * \return true if it succeeds, false if it fails.
 */
bool CModelManager::modelExists(const std::string &uid)
{
	return uidToStorageId(uid) > 0;
}

/**
 * \fn int CModelManager::uidToStorageId(const std::string &uid)
 *
 * \brief Convert UID to storage identifier.
 *
 * \param	uid The UID.
 *
 * \return An int.
 */
int CModelManager::uidToStorageId(const std::string &uid)
{
	// No uid? No storage id.
	if (uid.empty())
		return -1;

	// Try to find model with given uid
	for (int id = Storage::BonesModel::Id; id < Storage::ImportedModel::Id + MAX_IMPORTED_MODELS; ++id)
	{
		// Get model
		CObjectPtr<CModel> spModel(APP_STORAGE.getEntry(id));

		if (spModel->getProperty("model_uid").compare(uid) == 0)
			return id;
	}

	// Uid not found
	return -1;
}

/**
 * \fn std::string CModelManager::storageIdToUId(int storage_id)
 *
 * \brief Convert storage identifier to unique identifier.
 *
 * \param	storage_id Identifier for the storage.
 *
 * \return A std::string.
 */
std::string CModelManager::storageIdToUId(int storage_id)
{
	if (storage_id < 0)
		return std::string("");

	CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(storage_id));
	return spModel->getProperty("base_model_ref");
}

/**
 * \fn  bool CModelManager::isBaseModel(int storage_id)
 *
 * \brief   Query if 'storage_id' is base model.
 *
 * \param   storage_id  Identifier for the storage.
 *
 * \return  true if base model, false if not.
 */

bool CModelManager::isBaseModel(int storage_id)
{
    return storage_id == getBaseModelId(storage_id);
}

void CModelManager::setMultiSelectionEnabled(bool enabled)
{
    m_multiSelectionEnabled = enabled;
}

} // namespace data

