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

#ifndef CVolumeOfInterestVisualizer_H
#define CVolumeOfInterestVisualizer_H

#include <osg/CObjectObserverOSG.h>
#include <osg/Geometry>
#include <data/CDensityData.h>
#include <data/CVolumeOfInterestData.h>
#include <coremedi/app/Signals.h>
#include <alg/cmarchingsquares.h>
#include <data/CDataStorage.h>

#include <osg/CMultiObjectObserverOSG.h>
#include <data/CRegionData.h>
#include <data/COrthoSlice.h>

namespace osg
{
	class CVolumeOfInterestVisualizer : public osg::MatrixTransform
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
		CVolumeOfInterestVisualizer(osg::Matrix matrix, OSGCanvas *canvas, bool autoPositioning = true);

        //! Dtor
		~CVolumeOfInterestVisualizer();

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
	class CVolumeOfInterestVisualizerForSlice : public CVolumeOfInterestVisualizer, public scene::CMultiObjectObserverOSG<T1, T2>
    {
    private:
		int m_sliceId;

    public:
        //! Ctor
		CVolumeOfInterestVisualizerForSlice(osg::Matrix matrix, int sliceId, OSGCanvas *canvas, bool autoPositioning = true)
			: CVolumeOfInterestVisualizer(matrix, canvas, autoPositioning),
			m_sliceId(sliceId)
        {
            this->setCanvas(canvas);
			APP_STORAGE.connect(sliceId, this);
			APP_STORAGE.connect(data::Storage::VolumeOfInterestData::Id, this);
            this->setupObserver(this);
        }

        //! Dtor
		~CVolumeOfInterestVisualizerForSlice()
        { }

        //! Updates geometry according to changes in storage
		virtual void updateFromStorage(int ChangedEntries)
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
		virtual void getRectangleOfInterest(std::vector<geometry::Vec2>& vertices, std::vector<int>& indices, int sliceId){};

        //! Updates data
        virtual void updateData()
        {
            m_vertices->clear();
            m_indices->clear();

			std::vector<geometry::Vec2> vertices;
			std::vector<int> indices;

			getRectangleOfInterest(vertices, indices, m_sliceId);

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

            m_geometry->dirtyDisplayList();
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

            data::CColor4f color(1, 1, 0, 1);
            setColor(osg::Vec4(color[0], color[1], color[2], color[3]));

            m_validData = true;
        }

    };

	class CVolumeOfInterestVisualizerForSliceXY : public CVolumeOfInterestVisualizerForSlice < data::COrthoSliceXY, data::CVolumeOfInterestData >
	{
		public:
			//! Ctor
			CVolumeOfInterestVisualizerForSliceXY(osg::Matrix matrix, int sliceId, OSGCanvas *canvas, bool autoPositioning = true)
				: CVolumeOfInterestVisualizerForSlice(matrix, sliceId, canvas, autoPositioning)
			{ }

			//! Dtor
			~CVolumeOfInterestVisualizerForSliceXY()
			{ }

		protected:
			void getRectangleOfInterest(std::vector<geometry::Vec2>& vertices, std::vector<int>& indices, int sliceId)
			{
				data::CObjectPtr<data::CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(data::Storage::VolumeOfInterestData::Id));
				data::CObjectPtr<data::COrthoSliceXY> pSlice(APP_STORAGE.getEntry(sliceId));

				if (pSlice->getPosition() < spVOI->getMinZ() || pSlice->getPosition() > spVOI->getMaxZ())
					return;

				vertices.push_back(geometry::Vec2(spVOI->getMinX(), spVOI->getMinY()));
				vertices.push_back(geometry::Vec2(spVOI->getMinX(), spVOI->getMaxY() + 1));
				vertices.push_back(geometry::Vec2(spVOI->getMaxX() + 1, spVOI->getMinY()));
				vertices.push_back(geometry::Vec2(spVOI->getMaxX() + 1, spVOI->getMaxY() + 1));

				indices.push_back(0);
				indices.push_back(1);
				indices.push_back(0);
				indices.push_back(2);
				indices.push_back(1);
				indices.push_back(3);
				indices.push_back(2);
				indices.push_back(3);
			}
	};

	class CVolumeOfInterestVisualizerForSliceXZ : public CVolumeOfInterestVisualizerForSlice < data::COrthoSliceXZ, data::CVolumeOfInterestData >
	{
		public:
			//! Ctor
			CVolumeOfInterestVisualizerForSliceXZ(osg::Matrix matrix, int sliceId, OSGCanvas *canvas, bool autoPositioning = true)
				: CVolumeOfInterestVisualizerForSlice(matrix, sliceId, canvas, autoPositioning)
			{ }

			//! Dtor
			~CVolumeOfInterestVisualizerForSliceXZ()
			{ }

		protected:
			void getRectangleOfInterest(std::vector<geometry::Vec2>& vertices, std::vector<int>& indices, int sliceId)
			{
				data::CObjectPtr<data::CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(data::Storage::VolumeOfInterestData::Id));
				data::CObjectPtr<data::COrthoSliceXZ> pSlice(APP_STORAGE.getEntry(sliceId));

				if (pSlice->getPosition() < spVOI->getMinY() || pSlice->getPosition() > spVOI->getMaxY())
					return;

				vertices.push_back(geometry::Vec2(spVOI->getMinX(), spVOI->getMinZ()));
				vertices.push_back(geometry::Vec2(spVOI->getMinX(), spVOI->getMaxZ() + 1));
				vertices.push_back(geometry::Vec2(spVOI->getMaxX() + 1, spVOI->getMinZ()));
				vertices.push_back(geometry::Vec2(spVOI->getMaxX() + 1, spVOI->getMaxZ() + 1));

				indices.push_back(0);
				indices.push_back(1);
				indices.push_back(0);
				indices.push_back(2);
				indices.push_back(1);
				indices.push_back(3);
				indices.push_back(2);
				indices.push_back(3);
			}
	};

	class CVolumeOfInterestVisualizerForSliceYZ : public CVolumeOfInterestVisualizerForSlice < data::COrthoSliceYZ, data::CVolumeOfInterestData >
	{
		public:
			//! Ctor
			CVolumeOfInterestVisualizerForSliceYZ(osg::Matrix matrix, int sliceId, OSGCanvas *canvas, bool autoPositioning = true)
				: CVolumeOfInterestVisualizerForSlice(matrix, sliceId, canvas, autoPositioning)
			{ }

			//! Dtor
			~CVolumeOfInterestVisualizerForSliceYZ()
			{ }

		protected:
			void getRectangleOfInterest(std::vector<geometry::Vec2>& vertices, std::vector<int>& indices, int sliceId)
			{
				data::CObjectPtr<data::CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(data::Storage::VolumeOfInterestData::Id));
				data::CObjectPtr<data::COrthoSliceYZ> pSlice(APP_STORAGE.getEntry(sliceId));

				if (pSlice->getPosition() < spVOI->getMinX() || pSlice->getPosition() > spVOI->getMaxX())
					return;

				vertices.push_back(geometry::Vec2(spVOI->getMinY(), spVOI->getMinZ()));
				vertices.push_back(geometry::Vec2(spVOI->getMinY(), spVOI->getMaxZ() + 1));
				vertices.push_back(geometry::Vec2(spVOI->getMaxY() + 1, spVOI->getMinZ()));
				vertices.push_back(geometry::Vec2(spVOI->getMaxY() + 1, spVOI->getMaxZ() + 1));

				indices.push_back(0);
				indices.push_back(1);
				indices.push_back(0);
				indices.push_back(2);
				indices.push_back(1);
				indices.push_back(3);
				indices.push_back(2);
				indices.push_back(3);
			}
	};

} // namespace osg

#endif // CVolumeOfInterestVisualizer_H
