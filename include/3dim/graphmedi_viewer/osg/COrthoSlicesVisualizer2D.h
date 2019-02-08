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

#ifndef CORTHOSLICESVISUALIZER2D_H_INCLUDED
#define CORTHOSLICESVISUALIZER2D_H_INCLUDED


#include <osg/CDraggableGeometry.h>
#include <osg/CGeneralObjectObserverOSG.h>
#include <osg/CActiveObjectBase.h>
#include <osg/COnOffNode.h>
#include <osg/CAppMode.h>
#include <3dim/graphmedi/draggers/CDraggerRotate.h>
#include <3dim/graphmedi/draggers/CDraggerPlane.h>
#include <3dim/graphmedi/draggers/CDraggerBaseComposite.h>
#include <geometry/base/types.h>
#include <geometry/base/CPlane.h>


namespace osg
{
    class COrthoSlicesVisualizer2D : public osg::CDraggableGeometry, public scene::CGeneralObjectObserverOSG<COrthoSlicesVisualizer2D>, public osg::CActiveObjectBase
    {
    public:
        enum ESceneType
        {
            EST_XY = 0,
            EST_XZ,
            EST_YZ
        };

        explicit COrthoSlicesVisualizer2D(OSGCanvas *pCanvas, int id, ESceneType sceneType);

        ~COrthoSlicesVisualizer2D();

        void updateGeometry();

    protected:
        //!  Called upon updating from the storage
        virtual void updateFromStorage(void);

        void createDraggers();

        //! \fn void updateDraggerSize();
        //!
        //! \brief  Updates the dragger size
        void updateDraggerSize();

        void updateDraggers();

        //! \fn virtual bool sigCommandFromDG(const osg::Matrix & matrix, const osgManipulator::MotionCommand &command, int command_type, long dg_id);
        //!
        //! \brief  Answer to the command signal from draggable geometry.
        //!
        //! \param  matrix          The matrix.
        //! \param  command         The command.
        //! \param  command_type    Type of the command.
        //! \param  dg_id           Identifier for the dragger.
        //!
        //! \return True if it succeeds, false if it fails.
        virtual bool sigCommandFromDG(const osg::Matrix & matrix, const osgManipulator::MotionCommand &command, int command_type, long dg_id);

        void prepareDraggersMaterials();

        void onNewDensityData(data::CStorageEntry* entry, const data::CChangedEntries& changes);
        void onArbSliceChanged(data::CStorageEntry* entry, const data::CChangedEntries& changes);

        //! On mode changed - signal response
        void onModeChanged(scene::CAppMode::tMode mode);

        void getGeometryPoints(const osg::Vec3& inPoint, const osg::Vec3& direction, osg::Vec3& outPoint1, osg::Vec3& outPoint2);
        
        osg::ref_ptr<osg::COnOffNode> m_onOffNode;

        osg::ref_ptr<osg::Geode> m_sliceGeometry;
        osg::ref_ptr<osg::Geode> m_transDraggerGeometry;

        //translate dragger
        osg::ref_ptr<osgManipulator::CDraggerPlane> m_translatePlaneDragger;

        osg::ref_ptr<osgManipulator::CDraggerBaseComposite> m_compositeDragger;

        osg::Matrix m_prevMatrix;
        double m_startPosition;
        geometry::CPlane m_slicePlane;
        geometry::Vec3 m_slicePlaneNormal;
        geometry::Vec3 m_slicePlanePoint;

        //! App mode changed signal connection
        vpl::mod::tSignalConnection m_conAppModeChanged;

        //! Visibility connections
        vpl::mod::tSignalConnection  m_conVis[2];

        ESceneType m_sceneType;
    };
}

#endif // CORTHOSLICESVISUALIZER2D_H_INCLUDED

