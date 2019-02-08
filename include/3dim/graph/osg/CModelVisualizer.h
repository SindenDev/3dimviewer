////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////

#ifndef CModelVisualizer_H
#define CModelVisualizer_H

#include <osg/CTriMesh.h>
#include <osg/COnOffNode.h>
#include "CGeneralObjectObserverOSG.h"
#include <osg/CActiveObjectBase.h>
#include <osgManipulator/Dragger>

namespace osg
{
    ///////////////////////////////////////////////////////////////////////////////
    //! Surface model visualizer.
    class CModelVisualizer : public COnOffNode, public scene::CGeneralObjectObserverOSG<CModelVisualizer>
    {
    public:
        /**
        * \class   CModelEventsHelperDragger
        *
        * \brief   A model events helper dragger - used for CDraggerEventHandler activation.
        */
        class CModelEventsHelperDragger : public osgManipulator::Dragger, public CActiveObjectBase
        {
        public:
            CModelEventsHelperDragger(int id) : osg::CActiveObjectBase("CModelEventsHelperDragger", 4), m_id(id) {}

            int getModelId() const { return m_id; }

        protected:
            int m_id;
        };

    public:
        CModelVisualizer(int modelId);

        virtual void updateFromStorage() override;

        int getId() const;
        void setId(int id);

        void setManualUpdates(bool bSet);
        bool getManualUpdates() const;

        osg::observer_ptr<CTriMesh> getMesh();
        osg::observer_ptr<osg::MatrixTransform> getModelTransform();
        osg::observer_ptr<CModelEventsHelperDragger> getDragger();

        void showWireframe(bool bShow);

        void updatePartOfMesh(const std::vector<std::pair<long, osg::CTriMesh::SPositionNormal>>& handles);

    public:
        osg::ref_ptr<osg::CPseudoMaterial> m_materialRegular;
        osg::ref_ptr<osg::CPseudoMaterial> m_materialSelected;
        osg::ref_ptr<osg::CPseudoMaterial> m_materialSkinnedRegular;
        osg::ref_ptr<osg::CPseudoMaterial> m_materialSkinnedSelected;

    protected:
        int m_modelId;

        osg::ref_ptr<CTriMesh> m_pMesh;
        osg::ref_ptr<osg::MatrixTransform> m_pTransform;
        osg::ref_ptr<CModelEventsHelperDragger> m_dragger;

        bool m_bManualUpdate;
        bool m_bUseKDTree;
    };

} // namespace osg

#endif // CModelVisualizer_H
