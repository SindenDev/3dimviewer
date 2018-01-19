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

#ifndef CStorageInterface_H
#define CStorageInterface_H

#include <VPL/Base/SharedPtr.h>

#include "CDataStorage.h"
#include "CObjectHolder.h"
#include "CObjectObserver.h"
#include "CObjectPtr.h"
#include "CDummyStorable.h"


namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Useful macros

//! Helper macro used to declare data type of an storage entry and it's identifier.
//! - The CObjectHolder<MyType> class is used!
#define DECLARE_OBJECT(Name, MyType, MyId) \
    struct Name \
    { \
        enum { Id = MyId }; \
        typedef CObjectHolder< MyType > Type; \
    }

// Usage:
//   DECLARE_OBJECT(MyData, CMyData, 123);
//   ...
//   STORABLE_FACTORY.registerObject(Storage::MyData::Id, CMyData::create, CEntryDeps() | Storage::MyParentData::Id);
//   ...
//   CObjectPtr<CMyData> spData = APP_STORAGE.getEntryPtr(Storage::MyData::Id);


///////////////////////////////////////////////////////////////////////////////
// Storage entries

namespace Storage
{
    //! Dummy data...
    DECLARE_ENTRY(DummyStorable, CDummyStorable, 1);
}


///////////////////////////////////////////////////////////////////////////////
//! Initializes the application storage and provides basic functionality.

class CStorageInterface : public vpl::base::CObject
{
public:
    //! Data storage...
    typedef CDataStorage tStorage;

    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CStorageInterface);

public:
	//! Constructor.
    //! - Calls the init() method.
    CStorageInterface(tStorage& Storage = APP_STORAGE) : m_Storage(Storage)
    {
        init();
    }

	//! Virtual destructor.
    virtual ~CStorageInterface() {}

    //! Initializes the storage.
    //! - Registers creation functions of all storage entries.
    //! - Non-virtual method!
    void init()
    {
        // Register storable objects
        STORABLE_FACTORY.registerObject(Storage::DummyStorable::Id, CDummyStorable::create);
    }

    //! Returns reference to the data storage.
    tStorage& getStorage() { return m_Storage; }
    const tStorage& getStorage() const { return m_Storage; }

protected:
    //! Reference to the data storage.
    tStorage& m_Storage;
};


} // namespace data

#endif // CStorageInterface_H

