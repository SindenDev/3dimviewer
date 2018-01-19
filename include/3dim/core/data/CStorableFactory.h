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

#ifndef CStorableFactory_H
#define CStorableFactory_H

#include <VPL/Base/Factory.h>

#include "CEntryDeps.h"
#include "CStorableData.h"
#include "CDummyStorable.h"


namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Useful macros

//! Returns reference to the storable factory.
#define STORABLE_FACTORY    VPL_SINGLETON(data::CStorableFactory)


///////////////////////////////////////////////////////////////////////////////
//! Creation of objects derived from CStorableData by identifier.

class CStorableFactory
    : public vpl::base::CSingleton<vpl::base::SL_LONG>
    , public vpl::base::CLibraryLockableClass<CStorableFactory>
{
public:
    //! Scoped lock.
    typedef vpl::base::CLibraryLockableClass<CStorableFactory>::CLock tLock;

    //! Object identifier type.
    typedef int tIdentifier;

    //! Object creation function.
    typedef CStorableData *(*tCreationFunction)();

public:
    //! Destructor.
    ~CStorableFactory() {}

    //! Registers an object.
    bool registerObject(int Id, tCreationFunction Function, const CEntryDeps& Deps = CEntryDeps())
    {
        VPL_CHECK(Function, return false);

        tLock Lock(*this);
        m_Deps.insert(tDeps::value_type(Id, Deps));
        return (m_Functions.insert(tFunctions::value_type(Id, Function)).second);
    }

    //! Unregisters the object.
    bool unregisterObject(int Id)
    {
        tLock Lock(*this);
        m_Deps.erase(Id);
        return (m_Functions.erase(Id) == 1);
    }

    //! Creates a new object having the identifier 'Id'.
    CStorableData *create(int Id)
    {
        tLock Lock(*this);
        tFunctions::const_iterator i = m_Functions.find(Id);
        if( i == m_Functions.end() )
        {
            throw vpl::base::Factory::CCannotCreate();
        }
        return (i->second)();
    }

    //! Returns dependencies for a given object.
    const CEntryDeps& getDeps(int Id)
    {
        tLock Lock(*this);
        tDeps::const_iterator i = m_Deps.find(Id);
        if( i == m_Deps.end() )
        {
            return m_EmptyDeps;
        }
        return i->second;
    }

protected:
    //! Map of creation functions.
    typedef std::map<int, tCreationFunction> tFunctions;

    //! Map of all dependencies.
    typedef std::map<int, CEntryDeps> tDeps;

    //! Map of all registered creation functions.
    tFunctions m_Functions;

    //! Map of dependencies of all registered objects.
    tDeps m_Deps;

    //! Empty dependencies.
    const CEntryDeps m_EmptyDeps;

protected:
    //! Private constructor.
    CStorableFactory() : m_EmptyDeps() {}

    //! Private assignment operator.
    CStorableFactory& operator =(const CStorableFactory&);

    //! Allow factory instantiation using singleton holder.
    VPL_PRIVATE_SINGLETON(CStorableFactory);
};


} // namespace data

#endif // CStorableFactory_H
