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

#ifndef CStorableData_H
#define CStorableData_H

#include <VPL/Base/SharedPtr.h>
#include <VPL/Module/Serializable.h>
#include <VPL/Module/BinarySerializer.h>
#include "CChangedEntries.h"

namespace data
{

class CStorageEntry;

///////////////////////////////////////////////////////////////////////////////
// Global definitions

//! Helper macro used to declare data type of an storage entry
//! (= storable data) and it's identifier.
#define DECLARE_ENTRY(Name, MyType, MyId) \
    struct Name \
    { \
        enum { Id = MyId }; \
        typedef MyType Type; \
    }

// Usage:
//   DECLARE_ENTRY(MyData, CMyData, 123);
//   ...
//   STORABLE_FACTORY.registerObject(MyData::Id, CMyData::create);
//   ...
//   CEntryPtr<CMyData> spData = APP_STORAGE.getEntryPtr(MyData::Id);

///////////////////////////////////////////////////////////////////////////////
//! Base class for all objects that can be stored in the data storage.
//! - All storable objects have to declare a reference counting smart pointer!
//!   Please, use the VPL_SHAREDPTR() macro to define the tSmartPtr type.

class CStorableData : public vpl::base::CObject, public vpl::mod::CSerializable
{
public:
    //! Simple error checking...
    enum
    {
        STORABLE_DATA,
    };

    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CStorableData);

public:
    //! Object creation function.
    //! - Each derived object has to declare creation function like this.
    static CStorableData *create();

    //! Virtual destructor.
    virtual ~CStorableData()
    { }

    //! Regenerates the object state according to any changes in the data storage.
    //! - This method is called if any parent object has changed its value.
    //! - All updates are deferred until the data are going to be accessed.
    //! - The entry is locked automatically.
    virtual void update(const CChangedEntries &Changes) = 0;

    //! Initializes the object to its default state.
    //! - The entry is locked automatically.
    virtual void init() = 0;

    //! Returns true if changes of a given parent entry may affect
    //! this object. If so, the dirty flag is set and propagated down
    //! the dependency tree.
    //! - The entry is locked automatically.
    virtual bool checkDependency(CStorageEntry * VPL_UNUSED(pParent))
    {
        return true;
    }

    //! Returns true if the object contains a valid data.
    virtual bool hasData()
    {
        return true;
    }

    //! Serialize. 
    virtual void serialize(vpl::mod::CBinarySerializer & VPL_UNUSED(Writer))
    { }

    //! Deserialize
    virtual void deserialize(vpl::mod::CBinarySerializer & VPL_UNUSED(Reader))
    { }

    //! Deserialize
    virtual bool deserializationFinished()
    {
        return false;
    }
};

} // namespace data

#endif // CStorableData_H
