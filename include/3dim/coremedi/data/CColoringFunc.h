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

#ifndef CColoringFunc_H
#define CColoringFunc_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/SharedPtr.h>
#include <VPL/Image/PixelTypes.h>
#include <VPL/Math/Base.h>
#include <VPL/Image/Image.h>

#include "data/CSlice.h"
#include "data/CColorVector.h"


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//

namespace ColoringFunc
{
    //! Basic coloring models.
    enum EModel
    {
        //! Coloring is disabled.
        NO_COLORING     = 1,

        //! Constant coloring of a specified range of densities.
        CONST_COLORING  = 2,

        //! Full region coloring.
        REGION_COLORING = 4,

        //! Threshold coloring
        THRESHOLD_COLORING = 8,

        //! Multiple threshold coloring
        MULTIPLE_THRESHOLD_COLORING = 16,

        //! Complex (no LUT) coloring
        COMPLEX_COLORING = 32,
    };

} // namespace ColoringFunc


///////////////////////////////////////////////////////////////////////////////
//! Functor that estimates final voxel RGB color.

template <typename T>
class CColoringFunc : public vpl::base::CObject, public vpl::mod::CSerializable
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CColoringFunc);

    //! Output RGB color.
    typedef CColor4<T> tColor;

    //! Input density value.
    typedef vpl::img::tDensityPixel tPixel;

protected:
    bool m_overrideRegionColoring;

public:
    //! Default constructor.
    CColoringFunc()
        : m_overrideRegionColoring(false)
    { }

    //! Virtual destructor.
    virtual ~CColoringFunc()
    { }

    //! Returns type of the coloring function.
    virtual int getType() const = 0;

    //! Serialize
    template<class tpSerializer>
    void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
    { }

    //! Deserialize
    template<class tpSerializer>
    void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
    { }

    //! Colorize whole slice image
    virtual void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties) = 0;

    //! Coloring function.
    //! - Returns tColor() in case of no special coloring
    //!   is required for a given density value.
    virtual tColor makeColor(const tPixel& Density) = 0;

    bool overrideRegionColoring() const
    {
        return m_overrideRegionColoring;
    }
};

//! Conversion functor returning unsigned char (= byte) RGBA colors.
typedef CColoringFunc<unsigned char> CColoringFunc4b;


///////////////////////////////////////////////////////////////////////////////
//! Functor for no special coloring.

class CNoColoring : public CColoringFunc4b
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CNoColoring);

public:
    //! Default constructor.
    CNoColoring()
    { }

    //! Destructor.
    virtual ~CNoColoring()
    { }

    //! Returns type of the coloring function.
    virtual int getType() const { return ColoringFunc::NO_COLORING; }

    //! Serialize
    template < class tpSerializer >
    void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer) { }

    //! Deserialize
    template < class tpSerializer >
    void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader) { }

    //! Colorize whole slice image
    virtual void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties)
    {
        // nothing, really
    }

    //! Empty coloring function.
    virtual tColor makeColor(const tPixel&)
    {
        return tColor();
    }
};


///////////////////////////////////////////////////////////////////////////////
//! Functor for constant coloring of a given range of densities.

class CConstColoring : public CColoringFunc4b
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CConstColoring);

    //! Default color transparency. 
    enum { ALPHA = 128 };

public:
    //! Default constructor.
    CConstColoring(const tPixel& T1, const tPixel& T2, const tColor& Color)
        : m_T1(T1)
        , m_T2(T2)
        , m_Color(Color)
    { }

    //! Nonparametric constructor used for serialization only
    CConstColoring()
    { }

    //! Destructor.
    virtual ~CConstColoring()
    { }

    //! Returns type of the coloring function.
    virtual int getType() const { return ColoringFunc::CONST_COLORING; }

    //! Serialize
    template < class tpSerializer >
    void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
    {
        Writer.write( m_T1 );
        Writer.write( m_T2 );
        m_Color.serialize( Writer );
    }

    //! Deserialize
    template < class tpSerializer >
    void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
    {
        Reader.read( m_T1 );
        Reader.read( m_T2 );
        m_Color.deserialize( Reader );
    }

    //! Colorize whole slice image
    virtual void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties)
    {
		const int xSize = std::min(densityImage.getXSize(),rgbImage.getXSize());
		const int ySize = std::min(densityImage.getYSize(),rgbImage.getYSize());
        for (int y = 0; y < ySize; ++y)
        {
            for (int x = 0; x < xSize; ++x)
            {
                vpl::img::CRGBPixel pixel = rgbImage(x, y);
                tColor prevColor = *(reinterpret_cast<tColor *>(&pixel));
                tColor currColor = makeColor(densityImage(x, y));
                tColor newColor = blendColors(currColor, prevColor);
                rgbImage(x, y) = *(reinterpret_cast<vpl::img::tRGBPixel *>(&newColor));
            }
        }
    }

    //! Coloring function.
    virtual tColor makeColor(const tPixel& Density)
    {
        //return (Density > m_T1 && Density < m_T2) ? m_Color : tColor();
        return (Density >= m_T1 && Density <= m_T2) ? m_Color : tColor();
    }

protected:
    //! Density thresholds.
    tPixel m_T1, m_T2;

    //! Assigned color.
    tColor m_Color;
};

///////////////////////////////////////////////////////////////////////////////
//! Functor for threshold coloring

class CThresholdColoring : public CColoringFunc4b
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CThresholdColoring);

    //! Default color transparency. 
    enum { ALPHA = 128 };

    //! LUT size
    enum { LUT_SIZE = 8501 };

public:
    //! Default constructor.
    CThresholdColoring(const tPixel& T1, const tPixel& T2, const tColor& Color)
    {
        m_lut = new tColor[LUT_SIZE];
        int difference = T2 - T1;
        if (difference <= 0)
        {
            return;
        }

        int maxDistance = vpl::math::getMax<int>(20, static_cast<int>(difference * 0.25f));
        for (int i = T1 + 1500; i < T2 + 1500; ++i)
        {
            int distance = vpl::math::getMin<int>(i - (T1 + 1500), (T2 + 1500) - i);
            int alpha = 255 - distance * 255 / maxDistance;
            vpl::math::limit<int>(alpha, 0, 255);

            m_lut[i] = Color;
            m_lut[i].getA() = (unsigned char)alpha;
        }
    }

    //! Nonparametric constructor used for serialization only
    CThresholdColoring(){m_lut = new tColor[LUT_SIZE];}

    //! Destructor.
    virtual ~CThresholdColoring() { delete[] m_lut; }

    //! Returns type of the coloring function.
    virtual int getType() const { return ColoringFunc::THRESHOLD_COLORING; }

    //! Serialize
    template < class tpSerializer >
    void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
    {
        for( int i = 0; i < LUT_SIZE; ++i )
            m_lut[i].serialize( Writer );
    }

    //! Deserialize
    template < class tpSerializer >
    void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
    {
        for( int i = 0; i < LUT_SIZE; ++i )
            m_lut[i].deserialize( Reader );
    }

    //! Colorize whole slice image
    virtual void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties)
    {
        const int xSize = std::min(densityImage.getXSize(),rgbImage.getXSize());
		const int ySize = std::min(densityImage.getYSize(),rgbImage.getYSize());
		for (int y = 0; y < ySize; ++y)
		{
			for (int x = 0; x < xSize; ++x)
            {
                vpl::img::CRGBPixel pixel = rgbImage(x, y);
                tColor prevColor = *(reinterpret_cast<tColor *>(&pixel));
                tColor currColor = makeColor(densityImage(x, y));
                tColor newColor = blendColors(currColor, prevColor);
                rgbImage(x, y) = *(reinterpret_cast<vpl::img::tRGBPixel *>(&newColor));
            }
        }
    }

    //! Coloring function.
    virtual tColor makeColor(const tPixel& Density)
    {
        return m_lut[Density + 1500];
    }

protected:
    //! Density thresholds.
    tColor *m_lut;
};

///////////////////////////////////////////////////////////////////////////////
//! Functor for multiple threshold coloring

class CMultipleThresholdColoring : public CColoringFunc4b
{
public:
    struct SColoringEntry
    {
        tPixel T1, T2;
        tColor Color;

        SColoringEntry(tPixel t1, tPixel t2, tColor color)
        {
            T1 = t1;
            T2 = t2;
            Color = color;
        }
    };

public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CMultipleThresholdColoring);

    //! Default color transparency. 
    enum { ALPHA = 128 };

    //! LUT size
    enum { LUT_SIZE = 8501 };

public:
    //! Default constructor.
    CMultipleThresholdColoring(std::vector<SColoringEntry> coloringEntries)
    {
        m_lut = new tColor[LUT_SIZE];
        if (coloringEntries.size() == 0)
        {
            return;
        }

        for (int i = 0; i < (int)coloringEntries.size(); ++i)
        {
            int T1 = coloringEntries[i].T1;
            int T2 = coloringEntries[i].T2;
            float Color[4];
            Color[0] = coloringEntries[i].Color.getR() / 255.0f;
            Color[1] = coloringEntries[i].Color.getG() / 255.0f;
            Color[2] = coloringEntries[i].Color.getB() / 255.0f;
            Color[3] = coloringEntries[i].Color.getA() / 255.0f;

            int difference = T2 - T1;
            if (difference <= 0)
            {
                continue;
            }

            float maxDistance = vpl::math::getMax<float>(20, difference * 0.25f);
            for (int i = T1 + 1500; i < T2 + 1500; ++i)
            {
                int distance = vpl::math::getMin<int>(i - (T1 + 1500), (T2 + 1500) - i);
                float alpha = 1.0f - static_cast<float>(distance) / maxDistance;
                vpl::math::limit<float>(alpha, 0.0f, 1.0f);

                float LutColor[4];
                LutColor[0] = m_lut[i].getR() / 255.0f;
                LutColor[1] = m_lut[i].getG() / 255.0f;
                LutColor[2] = m_lut[i].getB() / 255.0f;
                LutColor[3] = m_lut[i].getA() / 255.0f;

                if (LutColor[3] == 0.0f) // first assignment into this pixel
                {
                    for (int c = 0; c < 3; ++c)
                    {
                        LutColor[c] = Color[c];
                    }
                    LutColor[3] = alpha;
                }
                else // blend with previous color
                {
                    for (int c = 0; c < 3; ++c)
                    {
                        LutColor[c] = LutColor[c] + alpha * (Color[c] - LutColor[c]);
                        vpl::math::limit<float>(LutColor[c], 0.0f, 1.0f);
                    }
                    LutColor[3] = LutColor[3] + alpha;
                    vpl::math::limit<float>(LutColor[3], 0.0f, 1.0f);
                }

                m_lut[i] = tColor((unsigned char)(LutColor[0] * 255.0f), 
                                  (unsigned char)(LutColor[1] * 255.0f), 
                                  (unsigned char)(LutColor[2] * 255.0f), 
                                  (unsigned char)(LutColor[3] * 255.0f));
            }
        }
    }

    //! Nonparametric constructor used for serialization only
    CMultipleThresholdColoring() { m_lut = new tColor[LUT_SIZE]; }

    //! Destructor.
    virtual ~CMultipleThresholdColoring() { delete[] m_lut; }

    //! Returns type of the coloring function.
    virtual int getType() const { return ColoringFunc::MULTIPLE_THRESHOLD_COLORING; }

    //! Serialize
    template < class tpSerializer >
    void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
    {
        for( int i = 0; i < LUT_SIZE; ++i )
            m_lut[i].serialize( Writer );
    }

    //! Deserialize
    template < class tpSerializer >
    void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
    {
        for( int i = 0; i < LUT_SIZE; ++i )
            m_lut[i].deserialize( Reader ); 
    }

    //! Colorize whole slice image
    virtual void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties)
    {
		const int xSize = std::min(densityImage.getXSize(),rgbImage.getXSize());
		const int ySize = std::min(densityImage.getYSize(),rgbImage.getYSize());
		for (int y = 0; y < ySize; ++y)
		{
			for (int x = 0; x < xSize; ++x)
            {
                vpl::img::CRGBPixel pixel = rgbImage(x, y);
                tColor prevColor = *(reinterpret_cast<tColor *>(&pixel));
                tColor currColor = makeColor(densityImage(x, y));
                tColor newColor = blendColors(currColor, prevColor);
                rgbImage(x, y) = *(reinterpret_cast<vpl::img::tRGBPixel *>(&newColor));
            }
        }
    }

    //! Coloring function.
    virtual tColor makeColor(const tPixel& Density)
    {
        return m_lut[Density + 1500];
    }

protected:
    //! Density thresholds.
    tColor *m_lut;
};

///////////////////////////////////////////////////////////////////////////////
//! Base functor for complex (context- and properties- aware) coloring.

class CComplexColoring : public CColoringFunc4b
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CComplexColoring);

public:
    //! Default constructor.
    CComplexColoring()
    { }

    //! Destructor.
    virtual ~CComplexColoring()
    { }

    //! Returns type of the coloring function.
    virtual int getType() const
    {
        return ColoringFunc::COMPLEX_COLORING;
    }

    //! Serialize
    template<class tpSerializer>
    void serialize(vpl::mod::CChannelSerializer<tpSerializer> &Writer)
    { }

    //! Deserialize
    template<class tpSerializer>
    void deserialize(vpl::mod::CChannelSerializer<tpSerializer> &Reader)
    { }

    //! Colorize whole slice image
    virtual void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties)
    {
        // base class of complex coloring func does nothing
    }

    //! Empty coloring function.
    virtual tColor makeColor(const tPixel&)
    {
        return tColor();
    }
};

} // namespace data

#endif // CColoringFunc_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
