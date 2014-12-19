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

#ifndef CEntryFlags_H
#define CEntryFlags_H

#include <VPL/Base/Setup.h>

// STL
#include <set>
#include <algorithm>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Accumulated history of flags passed to the invalidate() method 
//! of a storage entry.

class CEntryFlags
{
public:
    //! Maximal size.
    enum { MAX_SIZE = 100 };

    //! Set of flags.
    typedef std::set<int> tList;

public:
    //! Default constructor creates an empty list.
    CEntryFlags() {}

    //! Copy constructor.
    CEntryFlags(const CEntryFlags& List) : m_List(List.m_List) {}

    //! Destructor.
    ~CEntryFlags() {}

    //! Assignment operator.
    CEntryFlags& operator =(const CEntryFlags& List)
    {
        m_List = List.m_List;
        return *this;
    }

    //! Adds an integer containing binary flags to the list.
    CEntryFlags& insert(int Flags);

    //! Adds a given history to the current list.
    CEntryFlags& insert(const CEntryFlags& List);

    //! Clears the entire history.
    CEntryFlags& clear()
    {
        m_List.clear();
        return *this;
    }


    //! Checks if a given flag is present in all values in the list.
    bool checkFlagAll(int Value) const;
    
    //! Checks if a given flag is present in any value in the list.
    bool checkFlagAny(int Value) const;

    //! Adds a given value containing binary flags to the list.
    CEntryFlags& addFlags(int Flags)
    {
        return insert(Flags);
    }

	//! Are there any flags?
	bool isEmpty() const { return m_List.size() == 0; }

    //! Calls a given function object for every value in the list.
    template <class Function>
    inline Function forEach(Function Func) const
    {
        return std::for_each(m_List.begin(), m_List.end(), Func);
    }

protected:
    //! Set of all flags given while invalidating a storage entry.
    tList m_List;
};


} // namespace data

#endif // CEntryFlags_H

