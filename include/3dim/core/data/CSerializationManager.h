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

#ifndef CSerializationManager_H
#define CSerializationManager_H

#include <VPL/Module/Serializable.h>
#include <VPL/Module/BinarySerializer.h>
#include <VPL/Base/FullException.h>

#include <data/CDataStorage.h>
#include <data/CSavedEntries.h>

#define SERIALIZER_CURRENT_VERSION 60

///////////////////////////////////////////////////////////////////////////////
//! Encapsulates an exception thrown when serialization fails

class CSerializationFailure : public vpl::base::CException
{
public:
    //! Constructor parametrized by error description string
    CSerializationFailure( const std::string & reason )
        : vpl::base::CException()
        , m_sReason( reason )
    {
    }

    //! Virtual destructor.
    virtual ~CSerializationFailure() throw() {}

    //! Returns error description
    const std::string & reason() const throw()
    {
        return m_sReason;
    }

protected:
    //! Error description
    std::string     m_sReason;
};


///////////////////////////////////////////////////////////////////////////////
//! Serialization manager providing methods to load/save a sequence
//! of predefined storage entries.

class CSerializationManager
    : public vpl::mod::CSerializable
    , public vpl::mod::CProgress
{
public:
    //! Default class name.
    VPL_ENTITY_NAME("SerializationManager");

    //! Default compression method.
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

public:
    //! Vector of the ids type
    typedef std::set< vpl::sys::tInt32 > tIdVector;

public:
    //! Constructor
    CSerializationManager( data::CDataStorage * dataStorage );

    //! Serialize
    void serialize( vpl::mod::CBinarySerializer& Writer, const tIdVector & ids );

    //! Deserialize
    void deserialize( vpl::mod::CBinarySerializer& Reader, tIdVector & ids );

    //! Serialize header
    void serializeHeader( vpl::mod::CBinarySerializer & Writer, const tIdVector & ids );

    //! Deserialize header
    void deserializeHeader( vpl::mod::CBinarySerializer & Reader, tIdVector & ids );

protected:
    //! Data storage
    data::CDataStorage * m_dataStorage;

    //! Version number
    vpl::sys::tInt32 m_version;
};


#endif // CSerializationManager_H
