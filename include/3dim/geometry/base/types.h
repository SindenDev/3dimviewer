///////////////////////////////////////////////////////////////////////////////
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

#ifndef types_H_included
#define types_H_included

#include <VPL/Math/StaticVector.h>
#include <VPL/Math/TransformMatrix.h>
#include <VPL/Math/Quaternion.h>
#include <Eigen/src/StlSupport/StdVector.h>
#include <data/CSerializableData.h>

namespace geometry
{
    //! 2D vector type
    typedef vpl::math::CDVector2 Vec2;

    //! 3D vector type
    typedef vpl::math::CDVector3 Vec3;

    //! 4D vector type
    typedef vpl::math::CDVector4 Vec4;

    //! Transformation matrix
    typedef vpl::math::CDTransformMatrix Matrix;

    //! Matrix 3x3
    typedef vpl::math::CDMatrix3x3 Matrix3x3;

    //! Quaternion
    typedef vpl::math::CDQuat Quat;

    //! Vector of 3D vertices
    typedef std::vector<Vec3> Vec3Array;

    //! Maximal possible 3D vector
    const Vec3 MAX_VECTOR(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

    //! Minimal possible 3D vector
    const Vec3 MIN_VECTOR(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());

    //! Scalar type
    typedef vpl::CScalard Scalar;
    
    //! "Constructor" of 3x3 matrix
    template<typename M>
    M construct3x3(double a00, double a01, double a02, double a10, double a11, double a12, double a20, double a21, double a22)
    {
        M m;
        m(0, 0) = a00; m(0, 1) = a01; m(0, 2) = a02;
        m(1, 0) = a20; m(1, 1) = a11; m(1, 2) = a12;
        m(2, 0) = a10; m(2, 1) = a21; m(2, 2) = a22;
        return m;
    }
    
    //! Convert 3D vectors
    template<typename V1, typename V2>
    V1 convert3(const V2 &v){ return V1(v[0], v[1], v[2]); }

    //! Convert 4D vectors
    template<typename V1, typename V2>
    V1 convert4(const V2 &v){ return V1(v[0], v[1], v[2], v[3]); }

    //! Convert matrix 3x3
    template<typename M1, typename M2>
    M1 convert3x3(const M2 &m) 
    { 
        return M1(m(0, 0), m(0, 1), m(0, 2), 
                  m(1, 0), m(1, 1), m(1, 2),
                  m(2, 0), m(2, 1), m(2, 2)
                  ); 
    }

    //! Convert matrix 3x3 with transposition
    template<typename M1, typename M2>
    M1 convert3x3T(const M2 &m) 
    { 
        return M1(m(0, 0), m(1, 0), m(2, 0), 
                  m(0, 1), m(1, 1), m(2, 1),
                  m(0, 2), m(1, 2), m(2, 2)
            ); 
    }

    //! Convert matrix 3x3
    template<typename M1, typename M2>
    M1 convert4x4(const M2 &m) 
    { 
        return M1(m(0, 0), m(0, 1), m(0, 2), m(0, 3), 
                  m(1, 0), m(1, 1), m(1, 2), m(1, 3),
                  m(2, 0), m(2, 1), m(2, 2), m(2, 3),
                  m(3, 0), m(3, 1), m(3, 2), m(3, 3)
                  ); 
    }

    //! Convert matrix 3x3 with transposition
    template<typename M1, typename M2>
    M1 convert4x4T(const M2 &m) 
    { 
        return M1(m(0, 0), m(1, 0), m(2, 0), m(3, 0), 
                  m(0, 1), m(1, 1), m(2, 1), m(3, 1),
                  m(0, 2), m(1, 2), m(2, 2), m(3, 2),
                  m(0, 3), m(1, 3), m(2, 3), m(3, 3)
                  ); 
    }

// Helper method tests number validity. T should be float or double
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MWERKS__)
    template<typename T>
    inline bool isNaN(T v) { return _isnan(v)!=0; }
#else
#if defined(__APPLE__)
    template<typename T>
    inline bool isNaN(T v) { return std::isnan(v); }
#else
    // Need to use to std::isnan to avoid undef problem from <cmath>
    template<typename T>
    inline bool isNaN(T v) { return isnan(v); }
#endif
#endif

} // namespace geometry

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(geometry::Vec2)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(geometry::Vec3)
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(geometry::Vec4)

//----------------------------------------------------------------------------------
// SERIALIZATION

namespace data
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	VPL vector serializer. 

    //! Vector serializer generator
#define DECLARE_VPL_VECTOR_SERIALIZER( type ) \
    template<>						\
        class CSerializableData< type > \
        {																					\
        public:																			\
        VPL_ENTITY_NAME(#type);															\
        template < class tpSerializer >													\
        static void serialize( const type * sd, vpl::mod::CChannelSerializer<tpSerializer> & Writer ) \
        {																				\
        Writer.write( (int)sd->SIZE );									\
        for( int i = 0; i < sd->SIZE; ++i )								\
        Writer.write( (*sd)[i] );													\
    }																				\
        template< class tpSerializer >													\
        static void deserialize( type * sd, vpl::mod::CChannelSerializer<tpSerializer> & Reader )	\
        {																				\
        int size = 0;															    \
        Reader.read( size );														\
        assert( size == (int)sd->SIZE );									\
        if (size != (int)sd->SIZE)                                          \
            throw vpl::mod::Serializer::CReadFailed();                                  \
        for( int i = 0; i < sd->SIZE; ++i )								\
        Reader.read( (*sd)[i] );												\
    }																				\
    };

    DECLARE_VPL_VECTOR_SERIALIZER( geometry::Vec2 )
    DECLARE_VPL_VECTOR_SERIALIZER( geometry::Vec3 )
    DECLARE_VPL_VECTOR_SERIALIZER( geometry::Vec4 )

    /**
     * \brief   Quaternion serialization.
     */
    template<>
    class CSerializableData< geometry::Quat >
    {
    public:																			
        VPL_ENTITY_NAME("geometry::Quat");															
        template < class tpSerializer >													
        static void serialize( const geometry::Quat * sd, vpl::mod::CChannelSerializer<tpSerializer> & Writer ) 
        {																				
            for( int i = 0; i < 4; ++i )								
                Writer.write( (*sd)[i] );													
        }																				
        template< class tpSerializer >													
        static void deserialize( geometry::Quat * sd, vpl::mod::CChannelSerializer<tpSerializer> & Reader )	
        {																				
            for( int i = 0; i < 4; ++i )								
                Reader.read( (*sd)[i] );												
        }	   
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	VPL matrix serializer. 

#define DECLARE_VPL_MATRIX_SERIALIZER( tpMatrix ) \
    template <>						\
    class CSerializableData< tpMatrix > \
    {								\
    public:							\
    VPL_ENTITY_NAME(#tpMatrix);		\
    public:																				\
    template< class tpSerializer>                                                   \
    static void serialize( tpMatrix * m, vpl::mod::CChannelSerializer<tpSerializer> & s )	\
    {																				\
    vpl::sys::tInt32 rows(m->getNumOfRows()), columns(m->getNumOfCols());                                                               \
    s.write( rows );                                                        	\
    s.write( columns );                                                        	\
    for( int r = 0; r < rows; ++r )												\
    for( int c = 0; c < columns; ++c )											\
    s.write( (*m)( r, c ) );											\
    }																				\
    template< class tpSerializer >                                                  \
    static void deserialize( tpMatrix * m, vpl::mod::CChannelSerializer<tpSerializer> & s )	\
    {																				\
    int rows, columns;                                                              \
    s.read( rows );															        \
    s.read( columns );															    \
    assert(rows == m->getNumOfRows() && columns == m->getNumOfCols());              \
    if (rows != m->getNumOfRows() || columns != m->getNumOfCols())                  \
        throw vpl::mod::Serializer::CReadFailed();                                  \
    for( int r = 0; r < rows; ++r )												    \
    for( int c = 0; c < columns; ++c )											        \
    s.read( (*m)( r, c ) );												\
    }																				\
    };

    DECLARE_VPL_MATRIX_SERIALIZER( geometry::Matrix )
}

// types_H_included
#endif
