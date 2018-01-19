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

#ifndef CObjectPtr_H
#define CObjectPtr_H

#include "CEntryPtr.h"

#include <VPL/Base/Logging.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Global variables

namespace ObjectPtr
{
    //! Helper enum value used to initialize CObjectPtr<> to NULL.
    enum EInitNull { InitNull = 0 };
}


///////////////////////////////////////////////////////////////////////////////
//! Locking smart pointer to an object stored in data storage.
//! - You must specify a concrete object type T.
//! - Only for objects encapsulated in COnjectHolder<T> class!
//!

// Usage:
//   try {
//       CObjectPtr<CMyObject> spData( APP_STORAGE.getEntryPtr(MyObjectId) );
//       ...
//   }
//   catch( const vpl::base::CException& )
//   {
//       ...
//   }

template <class T, class LockingPolicy = CDefaultLocking>
class CObjectPtr
{
public:
    //! Reference counting smart pointer to the storage entry.
    typedef CStorageEntry::tSmartPtr tEntryPtr;
    
    //! Storable data type.
    typedef CObjectHolder<T> tObjectHolder;
    
    //! Locking policy.
    typedef LockingPolicy tLockingPolicy;

public:
    //! Constructor initializes the smart pointer and locks the storage entry.
    //! - Throws exception on failure!
    CObjectPtr(CStorageEntry *p) : m_spEntry(p), m_pObject(NULL)
    {
        if( !m_spEntry.get() )
        {
            throw Storage::CNullEntry();
        }

        tLockingPolicy::lock(m_spEntry.get());
        m_pObject = m_spEntry->getDataPtr<tObjectHolder>()->getObjectPtr();
    }

    //! Constructor initializes the smart pointer and locks the storage entry.
    //! - The entry is updated if it is marked as dirty.
    //! - Throws exception on failure!
    CObjectPtr(const CPtrWrapper<CStorageEntry>& p) : m_spEntry(p.get()), m_pObject(NULL)
    {
        if( !m_spEntry.get() )
        {
            throw Storage::CNullEntry();
        }

        tLockingPolicy::lock(m_spEntry.get());
        m_pObject = m_spEntry->getDataPtr<tObjectHolder>()->getObjectPtr();

        // Lazy update of the entry
        if( p.getFlags() & Storage::UPDATE )
        {
            doUpdate();
        }
    }

    //! Constructor initializes the smart pointer to NULL.
    //! - Use with care!
    CObjectPtr(ObjectPtr::EInitNull) : m_spEntry(NULL), m_pObject(NULL) {}

    //! Destructor unlocks the storage entry.
    ~CObjectPtr()
    {
        if( m_spEntry.get() )
        {
            tLockingPolicy::unlock(m_spEntry.get());
        }
    }

    //! Assignment operator.
    CObjectPtr& operator=(CStorageEntry *pEntry)
    {
        if( !pEntry )
        {
            throw Storage::CNullEntry();
        }

        release();
        m_spEntry = pEntry;

        tLockingPolicy::lock(m_spEntry.get());
        m_pObject = m_spEntry->getDataPtr<tObjectHolder>()->getObjectPtr();

        return *this;
    }

    //! Assignment operator.
    CObjectPtr& operator=(const CPtrWrapper<CStorageEntry>& p)
    {
        if( !p.get() )
        {
            throw Storage::CNullEntry();
        }

        release();
        m_spEntry = p.get();

        tLockingPolicy::lock(m_spEntry.get());
        m_pObject = m_spEntry->getDataPtr<tObjectHolder>()->getObjectPtr();

        // Lazy update of the entry
        if( p.getFlags() & Storage::UPDATE )
        {
            doUpdate();
        }

        return *this;
    }

    //! Returns pointer to the object.
    T *operator->() { return m_pObject; }

    //! Returns reference to the object.
    T& operator*() { return *m_pObject; }

    //! Returns pointer to the object.
    T *get() { return m_pObject; }

    //! Returns pointer to the storage entry.
    CStorageEntry *getEntryPtr() { return m_spEntry.get(); }

    //! Unlocks the data and releases the pointer.
    void release()
    {
        if( m_spEntry.get() )
        {
            m_pObject = NULL;
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
            VPL_LOG_INFO("CObjectPtr::doUpdate(): " << Exception.what() << ", Id = " << m_spEntry->getId());
        }
    }

private:
    //! Pointer to the storage entry.
    tEntryPtr m_spEntry;

    //! Pointer to the object.
    T *m_pObject;

    //! Private copy constructor.
    CObjectPtr(const CObjectPtr& p);

    //! Private assignment operator.
    CObjectPtr& operator=(const CObjectPtr& p);
};


} // namespace data

#endif // CObjectPtr_H

