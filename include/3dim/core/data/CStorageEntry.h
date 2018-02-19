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

#ifndef CStorageEntry_H
#define CStorageEntry_H

#include <VPL/Base/SharedPtr.h>
#include <VPL/Base/Lock.h>
#include <VPL/Base/Exception.h>
#include <VPL/System/ScopedLock.h>
#include <VPL/Module/Signal.h>
#include <VPL/Module/Progress.h>

#include "CStorageEntryDefs.h"
#include "CEntryDeps.h"
#include "CEntryChangeLog.h"
#include "CStorableData.h"


namespace data
{

class CEntryObserver;

///////////////////////////////////////////////////////////////////////////////
//! Base class for all storage entries.

class CStorageEntry 
	: public vpl::base::CObject
	, public vpl::base::CLockableObject<CStorageEntry>
	, public vpl::mod::CSerializable
{
public:
    //! Reference counting smart pointer.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CStorageEntry);

    //! Lock type.
    typedef vpl::base::CLockableObject<CStorageEntry>::CLock tLock;

    //! Signal invoked on any change of the entry.
    typedef vpl::mod::CSignal<void, CStorageEntry *> tSignal;

    //! Reference counting smart pointer to the data object.
    typedef CStorableData::tSmartPtr tDataPtr;

public:
    //! Default constructor.
    CStorageEntry(int Id = Storage::UNKNOWN, CStorableData *pData = NULL)
        : m_Id(Id)
        , m_State(0)
        , m_Version(1)
        , m_spData(pData)
    {}

	//! Virtual destructor.
    virtual ~CStorageEntry();


    //! Returns identifier of the entry.
    int getId() const { return m_Id; }

    //! Sets the identifier.
    CStorageEntry& setId(int Id)
    {
        m_Id = Id;
        return *this;
    }


    //! Returns the current flags detailing the entry state.
    int getState() const { return m_State; }
    
    //! Checks if a given state flag is set.
    bool checkStateFlag(int Value) const { return (m_State & Value) == Value; }
    
    //! Sets the entry state.
    CStorageEntry& setState(int Flags)
    {
        m_State = Flags;
        return *this;
    }
    
    //! Sets a single state flag.
    CStorageEntry& setStateFlag(int Value)
    {
        m_State |= Value;
        return *this;
    }
    
    //! Clears specified state flags.
    CStorageEntry& clearStateFlag(int Value)
    {
        m_State &= ~Value;
        return *this;
    }

    //! Checks the dirty flag.
    bool isDirty() const { return checkStateFlag(StorageEntry::DIRTY); }
    
    //! Makes the entry dirty.
    //! - Please specify Id of a changed parent entry.
    CStorageEntry& makeDirty(int Id, int UserFlags = 0)
    {
        tLock Lock(*this);
        
        m_ChangeLog.insert(Id, UserFlags);
        return setStateFlag(StorageEntry::DIRTY);
    }
    
	//! Add changes from the list and make item dirty
	CStorageEntry& addChanges( const CChangedEntries & Changes )
	{
        tLock Lock(*this);

        m_ChangeLog.insert(Changes);
		return setStateFlag(StorageEntry::DIRTY);
	}

    //! Clears the dirty flag.
    CStorageEntry& clearDirty()
    {
        tLock Lock(*this);
        
        m_Version = m_ChangeLog.getVersion();
        return clearStateFlag(StorageEntry::DIRTY);
    }


    //! Returns the current (non-updated) version of the entry.
    unsigned getDirtyVersion() const { return m_Version; }

    //! Returns the current version of the entry.
    unsigned getLatestVersion() const { return m_ChangeLog.getVersion(); }


    //! Returns pointer to the data.
    //! - You must specify concrete type of the data.
    //! - Throws exception on failure.
    //! - Uses RTTI and dynamic_cast<>.
    //! - Non-locking version!
    template <class T>
    T *getDataPtr()
    {
        T *pData = dynamic_cast<T *>(m_spData.get());
        if( !pData )
        {
			VPL_LOG_INFO("Typecast to " << typeid(T).name() << " failed for " << (NULL==m_spData.get() || NULL==m_spData->getName()? "null" : m_spData->getName()) << " id " << this->m_Id);
            throw Storage::CTypecastFailed();
        }
        return pData;
    }

    //! Checks if the data corresponds to a given type.
    //! - Uses RTTI and dynamic_cast<>.
    template <class T>
    bool checkType()
    {
        return (dynamic_cast<T *>(m_spData.get()) != NULL);
    }

    //! Returns pointer to the data.
    //! - Returns NULL if no data exists.
    //! - Non-locking version!
    CStorableData *getStorableDataPtr()
    {
        return m_spData.get();
    }

    //! Sets pointer to the data.
    CStorageEntry& setDataPtr(CStorableData *pData)
    {
        tLock Lock(*this);

        m_spData = pData;
        return *this;
    }

    //! Locks the data.
    CStorageEntry& lockData()
    {
        m_DataLock.lock();
        return *this;
    }

    //! Unlocks the data.
    CStorageEntry& unlockData()
    {
        m_DataLock.unlock();
        return *this;
    }

    //! Returns the mutex used to lock the data.
    vpl::sys::CMutex& getDataLock()
    {
        return m_DataLock;
    }


    //! Returns reference to the list of reverse dependencies.
    CEntryDeps& getDeps() { return m_Deps; }
    const CEntryDeps& getDeps() const { return m_Deps; }

    //! Returns the list of reverse dependencies.
    CStorageEntry& getDeps(CEntryDeps& Deps)
    {
        tLock Lock(*this);

        Deps = m_Deps;
        return *this;
    }

    //! Sets the list of reverse dependencies.
    CStorageEntry& setDeps(const CEntryDeps& Deps)
    {
        tLock Lock(*this);

        m_Deps = Deps;
        return *this;
    }


    //! Returns reference to the signal invoked on any change.
    tSignal& getSignal() { return m_Signal; }
    const tSignal& getSignal() const { return m_Signal; }

    //! Registers an observer.
    void connect(CEntryObserver *pObserver);

    //! Deregisters a given observer.
    void disconnect(CEntryObserver *pObserver);

    //! Registers an observer.
    void block(CEntryObserver *pObserver);

    //! Deregisters a given observer.
    void unblock(CEntryObserver *pObserver);

    //! Returns true if the object contains a valid data.
    virtual bool hasData()
    {
        if( m_spData.get() )
        {
            vpl::sys::tScopedLock dataLock(m_DataLock);
            return m_spData->hasData();
        }
        return false;        
    }

    //! Regenerates the entry according to any changes in the data storage.
    //! - This method is called if any parent entry has changed its value.
    //! - All updates are deferred until the data are going to be accessed by an user.
    //! - Refreshes the version number and clears the dirty flag!
    void update()
    {
        if( !m_spData.get() )
        {
            return;
        }
        
        // Double check for the dirty flag to avoid race conditions
        if( isDirty() )
        {
            tLock Lock(*this);
            
            if( isDirty() )
            {
                // Retrieve changes made since the last update
                CChangedEntries Changes;
                getChanges(m_Version, Changes);

                vpl::sys::tScopedLock dataLock(m_DataLock);
                m_spData->update(Changes);
                
                // Clear the dirty flag and actualize the version number
                clearDirty();
            }
        }
    }

	//! Initializes the data to its default state.
    void init()
    {
        if( m_spData.get() )
        {
            vpl::sys::tScopedLock dataLock(m_DataLock);
            m_spData->init();
        }
    }

    //! Returns true if changes of a given parent entry may affect
    //! this object. If so, the dirty flag is set and propagated down
    //! the dependency tree.
    bool checkDependency(CStorageEntry *pParent)
    {
        // Locking should not be necessary
//        vpl::sys::tScopedLock dataLock(m_DataLock);

        return (m_spData.get()) ? m_spData->checkDependency(pParent) : true;
    }

    //! Serialize the entry data. 
    virtual void serialize(vpl::mod::CBinarySerializer & Writer) 
    {
        if( m_spData.get() )
        {
            vpl::sys::tScopedLock dataLock(m_DataLock);

            m_spData->serialize( Writer );
        }
    }

    //! Deserialize the entry data
    virtual void deserialize(vpl::mod::CBinarySerializer& Reader) 
    {
        if( m_spData.get() )
        {
            vpl::sys::tScopedLock dataLock(m_DataLock);

            m_spData->deserialize( Reader );
        }
    }

    //! Post-processing called after all entries have been deserialized. Returns true if this entry has changed and needs to be invalidated.
    virtual bool deserializationFinished()
    {
        if (m_spData.get())
        {
            vpl::sys::tScopedLock dataLock(m_DataLock);

            return m_spData->deserializationFinished();
        }

        return false;
    }

    //! Announces all observers about the change.
    //! - Invokes the internal signal.
    void notify()
    {
        // Locking should not be necessary
//        tLock Lock(*this);

        m_Signal.invoke(this);
    }

    //! Retrieves changes since a given version.
    //! - Returns false if it was not possible to retrieve a complete list of changes
    //!   due to a limited size of the log.
    bool getChanges(unsigned VersionNum, CChangedEntries& Changes)
    {
        tLock Lock(*this);

        Changes.setEntryId(m_Id);
        return m_ChangeLog.getChanges( VersionNum, Changes );
    }

    //! Retrieves changes made to the dirty entry.
    //! - Returns false if it was not possible to retrieve a complete list of changes
    //!   due to a limited size of the log.
    bool getChanges(CChangedEntries& Changes)
    {
        tLock Lock(*this);

        Changes.setEntryId(m_Id);
        return m_ChangeLog.getChanges(m_Version, Changes);
    }

protected:
    //! Identifier of the entry.
    int m_Id;

    //! Internal flags detailing current state of the entry.
    volatile int m_State;
    
    //! Counter increased on any change of the entry.
    volatile unsigned m_Version;
    
    //! Pointer to a storable data.
    tDataPtr m_spData;
    
    //! Reverse dependencies (list of all dependent/child entries).
    CEntryDeps m_Deps;
    
    //! Signal invoked on entry change.
    tSignal m_Signal;
    
    //! History of changes.
    CEntryChangeLog m_ChangeLog;
    
    //! Mutual access to entry data.
    vpl::sys::CMutex m_DataLock;
};


///////////////////////////////////////////////////////////////////////////////
//! Simple wrapper that encapsulates pointer to an object
//! as well as some additional flags.

template <typename T>
class CPtrWrapper
{
public:
    //! Type of the object.
    typedef T tEntry;

    //! Pointer to the object.
    typedef T *tEntryPtr;

public:
    //! Constructor initializes the pointer.
    CPtrWrapper(tEntryPtr p = NULL, int Flags = 0) : m_pEntry(p), m_Flags(Flags) {}

    //! Private copy constructor.
    CPtrWrapper(const CPtrWrapper& p) : m_pEntry(p.m_pEntry), m_Flags(p.m_Flags) {}

    //! Private assignment operator.
    CPtrWrapper& operator=(const CPtrWrapper& p)
    {
        m_pEntry = p.m_pEntry;
        m_Flags = p.m_Flags;
        return *this;
    }

    //! Destructor does nothing.
    ~CPtrWrapper() {}

    //! Returns pointer to the object.
    tEntryPtr get() const { return m_pEntry; }

    //! Returns pointer to the additional flags.
    int getFlags() const { return m_Flags; }

private:
    //! Pointer to the storage entry.
    tEntryPtr m_pEntry;

    //! Additional flags.
    int m_Flags;
};


} // namespace data

#endif // CStorageEntry_H
