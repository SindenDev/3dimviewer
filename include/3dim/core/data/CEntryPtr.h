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

#ifndef CEntryPtr_H
#define CEntryPtr_H

#include "CStorageEntry.h"
#include "CDataStorage.h"

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Base class for locking policy of smart pointers to storage entries.

struct CLockingPolicy {};


///////////////////////////////////////////////////////////////////////////////
//! Default locking policy for entry smart pointer.

struct CDefaultLocking : public CLockingPolicy
{
    //! Locks the entry.
    static void lock(CStorageEntry *pEntry)
    { 
        pEntry->lockData();
    }

    //! Unlocks the entry.
    static void unlock(CStorageEntry *pEntry)
    {
        pEntry->unlockData();
    }
};


///////////////////////////////////////////////////////////////////////////////
//! Non-locking policy.

struct CNoLocking : public CLockingPolicy
{
    //! Locks the entry.
    static void lock(CStorageEntry * VPL_UNUSED(pEntry)) {}

    //! Unlocks the entry.
    static void unlock(CStorageEntry * VPL_UNUSED(pEntry)) {}
};


///////////////////////////////////////////////////////////////////////////////
//! Locking smart pointer to storage entry.
//! - You must specify a type derived from the CStorableData class
//!   as the template parameter T.

// Usage:
//   try {
//       CEntryPtr<CMyStorableData> spData = APP_STORAGE.getEntryPtr(MyDataId);
//       ...
//   }
//   catch( vpl::base::CException& )
//   {
//       ...
//   }

template <class T, class LockingPolicy = CDefaultLocking>
class CEntryPtr
{
public:
    //! Reference counting smart pointer to the storage entry.
    typedef CStorageEntry::tSmartPtr tEntryPtr;

    //! Locking policy.
    typedef LockingPolicy tLockingPolicy;

public:
    //! Constructor initializes the smart pointer and locks the storage entry.
    //! - Throws exception on failure!
    CEntryPtr(CStorageEntry *p) : m_spEntry(p), m_pData(NULL)
    {
        if( !m_spEntry.get() )
        {
            throw Storage::CNullEntry();
        }

        tLockingPolicy::lock(m_spEntry.get());
        m_pData = m_spEntry->getDataPtr<T>();
    }

    //! Constructor initializes the smart pointer and locks the storage entry.
    //! - The entry is updated if it is marked dirty.
    //! - Throws exception on failure!
    CEntryPtr(const CPtrWrapper<CStorageEntry>& p) : m_spEntry(p.get()), m_pData(NULL)
    {
        if( !m_spEntry.get() )
        {
            throw Storage::CNullEntry();
        }

        tLockingPolicy::lock(m_spEntry.get());
        m_pData = m_spEntry->getDataPtr<T>();

        if( p.getFlags() & Storage::UPDATE )
        {
            doUpdate();
        }
    }

    //! Destructor unlocks the storage entry.
    ~CEntryPtr()
    {
        if( m_spEntry.get() )
        {
            tLockingPolicy::unlock(m_spEntry.get());
        }
    }

    //! Returns pointer to storage entry data.
    T *operator->() { return m_pData; }

    //! Returns reference to storage entry data.
    T& operator*() { return *m_pData; }

    //! Returns pointer to storage entry data.
    T *get() { return m_pData; }

    //! Returns pointer to the storage entry.
    CStorageEntry *getEntryPtr() { return m_spEntry.get(); }

    //! Unlocks the data and releases the pointer.
    void release()
    {
        if( m_spEntry.get() )
        {
            m_pData = NULL;
            tLockingPolicy::unlock(m_spEntry.get());
            m_spEntry = NULL;
        }
    }

protected:
    // Updates the entry if it is dirty.
    void doUpdate()
    {
        if( !m_spEntry.get() )
        {
            return;
        }

        try {
            m_spEntry->update();
        }
        catch( const vpl::base::CException& Exception )
        {
            VPL_LOG_INFO("CEntryPtr::doUpdate(): " << Exception.what() << ", Id = " << m_spEntry->getId());
        }
    }

private:
    //! Pointer to the storage entry.
    tEntryPtr m_spEntry;

    //! Pointer to the data.
    T *m_pData;

    //! Private copy constructor.
    CEntryPtr(const CEntryPtr& p);

    //! Private assignment operator.
    CEntryPtr& operator=(const CEntryPtr& p);
};


} // namespace data

#endif // CEntryPtr_H
