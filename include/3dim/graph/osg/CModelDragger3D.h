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

#ifndef CMODELDRAGGER3D_H
#define CMODELDRAGGER3D_H

#include <data/CModel.h>
#include <osg/CSceneOSG.h>
#include <osg/CActiveObjectBase.h>

#include <osgGA/GUIEventHandler>
#include <osg/Node>
#include <osgUtil/LineSegmentIntersector>
#include <osg/CSprite.h>
#include <osg/CRotate2DDragger.h>
#include <osg/CTranslate1DDragger.h>
#include <osg/CTranslate2DDragger.h>
#include <osg/CGeometryGenerator.h>
#include <osg/CCylinderDragger.h>
#include <osg/CIntersectionProspector.h>

namespace osg
{
    class CFreeModelVisualizer;
}

///////////////////////////////////////////////////////////////////////////////
namespace osgManipulator
{

    ///////////////////////////////////////////////////////////////////////////////
    // CModel3DRotationDragger - rotation
    class CModel3DRotationDragger : public CompositeDragger
    {
    public:
        //! Constructor
        CModel3DRotationDragger();

        /** Setup default geometry for dragger. */
        void setupDefaultGeometry();

        //! Set ring radius
        void setRadius(float r1, float r2);

        //! Update
        virtual void updateGeometry();

        //! Get x axis dragger node reference
        CCylinderDragger & getXDragger() { return *m_rxDragger; }

        //! Get x axis dragger node reference
        CCylinderDragger & getYDragger() { return *m_ryDragger; }

        //! Get x axis dragger node reference
        CCylinderDragger & getZDragger() { return *m_rzDragger; }

    protected:
        //! Destructor

        // Rotation draggers
        osg::ref_ptr<CCylinderDragger> m_rxDragger;
        osg::ref_ptr<CCylinderDragger> m_ryDragger;
        osg::ref_ptr<CCylinderDragger> m_rzDragger;

        //! Geometry
        osg::ref_ptr< osg::CDonutGeometry > m_ring;
        osg::ref_ptr< osg::CDonutGeometry > m_invisibleRing;
    }; // CModel3DRotationDragger

       ///////////////////////////////////////////////////////////////////////////////
       // CModel3DTranslationDragger - movement along axis
    class CModel3DTranslationDragger : public osgManipulator::CTranslate1DDragger
    {
    public:
        //! Constructor.
        CModel3DTranslationDragger();

        //! Set translation line constructor
        CModel3DTranslationDragger(const osg::Vec3& s, const osg::Vec3& e);

        //! Setup default geometry for dragger.
        virtual void setupDefaultGeometry();

        //! Set sizes
        virtual void setOffset(float offset_head, float offset_tail);

        //! Set scale
        virtual void setScale(float scale);

        //! Update changed geometry.
        virtual void updateGeometry();

    protected:

        //! Arrows
        osg::ref_ptr<osg::CArrow3DGeometry> m_arrow;

        //! Shifting transforms
        osg::ref_ptr<osg::MatrixTransform> m_shiftHead, m_shiftTail;

        //! Scaling transforms
        osg::ref_ptr<osg::MatrixTransform> m_scaleHead, m_scaleTail;
    };

    ///////////////////////////////////////////////////////////////////////////////
    // CModel3DViewPlaneDragger - movement along plane
    class CModel3DViewPlaneDragger : public osgManipulator::CTranslate2DDragger, public osgManipulator::IHoverDragger
    {
    public:
        //! Constructor
        CModel3DViewPlaneDragger(geometry::CMesh* pMesh, osg::Matrix modelPos);

        //! Setup default geometry for dragger.
        virtual void setupDefaultGeometry();

        //! Update geometry
        virtual void updateGeometry();

        virtual void accept(osg::NodeVisitor& nv);

        void updateModelMatrix(const osg::Matrix &modelPos);

        //! Update
        void updateModel(int storageID);

        //! Set sphere radius
        void setRadius(float radius);

        void onMouseEnter() override;
        void onMouseLeave() override;

    protected:
        //! Revert plane rotation given by matrix transformations
        virtual void revertTransformsOnPlane();

    protected:
        //! geometry
        osg::CFreeModelVisualizer* m_pVisualizer;

        //! pointer to moved object mesh (so the object itself can be draggable)
        geometry::CMesh* m_pMesh;

        //! Shifting transforms
        osg::ref_ptr<osg::MatrixTransform> m_shift;

        //! Nodepath matrix
        osg::Matrix m_nodepath_matrix;

        //! View matrix
        osg::Matrix m_view_matrix;

        //! Sphere geometry
        osg::ref_ptr<osg::CSphereGeometry> m_sphere;
    };

} // namespace osgManipulator

  ///////////////////////////////////////////////////////////////////////////////
  // Model manipulator
namespace osg
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief    3D pivot dragger holder.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CPivotDraggerHolder : public osgManipulator::CompositeDragger, data::CGeneralObjectObserver<CPivotDraggerHolder>, public CActiveObjectBase
    {
    private:
        osg::ref_ptr<osgManipulator::CModel3DTranslationDragger> m_translationXDragger;
        osg::ref_ptr<osgManipulator::CModel3DTranslationDragger> m_translationYDragger;
        osg::ref_ptr<osgManipulator::CModel3DTranslationDragger> m_translationZDragger;

    public:
        CPivotDraggerHolder();
        ~CPivotDraggerHolder();

        //! Handle dragger move
        virtual bool handle(const osgManipulator::PointerInfo &pi, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

        void onMouseEnter(const osgGA::GUIEventAdapter& ea, bool command_mode) override;
        void onMouseExit(const osgGA::GUIEventAdapter& ea, bool command_mode) override;

        //! Sets sizes of translation draggers
        void setOffset(float offset);
        void setScale(float scale);

        //! Prepare materials
        void prepareMaterials();

        //!
        virtual void objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);

        //! Pivot observer method
        void sigPivotChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief    Base class for model dragger holders.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CModelDraggerHolder : public osgManipulator::CompositeDragger, public data::CGeneralObjectObserver<CModelDraggerHolder>
    {
    public:
        //! Constructor
        CModelDraggerHolder(int idModel, bool signalDraggerMove, bool signalDraggerMoveSetMatrix);
        //! Destructor
        ~CModelDraggerHolder();
        //! For extern update of model matrix
        virtual void updateModelMatrix(const osg::Matrix &modelPos) { }
        //! Set ID of current handled model
        void setModelID(int id, bool setPivot);
        //! Get ID of current handled model
        int getModelID() const { return m_idModel; }
        //! Handle dragger move
        virtual bool handle(const osgManipulator::PointerInfo &pi, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    protected:
        //! Prepare materials
        virtual void prepareMaterials() { }

        //! Model observer method
        virtual void objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);
        void sigModelChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);
        void sigPivotChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);
        virtual void internalModelChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);
        virtual void internalPivotChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);

    protected:
        //! Flag if pivot should be set to model's position
        bool m_bSetPivot;

        //! Flags for signalling DraggerMove
        bool m_bSignalDraggerMove;
        bool m_bSignalDraggerMoveSetMatrix;

        //! Associated model ID
        int m_idModel;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief    3D model dragger holder. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class C3DModelDraggerHolder : public CModelDraggerHolder, public CActiveObjectBase
    {
    public:
        //! Constructor for use in scan appliance alignment dialog
        C3DModelDraggerHolder(CFreeModelVisualizer* pVisualizer, const osg::Matrix &modelPos, const osg::Vec3& sceneSize, bool signalDraggerMove, bool signalDraggerMoveSetMatrix);
        //! Constructor for use in main 3D window
        C3DModelDraggerHolder(int modelID, const osg::Matrix &modelPos, const osg::Vec3& sceneSize, bool signalDraggerMove, bool signalDraggerMoveSetMatrix);
        //! Destructor
        ~C3DModelDraggerHolder();
        //! For extern update of model matrix
        virtual void updateModelMatrix(const osg::Matrix &modelPos);
        //! Sets pivot dragger
        void setPivotDragger(CPivotDraggerHolder *pivotDragger);
        void updatePivotDraggerMatrix();
        //! Handle dragger move
        virtual bool handle(const osgManipulator::PointerInfo &pi, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

        void onMouseEnter(const osgGA::GUIEventAdapter& ea, bool command_mode) override;
        void onMouseExit(const osgGA::GUIEventAdapter& ea, bool command_mode) override;

    protected:
        //! Prepare materials
        virtual void prepareMaterials();

        //! Model observer method
        virtual void internalModelChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);
        virtual void internalPivotChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);

    protected:
        //! Pivot dragger
        osg::ref_ptr<osg::CPivotDraggerHolder> m_pivot_dragger;
        double m_pivot_dragger_offset;
        double m_pivot_dragger_scale;
        geometry::CMesh::Point m_bbMin, m_bbMax;

        //! Trackball dragger
        osg::ref_ptr<osgManipulator::CModel3DRotationDragger> m_rotation_dragger;

        //! Translation draggers
        osg::ref_ptr<osgManipulator::CModel3DTranslationDragger> m_x_translate_dragger;
        osg::ref_ptr<osgManipulator::CModel3DTranslationDragger> m_y_translate_dragger;
        osg::ref_ptr<osgManipulator::CModel3DTranslationDragger> m_z_translate_dragger;

        //! Translation draggers
        osg::ref_ptr<osgManipulator::CModel3DViewPlaneDragger> m_translate_dragger;
    };

} // namespace osg

#endif // CMODELDRAGGER3D_H