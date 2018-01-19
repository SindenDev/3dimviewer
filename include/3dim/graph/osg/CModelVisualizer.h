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
//#include <coremedi/app/Signals.h>
#include <osg/PolygonMode>
#include <osg/CPseudoMaterial.h>

#include "CGeneralObjectObserverOSG.h"

#include <data/CDataStorage.h>
#include <data/CModel.h>
#include <data/CObjectPtr.h>

#include <osgManipulator/Dragger>

namespace osg
{
    ///////////////////////////////////////////////////////////////////////////////
    // Global functions
    namespace ModelVisualizer
    {
        //! Setups all OSG properties required for correct visualization of a surface mesh.
        //bool setupModelStateSet( osg::Node *pMesh );
        bool setupModelStateSet(osg::Geode *pMesh);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //! Surface model visualizer.
    //! - Parameter T should be data::CModel class or any other derived from it.
    //! - The parametr prescribes type of a storage item whose Id is given
    //!   in the constructor.
    template <class T>
    class CAnyModelVisualizer : public COnOffNode, public scene::CGeneralObjectObserverOSG<CAnyModelVisualizer<T> >
    {
    public:
        //! Storage item type.
        typedef T tModel;

        //! Storage observer type.
        typedef scene::CGeneralObjectObserverOSG<CAnyModelVisualizer<T> > tObserver;

        //! Materials
        osg::ref_ptr<osg::CPseudoMaterial> m_materialRegular;
        osg::ref_ptr<osg::CPseudoMaterial> m_materialSelected;
        osg::ref_ptr<osg::CPseudoMaterial> m_materialSkinnedRegular;
        osg::ref_ptr<osg::CPseudoMaterial> m_materialSkinnedSelected;

    public:
        //! Constructor
        CAnyModelVisualizer(int ModelId);

        //! Method called on OSG update callback.
        virtual void updateFromStorage();

        //! Update only part of mesh
        virtual void updatePartOfMesh(const CTriMesh::tIdPosVec &handles);

        //! Get model id
        int getId()
        {
            return m_ModelId;
        }

        //! Get model transform
        osg::MatrixTransform* getModelTransform()
        {
            return m_pTransform.get();
        }

        //! Set/unset manual updates
        void setManualUpdates(bool bSet)
        {
            m_bManualUpdate = bSet;
        }

        //! Get manual updates flag value
        bool getManualUpdates()
        {
            return m_bManualUpdate;
        }

        //! Get mesh
        CTriMesh *getMesh()
        {
            return m_pMesh;
        }

        const CTriMesh *getMesh() const
        {
            return m_pMesh;
        }

        //! Enable or disable wireframe mode
        void showWireframe(bool bShow);

    protected:
        //! Build KD-tree for visualizer
        void buildKDTree();

    public:

        /**
         * \class   CModelEventsHelperDragger
         *
         * \brief   A model events helper dragger - used for CDraggerEventHandler activation.
         */

        class CModelEventsHelperDragger : public osgManipulator::Dragger
        {
        public:
            CModelEventsHelperDragger(int id) : m_id(id) {}

            int getModelId() const { return m_id; }

        protected:
            // Model id
            int m_id;
        };

    protected:
        //! Identifier of a concrete model.
        int m_ModelId;

        //! Triangles...
        osg::ref_ptr<CTriMesh> m_pMesh;

        //! Matrix transform of the model
        osg::ref_ptr<osg::MatrixTransform> m_pTransform;

        //! Object is manually updated?
        bool m_bManualUpdate;

        //! Should kd-tree be used?
        bool m_bUseKDTree;
    };

    ///////////////////////////////////////////////////////////////////////////////
    //! Surface model visualizer.
    typedef CAnyModelVisualizer<data::CModel> CModelVisualizer;

    ///////////////////////////////////////////////////////////////////////////////
    // Method templates
    #include "CModelVisualizer.hxx"

} // namespace osg

#endif // CModelVisualizer_H
