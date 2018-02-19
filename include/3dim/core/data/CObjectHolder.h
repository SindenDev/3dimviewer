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

#ifndef CObjectHolder_H
#define CObjectHolder_H

#include <VPL/Base/SharedPtr.h>
#include <VPL/Module/Serializable.h>
#include <data/CSerializableData.h>

#include "CStorableData.h"

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Class derived from CStorableData holding pointer to some other class.
//! - A reference counting smart pointer must be declared inside the T type
//!   using VPL_SHAREDPTR() macro.
//! - The default constructor, method update(), and method init()
//!   must be also defined.

template <class T>
class CObjectHolder : public CStorableData
{
public:   
    //! Object type.
    typedef T tObject;

    //! Reference counting smart pointer to the object.
    typedef typename T::tSmartPtr tObjectPtr;

public:
    //! Object creation function.
    static CStorableData *create()
    {
        return new CObjectHolder(new T());
    }

    //! Virtual destructor.
    virtual ~CObjectHolder()
    { }

    //! Returns pointer to the data.
    T *getObjectPtr()
    {
        return m_spObject.get();
    }

    //! Forwards the update() method...
    virtual void update(const CChangedEntries &Changes)
    {
        if (m_spObject.get())
        {
            m_spObject->update(Changes);
        }
    }

    //! Forwards the init() method...
    virtual void init()
    {
        if (m_spObject.get())
        {
            m_spObject->init();
        }
    }

    //! Forwards the checkDependency() method...
    virtual bool checkDependency(CStorageEntry *pParent)
    {
        if (!m_spObject.get())
        {
            return true;
        }

        return m_spObject->checkDependency(pParent);
    }

    //! Forwards hasData() method - returns true, if object contains relevant data
    virtual bool hasData()
    {
        if (!m_spObject.get())
        {
            return false;
        }

        return m_spObject->hasData();  
    }

    //! Serialize. 
    virtual void serialize(vpl::mod::CBinarySerializer &Writer)
    {
        data::CSerializableData<T>::template serialize<vpl::mod::CBinarySerializer>(m_spObject.get(), Writer);
    }

    //! Deserialize
    virtual void deserialize(vpl::mod::CBinarySerializer &Reader)
    {
        data::CSerializableData<T>::template deserialize<vpl::mod::CBinarySerializer>(m_spObject.get(), Reader);
    }

    //! Deserialize
    virtual bool deserializationFinished()
    {
        return data::CSerializableData<T>::deserializationFinished(m_spObject.get());
    }

protected:
    //! Constructor.
    CObjectHolder(T *pObj = NULL)
        : m_spObject(pObj)
    { }

protected:
    //! Pointer to the object.
    tObjectPtr m_spObject;
};

} // namespace data

#endif // CObjectHolder_H
