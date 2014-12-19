//////////////////////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// include files

#include <data/CDensityWindow.h>
#include <data/CRegionColoring.h>

#include <VPL/Math/Base.h>


namespace data
{

//////////////////////////////////////////////////////////////////////////////////////////////////
// 

CDensityWindow::CDensityWindow()
    : CColorVector4b(getDensityRange(), CColor4b(0, 0, 0, 255), CColor4b(255, 255, 255, 255))
    , m_Params(DEFAULT_DENSITY_WINDOW)
    , m_DefaultParams(DEFAULT_DENSITY_WINDOW)
    , m_spColoring(new CNoColoring())
{
    makeColorVector();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 

CDensityWindow::CDensityWindow(const SDensityWindow& Params)
    : CColorVector4b(getDensityRange(), CColor4b(0, 0, 0, 255), CColor4b(255, 255, 255, 255))
    , m_Params(Params)
    , m_DefaultParams(DEFAULT_DENSITY_WINDOW)
    , m_spColoring(new CNoColoring())
{
    checkParams(m_Params);

    makeColorVector();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 

void CDensityWindow::setCenter(int Center)
{   
    m_Params.m_Center = Center;

    checkParams(m_Params);

    makeColorVector();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 

void CDensityWindow::setWidth(int Width)
{   
    m_Params.m_Width = Width;

    checkParams(m_Params);
    
    makeColorVector();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 

void CDensityWindow::setParams(const SDensityWindow& Params)
{ 
    m_Params = Params;
    
    checkParams(m_Params);

    makeColorVector();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 

void CDensityWindow::setDefaultParams(const SDensityWindow& Params)
{
    m_DefaultParams = Params;

    checkParams(m_DefaultParams);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//

void CDensityWindow::restoreDefault()
{
    m_Params = m_DefaultParams;

    makeColorVector();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 

void CDensityWindow::checkParams(SDensityWindow& Params)
{
    vpl::math::limit<int>(Params.m_Center, getMinDensity(), getMaxDensity());

    Params.m_Width = vpl::math::getMax(Params.m_Width, 0);
    Params.m_Width = vpl::math::getMin(Params.m_Width, (Params.m_Center - getMinDensity()) * 2);
    Params.m_Width = vpl::math::getMin(Params.m_Width, (getMaxDensity() - Params.m_Center) * 2);
}

void CDensityWindow::colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties)
{
	const int xSize = std::min(densityImage.getXSize(),rgbImage.getXSize());
	const int ySize = std::min(densityImage.getYSize(),rgbImage.getYSize());

    // basic colors (without context and properties) are cached internally, so apply them now
    for (int y = 0; y < ySize; ++y)
    {
        for (int x = 0; x < xSize; ++x)
        {
            CColor4b Color = getColorSafe(int(densityImage(x, y)));
            rgbImage(x, y) = *(reinterpret_cast<vpl::img::tRGBPixel *>(&Color));
        }
    }

    // add context and property awareness if needed
    if (m_spColoring->getType() == ColoringFunc::COMPLEX_COLORING)
    {
        m_spColoring->colorize(rgbImage, densityImage, properties);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 

void CDensityWindow::setColoring(CColoringFunc4b *pFunc)
{
    if( !pFunc )
    {
        return;
    }

    m_spColoring = pFunc;
    makeColorVector();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

const CColoringFunc4b * CDensityWindow::getColoring()
{
    return m_spColoring.get();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// 

void CDensityWindow::makeColorVector()
{
    static const CColor4b Black(0, 0, 0, 255);
    static const CColor4b White(255, 255, 255, 255);
    static const CColor4b noColor(0, 0, 0, 0);
    CColor4b currentColor;

    // Maximum and minimum index of density window
    int Max = getMax();
    int Min = getMin();

    // Linear density window approximation ratio evaluation
    double dRatio = 255.0 / double(Max - Min);

    // Fill black color below density window
    for (int i = getMinDensity(); i < Min; ++i)
    {
        currentColor = m_spColoring->getType() == ColoringFunc::COMPLEX_COLORING ? noColor : m_spColoring->makeColor(CColoringFunc4b::tPixel(i));
        getColor(i).setColor(blendColors(currentColor, Black));
    }

    // Fill colors of density window
    CColor4b Gray(0, 0, 0, 255);
    for (int j = Min; j < Max; ++j)
    {
        unsigned char ucGrayLevel = (unsigned char)(dRatio * (j - Min));
        Gray.setColor(ucGrayLevel, ucGrayLevel, ucGrayLevel, 255);
        currentColor = m_spColoring->getType() == ColoringFunc::COMPLEX_COLORING ? noColor : m_spColoring->makeColor(CColoringFunc4b::tPixel(j));
        getColor(j).setColor(blendColors(currentColor, Gray));
    }

    // Fill white color over density window
    for (int k = Max; k <= getMaxDensity(); ++k)
    {
        currentColor = m_spColoring->getType() == ColoringFunc::COMPLEX_COLORING ? noColor : m_spColoring->makeColor(CColoringFunc4b::tPixel(k));
        getColor(k).setColor(blendColors(currentColor, White));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//

void CDensityWindow::update(const CChangedEntries& Changes)
{
    // Does nothing...
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//

void CDensityWindow::init()
{
    m_Params = DEFAULT_DENSITY_WINDOW;
    m_DefaultParams = DEFAULT_DENSITY_WINDOW;
    m_spColoring = new CNoColoring();

    checkParams(m_Params);

    makeColorVector();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Serialize. 
//!
//!\param [in,out]  Writer  the writer. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDensityWindow::serialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Writer)
{
    Writer.beginWrite( *this );

    WRITEINT32( 1 ); // version

    CColorVector4b::serialize( Writer );

    Writer.write( (vpl::sys::tInt32)m_Params.m_Center );
    Writer.write( (vpl::sys::tInt32)m_Params.m_Width );

    // Write coloring function type
    Writer.write( (vpl::sys::tInt32)m_spColoring->getType() );
    m_spColoring->serialize( Writer );

    Writer.endWrite( *this );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Deserialize. 
//!
//!\param [in,out]  Reader  the reader. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDensityWindow::deserialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Reader)
{
    Reader.beginRead( *this );

    int version = 0;
    READINT32( version );

    CColorVector4b::deserialize( Reader );

    #define READINT32(x) { vpl::sys::tInt32 v; Reader.read( v ); x = v; }
    READINT32( m_Params.m_Center );
    READINT32( m_Params.m_Width );

    // Read coloring function type
    int cfType; 
    READINT32( cfType );

    // Create appropriate object
    switch( cfType )
    {
    case ColoringFunc::NO_COLORING:
        m_spColoring = new CNoColoring();
        break;

    case ColoringFunc::CONST_COLORING:
        m_spColoring = new CConstColoring();
        break;

    case ColoringFunc::THRESHOLD_COLORING:
        m_spColoring = new CThresholdColoring();
        break;

    case ColoringFunc::MULTIPLE_THRESHOLD_COLORING:
        m_spColoring = new CMultipleThresholdColoring();
        break;

    case ColoringFunc::REGION_COLORING:
        m_spColoring = new CRegionColoring();
        break;

    case ColoringFunc::COMPLEX_COLORING:
        m_spColoring = new CComplexColoring();
        break;


    default:
        // Something wrong has happened
        assert( false );
    }

    m_spColoring->deserialize( Reader );

    Reader.endRead( *this );    
}

} // namespace data

