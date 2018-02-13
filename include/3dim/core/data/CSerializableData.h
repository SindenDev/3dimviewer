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

#ifndef CSerializableData_H
#define CSerializableData_H

#include <VPL/Module/Serializer.h>
#include <VPL/Math/Base.h>

#define READINT32(x) { vpl::sys::tInt32 v; Reader.read( v ); x = v; }
#define WRITEINT32(x) { Writer.write( (vpl::sys::tInt32) (x) ); }

#define READBOOL32(x) { vpl::sys::tInt32 v; Reader.read(v); x = vpl::math::conv2Bool<vpl::sys::tInt32>(v); }
#define WRITEBOOL32(x) { Writer.write(vpl::sys::tInt32(x)); }

namespace data
{

#define SERIALIZE( type, variable, serializer ) \
    data::CSerializableData< type >::serialize( variable, serializer )

////////////////////////////////////////////////////////////////////////////////////////////////////
//! Object serialization wrapper.

template <typename T>
class CSerializableData
{
public:
    //! Serialize
    template <class tpSerializer>
    static void serialize(const T * VPL_UNUSED(sd), vpl::mod::CChannelSerializer<tpSerializer> &Writer)
    {
#pragma unused(Writer)
        assert(false);
    }

    //! Deserialize
    template <class tpSerializer>
    static void deserialize(T * VPL_UNUSED(sd), vpl::mod::CChannelSerializer<tpSerializer> &Reader)
    {
#pragma unused(Reader)
        assert(false);
    }

    static bool deserializationFinished(T * VPL_UNUSED(sd))
    {
        assert(false);
        return false;
    }

}; // class CSerializableData


////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Declare serialization wrapper for serializable classes. 
//!
//!\remarks	Wik, 22.2.2010. 
//!
//!\param	type	The type to serialize. 

#define DECLARE_SERIALIZATION_WRAPPER( type ) \
    template <> \
    class CSerializableData<type> \
    { \
    public: \
        template <class tpSerializer> \
        static void serialize(type *sd, vpl::mod::CChannelSerializer<tpSerializer> &Writer ) \
        { \
            sd->serialize( Writer ); \
        } \
        template <class tpSerializer > \
        static void deserialize(type *sd, vpl::mod::CChannelSerializer<tpSerializer> &Reader ) \
        { \
            sd->deserialize(Reader); \
        } \
        static bool deserializationFinished(type *sd) \
        { \
            return false; \
        } \
    };

#define DECLARE_EXTENDED_SERIALIZATION_WRAPPER( type ) \
    template <> \
    class CSerializableData<type> \
    { \
    public: \
        template <class tpSerializer> \
        static void serialize(type *sd, vpl::mod::CChannelSerializer<tpSerializer> &Writer ) \
        { \
            sd->serialize( Writer ); \
        } \
        template <class tpSerializer > \
        static void deserialize(type *sd, vpl::mod::CChannelSerializer<tpSerializer> &Reader ) \
        { \
            sd->deserialize(Reader); \
        } \
        static bool deserializationFinished(type *sd) \
        { \
            return sd->deserializationFinished(); \
        } \
    };


} // namespace data

#endif // CSerializableData_H
