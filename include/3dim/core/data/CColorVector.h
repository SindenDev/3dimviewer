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

#ifndef CColorVector_H
#define CColorVector_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/Assert.h>
#include <VPL/Math/Random.h>
#include <VPL/Module/Serializer.h>
#include <VPL/Module/Serializable.h>

// STL
#include <vector>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Class template representing RGBA color.
//! Its component type is defined by template parameter.

template <typename T> 
class CColor4 : public vpl::mod::CSerializable
{
public:
	//! Default class name.
    VPL_ENTITY_NAME("CColor4");

    //! Default compression method.
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

    //! Component type.
    typedef T Component;

public:
    //! Default constructor.
    CColor4() { m_Color[0] = m_Color[1] = m_Color[2] = m_Color[3] = 0; }

    //! Parametric constructor.
    CColor4(T r, T g, T b, T a = 0)
    {
        m_Color[0] = r;
        m_Color[1] = g;
        m_Color[2] = b;
        m_Color[3] = a;
    }

    //! Copy constructor.
    CColor4(const CColor4& color)
    {
        m_Color[0] = color.m_Color[0];
        m_Color[1] = color.m_Color[1];
        m_Color[2] = color.m_Color[2];
        m_Color[3] = color.m_Color[3];
    }

    //! Destructor of the class.
    ~CColor4() {}

    //! Sets all color components.
    void setColor(const CColor4& color)
    {
        m_Color[0] = color.m_Color[0];
        m_Color[1] = color.m_Color[1];
        m_Color[2] = color.m_Color[2];
        m_Color[3] = color.m_Color[3];
    }

    //! Sets all color components.
    void setColor(T r, T g, T b, T a = 0)
    {
        m_Color[0] = r;
        m_Color[1] = g;
        m_Color[2] = b;
        m_Color[3] = a;
    }

    //! Sets all color components.
    void setColor(T *color[4])
    {
        m_Color[0] = color[0];
        m_Color[1] = color[1];
        m_Color[2] = color[2];
        m_Color[3] = color[3];
    }

    //! Returns all color components.
    void getColor(T &r, T &g, T &b, T &a) const
    {
        r = m_Color[0];
        g = m_Color[1];
        b = m_Color[2];
        a = m_Color[3];
    }

    //! Returns all color components.
    void getColor(T color[4]) const
    {
        color[0] = m_Color[0];
        color[1] = m_Color[1];
        color[2] = m_Color[2];
        color[3] = m_Color[3];
    }

    //! Returns the red color component.
    T& getR() { return m_Color[0]; }
    const T& getR() const { return m_Color[0]; }

    //! Returns the green color component.
    T& getG() { return m_Color[1]; }
    const T& getG() const { return m_Color[1]; }

    //! Returns the blue color component.
    T& getB() { return m_Color[2]; }
    const T& getB() const { return m_Color[2]; }

    //! Returns the alpha color component.
    T& getA() { return m_Color[3]; }
    const T& getA() const { return m_Color[3]; }

    //! Returns pointer to all color elements.
    T *getData() { return m_Color; }
    const T *getData() const { return m_Color; }

	//! Serialize
	template < class tpSerializer >
	void serialize( vpl::mod::CChannelSerializer<tpSerializer> & Writer )
	{
		Writer.write( m_Color[ 0 ] );
		Writer.write( m_Color[ 1 ] );
		Writer.write( m_Color[ 2 ] );
		Writer.write( m_Color[ 3 ] );
	}

	//! Deserialize
	template < class tpSerializer >
	void deserialize( vpl::mod::CChannelSerializer<tpSerializer> & Reader )
	{
		Reader.read( m_Color[ 0 ] );
		Reader.read( m_Color[ 1 ] );
		Reader.read( m_Color[ 2 ] );
		Reader.read( m_Color[ 3 ] );
	}

	T &operator[](size_t index)
	{
		assert(index < 4);
		return m_Color[index];
	}

	const T &operator[](size_t index) const
	{
		assert(index < 4);
		return m_Color[index];
	}

protected:
    //! RGBA color components.
    T m_Color[4];
};


//! RGBA color having unsigned char (char) components.
typedef CColor4<unsigned char> CColor4b;

//! RGBA color having float components.
typedef CColor4<float> CColor4f;


//! Function blends two colors (c1 over c2) with respect to the alpha component
//! of the first color.
inline CColor4b blendColors(const CColor4b& c1, const CColor4b& c2)
{
    static const double dNorm = 1.0 / 255;

    double w1 = dNorm * c1.getA();
    double w2 = 1.0 - w1;

    unsigned char r = (unsigned char)(w1 * c1.getR() + w2 * c2.getR());
    unsigned char g = (unsigned char)(w1 * c1.getG() + w2 * c2.getG());
    unsigned char b = (unsigned char)(w1 * c1.getB() + w2 * c2.getB());
    unsigned char a = (unsigned char)(w1 * c1.getA() + w2 * c2.getA());

    return CColor4b(r, g, b, a);
}


///////////////////////////////////////////////////////////////////////////////
//! Class template for color vector having a predefined size.

template <typename T>
class CColorVector : public vpl::mod::CSerializable
{
public:
    //! Color type.
    typedef CColor4<T> tColor;

    //! Type used to represent color vector.
    typedef std::vector<tColor> tColorVector;

public:
    //! Constructor.
    CColorVector(int Size, const tColor& MinValue = tColor(), const tColor& MaxValue = tColor())
        : m_ColorVector(typename tColorVector::size_type(Size), MinValue)
        , m_DefaultMin(MinValue)
        , m_DefaultMax(MaxValue)
    {
        VPL_ASSERT(Size > 0);
    }

    //! Destructor.
    ~CColorVector() {}

    //! Resizes the color vector.
    void resize(int Size, bool bRandomColor = false, unsigned char Alpha = 255)
    {
        if( Size > getSize() )
        {
            if( bRandomColor )
            {
                tColor NewColor(0, 0, 0, Alpha);
                for( int i = getSize(); i < Size; ++i )
                {
                    NewColor.getR() = (unsigned char)(m_Rand.random(0, 255));
                    NewColor.getG() = (unsigned char)(m_Rand.random(0, 255));
                    NewColor.getB() = (unsigned char)(m_Rand.random(0, 255));
                    m_ColorVector.push_back(NewColor);
                }
            }
            else
            {
                for( int i = getSize(); i < Size; ++i )
                {
                    m_ColorVector.push_back(m_DefaultMin);
                }
            }
        }
        else
        {
            m_ColorVector.resize(Size);
        }
    }

    //! Returns vector size.
    int getSize() const { return int(m_ColorVector.size()); }

    //! Returns color of the i-th region.
    tColor& getColor(int i) { return m_ColorVector[i]; }
    const tColor& getColor(int i) const { return m_ColorVector[i]; }

    //! Returns color by given index.
    //! If the index exceeds allowed range, one of default values is returned.
    tColor& getColorSafe(int i)
    {
        return (i < 0) ? m_DefaultMin : ((i >= getSize()) ? m_DefaultMax : m_ColorVector[i]);
    }
    const tColor& getColorSafe(int i) const
    {
        return (i < 0) ? m_DefaultMin : ((i >= getSize()) ? m_DefaultMax : m_ColorVector[i]);
    }

    //! Returns color of the i-th region.
    tColor& operator [](int i) { return getColorSafe(i); }
    const tColor& operator [](int i) const { return getColorSafe(i); }

    //! Sets the subscripted color.
    void setColor(int i, const tColor& Color) { m_ColorVector[i] = Color; }

    //! Returns color by given index.
    //! If the index exceeds allowed range, one of default values is returned.
    void setColorSafe(int i, const tColor& Color)
    {
        if( i >= 0 && i < getSize() )
        {
            m_ColorVector[i] = Color;
        }
    }

    //! Fills the color vector.
    void fill(const tColor& Value)
    {
        typename tColorVector::iterator itEnd = m_ColorVector.end();
        for( typename tColorVector::iterator it = m_ColorVector.begin(); it != itEnd; ++it )
        {
            *it = Value;
        }
    }

	//! Serialize
	template < class tpSerializer >
	void serialize( vpl::mod::CChannelSerializer<tpSerializer> & Writer )
	{
		// Store vector size
		Writer.write( (vpl::sys::tUInt32) m_ColorVector.size() );

		// Store color values
		typename tColorVector::iterator it, itEnd( m_ColorVector.end() );	
		for( it = m_ColorVector.begin(); it != itEnd; ++it )
			it->serialize( Writer );

		// Store minimal and maximal value
		m_DefaultMin.serialize( Writer );
		m_DefaultMax.serialize( Writer );
	}

	//! Deserialize
	template < class tpSerializer >
	void deserialize( vpl::mod::CChannelSerializer<tpSerializer> & Reader )
	{
		vpl::sys::tUInt32 size;

		// Read vector size
		Reader.read( size );

        if (size>1024*1024) // max allowed size of color vector
            throw vpl::mod::Serializer::CReadFailed();

		m_ColorVector.resize( size );

		// Read values
		typename tColorVector::iterator it, itEnd( m_ColorVector.end() );	
		for( it = m_ColorVector.begin(); it != itEnd; ++it )
			it->deserialize( Reader );

		// Read minimal and maximal value
		m_DefaultMin.deserialize( Reader );
		m_DefaultMax.deserialize( Reader );
	}

protected:
    //! Color vector.
    tColorVector m_ColorVector;

    //! Default minimal and maximal value.
    tColor m_DefaultMin, m_DefaultMax;

    //! Random number generator.
    vpl::math::CUniformPRNG m_Rand;
};


//! Vector of unsigned char (= byte) RGBA colors.
typedef CColorVector<unsigned char> CColorVector4b;

//! Vector of float RGBA colors.
typedef CColorVector<float> CColorVector4f;


} // namespace data

#endif // CColorVector_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

