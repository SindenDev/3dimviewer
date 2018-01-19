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

#ifndef CModelCutVisualizer_H
#define CModelCutVisualizer_H

#include <osg/CObjectObserverOSG.h>
#include <osg/Geometry>
#include <data/CModelCut.h>
#include <data/CDensityData.h>
#include <coremedi/app/Signals.h>

namespace osg
{
    class CModelCutVisualizer : public osg::MatrixTransform
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

    public:
        //! Ctor
        CModelCutVisualizer(OSGCanvas *canvas, bool autoPositioning = true);

        //! Dtor
        ~CModelCutVisualizer();

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

    template <class T>
    class CModelCutVisualizerForSlice : public CModelCutVisualizer, public scene::CObjectObserverOSG<T>
    {
    private:
        //! Model Cut ID
        int m_modelCutId;
    public:
        //! Ctor
        CModelCutVisualizerForSlice(int modelCutId, OSGCanvas *canvas, bool autoPositioning = true)
            : CModelCutVisualizer(canvas, autoPositioning)
            , m_modelCutId(modelCutId)
        {
            this->setCanvas(canvas);
            APP_STORAGE.connect(modelCutId, this);
            this->setupObserver(this);
        }

        //! Dtor
        ~CModelCutVisualizerForSlice()
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
        //! Updates data
        virtual void updateData()
        {
            data::CObjectPtr<T> spModelCut(APP_STORAGE.getEntry(m_modelCutId));

            m_vertices->clear();
            m_indices->clear();

            osg::Vec3Array *vertices = spModelCut->getVertices();
            osg::DrawElementsUInt *indices = spModelCut->getIndices();

            for (std::size_t i = 0; i < vertices->size(); i++)
            {
                m_vertices->push_back(vertices->operator[](i));
            }

            for (std::size_t i = 0; i < indices->size(); i++)
            {
                m_indices->push_back(indices->operator[](i));
            }

            if (0==m_vertices->size()) // prevent crash
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
                    setMatrix(osg::Matrix::translate(-translation));
                }
            }

            m_transform->setMatrix(spModelCut->getTransformationMatrix());

            data::CColor4f color = spModelCut->getColor();
            setColor(osg::Vec4(color[0], color[1], color[2], color[3]));

            m_validData = true;
        }

    };

    typedef CModelCutVisualizerForSlice<data::CModelCutSliceXY>             CModelCutVisualizerSliceXY;
    typedef CModelCutVisualizerForSlice<data::CModelCutSliceXZ>             CModelCutVisualizerSliceXZ;
    typedef CModelCutVisualizerForSlice<data::CModelCutSliceYZ>             CModelCutVisualizerSliceYZ;
} // namespace osg

#endif // CModelCutVisualizer_H
