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

#ifndef __CARBITRARYSLICE_H__
#define __CARBITRARYSLICE_H__

#include <data/CStorageInterface.h>
#include <data/CObjectHolder.h>
#include <data/CSlice.h>
#include <data/storage_ids_core.h>
#include <data/CDensityData.h>
#include <app/Signals.h>

namespace data
{
    class CArbitrarySlice : public CSlice
    {
    public:
        VPL_SHAREDPTR(CArbitrarySlice);

    protected:
        //! Scene voxel size.
        osg::Vec3 m_VoxelSize;

        //! Type of interpolation ( can be nearest or bilinear )
        TInterpolationType m_InterpolationType;

        //! Current plane position
        osg::Vec3 m_center;

        osg::Vec3 m_normal;

        osg::Vec3 m_origin;

        //! Right vector
        osg::Vec3 m_right;

        //! Size of slice
        double m_fWidth, m_fHeight;

        int m_position;
        int m_positionMin;
        int m_positionMax;

        osg::Matrix m_rotationMatrix;

        vpl::img::CImage16 m_regionXCoords;
        vpl::img::CImage16 m_regionYCoords;
        vpl::img::CImage16 m_regionZCoords;
        bool m_regionCoordsInitialized;

    public:
        //! Constructor.
        CArbitrarySlice();

        //! Destructor.
        virtual ~CArbitrarySlice();

        //! Called upon updating from storage
        virtual void update(const CChangedEntries& Changes);

        //! Returns true if changes of a given parent entry may affect this object.
        bool checkDependency(CStorageEntry* pParent)
        {
            return true; 
        }

        //! Re-initializes the slice.
        virtual void init();

        //! Does object contain relevant data?
        virtual bool hasData()
        {
            return false;
        }

        //! Returns voxel parameters
        const osg::Vec3& getSliceVoxelSize() const
        {
            return m_VoxelSize;
        }

        void setPosition(double position);

        //! Return plane position
        int getPosition() const
        {
            return m_position;
        }

        int getPositionMax()
        {
            return m_positionMax;
        }

        int getPositionMin()
        {
            return m_positionMin;
        }

        void setPlaneCenter(const osg::Vec3& newCenter)
        {
            m_center = newCenter;
            recomputePosition();
        }

        //! Return plane position
        const osg::Vec3& getPlaneCenter() const
        {
            return m_center;
        }

        //! Return plane normal
        const osg::Vec3& getPlaneNormal() const
        {
            return m_normal;
        }

        //! Return plane position
        const osg::Vec3& getOrigin() const
        {
            return m_origin;
        }

        //! Return plane up vector
        const osg::Vec3& getPlaneRight() const
        {
            return m_right;
        }

        double getSliceWidth()
        {
            return m_fWidth;
        }

        void setSliceWidth(double width)
        {
            m_fWidth = width;
        }

        double getSliceHeight()
        {
            return m_fHeight;
        }

        void setSliceHeight(double height)
        {
            m_fHeight = height;
        }

        //! Returns width (x-size) of the original image.
        virtual vpl::tSize getWidth() const;

        //! Returns height (y-size) of the original image.
        virtual vpl::tSize getHeight() const;


        void setRotationMatrix(const osg::Matrix& matrix)
        {
            m_rotationMatrix = matrix;
            recomputePosition();
        }

        const osg::Matrix& getRotationMatrix()
        {
            return m_rotationMatrix;
        }

        bool computeSamplingParameters(osg::Vec3& outPosition, osg::Vec3& outVec1, osg::Vec3& outVec2, const CChangedEntries* Changes = NULL);

        template<class VolumeType, class SliceType>
        bool updateProperty(VolumeType* volume, SliceType* slice)
        {
            osg::Vec3 realPosition;
            osg::Vec3 vec1;
            osg::Vec3 vec2;

            if (!computeSamplingParameters(realPosition, vec1, vec2))
            {
                return false;
            }

            data::CObjectPtr<data::CDensityData> spDensityVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()));

            // Calculate voxel size
#if(0)
            osg::Vec3 v2(spDensityVolume->getDX() * vec2[0], spDensityVolume->getDY() * vec2[1], spDensityVolume->getDZ() * vec2[2]);
            m_VoxelSize[0] = v2.length();
            osg::Vec3 v1(spDensityVolume->getDX() * vec1[0], spDensityVolume->getDY() * vec1[1], spDensityVolume->getDZ() * vec1[2]);
            m_VoxelSize[1] = v1.length();
#else
            osg::Vec3f voxelSizeB = osg::Vec3f(1 / spDensityVolume->getDX(), 1 / spDensityVolume->getDY(), 1 / spDensityVolume->getDZ());

            osg::Vec3f vvec1 = osg::componentMultiply(voxelSizeB, vec1);
            vvec1.normalize();
            osg::Vec3f vvec2 = osg::componentMultiply(voxelSizeB, vec2);
            vvec2.normalize();

            osg::Vec3 v2(spDensityVolume->getDX() * vvec2[0], spDensityVolume->getDY() * vvec2[1], spDensityVolume->getDZ() * vvec2[2]);
            m_VoxelSize[0] = v2.length();
            osg::Vec3 v1(spDensityVolume->getDX() * vvec1[0], spDensityVolume->getDY() * vvec1[1], spDensityVolume->getDZ() * vvec1[2]);
            m_VoxelSize[1] = v1.length();
#endif
            if (0 == m_VoxelSize[0] || 0 == m_VoxelSize[1])
            {
                return false;
            }

            // hotfix for extra small voxel sizes to avoid excessive memory requirements
            m_VoxelSize[0] = std::max(0.01f, m_VoxelSize[0]);
            m_VoxelSize[1] = std::max(0.01f, m_VoxelSize[1]);

            // convert to volume coordinates
            data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();

            // Calculate voxel size of slice
            //osg::Vec3d voxelSize = osg::Vec3d(spDensityVolume->getDX(), spDensityVolume->getDY(), spDensityVolume->getDZ());
            double plengthW = m_fWidth / m_VoxelSize[0];
            double plengthH = m_fHeight / m_VoxelSize[1];

            // initialize parameter along the slice
            //double tW = -(plengthW - 1) * 0.5,
            //       tH = -(plengthH - 1) * 0.5;

            // new size of the slice
            vpl::tSize Width = static_cast<vpl::tSize>(plengthW);
            vpl::tSize Height = static_cast<vpl::tSize>(plengthH);
            double cW = (Width - 1) * 0.5;
            double cH = (Height - 1) * 0.5;

            slice->resize(Width, Height);

            osg::Vec3 positionR;
            positionR[0] = CoordConv.fromRealXd(realPosition[0]) + 0.001;
            positionR[1] = CoordConv.fromRealYd(realPosition[1]) + 0.001;
            positionR[2] = CoordConv.fromRealZd(realPosition[2]) + 0.001;

            #pragma omp parallel for
            for (vpl::tSize i = 0; i < Width; i++)
            {
                for (vpl::tSize j = 0; j < Height; j++)
                {
                    // compute point on the slice
                    osg::Vec3d point = positionR + vvec2 * (i - cW) + vvec1 * (j - cH);
                    point[2] += m_VoxelSize[2] * 0.5;

                    if (!(point[0] < 0 || point[0] >= volume->getXSize() ||
                          point[1] < 0 || point[1] >= volume->getYSize() ||
                          point[2] < 0 || point[2] >= volume->getZSize()))
                    {
                        vpl::img::CPoint3D p(point[0], point[1], point[2]);

                        vpl::tSize xx, yy, zz;
                        xx = static_cast<vpl::tSize>(point[0]);
                        yy = static_cast<vpl::tSize>(point[1]);
                        zz = static_cast<vpl::tSize>(point[2]);
                        (*slice)(i, j) = volume->at(xx, yy, zz);
                    }
                    else
                    {
                        (*slice)(i, j) = 0;
                    }
                }
            }

            return true;
        }

    protected:
        //! Update slice texture
        void updateTextureData(const osg::Vec3& position, const osg::Vec3& vec1, const osg::Vec3& vec2, bool updateDensityImage = true, bool updateRegionImage = true);

        void recomputePosition();
    };

    namespace Storage
    {
        //! Storage identifier of arbitrary slice
        DECLARE_OBJECT(ArbitrarySlice, data::CArbitrarySlice, CORE_STORAGE_SLICE_ARB_ID);
    }
} // namespace data

#endif // __CARBITRARYSLICE_H__
