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

#ifndef CSceneOSG_H
#define CSceneOSG_H

//#include <osg/OSGCanvas.h>
//#include <osg/OrthoManipulator.h>
//#include <osg/COnOffNode.h>
#include <osg/OSGOrtho2DCanvas.h>

#include <osg/CDraggableSlice.h>
#include <osg/CDraggerEventHandler.h>
#include <osg/CDensityWindowEventHandler.h>
#include <osg/CCommandEventHandler.h>
#include <osg/NodeMasks.h>
#include <osg/CActiveObjectBase.h>
#include <osg/CDraggableGeometry.h>
#include <osg/CArbitrarySliceGeometry.h>
#include <osg/CArbitrarySliceVisualizer2D.h>
#include <osg/COrthoSlicesVisualizer2D.h>

#include <osg/CGeneralObjectObserverOSG.h>
#include <data/CActiveDataSet.h>

#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osg/CoordinateSystemNode>
#include <osgText/Text>

// Widgets
#include <widgets/Widgets.h>
#include <widgets/CWidgetOverlayNode.h>

#include <osg/CAppMode.h>
///////////////////////////////////////////////////////////////////////////////
// forward declarations
namespace PSVR { class PSVolumeRendering; }

namespace scene
{

///////////////////////////////////////////////////////////////////////////////
// Global definitions

// Rendering order
#define LAYER_SCENE 10
#define LAYER_GIZMOS 30 //20
#define LAYER_LANDMARK_ANNOTATIONS 25
#define LAYER_VOLUME_RENDERING 20 //30
#define LAYER_HUD 40
#define LAYER_WIDGETS 50


///////////////////////////////////////////////////////////////////////////////
//! Base OSG scene in a window.
class CSceneBase : public osg::MatrixTransform, public CGeneralObjectObserverOSG<CSceneBase>
{
public:
    //! Constructor
    CSceneBase(OSGCanvas *pCanvas);

    //! Destructor
    virtual ~CSceneBase();

    //! Add gizmo
    void addGizmo(osg::Node * gizmo)
    {
        gizmo->setNodeMask(MASK_GIZMO);
        m_gizmosGroup->addChild(gizmo);
        m_pCanvas->Refresh(false);
    }

    //! Clear all gizmos
    void clearGizmos()
    {
        if (m_gizmosGroup->getNumChildren()>0)
        {
            m_gizmosGroup->removeChildren(0, m_gizmosGroup->getNumChildren());
            m_pCanvas->Refresh(false);
        }
    }

    void removeGizmo(osg::Node * gizmo)
    {
        m_gizmosGroup->removeChild(gizmo);
        m_pCanvas->Refresh(false);
    }

    //! Add landmark annotation with a given id
    void addLandmarkAnnotation(osg::COnOffNode * landmarkAnnotation, std::string landmarkId)
    {
        landmarkAnnotation->setNodeMask(MASK_LANDMARK_ANNOTATION);
        m_landmarkAnnotationsGroup->addChild(landmarkAnnotation);
        m_landmarkIdsToAnnotationNodes[landmarkId] = landmarkAnnotation;
        m_pCanvas->Refresh(false);
    }

    //! Clear landmark annotation with a given id
    void clearLandmarkAnnotation(std::string landmarkId)
    {
        auto it = m_landmarkIdsToAnnotationNodes.find(landmarkId);
        if (it == m_landmarkIdsToAnnotationNodes.end())
            return;

        int childIndex = m_landmarkAnnotationsGroup->getChildIndex(it->second);
        assert(childIndex != m_landmarkAnnotationsGroup->getNumChildren());
        if (childIndex != m_landmarkAnnotationsGroup->getNumChildren())
        {
            bool childFound = m_landmarkAnnotationsGroup->removeChild(childIndex);
            assert(childFound);
            m_landmarkIdsToAnnotationNodes.erase(landmarkId);
            m_pCanvas->Refresh(false);
        }
    }

    //! Clear all landmark annotations
    void clearAllLandmarkAnnotations()
    {
        if (m_landmarkAnnotationsGroup->getNumChildren() > 0)
        {
            m_landmarkIdsToAnnotationNodes.clear();
            m_landmarkAnnotationsGroup->removeChildren(0, m_landmarkAnnotationsGroup->getNumChildren());
            m_pCanvas->Refresh(false);
        }
    }

    void setLandmarkAnnotationsVisibility(bool visible)
    {
        m_landmarkAnnotationsGroup->setOnOffState(visible);
        m_pCanvas->Refresh(false);
    }

    //! Method called on OSG update callback.
    virtual void updateFromStorage();

    //! Positions of slices
    virtual void defaultPositioning();

    //! Sets the scene up
    virtual void setupScene(data::CDensityData & data);

    //! Anchors osg subtree to scene
    void anchorToScene(osg::Node * node, bool bCenter = false);

    //! Scene shift is used in ortho windows
    virtual osg::Matrix getOrthoTransformMatrix()
    {
        return m_orthoTransformMatrix;
    }

    //! Returns unortho matrix
    virtual osg::Matrix getUnOrthoMatrix()
    {
        return osg::Matrix::inverse(m_orthoTransformMatrix);
    }

    //! Get dragger event handler
    CDraggerEventHandler * getDraggerEventHandler() { return p_DraggerEventHandler.get(); }

    //! Set home position
    void home() { m_pCanvas->getView()->getCameraManipulator()->computeHomePosition(); }

protected:
    //! Scene event handlers
    osg::ref_ptr<CDraggerEventHandler>       p_DraggerEventHandler;
    osg::ref_ptr<CDensityWindowEventHandler> p_DensityWindowEventHandler;
    osg::ref_ptr<CCommandEventHandler>       p_CommandEventHandler;

    //! Connection to clear all gizmos signal
    vpl::mod::tSignalConnection		m_ClearAllGizmosConnection;
    vpl::mod::tSignalConnection		m_ClearMeasurementsConnection;
    
    //! Clear landmark annotation with the specified id from scene signal connection
    vpl::mod::tSignalConnection m_ClearLandmarkAnnotation;

    //! Clear all landmark annotations from scene signal connection
    vpl::mod::tSignalConnection	m_ClearAllLandmarkAnnotations;

    //! Set landmark annotation visibility signal connection
    vpl::mod::tSignalConnection	m_SetLandmarkAnnotationVisibility;

    //! Drawing gizmos node
    osg::ref_ptr< osg::COnOffNode > m_gizmosGroup;

    //! Drawing landmark annotations node
    osg::ref_ptr< osg::COnOffNode > m_landmarkAnnotationsGroup;

    //! Mapping from annotated landmark ids to annotation nodes
    std::map<std::string, osg::Node *> m_landmarkIdsToAnnotationNodes;

    //! Scene shift is used in ortho windows to move scene from origin. This const defines ammount of the shift.
    static const long SCENE_SHIFT_AMMOUNT = 1000;

    //! Scene transform matrix used in ortho views. Identity elsewhere.
    osg::Matrix m_orthoTransformMatrix;

    //! Scene inverse transform matrix used in ortho views. Identity elsewhere.
    osg::Matrix m_unOrthoTransformMatrix;

    //! Scene shift vector
    osg::Vec3 m_sceneShiftVector;

    //!
    osg::ref_ptr<osg::MatrixTransform> p_AnchorGroup, p_AnchorAndCenterGroup;
};


///////////////////////////////////////////////////////////////////////////////
//! Basic OSG scene providing multiplanar view of volumetric data.
class CSceneOSG : public CSceneBase
{
public:
    //! constructor
    CSceneOSG(OSGCanvas *pCanvas, bool xyOrtho, bool xzOrtho, bool yzOrtho, bool bCreateScene = true);

    //! Destructor
    virtual ~CSceneOSG();

    //! Method called on OSG update callback.
    virtual void updateFromStorage();

    //! Positions XY, XZ and YZ planes to their default 
    virtual void defaultPositioning();

    //! Sets the scene up
    virtual void setupScene(data::CDensityData & data);

    //! Returns number of voxels in X direction
    int	getXSize() const;

    //! Returns number of voxels in Y direction
    int	getYSize() const;

    //! Returns number of voxels in Z direction
    int	getZSize() const;

    //! Returns voxel size in X direction
    float getDX() const;

    //! Returns voxel size in Y direction
    float getDY() const;

    //! Returns voxel size in Z direction
    float getDZ() const;

    //! Returns thickness of the dummy geometry. 
    float getThin() const;

    //! Returns pointer to XY slice
    CDraggableSlice * getSliceXY()
    {
        return p_DraggableSlice[0].get();
    }

    //! Returns pointer to XZ slice
    CDraggableSlice * getSliceXZ()
    {
        return p_DraggableSlice[1].get();
    }

    //! Returns pointer to YZ slice
    CDraggableSlice * getSliceYZ()
    {
        return p_DraggableSlice[2].get();
    }

    //! Create default scene
    virtual void createScene();

    void updateArbSliceGeometry();

    void setArbSliceVisibility(bool visible)
    {
        m_arbSliceVisibility = visible;
        updateArbSliceGeometry();
    }

protected:
    //! Create widgets overlay scene
    void createWidgetsScene(OSGCanvas *pCanvas, const osg::Matrix & viewMatrix, int Flags);

    //! App mode changed callback
    void onAppModeChanged(scene::CAppMode::tMode mode);

    void orthoSliceMoved();

protected:
    //! Info text
    osg::ref_ptr< osgText::Text > hudText;

    //! 3 pointers to each draggable slice
    osg::ref_ptr< CDraggableSlice > p_DraggableSlice[3];

    //! 4 On/off nodes (used to show/hide draggable slices
    osg::ref_ptr< osg::COnOffNode > p_onOffNode[4];

    //! Scene moved signal
    vpl::mod::CSignal< void > m_SceneChangedSignal;

    //! Slice thinning factor (dummy geometry scale in z axis)
    float f_Thin;

    //! Scene type.
    bool m_bXYOrtho, m_bXZOrtho, m_bYZOrtho;

    //! Slice moved signal connections
    vpl::mod::tSignalConnection m_conSliceMoved;

    //! Scene orhto and center matrix
    osg::ref_ptr< osg::MatrixTransform > p_OrthoAnchorAndCenterGroup;

    //! Widget overlay node
    osg::ref_ptr< CWidgetOverlayNode > m_widgetOverlay;

    //! Base color of the view - in ortho it is color of the slice frame.
    osg::Vec4 m_viewColor;

    //! Widgets base color
    osg::Vec4 m_widgetsColor;

    //! Visibility connections
    vpl::mod::tSignalConnection  m_conVis[6];

    osg::ref_ptr<osg::CArbitrarySliceVisualizer2D> m_arbSlice2D;

    bool m_arbSliceVisibility;
};


////////////////////////////////////////////////////////////
//! a Class
class CSceneXY : public CSceneOSG
{
public:
    CSceneXY(OSGCanvas * canvas);

    //! Move slice up/down event handler. True means up.
    void sliceUpDown(int direction);

protected:
    // Widgets
};


////////////////////////////////////////////////////////////
//! a Class
class CSceneXZ : public CSceneOSG
{
public:
    CSceneXZ(OSGCanvas * canvas);

    //! Move slice up/down event handler. True means up.
    void sliceUpDown(int direction);
};


////////////////////////////////////////////////////////////
//! a Class
class CSceneYZ : public CSceneOSG
{
public:
    CSceneYZ(OSGCanvas * canvas);

    //! Move slice up/down event handler. True means up.
    void sliceUpDown(int direction);
};


////////////////////////////////////////////////////////////
//! a Class
class CScene3DBase : public CSceneOSG
{
public:
    //! constructor
    CScene3DBase(OSGCanvas *canvas);

    //! destructor
    virtual ~CScene3DBase();

    osg::Vec3 getXYWorld();
    osg::Vec3 getXZWorld();
    osg::Vec3 getYZWorld();

protected:
    vpl::mod::tSignalConnection m_conWorld[3];
};

////////////////////////////////////////////////////////////
//! a Class
class CScene3D : public CScene3DBase
{
public:
    //! constructor
    CScene3D(OSGCanvas *canvas);

    //! destructor
    virtual ~CScene3D();

    //! Sets the renderer.
    void setRenderer(PSVR::PSVolumeRendering *pRenderer);

    //! Enable/disable renderer
    void enableRenderer(bool enable);

    osg::Geode *getVRGeode();

protected:
    // Update scene from the storage
    virtual void updateFromStorage();

protected:
    //! Renderer scene group 
    osg::ref_ptr<osg::MatrixTransform> m_renderedGroup;

    //! Drawable...
    osg::ref_ptr<osg::Drawable> m_vrDrawable;

    //! Geode...
    osg::ref_ptr<osg::Geode> m_vrGeode;

    //! Rendered group state set
    osg::ref_ptr<osg::StateSet> m_renderedSS;
};


class CArbitrarySliceScene : public CSceneBase
{
public:
    //! Constructor parametrized by canvas pointer
    CArbitrarySliceScene(OSGOrtho2DCanvas * canvas = 0);

    //! Destructor
    ~CArbitrarySliceScene();

    //! Method called on OSG update callback.
    virtual void updateFromStorage() override;
    void updateFromStorageArbitrarySlice();
    void updateGeometry();

    //
    virtual void setupScene(data::CDensityData & data) override;

    osg::CArbitrarySliceGeometry* getSlice() const;

    //! Move slice up/down event handler. True means up.
    void sliceUpDown(int direction);

protected:
    osg::ref_ptr< osg::CArbitrarySliceGeometry > m_pImplantSlice;

    //! Plane dragger
    osg::ref_ptr<osgManipulator::Dragger> m_planeDragger;
    osg::ref_ptr< scene::CPlaneARBUpdateSelection > m_selection;

    osg::ref_ptr<osg::COrthoSlicesVisualizer2D> m_xySlice2D;
    osg::ref_ptr<osg::COrthoSlicesVisualizer2D> m_xzSlice2D;
    osg::ref_ptr<osg::COrthoSlicesVisualizer2D> m_yzSlice2D;

    osg::ref_ptr<osg::COnOffNode> p_onOffNode[3];

    //! Signal connection
    vpl::mod::tSignalConnection m_c1;

    //! Slice moved signal connections
    vpl::mod::tSignalConnection m_conSliceMoved;

    //! Widget overlay node
    osg::ref_ptr<CWidgetOverlayNode> m_widgetOverlay;
    osg::ref_ptr<scene::CSceneOrientationWidget> m_orientationWidget;

    //! Widgets base color
    osg::Vec4 m_widgetsColor;

    void orthoSliceMoved();

    //! Create widgets overlay scene
    void createWidgetsScene(OSGCanvas *pCanvas, const osg::Matrix & viewMatrix, int Flags);
};


}

#endif // CSceneOSG_H
