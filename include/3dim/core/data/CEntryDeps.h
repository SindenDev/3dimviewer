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

#ifndef CEntryDeps_H
#define CEntryDeps_H

#include <VPL/Base/Setup.h>

// STL
#include <set>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! A list of parent storage entries an object is depending on.
//! - An object is notified about changes of its parents.

class CEntryDeps
{
public:
    //! List of all parents.
    typedef std::set<int> tDeps;

public:
    //! Default constructor.
    //! - Creates empty list of dependecies.
    CEntryDeps() {}

    //! Copy constructor.
    CEntryDeps(const CEntryDeps& Deps) : m_Deps(Deps.m_Deps) {}

    //! Destructor.
    ~CEntryDeps() {}

    //! Assignment operator.
    CEntryDeps& operator =(const CEntryDeps& Deps)
    {
        m_Deps = Deps.m_Deps;
        return *this;
    }

	//! Returns size of the list.
    tDeps::size_type size() const { return m_Deps.size(); }

    //! Returns true if the list is empty
    bool isEmpty() const { return m_Deps.empty(); }

    //! Adds a new dependency, identifier of a parent entry, to the list.
    CEntryDeps& insert(int Id)
    {
        m_Deps.insert(Id);
        return *this;
    }

    //! Adds a new dependency, identifier of a parent entry, to the list.
    CEntryDeps& operator |(int Id)
    {
        return insert(Id);
    }

    //! Calls a given function object for every dependency in the list.
    template <class Function>
    inline Function forEach(Function Func) const
    {
        return std::for_each(m_Deps.begin(), m_Deps.end(), Func);
    }

    //! Clears the dependencies.
    void clear()
    {
        m_Deps.clear();
    }

    //! Returns reference to the internal STL container of dependencies.
    tDeps& getImpl() { return m_Deps; }
    const tDeps& getImpl() const { return m_Deps; }

	//! Returns true if list contains given id.
	bool contains( int id )
	{
		return m_Deps.find( id ) != m_Deps.end();
	}

protected:
    //! List of all dependecies.
    tDeps m_Deps;
};


} // namespace data

#endif // CEntryDeps_H
