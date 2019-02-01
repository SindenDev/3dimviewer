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

#ifndef CDensityWindow_H
#define CDensityWindow_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Image/PixelTraits.h>

#include "data/CColorVector.h"
#include "CColoringFunc.h"
#include "data/CObjectHolder.h"

#include <data/CSerializableData.h>
#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Structure holding density window attributes.

struct SDensityWindow
{
    //! Density window width.
    int m_Center;

    //! Density window center.
    int m_Width;

    //! Constructor.
    SDensityWindow(int Center = 0, int Width = 0)
        : m_Center(Center)
        , m_Width(Width)
    { }

    //! Copy constructor.
    SDensityWindow(const SDensityWindow& w)
        : m_Center(w.m_Center)
        , m_Width(w.m_Width)
    { }

    //! Compare operator
    bool operator==(const SDensityWindow &w) const
    {
        return m_Center == w.m_Center && m_Width == w.m_Width;
    }
};


//! Default density window.
//const SDensityWindow DEFAULT_DENSITY_WINDOW (500, 2000);
const SDensityWindow DEFAULT_DENSITY_WINDOW (1500, 4200);

//! Default density window for X-Ray
const SDensityWindow DEFAULT_XRAY_DENSITY_WINDOW(-10, 1900);

//! Typical density window applied to highlight bones.
//const SDensityWindow BONES_DENSITY_WINDOW   (500, 400);
const SDensityWindow BONES_DENSITY_WINDOW   (1800, 3500);

//! Typical density window applied to highlight soft tissues.
const SDensityWindow SOFT_TISSUES_DENSITY_WINDOW   (200, 600);


///////////////////////////////////////////////////////////////////////////////
//! Class represents color palette (lookup table, color vector)
//! for Density Window which transforms density values
//! into intensities (grayscale values).

class CDensityWindow : public vpl::base::CObject, public CColorVector4b
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CDensityWindow);

    //! Default class name.
    VPL_ENTITY_NAME("DensityWindow");

    //! Default compression method.
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

    enum EOptimumEstimationMethod
    {
        OEM_MINMAX,
        OEM_MINMAX_POSITIVE,
        OEM_HISTOGRAM_MEAN_PERCENTAGE
    };

public:
    //! Default constructor.
    CDensityWindow();

    //! Constructor.
    CDensityWindow(const SDensityWindow& Params);

    //! Destructor.
    ~CDensityWindow()
    { }

    void colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CDImage &densityImage, const data::CSlicePropertyContainer &properties, const vpl::img::CDImage &originalImage);

    //! Returns density window attributes.
    const SDensityWindow& getParams() const { return m_Params; }

    //! Returns current density window center.
    int getCenter() const { return m_Params.m_Center; }

    //! Returns density window width.
    int getWidth() const { return m_Params.m_Width; }

    //! Returns upper density window boundary.
    int getMax() const { return (m_Params.m_Center + m_Params.m_Width / 2); }

    //! Returns lower density window boundary.
    int getMin() const { return (m_Params.m_Center - m_Params.m_Width / 2); }

    //! Sets new density window parameters and regenerates the color vector.
    void setParams(const SDensityWindow& Params);

    //! Sets new density window center and regenerates color vector.
    void setCenter(int Center);

    //! Sets new density window width value and regenerates the color vector.
    void setWidth(int Width);

    //! Sets default density window parameters.
    void setDefaultParams(const SDensityWindow& Params);

    //! Sets density window parameters and regenerates the color vector.
    void restoreDefault();

    //! Does object contain relevant data?
    //! - Returns always true.
    virtual bool hasData() { return true; }

    //! Returns color by given density.
    CColor4b& getColor(int i)
    {
        return CColorVector4b::getColor(i - getMinDensity());
    }
    const CColor4b& getColor(int i) const
    {
        return CColorVector4b::getColor(i - getMinDensity());
    }

    //! Returns color by given density.
    //! If the index exceeds allowed range, one of default values is returned.
    CColor4b& getColorSafe(int i)
    {
        return CColorVector4b::getColorSafe(i - getMinDensity());
    }
    const CColor4b& getColorSafe(int i) const
    {
        return CColorVector4b::getColorSafe(i - getMinDensity());
    }

    //! Returns color by given density.
    CColor4b& operator[](int i)
    {
        return CColorVector4b::getColor(i - getMinDensity());
    }
    const CColor4b& operator[](int i) const
    {
        return CColorVector4b::getColor(i - getMinDensity());
    }

    //! Returns maximum allowed input density value.
    static int getMaxDensity()
    {
        return vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMax();
    }

    //! Returns minimum allowed input density value.
    static int getMinDensity()
    {
        return vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin();
    }

    //! Returns number of different density values.
    static int getDensityRange() { return (getMaxDensity() - getMinDensity() + 1); }


    //! Sets internal functor for the special coloring.
    void setColoring(CColoringFunc4b *pFunc);

    //! Gets coloring functor
    const CColoringFunc4b * getColoring();

    //! Sets the default coloring.
    void setDefaultColoring() { m_spColoring = m_spDefaultColoring; }

    //! Returns current coloring model.
    int getColoringType() const { return m_spColoring->getType(); }

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& Changes);

    //! Initializes the object to its default state.
    void init();

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    //! Serialize
    void serialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> & Writer);

    //! Deserialize
    void deserialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> & Reader);

    //! Try to find optimal density window
    void estimateOptimal(const vpl::img::CDImage &densityData, EOptimumEstimationMethod method = OEM_HISTOGRAM_MEAN_PERCENTAGE);

    //! Was this density window deserialized?
    bool wasModified() const { return m_bModifiedFlag; }

protected:
    //! Density window parameters (center and width).
    SDensityWindow m_Params;

    //! Default density window parameters.
    SDensityWindow m_DefaultParams;

    //! Coloring functor.
    CColoringFunc4b::tSmartPtr m_spColoring;
    CColoringFunc4b::tSmartPtr m_spDefaultColoring;

    //! Was window deserialized?
    bool m_bModifiedFlag;

protected:
    //! Checks and normalizes density window parameters.
    static void checkParams(SDensityWindow& Params);

    //! Generates color vector with respect to the density window properties.
    void makeColorVector();
};

DECLARE_SERIALIZATION_WRAPPER( CDensityWindow )

namespace Storage
{
	//! Identifier of a density window - ortho and MIP slices.
	DECLARE_OBJECT(DensityWindow, CDensityWindow, CORE_STORAGE_DENSITY_WINDOW_ID);

	//! Identifier of a density window - RTG slices.
	DECLARE_OBJECT(RTGDensityWindow, CDensityWindow, CORE_STORAGE_RTG_DENSITY_WINDOW_ID);
}

} // namespace data

#endif // CDensityWindow_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
