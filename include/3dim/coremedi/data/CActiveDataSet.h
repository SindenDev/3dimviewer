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

#ifndef CActiveDataSet_H
#define CActiveDataSet_H

#include <VPL/Base/SharedPtr.h>

#include "data/CObjectHolder.h"
#include "data/CStorageEntry.h"

#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Class encapsulates id of an active data set.

class CActiveDataSet : public vpl::base::CObject
{
public:
    //! Smart pointer type.
    VPL_SHAREDPTR(CActiveDataSet);

public:
	//! Default constructor
    CActiveDataSet(int Id = PATIENT_DATA) : m_Id(Id) {}

	//! Destructor.
    ~CActiveDataSet() {}

    //! Returns identifier of the selected/active data set.
    int getId() const { return m_Id; }

    //! Sets the active data set.
    void setId(int Id) { m_Id = Id; }

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& VPL_UNUSED(Changes))
    {
        // Does nothing...
    }

    //! Does object contain relevant data?
    virtual bool hasData(){ return false; }

	//! Initializes the object to its default state.
    void init()
    {
        m_Id = PATIENT_DATA;
    }

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry *pParent)
    {
        // Check if active data set has been changed
        return (pParent->getId() == m_Id);
    }

protected:
    //! Active data set.
    int m_Id;
};

namespace Storage
{
	//! Active data set.
	DECLARE_OBJECT(ActiveDataSet, CActiveDataSet, CORE_STORAGE_ACTIVE_DATASET_ID);

}

} // namespace data

#endif // CActiveDataSet_H

