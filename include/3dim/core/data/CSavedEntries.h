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

#ifndef CSavedEntries_H
#define CSavedEntries_H

#include <data/CDataStorage.h>
#include <set>
#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

///////////////////////////////////////////////////////////////////////////////
//! Storage entries that will be saved on workspace serialization.

namespace data
{

class CSavedEntries: public vpl::base::CObject
{
public:
    //! Smart pointer type.
    VPL_SHAREDPTR(CSavedEntries);

protected:
    //! ID type
    typedef int tId;

public:
    //! Vector of the ids type
    typedef std::set< tId > tIdVector;

public:
    //! Default constructor.
    CSavedEntries() {}

    //! Does object contain relevant data?
    virtual bool hasData(){ return m_ids.size() > 0; }

    //! Regenerates the object state according to any changes in the data storage.
    void update(const data::CChangedEntries& VPL_UNUSED(Changes))
    {
    }

    //! Add new id to serialize
    void addId( tId id ) { m_ids.insert( id ); }

    //! Cear array
    // void clear(){ m_ids.clear(); }

    const tIdVector & getIds() { return m_ids; }

    //! Initializes the object to its default state.
    virtual void init() {};

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(data::CStorageEntry * VPL_UNUSED(pParent)) { return true; }

protected:
    //! Saved entries vector
    tIdVector m_ids;

    friend class CSerializationManager;
};

namespace Storage
{
	//! Add saved entries vector to the storage
	DECLARE_OBJECT(SavedEntries, CSavedEntries, CORE_STORAGE_SAVED_ENTRIES_ID );
}

} // namespace data

#endif // CSavedEntries_H
