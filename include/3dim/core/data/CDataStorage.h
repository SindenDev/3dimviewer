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

#ifndef CDataStorage_H
#define CDataStorage_H

#include <VPL/Base/Exception.h>
#include <VPL/Base/Singleton.h>
#include <VPL/Base/Lock.h>
#include <VPL/System/Thread.h>
#include <VPL/System/Condition.h>

// Serialization
#include <VPL/Module/Serializable.h>
#include <VPL/Module/Serializer.h>

#include <configure.h>

#include "CStorageEntry.h"
#include "CStorableFactory.h"
#include "CObjectHolder.h"


namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Useful macros

//! Returns reference to the application data storage.
#define APP_STORAGE     VPL_SINGLETON(data::CDataStorage)


///////////////////////////////////////////////////////////////////////////////
// Global definitions.

namespace Storage
{

//! Exception thrown on an unknown entry.
VPL_DECLARE_EXCEPTION(CUnknowEntry, "Failed to recognize a storage entry")

//! Maximal allowed value of an entry identifier.
enum { MAX_ID = 3500 };

//! This flags can be passed to the invalidate() method.
enum EInvalidateFlags
{
    //! Flag used to enforce update() of all dependent entries during
    //! the invalidation of an entry.
    FORCE_UPDATE            = 1 << 22,

    //! Flag used to notify entries that data storage is beeing reset.
    STORAGE_RESET           = 1 << 23
};

//! This flags can be passed to the findEntry() method of the data storage.
enum ELookupFlags
{
    //! If the entry doesn't exist, it will be created and initialized automatically.
    CREATE      = 1 << 3,

    //! If the entry is marked as dirty, it will be updated automatically.
    UPDATE      = 1 << 4,

    //! Default lookup flags.
    CRUP        = CREATE | UPDATE,

    //! Create the entry automatically but don't update it.
    NO_UPDATE   = CREATE
};

} // namespace Storage


///////////////////////////////////////////////////////////////////////////////
//! Data storage
//! - Creates the entry data automatically on first access.
//! - Once the data object is created, its pointer never changes!

class CDataStorage
    : public vpl::base::CSingleton<vpl::base::SL_LONG>
    , public vpl::base::CLibraryLockableClass<CDataStorage>
	, public vpl::mod::CSerializable
{
public:
    //! Scoped lock.
    typedef vpl::base::CLibraryLockableClass<CDataStorage>::CLock tLock;

    //! Signal invoked on any change of an entry.
    typedef CStorageEntry::tSignal tSigEntryChanged;

	//! Default class name.
    VPL_ENTITY_NAME("CDataStorage");

public:
    //! Destructor.
    ~CDataStorage() {}

    //! Checks if a given entry Id is valid.
    bool checkId(int Id) { return (Id > Storage::UNKNOWN && Id < Storage::MAX_ID); }

    //! Sets a specific storable objects factory to be used by the storage.
    //! - Please, use the NULL value to use the default factory accessible
    //!   via the STORABLE_FACTORY macro.
    void changeFactory(CStorableFactory *pFactory = NULL) { m_pFactory = pFactory; }


    //! Returns pointer to a storage entry.
    //! - Clear the data::storage::UPDATE flag, if you want to modify the entry state
    //!   regardless its current data. This will reduce unnecessary overhead
    //!   (i.e. you can change position of an ortho slice multiple times,
    //!   while the internal image data will be updated once when the OSG renders
    //!   any scene visualizing the slice).
    //! - Throws exception on failure.
    //! - Uses RTTI and dynamic_cast<> operator.
    CPtrWrapper<CStorageEntry> getEntry(int Id, int Flags = Storage::CRUP);
    
    //! Returns true if a specified entry exists, and its data are valid.
    //! - Doesn't throws any exception!
    bool isEntryValid(int Id);

    //! Finds a storage entry containing data of a specified type
    //! that is present in a given list of changed entries.
    //! - Throws exception on failure.
    //! - Uses RTTI and dynamic_cast<> operator.
    //! - Bottom-up implementation (the root item is proccesed last).
    template <class T>
    inline CPtrWrapper<CStorageEntry> findEntry(const CChangedEntries& Changes, int Flags = Storage::UPDATE);

    //! Finds a storage entry containing data of a specified type
    //! that is present in a given list of changed entries.
    //! - Throws exception on failure.
    //! - Uses RTTI and dynamic_cast<> operator.
    //! - Bottom-up implementation (the root item is proccesed last).
    template <class T>
    inline int findEntryId(const CChangedEntries& Changes);

    //! Finds a storage entry containing object of a specified type
    //! that is present in a given list of changed entries.
    //! - Throws exception on failure.
    //! - Uses RTTI and dynamic_cast<> operator.
    //! - Bottom-up implementation (the root item is proccesed last).
    template <class T>
    inline CPtrWrapper<CStorageEntry> findObject(const CChangedEntries& Changes, int Flags = Storage::UPDATE);

    //! Increases version of a given entry, invalidates all dependent entries,
    //! and notifies all observers (including observers of dependent entries)
    //! about the change.
    //! - The method must be called to your changes take effect.
    void invalidate(CStorageEntry *pEntry, int Flags = 0);

    //! Re-initializes the storage.
    //! - Calls init() method of all existing storage entries.
    //! - Invalidates all entries and notifies registered observers.
    void reset();


    //! Registers an entry observer.
    void connect(int Id, CEntryObserver *pObserver);

    //! De-registers an entry observer.
    void disconnect(int Id, CEntryObserver *pObserver);

    //! Blocks an entry observer.
    void block(int Id, CEntryObserver *pObserver);

    //! Unblocks an entry observer.
    void unblock(int Id, CEntryObserver *pObserver);

    //! Returns reference to a signal invoked on an entry change.
    //! - Can be used to register any callback function.
    //! - Throws exception on failure!
    inline tSigEntryChanged& getEntrySignal(int Id);

	//! Serialize
	template <class S>
	void serialize(vpl::mod::CChannelSerializer<S>& Writer);

	//! Deserialize
	template <class S>
	void deserialize(vpl::mod::CChannelSerializer<S>& Reader);

	//! Stop sending invalidation signals
	void lockInvalidation();

	//! Start sending invalidation signals, invalidate
	void unlockInvalidation();

	//! Is invalidation locked
	bool invalidationLocked() const { return !m_bCanInvalidate; }

protected:
	//! Invalidated entries map
	typedef std::map< int, CChangedEntries > tInvalidatedEntriesMap;
	typedef std::vector< int > tInvalidatedOrderVec;

    //! Container of all entries.
    typedef std::vector<CStorageEntry::tSmartPtr> tStorage;

    //! Container representing the data storage.
    tStorage m_Storage;

    //! User-specific factory of storable objects.
    CStorableFactory *m_pFactory;

	//! Is storage invalidation disabled
	bool m_bCanInvalidate;

	//! Invalidated entries vector - used for postponed invalidation
	tInvalidatedEntriesMap m_invalidatedEntries, m_invalidatedDeps;

	//! Invalidated entries order
	tInvalidatedOrderVec m_invalidationOrder;

protected:
    //! Returns reference to a factory of storable objects.
    inline CStorableFactory& getFactory();

    //! Constructs reverse dependencies.
    void createReverseDeps(int Id, const CEntryDeps& Deps);

    //! Walks recursively through the dependency tree and invalidates entries.
    //! - Returns list of all invalidated entries.
    void invalidateDeps(CStorageEntry *pRoot, CEntryDeps& List, int Flags = 0);

	//! Do a postponed invalidation
	void doPostponedInvalidation(bool bForceUpdate = true);

	//! Do "fake" (postponed) invalidation
	void fakeInvalidate(CStorageEntry *pEntry, int Flags = 0);

	//! Walks recursively through the dependency tree and stores invalidated entries.
	void fakeInvaldateDeps(CStorageEntry *pRoot, int Flags = 0);

	//! Store dependent entries to the list
	void storeDependencies(CStorageEntry *pRoot, CEntryDeps& List);

private:
    //! Private constructor.
    //! - Initializes the vector of storage entries.
    CDataStorage();

    //! Allow factory instantiation using singleton holder
    VPL_PRIVATE_SINGLETON(CDataStorage);

private:
    //! Update all found dependent entries
    struct SUpdateDeps
    {
        SUpdateDeps(tStorage& Storage) : m_Storage(Storage) {}

        void operator =(const SUpdateDeps &) {}
        
        void operator() (int i)
        {
            try {
                m_Storage[i]->update();
            }
            catch( const vpl::base::CException& /*Exception*/ )
            {
//                VPL_LOG_INFO("CDataStorage::invalidate(): " << Exception.what() << ", Id = " << m_Storage[i]->getId());
            }
        }

        tStorage& m_Storage;
    };

    //! Notify all found dependent entries
    struct SNotifyDeps
    {
        SNotifyDeps(tStorage& Storage) : m_Storage(Storage) {}

        void operator =(const SNotifyDeps &) {}

        void operator() (int i)
        {
            m_Storage[i]->notify();
        }

        tStorage& m_Storage;
    };

    //! Make dependencies dirty
    struct SDirtyDeps
    {
        SDirtyDeps(tStorage& Storage, int Id, int Flags = 0)
            : m_Storage(Storage)
            , m_Id(Id) 
            , m_Flags(Flags)
        {}

        void operator =(const SDirtyDeps &) {}

        void operator() (int i)
        {
            m_Storage[i]->makeDirty(m_Id, m_Flags);
        }

        tStorage& m_Storage;
        int m_Id, m_Flags;
    };

    //! Create dependencies
    struct SBuildDeps
    {
        SBuildDeps(tStorage& Storage, int Id) : m_Storage(Storage), m_Id(Id) {}

        void operator =(const SBuildDeps &) {}

        void operator() (int i)
        {
            m_Storage[i]->getDeps().insert(m_Id);
        }

        tStorage& m_Storage;
        int m_Id;
    };
};


///////////////////////////////////////////////////////////////////////////////
//

#include "CDataStorage.hxx"

} // namespace data

#endif // CDataStorage_H

