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

#ifndef CSerializableVE_H
#define CSerializableVE_H

#include <data/CSerializableData.h>
#include <VPL/Base/Types.h>

namespace data
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	VectorEntity vector serializer. 

//! Vector serializer generator
#define DECLARE_VE_VECTOR_SERIALIZER( type ) \
    template<>						\
    class CSerializableData< type > \
    {																					\
	    public:																			\
	    VPL_ENTITY_NAME(#type);															\
	    template < class tpSerializer >													\
	    static void serialize( type * sd, vpl::mod::CChannelSerializer<tpSerializer> & Writer ) \
	    {																				\
		    Writer.write( sd->GetX() );													\
		    Writer.write( sd->GetY() );													\
		    Writer.write( sd->GetZ() );													\
	    }																				\
	    template< class tpSerializer >													\
	    static void deserialize( type * sd, vpl::mod::CChannelSerializer<tpSerializer> & Reader )	\
	    {																				\
            type::tComponent x,y,z;                                                     \
		    Reader.read( x );													        \
		    Reader.read( y );													        \
		    Reader.read( z );													        \
            sd->SetXYZ( x, y, z );                                                      \
	    }																				\
    };


////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	VectorEntity matrix serializer. 

#define DECLARE_VE_MATRIX_SERIALIZER( type ) \
    template <>						\
    class CSerializableData< type > \
    {								\
    public:							\
        VPL_ENTITY_NAME(#type);		\
    public:																				\
	    template< class tpSerializer >													\
	    static void serialize( type * m, vpl::mod::CChannelSerializer<tpSerializer> & s )	\
	    {																				\
            vpl::tSize rows( m->rows() ), cols( m->cols() );                            \
		    s.write( (int)rows ); s.write( (int)cols );									\
		    for( int r = 0; r < rows; ++r )												\
			    for( int c = 0; c < cols; ++c )											\
				    s.write( (*m)( r, c ) );											\
	    }																				\
	    template< class tpSerializer >													\
	    static void deserialize( type * m, vpl::mod::CChannelSerializer<tpSerializer> & Reader ) \
        { \
            vpl::tSize rows = 0, cols = 0; \
            Reader.read( rows ); \
		    Reader.read( cols ); 															\
		    assert( rows == m->rows() && cols  == m->cols() );                          \
		    for( int r = 0; r < rows; ++r )												\
			    for( int c = 0; c < cols; ++c )											\
				    Reader.read( (*m)( r, c ) );												\
	    }																				\
    };

DECLARE_VE_VECTOR_SERIALIZER( vctl::MCVector3D )
DECLARE_VE_MATRIX_SERIALIZER( vctl::MCTransformMatrix )


} // namespace data

#endif // CSerializableVE_H_included
