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

#ifndef CREGIONSVISUALIZER_H
#define CREGIONSVISUALIZER_H

#include <osg/Geometry>
#include <data/CDensityData.h>
#include <coremedi/app/Signals.h>
#include <alg/cmarchingsquares.h>

#include <osg/CGeneralObjectObserverOSG.h>
#include <data/CRegionData.h>
#include <data/COrthoSlice.h>

namespace osg
{
    class CRegionsVisualizer : public osg::MatrixTransform
    {
    protected:
        OSGCanvas *m_canvas;
        osg::ref_ptr<osg::Geode> m_geode;
        osg::ref_ptr<osg::Geometry> m_geometry;
        osg::ref_ptr<osg::Vec3Array> m_vertices;
        osg::ref_ptr<osg::DrawElementsUInt> m_indices;
        osg::ref_ptr<osg::MatrixTransform> m_transform;
        bool m_visible;
        bool m_validData;
        bool m_autoPositioning;
		osg::Matrix m_matrix;

    public:
        //! Ctor
		CRegionsVisualizer(osg::Matrix matrix, OSGCanvas *canvas, bool autoPositioning = true);

        //! Dtor
		~CRegionsVisualizer();

        //! Sets visibility
        void setVisibility(bool show, bool forceRedraw = false);

        //! Gets visibility
        bool isVisible();

        //! Set color
        void setColor(const osg::Vec4 & color);

    protected:

        //! Updates data
        virtual void updateData();
    };

    // Model cut visualizer class template

    template <class T1, class T2>
	class CRegionsVisualizerForSlice : public CRegionsVisualizer, public scene::CGeneralObjectObserverOSG<CRegionsVisualizerForSlice<T1, T2> >
    {
    private:
		int m_sliceId;

    public:
        //! Ctor
		CRegionsVisualizerForSlice(osg::Matrix matrix, int sliceId, OSGCanvas *canvas, bool autoPositioning = true)
			: CRegionsVisualizer(matrix, canvas, autoPositioning),
			m_sliceId(sliceId)
        {
            this->setCanvas(canvas);
            scene::CGeneralObjectObserverOSG<CRegionsVisualizerForSlice<T1, T2> >::connect(APP_STORAGE.getEntry(sliceId).get());
            scene::CGeneralObjectObserverOSG<CRegionsVisualizerForSlice<T1, T2> >::connect(APP_STORAGE.getEntry(data::Storage::RegionData::Id).get());
            this->setupObserver(this);
        }

        //! Dtor
		~CRegionsVisualizerForSlice()
        { }

        //! Updates geometry according to changes in storage
		virtual void updateFromStorage()
        {
            if (!m_visible)
            {
                this->m_validData = false;
                return;
            }

            this->updateData();
            this->setVisibility(m_visible);
        }
    protected:
		virtual void getRegionData(vpl::img::CImage16 &slice, int sliceId){};

        //! Updates data
        virtual void updateData()
        {
			vpl::img::CImage16 slice;

			getRegionData(slice, m_sliceId);

            m_vertices->clear();
            m_indices->clear();

			alg::CMarchingSquares<vpl::img::tPixel16> ms(slice);

			std::vector<geometry::Vec2> vertices;
			std::vector<int> indices;

			ms.process(vertices, indices);

            for (std::size_t i = 0; i < vertices.size(); i++)
            {
				osg::Vec4 v4 = m_matrix * osg::Vec4(vertices[i][0], vertices[i][1], 0.0, 1.0);
				m_vertices->push_back(osg::Vec3(v4[0], v4[1], v4[2]));
            }

            for (std::size_t i = 0; i < indices.size(); i++)
            {
                m_indices->push_back(indices[i]);
            }

            if (0 == m_vertices->size()) // prevent crash
            {
                m_vertices->push_back(osg::Vec3(0,0,0));
                m_indices->push_back(0);
            }

            m_vertices->dirty();
            m_indices->dirty();

            m_geometry->dirtyGLObjects();
            m_geometry->dirtyBound();

            if (m_autoPositioning)
            {
                int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
                if (datasetId != data::CUSTOM_DATA)
                {
                    data::CObjectPtr<data::CDensityData> spDensityData(APP_STORAGE.getEntry(datasetId));
                    osg::Vec3 translation = osg::Vec3(spDensityData->getXSize(), spDensityData->getYSize(), spDensityData->getZSize());
                    translation = osg::componentMultiply(translation, osg::Vec3(spDensityData->getDX(), spDensityData->getDY(), spDensityData->getDZ()));
                    translation = osg::componentMultiply(translation, osg::Vec3(0.5f, 0.5f, 0.5f));
					setMatrix(osg::Matrix::scale(spDensityData->getDX(), spDensityData->getDY(), spDensityData->getDZ()) * osg::Matrix::translate(-translation));
                }
            }

            //m_transform->setMatrix(spModelCut->getTransformationMatrix());

            data::CColor4f color(0.6, 0.6, 0.3, 1);
            setColor(osg::Vec4(color[0], color[1], color[2], color[3]));

            m_validData = true;
        }

    };

	class CRegionsVisualizerForSliceXY : public CRegionsVisualizerForSlice < data::COrthoSliceXY, data::CRegionData >
	{
		public:
			//! Ctor
			CRegionsVisualizerForSliceXY(osg::Matrix matrix, int sliceId, OSGCanvas *canvas, bool autoPositioning = true)
				: CRegionsVisualizerForSlice(matrix, sliceId, canvas, autoPositioning)
			{ }

			//! Dtor
			~CRegionsVisualizerForSliceXY()
			{ }

		protected:
			void getRegionData(vpl::img::CImage16 &slice, int sliceId)
			{
				data::CObjectPtr<data::CRegionData> pRegion(APP_STORAGE.getEntry(data::Storage::RegionData::Id));
				data::CObjectPtr<data::COrthoSliceXY> pSlice(APP_STORAGE.getEntry(sliceId));

				slice.resize(pRegion->getXSize(), pRegion->getYSize());

				pRegion->getPlaneXY(pSlice->getPosition(), slice);
			}
	};

	class CRegionsVisualizerForSliceXZ : public CRegionsVisualizerForSlice < data::COrthoSliceXZ, data::CRegionData >
	{
		public:
			//! Ctor
			CRegionsVisualizerForSliceXZ(osg::Matrix matrix, int sliceId, OSGCanvas *canvas, bool autoPositioning = true)
				: CRegionsVisualizerForSlice(matrix, sliceId, canvas, autoPositioning)
			{ }

			//! Dtor
			~CRegionsVisualizerForSliceXZ()
			{ }

		protected:
			void getRegionData(vpl::img::CImage16 &slice, int sliceId)
			{
				data::CObjectPtr<data::CRegionData> pRegion(APP_STORAGE.getEntry(data::Storage::RegionData::Id));
				data::CObjectPtr<data::COrthoSliceXZ> pSlice(APP_STORAGE.getEntry(sliceId));

				slice.resize(pRegion->getXSize(), pRegion->getZSize());

				pRegion->getPlaneXZ(pSlice->getPosition(), slice);
			}
	};

	class CRegionsVisualizerForSliceYZ : public CRegionsVisualizerForSlice < data::COrthoSliceYZ, data::CRegionData >
	{
		public:
			//! Ctor
			CRegionsVisualizerForSliceYZ(osg::Matrix matrix, int sliceId, OSGCanvas *canvas, bool autoPositioning = true)
				: CRegionsVisualizerForSlice(matrix, sliceId, canvas, autoPositioning)
			{ }

			//! Dtor
			~CRegionsVisualizerForSliceYZ()
			{ }

		protected:
			void getRegionData(vpl::img::CImage16 &slice, int sliceId)
			{
				data::CObjectPtr<data::CRegionData> pRegion(APP_STORAGE.getEntry(data::Storage::RegionData::Id));
				data::CObjectPtr<data::COrthoSliceYZ> pSlice(APP_STORAGE.getEntry(sliceId));

				slice.resize(pRegion->getYSize(), pRegion->getZSize());

				pRegion->getPlaneYZ(pSlice->getPosition(), slice);
			}
	};

} // namespace osg

#endif // CREGIONSVISUALIZER_H
