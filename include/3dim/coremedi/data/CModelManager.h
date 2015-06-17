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

#ifndef CModelManager_H
#define CModelManager_H

#include "data/CStorageInterface.h"
#include "CModel.h"
#include <data/CSnapshot.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Storage entries...

namespace Storage
{

//! Bone tissue model.
DECLARE_OBJECT(BonesModel, CModel, 801);

//! Soft tissues model (not used yet).
DECLARE_OBJECT(SoftTissuesModel, CModel, 802);

//! Optional mucosa/imprint model...
DECLARE_OBJECT(ImprintModel, CModel, 803);

//! Optional model...
DECLARE_OBJECT(TemplateModel, CModel, 804);

//! Optional model...
DECLARE_OBJECT(ImportedModel, CModel, 805);

} // namespace Storage

#define MAX_IMPORTED_MODELS 20

#define MAX_MODELS  4 + MAX_IMPORTED_MODELS

/******************************************************************************
        CLASS CModelSnapshot
******************************************************************************/

class CModelSnapshot
        : public data::CSnapshot
{
public:
        //! Constructor
        CModelSnapshot( CUndoProvider * provider = NULL ) : CSnapshot( data::UNDO_MODELS, provider ) {}

        //! Each snapshot object must return its data size in bytes
        virtual long getDataSize() {return sizeof( CModel ) * MAX_MODELS; }

protected:
        //! Array of implants
        CModel m_modelArray[ MAX_MODELS ];

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

    //! Restore state from the snapshot
    virtual void restore( CSnapshot * snapshot );

    //! Creates and stores snapshot.
    void createAndStoreSnapshot();

protected:
    //! Dummy model.
    CModel m_DummyModel;

    //! flag if transparency is needed
    bool m_transparencyNeeded;

    //! Selected implant
    int m_selectedModel;

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

