///////////////////////////////////////////////////////////////////////////////
// $Id:$
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

#ifndef CEntryChangeLog_H
#define CEntryChangeLog_H

#include "CStorageEntryDefs.h"
#include "CChangedEntries.h"
#include "CCircularBuffer.h"


namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Global definitions

namespace StorageEntry
{

//! Maximal allowed size of the history.
const int HISTORY_SIZE = 1000;

} // namespace StorageEntry


///////////////////////////////////////////////////////////////////////////////
//! Structure stored as a record of changes made to an entry.

class CEntryRecord
{
public:
    //! Default constructor.
    CEntryRecord() : m_ParentId(Storage::UNKNOWN), m_Flags(0) {}

    //! Constructor.
    CEntryRecord(int ParentId, int Flags = 0) : m_ParentId(ParentId), m_Flags(Flags) {}

    //! Copy constructor.
    CEntryRecord(const CEntryRecord& r) 
        : m_ParentId(r.m_ParentId)
        , m_Flags(r.m_Flags)
    {}

    //! Assignment operator.
    CEntryRecord& operator= (const CEntryRecord& r)
    {
        m_ParentId = r.m_ParentId;
        m_Flags = r.m_Flags;
        return *this;
    }

    //! Sets id of the changed parent entry.
    CEntryRecord& setParent(int Id)
    {
        m_ParentId = Id;
        return *this;
    }

    //! Returns id of the changed parent entry.
    int getParent() const { return m_ParentId; }

    //! Sets flags passed to the invalidate method.
    CEntryRecord& setFlags(int Flags)
    {
        m_Flags = Flags;
        return *this;
    }

    //! Returns flags passed to the invalidate method.
    int getFlags() const { return m_Flags; }

protected:
    //! Id of the changed parent entry.
    int m_ParentId;

    //! Flags passed to the invalidate method.
    int m_Flags;
};


///////////////////////////////////////////////////////////////////////////////
//! A history of changed parent storage entries an object is depending on.

class CEntryChangeLog
{
public:
    //! History of recent changes.
    typedef CCircularBuffer<CEntryRecord, StorageEntry::HISTORY_SIZE> tChangeLog;

public:
    //! Default constructor creates an empty list.
    CEntryChangeLog();

    //! Destructor.
    ~CEntryChangeLog() {}

    //! Returns the current version number.
    unsigned getVersion() const { return m_Version; }

    //! Changes the version number.
    //! - Manual change of the version number clears the whole log. Use with care!
    CEntryChangeLog& setVersion(int Value)
    {
        m_Version = Value;
        m_Log.clear();
        return *this;
    }


    //! Creates a new record about a change made to specified parent entry.
    CEntryChangeLog& insert(int ParentId, int Flags)
    {
        m_Log.push( CEntryRecord(ParentId, Flags) );
        ++m_Version;
        return *this;
    }
    
    //! Retrieves changes made to the entry since a given version number.
    //! - Returns false if the version number is too old so the list
    //!   of changes cannot be retrieved.
    bool getChanges(unsigned VersionNum, CChangedEntries& Changes) const;

    //! Creates new records about changes made to the entry
    CEntryChangeLog& insert(const CChangedEntries & Changes);

protected:
    //! History of changes made to the entry.
    tChangeLog m_Log;

    //! Version number that is automatically increased when a new record is made.
    volatile unsigned m_Version;

private:
    //! Private copy constructor.
    CEntryChangeLog(const CEntryChangeLog& Deps)
    {
#pragma unused(Deps)
    }

    //! Private assignment operator
    CEntryChangeLog& operator= (const CEntryChangeLog& Deps)
    {
#pragma unused(Deps)
        return *this;
    }

    //! Add changes to the container
    struct SGetChanges
    {
        SGetChanges(CChangedEntries& Changes) : m_Changes(Changes) {}

        void operator =(const SGetChanges &) {}
        
        void operator() (const CEntryRecord& v)
        {
            m_Changes.insert( v.getParent(), v.getFlags() );
        }

        CChangedEntries& m_Changes;
    };
};


} // namespace data

#endif // CEntryChangeLog_H
