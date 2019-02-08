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

#ifndef CModelManager_H
#define CModelManager_H

#include "data/CStorageInterface.h"
#include "CModel.h"
#include <data/ESnapshotType.h>
#include <data/CColorTable.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Storage entries...

namespace Storage
{

//! Reference anatomical landmark model.
DECLARE_OBJECT(ReferenceAnatomicalLandmarkModel, CModel, 2490);

//! Bone tissue model.
DECLARE_OBJECT(BonesModel, CModel, 2500);

//! Soft tissues model (not used yet).
DECLARE_OBJECT(SoftTissuesModel, CModel, 2501);

//! Optional mucosa/imprint model...
DECLARE_OBJECT(ImprintModel, CModel, 2502);

//! Optional model...
DECLARE_OBJECT(TemplateModel, CModel, 2503);

//! Optional model...
DECLARE_OBJECT(ImportedModel, CModel, 2504);

} // namespace Storage

#define MAX_IMPORTED_MODELS 96
#define OTHER_MODELS 4
#define MAX_MODELS  OTHER_MODELS + MAX_IMPORTED_MODELS

/******************************************************************************
        CLASS CModelSnapshot
******************************************************************************/

class CModelSnapshot
        : public data::CSnapshot
{
public:
        //! Constructor
        CModelSnapshot( CUndoProvider * provider = nullptr ) : CSnapshot( data::UNDO_MODELS, provider ) {}

        //! Each snapshot object must return its data size in bytes
        virtual long getDataSize() override
        {
            return sizeof( CModel ) * m_modelMap.size(); // up to MAX_MODELS
        }

protected:
        //! map of stored models and their ids
        std::map<int,CModel> m_modelMap;

        // friend class - undo provider
        friend class CModelManager;

}; // class CModelSnapshot

///////////////////////////////////////////////////////////////////////////////
//! Manager of surface models.

class CModelManager : public CStorageInterface, public CUndoProvider
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CModelManager);

public:
    //! Constructor
    CModelManager();

    //! Virtual destructor.
    virtual ~CModelManager();

    //! Initializes the manager.
    //! - Registers creation functions of all storage entries.
    //! - Non-virtual method! It must be called manually somewhere in the class constructor.
    void init();

    //! Initializes mesh undo provider for specified model
    void initMeshUndoProvider(int storageId);

    //! Sets model geometry.
    //! - Method can be called directly, or using appropriate signal.
    void setModel(int id, geometry::CMesh * pMesh);

    //! Test if given model id is valid
    static bool validModelId(int id) 
    {
        return (id >= Storage::BonesModel::Id && id < Storage::BonesModel::Id + MAX_MODELS);
    }

    //! Selects model
    void removeModel(int id);

    //! Selects model
    void selectModel(int id);

    //! Returns ID of selected model
    int getSelectedModel() const
    {
        return m_selectedModel;
    }

    //! Returns true if a specified model is shown.
    //! - Method can be called directly, or using appropriate signal.
    bool isModelShown(int id);

    //! Sets visibility of the model.
    //! - Method can be called directly, or using appropriate signal.
    void setModelVisibility(int id, bool visible);


    //! Sets model color.
    void setModelColor(int id, float r, float g, float b, float a = 1.0f);

    //! Sets model color.
    //! - Method can be called directly, or using appropriate signal.
    void setModelColor2(int id, const CColor4f& Color);

    //! Returns model color.
    void getModelColor(int id, float& r, float& g, float& b, float& a);

    //! Returns model color.
    //! - Method can be called directly, or using appropriate signal.
    CColor4f getModelColor2(int id);

    // undo provider interface

    //! Create snapshot of the current state. Parameter snapshot
    //! is NULL, or old snapshot.
    virtual CSnapshot * getSnapshot( CSnapshot * snapshot );

    //! Create snapshot of the current state. Parameter snapshot
    //! is NULL, or old snapshot.
    virtual CSnapshot * getSnapshot( CSnapshot * snapshot, std::vector<int> modelList, bool bIncludeMesh );

    //! Restore state from the snapshot
    virtual void restore( CSnapshot * snapshot );

    //! Creates and stores snapshot. Optional "child" snapshot, takes list of ids, pass empty list as all
    void createAndStoreSnapshot(std::vector<int> modelIDs, bool bIncludeMesh = false, CSnapshot *childSnapshot = NULL);

	//! Update links between models after load
	static void updateModelLinks();

	//! Just for debugging - write models info
	static void writeLinks();

	//! Get base model of the given model
	static std::string getBaseModel(int storage_id);

    //! Try to find base model id
    static int getBaseModelId(int model_id);

	//! Does model with this uid exist?
	static bool modelExists(const std::string &uid);

	//! Get model storage id 
	static int uidToStorageId(const std::string &uid);

	//! Get model uid from storage id
	static std::string storageIdToUId(int storage_id);

    //! Is current model base?
    static bool isBaseModel(int storage_id);

    //! Orpedigs stuff - enables multi selection of models - the previously selected models are just not deselected
    void setMultiSelectionEnabled(bool enabled);

    static int findFreeModelId();

    /**
     * Automatic assign model color
     *
     * \param   modelId         Identifier for the model.
     * \param   doInvalidation  (Optional) True to do storage invalidation automatically.
     */

    void autoAssignModelColor(int modelId, bool doInvalidation = true);

protected:
    //! Dummy model.
    CModel m_DummyModel;

    //! flag if transparency is needed
    bool m_transparencyNeeded;

    //! Selected implant
    int m_selectedModel;

    //! Multi selection of models en/disabled.
    bool m_multiSelectionEnabled;

    data::CColorTable m_colorTable;

private:
    //! Private copy constructor.
    CModelManager(const CModelManager&);

    //! Private assignment operator.
    CModelManager& operator =(const CModelManager&);

    //! Updates flag if transparency is needed
    void updateTransparencyNeeded();

public:
    //! signals "TransparencyNeededChange" if needed
    void resolveTransparency();
};


} // namespace data

#endif // CModelManager_H

