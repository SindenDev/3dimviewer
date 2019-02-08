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

#include <data/CMultiClassRegionColoring.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn data::CMultiClassRegionColoring::CMultiClassRegionColoring(void)
//
//\brief ! Default constructor. 
////////////////////////////////////////////////////////////////////////////////////////////////////

data::CMultiClassRegionColoring::CMultiClassRegionColoring(void)
    : CUndoProvider( data::Storage::MultiClassRegionColoring::Id, true )
    , m_Colors(1)
    , m_colorsTable(9)
    , m_Regions(tRegions::size_type(NUM_OF_REGIONS))
    , m_DummyRegion("Dummy Region")
    , m_Active(0)
    , m_DummyColor(0, 0, 0, 0)
    , m_maxRegionIndex(0)
{
    // Initialize the storage
    CMultiClassRegionColoring::init();

    m_colorsTable[0] = tColor(255, 128, 128, ALPHA);
    m_colorsTable[1] = tColor(128, 255, 128, ALPHA);
    m_colorsTable[2] = tColor(128, 128, 255, ALPHA);
    m_colorsTable[3] = tColor(255, 255, 128, ALPHA);
    m_colorsTable[4] = tColor(128, 255, 255, ALPHA);
    m_colorsTable[5] = tColor(255, 128, 255, ALPHA);
    m_colorsTable[6] = tColor(255, 128, 64, ALPHA);
    m_colorsTable[7] = tColor(0, 128, 128, ALPHA);
    m_colorsTable[8] = tColor(128, 128, 64, ALPHA);
}

void data::CMultiClassRegionColoring::colorize(vpl::img::CRGBImage &rgbImage, const vpl::img::CImage<data::tRegionVoxel> &regionImage)
{
    const int xSize = std::min(regionImage.getXSize(), rgbImage.getXSize());
    const int ySize = std::min(regionImage.getYSize(), rgbImage.getYSize());
#pragma omp parallel for
    for (int y = 0; y < ySize; ++y)
    {
        for (int x = 0; x < xSize; ++x)
        {
            // NOTE: JS
            // code is commented, because it is slowing down the colorization (eg during drawing)
            // this will affect super voxels plugin, where regions can be selected (and drawn in yellow)
            /*if (getRegionInfo(regionImage(x, y)).isSelected())
            {
                tColor newColor(255, 255, 0, 255);
                rgbImage(x, y) = *(reinterpret_cast<vpl::img::tRGBPixel *>(&newColor));
            }
            else*/
            {
                vpl::img::CRGBPixel pixel = rgbImage(x, y);
                tColor prevColor = *(reinterpret_cast<tColor *>(&pixel));
                tColor currColor = makeColor(regionImage(x, y));
                tColor newColor = blendColors(currColor, prevColor);
                rgbImage(x, y) = *(reinterpret_cast<vpl::img::tRGBPixel *>(&newColor));
            }
        }
    }
}