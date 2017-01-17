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

#ifndef CChangedEntries_H
#define CChangedEntries_H

#include "CStorageEntryDefs.h"

// STL
#include <map>
#include <set>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Forward declarations.

class CStorageEntry;

template <class T>
class CObjectObserver;

template <class T1, class T2>
class CMultiObjectObserver;


///////////////////////////////////////////////////////////////////////////////
//! A list of recently changed storage entries an object is depending on.

class CChangedEntries
{
public:
    //! Map of changed parent entries and flags passed to the invalidate method.
    typedef std::multimap<int, int> tChanges;

    //! Set of IDs used as a filter for flag checking
    typedef std::set<int> tFilter;

public:
    //! Default constructor creates an empty list.
    CChangedEntries() : m_EntryId(Storage::UNKNOWN) {}

    //! Copy constructor.
    CChangedEntries(const CChangedEntries& Changes);

    //! Constructor retrieves the list of recently changed storage entries
    //! from a specified entry.
    CChangedEntries(CStorageEntry *pEntry);

    //! Constructor retrieves the list of changes from the observer.
    template <class T>
    inline CChangedEntries(CObjectObserver<T> *pObserver);

    //! Constructor retrieves the list of changes from the observer.
    template <class T1, class T2>
    inline CChangedEntries(int ObjectNum, CMultiObjectObserver<T1, T2> *pObserver);


    //! Destructor.
    ~CChangedEntries() {}

    //! Assignment operator.
    CChangedEntries& operator =(const CChangedEntries& Changes);

    //! Returns Id of the entry whose changed parent entries are listed.
    int getEntryId() const { return m_EntryId; }

    //! Checks if the list of changes corresponds to a specified entry.
    bool checkIdentity(int Id) const { return (m_EntryId == Id); }

    //! Changes id of the entry whose changed parent entries are listed.
    CChangedEntries& setEntryId(int Id)
    {
        m_EntryId = Id;
        return *this;
    }


    //! Adds an identifier to the list.
    CChangedEntries& insert(int ParentId, int Flags)
    {
        m_Changes.insert( tChanges::value_type(ParentId, Flags) );
        return *this;
    }

    //! Adds a given list.
    CChangedEntries& insert(const CChangedEntries& Changes)
    {
        m_Changes.insert( Changes.m_Changes.begin(), Changes.m_Changes.end() );
        return *this;
    }

    //! Removes an identifier from the list.
    CChangedEntries& erase(int Id)
    {
        m_Changes.erase(Id);
        return *this;
    }

    //! Checks if a given identifier is present in the list.
    bool hasChanged(int Id) const
    {
        return (m_Changes.find(Id) != m_Changes.end());
    }

    //! Returns number of identifiers in the list.
    int getSize() const { return int(m_Changes.size()); }

    //! Returns true if the list is empty.
    bool isEmpty() const { return m_Changes.empty(); }

    //! Clears the list.
    CChangedEntries& clear()
    {
        m_Changes.clear();
        return *this;
    }
    

    //! Checks if given flag combination are present in all values in the list - performs exact matching of given flags with real flags
    bool checkExactFlagsAll(int Value, int Mask = 0x7fffffff, tFilter Filter = tFilter()) const;

    //! Checks if given flag combination are present in any values in the list - performs exact matching of given flags with real flags
    bool checkExactFlagsAny(int Value, int Mask = 0x7fffffff, tFilter Filter = tFilter()) const;

    //! Checks if a given flag is present in all values in the list - this version does not care about other flags
    bool checkFlagAll(int Value) const;
    bool checkFlagAll(int Value, tFilter Filter) const;

    //! Checks if a given flag is present in any value in the list - this version does not care about other flags
    bool checkFlagAny(int Value) const;
    bool checkFlagAny(int Value, tFilter Filter) const;

	//! checks for entry with (Value!=(Mask&Flags))
	bool checkFlagsAnyNonEq(int Value, int Mask, const CChangedEntries::tFilter &Filter = tFilter()) const;

	//! checks for entry with (Value==(Mask&Flags))
	bool checkFlagsAnyEq(int Value, int Mask, const CChangedEntries::tFilter &Filter = tFilter()) const;

	//! Checks for presence of a change that has one or more flags from the Mask set
	bool checkFlagsAnySet(int Mask, const CChangedEntries::tFilter &Filter = tFilter()) const;

	//! Checks for presence of a change that has one or more flags from the Mask zero
	bool checkFlagsAnyNotSet(int Mask, const CChangedEntries::tFilter &Filter = tFilter()) const;

	//! Checks for presence of a change that has flags that mach the mask
	bool checkFlagsAllSet(int Mask, const CChangedEntries::tFilter &Filter = tFilter()) const;

	//! Checks for presence of a change that has zero flags
	bool checkFlagsAllZero(const CChangedEntries::tFilter &Filter = tFilter()) const;

	//! Checks for presence of a change that has zero class specific flags
	bool checkFlagsAllClassSpecificZero(const CChangedEntries::tFilter &Filter = tFilter()) const;

    //! Checks if there is some valid parent entry in the list different
    //! from the entry whose changed parent entries are listed.
    bool isParentValid() const;

    //! Returns identifier of the first entry in the list.
    int getFirstParentId(int i) const 
    {
#pragma unused(i)
        return (m_Changes.size() > 0) ? m_Changes.begin()->first : Storage::UNKNOWN;
    }

    //! Returns flags of the first entry in the list.
    int getFirstFlags() const 
    { 
        return (m_Changes.size() > 0) ? m_Changes.begin()->second : 0;
    }


    //! Returns reference to the internal storage.
    tChanges& getImpl() { return m_Changes; }
    const tChanges& getImpl() const { return m_Changes; }

protected:
    //! Map of all changed parent storage entries and their flags.
    tChanges m_Changes;

    //! Id of the entry whose changed parent entries are listed.
    int m_EntryId;
};


///////////////////////////////////////////////////////////////////////////////
//

template <class T>
CChangedEntries::CChangedEntries(CObjectObserver<T> *pObserver)
{
    if( pObserver )
    {
        pObserver->getChanges(*this);
    }
}

///////////////////////////////////////////////////////////////////////////////
//

template <class T1, class T2>
CChangedEntries::CChangedEntries(int ObjectNum, CMultiObjectObserver<T1, T2> *pObserver)
{
    if( pObserver )
    {
        pObserver->getChanges(ObjectNum, *this);
    }
}


} // namespace data

#endif // CChangedEntries_H
