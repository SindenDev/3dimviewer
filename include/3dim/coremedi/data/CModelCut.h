///////////////////////////////////////////////////////////////////////////////
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

#ifndef CModelCut_H
#define CModelCut_H

#include <VPL/Module/Serializable.h>
#include "data/CObjectHolder.h"
#include <osg/Geometry>
#include "data/CModelManager.h"
#include <data/storage_ids_core.h>

namespace data
{

////////////////////////////////////////////////////////////
/*!
 * Base class for storing cut through model
 */
class CModelCut : public vpl::base::CObject
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CModelCut);

protected:
    osg::ref_ptr<osg::Vec3Array> m_vertices;
    osg::ref_ptr<osg::DrawElementsUInt> m_indices;
    //! Id of model that is cut
    int m_modelId;
    //! Transformation matrix
    osg::Matrix m_transformMatrix;
    data::CColor4f m_color;

public:
    //! Default constructor.
    CModelCut();

    //! Copy constructor.
    CModelCut(const CModelCut &modelCut);

    //! Destructor.
    virtual ~CModelCut();

    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries &changedEntries);

	//! Initializes the object to its default state.
    void init();

    //! Returns transformation correction matrix (identity in most cases)
    const osg::Matrix& getTransformationMatrix() { return m_transformMatrix; }

    //! Sets model storage id
    void setModelId(int storageID) { m_modelId = storageID; }

    data::CColor4f getColor() { return m_color; }

protected:
	//! Clears geometry data
    void clear();

public:
    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry *pParent)
    {
        return true;
    }

    //! Does object contain relevant data?
    bool hasData()
    {
        return true;
    }

    //! Returns vertices
    osg::Vec3Array *getVertices()
    {
        return m_vertices;
    }

    //! Returns indices
    osg::DrawElementsUInt *getIndices()
    {
        return m_indices;
    }
};


////////////////////////////////////////////////////////////
/*!
 * Cut by XY slice
 */
class CModelCutSliceXY : public CModelCut
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CModelCutSliceXY);

    //! Ctor
    CModelCutSliceXY()
    { }

    //! Dtor
    virtual ~CModelCutSliceXY()
    { }

    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries &changedEntries);
};

////////////////////////////////////////////////////////////
/*!
 * Cut by XZ slice
 */
class CModelCutSliceXZ : public CModelCut
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CModelCutSliceXZ);

    //! Ctor
    CModelCutSliceXZ()
    { }

    //! Dtor
    virtual ~CModelCutSliceXZ()
    { }

    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries &changedEntries);
};

////////////////////////////////////////////////////////////
/*!
 * Cut by YZ slice
 */
class CModelCutSliceYZ : public CModelCut
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CModelCutSliceYZ);

    //! Ctor
    CModelCutSliceYZ()
    { }

    //! Dtor
    virtual ~CModelCutSliceYZ()
    { }

    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries &changedEntries);
};


namespace Storage
{
	//! Cuts through model (watch reserved space for cuts of MAX_IMPORTED_MODELS)
	DECLARE_OBJECT(ImportedModelCutSliceXY, data::CModelCutSliceXY, CORE_STORAGE_IMPORTED_MODEL_CUTSLICE_XY_ID);
	DECLARE_OBJECT(ImportedModelCutSliceXZ, data::CModelCutSliceXZ, CORE_STORAGE_IMPORTED_MODEL_CUTSLICE_XZ_ID);
	DECLARE_OBJECT(ImportedModelCutSliceYZ, data::CModelCutSliceYZ, CORE_STORAGE_IMPORTED_MODEL_CUTSLICE_YZ_ID);
}
} // namespace data

#endif // CModelCut_H
