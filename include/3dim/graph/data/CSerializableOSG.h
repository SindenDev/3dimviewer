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

#ifndef CSerializableOSG_H
#define CSerializableOSG_H

#include <data/CSerializableData.h>

#include <osg/Vec2b>
#include <osg/Vec2d>
#include <osg/Vec2f>
#include <osg/Vec2s>

#include <osg/Vec3b>
#include <osg/Vec3d>
#include <osg/Vec3f>
#include <osg/Vec3s>

#include <osg/Vec4b>
#include <osg/Vec4d>
#include <osg/Vec4f>
#include <osg/Vec4s>

#include <osg/Matrix>

#include <vector>


namespace data
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	OSG vector serializer. 

//! Vector serializer generator
#define DECLARE_OSG_VECTOR_SERIALIZER( type ) \
    template<>						\
    class CSerializableData< type > \
    {																					\
	    public:																			\
	    VPL_ENTITY_NAME(#type);															\
	    template < class tpSerializer >													\
	    static void serialize( const type * sd, vpl::mod::CChannelSerializer<tpSerializer> & Writer ) \
	    {																				\
		    Writer.write( (int)sd->num_components );									\
		    for( int i = 0; i < sd->num_components; ++i )								\
			    Writer.write( (*sd)[i] );													\
	    }																				\
	    template< class tpSerializer >													\
	    static void deserialize( type * sd, vpl::mod::CChannelSerializer<tpSerializer> & Reader )	\
	    {																				\
		    int size = 0;															    \
		    Reader.read( size );														\
		    assert( size == (int)sd->num_components );									\
		    for( int i = 0; i < sd->num_components; ++i )								\
			    Reader.read( (*sd)[i] );												\
	    }																				\
    };

DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec2b )
DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec2d )
DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec2f )
DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec2s )

DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec3b )
DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec3d )
DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec3f )
DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec3s )

DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec4b )
DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec4d )
DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec4f )
DECLARE_OSG_VECTOR_SERIALIZER( osg::Vec4s )

template<>
class CSerializableData< osg::Quat >
{
public:																			
    VPL_ENTITY_NAME("osg::Quat");															
    template < class tpSerializer >													
    static void serialize( const osg::Quat * sd, vpl::mod::CChannelSerializer<tpSerializer> & Writer ) 
    {																				
    for( int i = 0; i < 4; ++i )								
        Writer.write( (*sd)[i] );													
    }																				
    template< class tpSerializer >													
    static void deserialize( osg::Quat * sd, vpl::mod::CChannelSerializer<tpSerializer> & Reader )	
    {																				
    for( int i = 0; i < 4; ++i )								
        Reader.read( (*sd)[i] );												
    }	   
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	OSG matrix serializer. 

#define DECLARE_OSG_MATRIX_SERIALIZER( tpMatrix ) \
    template <>						\
    class CSerializableData< tpMatrix > \
    {								\
    public:							\
        VPL_ENTITY_NAME(#tpMatrix);		\
    public:																				\
        template< class tpSerializer>                                                   \
	    static void serialize( tpMatrix * m, vpl::mod::CChannelSerializer<tpSerializer> & s )	\
	    {																				\
            vpl::sys::tInt32 size = 4;                                                               \
            s.write( size );                                                        	\
		    for( int r = 0; r < size; ++r )												\
			    for( int c = 0; c < size; ++c )											\
				    s.write( (*m)( r, c ) );											\
	    }																				\
        template< class tpSerializer >                                                  \
	    static void deserialize( tpMatrix * m, vpl::mod::CChannelSerializer<tpSerializer> & s )	\
	    {																				\
            int size = 4;                                                               \
		    vpl::sys::tInt32 bsize = 0;																\
		    s.read( bsize );															\
		    assert( bsize == size );													\
		    for( int r = 0; r < size; ++r )												\
			    for( int c = 0; c < size; ++c )											\
				    s.read( (*m)( r, c ) );												\
	    }																				\
    };

DECLARE_OSG_MATRIX_SERIALIZER( osg::Matrixd )

} // namespace data

#endif // CSerializableOSG_H_included
