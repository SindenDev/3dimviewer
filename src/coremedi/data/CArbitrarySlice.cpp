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

#include "data/CArbitrarySlice.h"
#include <data/CMultiClassRegionData.h>
#include <data/CRegionData.h>
#include <geometry/base/types.h>
#include <geometry/base/CPlane.h>

//=============================================================================
data::CArbitrarySlice::CArbitrarySlice()
    : CSlice()
    , m_VoxelSize(1.0, 1.0, 1.0)
    , m_InterpolationType(data::/*INTERPOLATION_NEAREST*/INTERPOLATION_BILINEAR)
    , m_center(0, 0, 0)
    , m_fWidth(0)
    , m_fHeight(0)
    , m_position(0.0)
    , m_positionMin(0)
    , m_positionMax(0)
    , m_origin(0, 0, 0)
    , m_regionXCoords(INIT_SIZE, INIT_SIZE, 0)
    , m_regionYCoords(INIT_SIZE, INIT_SIZE, 0)
    , m_regionZCoords(INIT_SIZE, INIT_SIZE, 0)
    , m_regionCoordsInitialized(false)
{
    init();
}

//=====================================================================================================================
data::CArbitrarySlice::~CArbitrarySlice()
{
}

//=====================================================================================================================
void data::CArbitrarySlice::init()
{
    CSlice::init();

    m_regionXCoords.fillEntire(vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin());
    m_regionYCoords.fillEntire(vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin());
    m_regionZCoords.fillEntire(vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin());

    m_regionCoordsInitialized = false;
    m_fWidth = 0;
    m_fHeight = 0;

    m_rotationMatrix = osg::Matrix::rotate(osg::DegreesToRadians(50.0), osg::Vec3(0.0, 1.0, 0.0));

    data::CObjectPtr<data::CDensityData> volume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));

    double l = osg::Vec3(volume->getXSize(), volume->getYSize(), volume->getZSize()).length();
    m_positionMin = 0;
    m_positionMax = l;

    m_fWidth = volume->getXSize() * volume->getDX();
    m_fHeight = volume->getYSize() * volume->getDY();

    // save slice position
    m_center = osg::Vec3(volume->getXSize() * volume->getDX() * 0.5, volume->getYSize() * volume->getDY() * 0.5, volume->getZSize() * volume->getDZ() * 0.5);

    osg::Vec3 normal(0, 0, 1);
    osg::Vec3 right(0, 1, 0);

    m_normal = normal * m_rotationMatrix;
    m_right = right * m_rotationMatrix;
    m_normal.normalize();
    m_right.normalize();

    recomputePosition();
}

//=============================================================================
void data::CArbitrarySlice::update(const CChangedEntries& Changes)
{
    if (!m_updateEnabled)
    {
        return;
    }

    //time_t t = clock();

    osg::Vec3 position;
    osg::Vec3 vec1;
    osg::Vec3 vec2;

    bool regionChanged = Changes.checkIdentity(Storage::MultiClassRegionData::Id) || Changes.checkIdentity(Storage::RegionData::Id) || Changes.hasChanged(Storage::MultiClassRegionData::Id) || Changes.hasChanged(Storage::RegionData::Id);

    if (regionChanged)
    {
        updateTextureData(m_center, m_right, m_normal ^ m_right, false, true);

        //t = clock() - t;
        //qDebug() << float(t) / CLOCKS_PER_SEC;
    }
    else
    {
        m_regionCoordsInitialized = false;

        if (computeSamplingParameters(position, vec1, vec2, &Changes))
        {
            updateTextureData(position, vec1, vec2);
        }
    }
}

void data::CArbitrarySlice::recomputePosition()
{
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));

    vpl::tSize xSize = spVolume->getXSize();
    vpl::tSize ySize = spVolume->getYSize();
    vpl::tSize zSize = spVolume->getZSize();

    double dx = spVolume->getDX();
    double dy = spVolume->getDY();
    double dz = spVolume->getDZ();

    osg::Vec3 normal(0, 0, 1);
    osg::Vec3 right(0, 1, 0);

    m_normal = normal * m_rotationMatrix;
    m_right = right * m_rotationMatrix;
    m_normal.normalize();
    m_right.normalize();

    osg::Vec3 p1 = m_center - (m_normal * 10000.0);

    geometry::CPlane slicePlane(geometry::convert3<geometry::Vec3, osg::Vec3>(m_normal), geometry::convert3<geometry::Vec3, osg::Vec3>(p1));
    std::vector<geometry::Vec3> volumeCorners;
    volumeCorners.push_back(geometry::Vec3(0.0, 0.0, 0.0));
    volumeCorners.push_back(geometry::Vec3(xSize * dx, 0.0, 0.0));
    volumeCorners.push_back(geometry::Vec3(0.0, ySize * dy, 0.0));
    volumeCorners.push_back(geometry::Vec3(xSize * dx, ySize * dy, 0.0));
    volumeCorners.push_back(geometry::Vec3(0.0, 0.0, zSize * dz));
    volumeCorners.push_back(geometry::Vec3(xSize * dx, 0.0, zSize * dz));
    volumeCorners.push_back(geometry::Vec3(0.0, ySize * dy, zSize * dz));
    volumeCorners.push_back(geometry::Vec3(xSize * dx, ySize * dy, zSize * dz));
    
    double minDist = 1000000.0;
    double maxDist = -1.0;

    for (int i = 0; i < volumeCorners.size(); ++i)
    {
        double d = std::fabs(slicePlane.distance(volumeCorners[i]));

        if (d < minDist)
        {
            minDist = d;
        }

        if (d > maxDist)
        {
            maxDist = d;
        }
    }

    // compute voxel size
    std::vector<osg::Vec3> planeNormals;
    std::vector<osg::Vec3> planePoints;

    planeNormals.push_back(osg::Vec3(0, 0, 1));
    planeNormals.push_back(osg::Vec3(0, 1, 0));
    planeNormals.push_back(osg::Vec3(1, 0, 0));
    planeNormals.push_back(osg::Vec3(0, 0, -1));
    planeNormals.push_back(osg::Vec3(0, -1, 0));
    planeNormals.push_back(osg::Vec3(-1, 0, 0));

    planePoints.push_back(osg::Vec3(0, 0, 0));
    planePoints.push_back(osg::Vec3(0, 0, 0));
    planePoints.push_back(osg::Vec3(0, 0, 0));
    planePoints.push_back(osg::Vec3(dx, dy, dz));
    planePoints.push_back(osg::Vec3(dx, dy, dz));
    planePoints.push_back(osg::Vec3(dx, dy, dz));

    osg::Vec3 p2 = osg::Vec3(dx * 0.5, dy * 0.5, dz * 0.5);

    // get intersections with voxel block
    for (int i = 0; i < planeNormals.size(); ++i)
    {
        float d = planeNormals[i] * planePoints[i];

        if (planeNormals[i] * m_normal == 0)
        {
            // No intersection, the line is parallel to the plane
            continue;
        }

        // Compute the X value for the directed line ray intersecting the plane
        float x = (d - (planeNormals[i] * p2)) / (planeNormals[i] * m_normal);

        // intersection
        osg::Vec3 intersection = p2 + (m_normal * x);
        if (intersection[0] + 0.0001 >= 0 && intersection[0] - 0.0001 <= dx && intersection[1] + 0.0001 >= 0 && intersection[1] - 0.0001 <= dy && intersection[2] + 0.0001 >= 0 && intersection[2] - 0.0001 <= dz)
        {
            double l = (p2 - intersection).length();
            m_VoxelSize[2] = l * 2.0;
            break;
        }
    }

    m_origin = p1 + (m_normal * minDist);

    bool neg = m_position < 0;

    m_position = (int)((std::fabs(slicePlane.distance(geometry::convert3<geometry::Vec3, osg::Vec3>(m_center))) - minDist) / m_VoxelSize[2]);

    m_positionMin = 0;
    m_positionMax = (maxDist - minDist) / m_VoxelSize[2] - 0.5;

    if (neg)
    {
        m_position = -m_position;
    }
}

bool data::CArbitrarySlice::computeSamplingParameters(osg::Vec3& outPosition, osg::Vec3& outVec1, osg::Vec3& outVec2, const CChangedEntries* Changes)
{
    osg::Vec3 normal(0, 0, 1);
    osg::Vec3 right(0, 1, 0);

    m_normal = normal * m_rotationMatrix;
    m_right = right * m_rotationMatrix;
    m_normal.normalize();
    m_right.normalize();

    // update texture
    outPosition = m_center;
    outVec1 = m_right;
    outVec2 = m_normal ^ m_right;
    
    return true;
}


//=============================================================================
// Update texture data from the volume
void data::CArbitrarySlice::updateTextureData(const osg::Vec3& realPosition, const osg::Vec3& vec1, const osg::Vec3& vec2, bool updateDensityImage, bool updateRegionImage)
{
    if (realPosition.isNaN() || vec1.isNaN() || vec2.isNaN())
    {
        return;
    }

    data::CObjectPtr<data::CDensityData> volume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()));

    // Calculate voxel size
#if(0)
    osg::Vec3 v2(volume->getDX() * vec2[0], volume->getDY() * vec2[1], volume->getDZ() * vec2[2]);
    m_VoxelSize[0] = v2.length();
    osg::Vec3 v1(volume->getDX() * vec1[0], volume->getDY() * vec1[1], volume->getDZ() * vec1[2]);
    m_VoxelSize[1] = v1.length();
#else
    osg::Vec3f voxelSizeB = osg::Vec3f(1 / volume->getDX(), 1 / volume->getDY(), 1 / volume->getDZ());

    osg::Vec3f vvec1 = osg::componentMultiply(voxelSizeB, vec1);
    vvec1.normalize();
    osg::Vec3f vvec2 = osg::componentMultiply(voxelSizeB, vec2);
    vvec2.normalize();

    osg::Vec3 v2(volume->getDX() * vvec2[0], volume->getDY() * vvec2[1], volume->getDZ() * vvec2[2]);
    m_VoxelSize[0] = v2.length();
    osg::Vec3 v1(volume->getDX() * vvec1[0], volume->getDY() * vvec1[1], volume->getDZ() * vvec1[2]);
    m_VoxelSize[1] = v1.length();
#endif

    if (0 == m_VoxelSize[0] || 0 == m_VoxelSize[1])
    {
        return;
    }

    // hotfix for extra small voxel sizes to avoid excessive memory requirements
    m_VoxelSize[0] = std::max(0.01f, m_VoxelSize[0]);
    m_VoxelSize[1] = std::max(0.01f, m_VoxelSize[1]);

    // convert to volume coordinates
    osg::Vec3 position;
    data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();    
    // subtract half voxel so the middle of the first voxel is mapped to zero and the middle of the last one is mapped to the last-1
    position[0] = CoordConv.fromRealXd(realPosition[0]) - 0.5 * m_VoxelSize[0]; 
    position[1] = CoordConv.fromRealYd(realPosition[1]) - 0.5 * m_VoxelSize[1];
    position[2] = CoordConv.fromRealZd(realPosition[2]) - 0.5 * m_VoxelSize[2];

    // Calculate voxel size of slice
//    osg::Vec3d voxelSize = osg::Vec3d(volume->getDX(), volume->getDY(), volume->getDZ());
    double plengthW = m_fWidth / m_VoxelSize[0];
    double plengthH = m_fHeight / m_VoxelSize[1];

    // initialize parameter along the slice
    //double tW = -(plengthW - 1) * 0.5,
    //       tH = -(plengthH - 1) * 0.5;

    // new size of the slice
    vpl::tSize Width = static_cast<vpl::tSize>(plengthW);
    vpl::tSize Height = static_cast<vpl::tSize>(plengthH);

    // create data
    m_DensityData.resize(Width, Height);

    if (NULL == m_DensityData.getRowPtr(0))
    {
        assert(false);
        return;
    }

    double cW = (Width - 1) * 0.5;
    double cH = (Height - 1) * 0.5;

    if (updateDensityImage)
    {
#pragma omp parallel for
        for (vpl::tSize i = 0; i < Width; ++i)
        {
            for (vpl::tSize j = 0; j < Height; ++j)
            {
                // compute point on the slice
                osg::Vec3d point = position + vvec2 * (i - cW) + vvec1 * (j - cH);

                if (!(point[0] < 0 || point[0] >= volume->getXSize() ||
                    point[1] < 0 || point[1] >= volume->getYSize() ||
                    point[2] < 0 || point[2] >= volume->getZSize()))
                {
                    vpl::img::CPoint3D p(point[0], point[1], point[2]);

                    int iDensity;
                    vpl::tSize xx, yy, zz;

                    switch (m_InterpolationType)
                    {
                    case data::INTERPOLATION_BILINEAR:
                        iDensity = volume->interpolate(p);
                        break;
                    default:
                        xx = static_cast<vpl::tSize>(point[0]);
                        yy = static_cast<vpl::tSize>(point[1]);
                        zz = static_cast<vpl::tSize>(point[2]);
                        iDensity = volume->at(xx, yy, zz);
                    }

                    m_DensityData(i, j) = iDensity;
                }
                else
                {
                    m_DensityData(i, j) = -1500;
                }
            }
        }
    }

//#define SEARCHNEIGHBORS

    if (updateRegionImage)
    {
        // Check if region coloring is enabled
        if (APP_STORAGE.isEntryValid(Storage::MultiClassRegionData::Id) && VPL_SIGNAL(SigIsMultiClassRegionColoringEnabled).invoke2())
        {
            CObjectPtr<CMultiClassRegionData> spRegionVolume(APP_STORAGE.getEntry(Storage::MultiClassRegionData::Id));

            if (spRegionVolume->isColoringEnabled())
            {
                osg::Vec3 positionR;

                m_multiClassRegionData.resize(Width, Height);

                if (!m_regionCoordsInitialized)
                {
                    m_regionXCoords.resize(Width, Height);
                    m_regionYCoords.resize(Width, Height);
                    m_regionZCoords.resize(Width, Height);

                    positionR[0] = CoordConv.fromRealXd(realPosition[0]);
                    positionR[1] = CoordConv.fromRealYd(realPosition[1]);
                    positionR[2] = CoordConv.fromRealZd(realPosition[2]);
                }

                vpl::tSize xSize = spRegionVolume->getXSize();
                vpl::tSize ySize = spRegionVolume->getYSize();
                vpl::tSize zSize = spRegionVolume->getZSize();


                if (!m_regionCoordsInitialized)
                {
#pragma omp parallel for
                    for (vpl::tSize i = 0; i < Width; ++i)
                    {
                        for (vpl::tSize j = 0; j < Height; ++j)
                        {
                            // compute point on the slice
                            osg::Vec3d point = positionR + vvec2 * (i - cW) + vvec1 * (j - cH);

                            if (point[0] >= 0 && point[0] < xSize &&
                                point[1] >= 0 && point[1] < ySize &&
                                point[2] >= 0 && point[2] < zSize)
                            {
                                vpl::img::CPoint3D p(point[0], point[1], point[2]);

                                vpl::tSize xx, yy, zz;
                                xx = static_cast<vpl::tSize>(point[0]);// + 0.5);
                                yy = static_cast<vpl::tSize>(point[1]);// + 0.5);
                                zz = static_cast<vpl::tSize>(point[2]);// + 0.5);

#ifdef SEARCHNEIGHBORS
                                CMultiClassRegionData::tVoxel val = 0;

                                /*for (int z = zz; z <= zz + 1; ++z)
                                {
                                    for (int y = yy; y <= yy + 1; ++y)
                                    {
                                        for (int x = xx; x <= xx + 1; ++x)
                                        {
                                            val |= spRegionVolume->at(x, y, z);
                                        }
                                    }
                                }*/

                                val |= spRegionVolume->at(xx, yy, zz);
                                val |= spRegionVolume->at(xx + 1, yy, zz);
                                val |= spRegionVolume->at(xx, yy + 1, zz);
                                val |= spRegionVolume->at(xx + 1, yy + 1, zz);
                                val |= spRegionVolume->at(xx, yy, zz + 1);
                                val |= spRegionVolume->at(xx + 1, yy, zz + 1);
                                val |= spRegionVolume->at(xx, yy + 1, zz + 1);
                                val |= spRegionVolume->at(xx + 1, yy + 1, zz + 1);

                                m_multiClassRegionData(i, j) = val;
#else
                                m_multiClassRegionData(i, j) = spRegionVolume->at(xx, yy, zz);
#endif
                                m_regionXCoords(i, j) = xx;
                                m_regionYCoords(i, j) = yy;
                                m_regionZCoords(i, j) = zz;
                            }
                            else
                            {
                                m_multiClassRegionData(i, j) = 0;
                                m_regionXCoords(i, j) = -1;
                                m_regionYCoords(i, j) = -1;
                                m_regionZCoords(i, j) = -1;
                            }
                        }
                    }
                }
                else
                {
#pragma omp parallel for
                    for (vpl::tSize i = 0; i < Width; ++i)
                    {
                        for (vpl::tSize j = 0; j < Height; ++j)
                        {
                            vpl::tSize xx, yy, zz;
                            xx = m_regionXCoords(i, j);
                            yy = m_regionYCoords(i, j);
                            zz = m_regionZCoords(i, j);

                            if (xx >= 0 && xx < xSize &&
                                yy >= 0 && yy < ySize &&
                                zz >= 0 && zz < zSize)
                            {
#ifdef SEARCHNEIGHBORS
                                CMultiClassRegionData::tVoxel val = 0;

                                /*for (int z = zz; z <= zz + 1; ++z)
                                {
                                    for (int y = yy; y <= yy + 1; ++y)
                                    {
                                        for (int x = xx; x <= xx + 1; ++x)
                                        {
                                            val |= spRegionVolume->at(x, y, z);
                                        }
                                    }
                                }*/

                                val |= spRegionVolume->at(xx, yy, zz);
                                val |= spRegionVolume->at(xx + 1, yy, zz);
                                val |= spRegionVolume->at(xx, yy + 1, zz);
                                val |= spRegionVolume->at(xx + 1, yy + 1, zz);
                                val |= spRegionVolume->at(xx, yy, zz + 1);
                                val |= spRegionVolume->at(xx + 1, yy, zz + 1);
                                val |= spRegionVolume->at(xx, yy + 1, zz + 1);
                                val |= spRegionVolume->at(xx + 1, yy + 1, zz + 1);

                                m_multiClassRegionData(i, j) = val;
#else
                                m_multiClassRegionData(i, j) = spRegionVolume->at(xx, yy, zz);
#endif
                            }
                            else
                            {
                                m_multiClassRegionData(i, j) = 0;
                            }
                        }
                    }
                }

                m_regionCoordsInitialized = true;
            }
            else
            {
                if (m_multiClassRegionData.width() != 0 || m_multiClassRegionData.height() != 0)
                {
                    m_multiClassRegionData.resize(0, 0);
                    m_regionXCoords.resize(0, 0);
                    m_regionYCoords.resize(0, 0);
                    m_regionZCoords.resize(0, 0);
                }
            }
        }
        else
        {
            if (m_multiClassRegionData.width() != 0 || m_multiClassRegionData.height() != 0)
            {
                m_multiClassRegionData.resize(0, 0);
                m_regionXCoords.resize(0, 0);
                m_regionYCoords.resize(0, 0);
                m_regionZCoords.resize(0, 0);
            }
        }
    }

    /*CSlicePropertyContainer::tPropertyList propertyList = m_properties.propertyList();
    for (CSlicePropertyContainer::tPropertyList::iterator it = propertyList.begin(); it != propertyList.end(); ++it)
    {
        (*it)->update(this);
    }*/

    // Update rgba data - HOTFIX false
    bool rgba_updated = this->updateRGBData(false, data::Storage::DensityWindow::Id);

    // Update the OSG texture
    updateTexture(rgba_updated);
}

///////////////////////////////////////////////////////////////////////////////

//! Returns width (x-size) of the original image.
vpl::tSize data::CArbitrarySlice::getWidth() const
{ 
    return m_DensityData.getXSize(); 
}

//! Returns height (y-size) of the original image.
vpl::tSize data::CArbitrarySlice::getHeight() const 
{ 
    return m_DensityData.getYSize(); 
}

void data::CArbitrarySlice::setPosition(double position)
{
    m_position = (int)position;

    double pos = (m_position + 0.5) * m_VoxelSize[2];

    m_center = m_origin + (m_normal * pos);

    recomputePosition();
}