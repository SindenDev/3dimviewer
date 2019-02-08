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

#ifndef CARBITRARYSLICEVISUALIZER_H_INCLUDED
#define CARBITRARYSLICEVISUALIZER_H_INCLUDED


#include <osg/CArbitrarySliceGeometry.h>
#include <osg/CDraggableGeometry.h>
#include <osg/CGeneralObjectObserverOSG.h>
#include <osg/CActiveObjectBase.h>
#include <osg/COnOffNode.h>
#include <osg/CAppMode.h>
#include <3dim/graphmedi/draggers/CDraggerTranslate.h>
#include <3dim/graphmedi/draggers/CDraggerRotate.h>
#include <3dim/graphmedi/draggers/CDraggerScale.h>
#include <3dim/graphmedi/draggers/CDraggerPlane.h>
#include <3dim/graphmedi/draggers/CDraggerBaseComposite.h>
#include <data/CArbitrarySlice.h>
#include <geometry/base/types.h>


namespace osg
{
    class CArbitrarySliceDraggerGeometry : public osg::Group
    {
    public:
        enum EDraggerType
        {
            EDT_X = 0,
            EDT_Y,
            EDT_Z
        };

        CArbitrarySliceDraggerGeometry(EDraggerType type);

        void resize(double size);

    protected:
        osg::ref_ptr<osg::MatrixTransform> m_arrow1MT;
        osg::ref_ptr<osg::MatrixTransform> m_arrow2MT;
        osg::ref_ptr<osg::CSemiCircularArrowGeometry> m_arrow1;
        osg::ref_ptr<osg::CSemiCircularArrowGeometry> m_arrow2;
        osg::ref_ptr<osg::CDonutGeometry> m_ring;

        double m_arrowSize;
        EDraggerType m_draggerType;
    };


    class CArbitrarySliceVisualizer : public osg::CDraggableGeometry, public scene::CGeneralObjectObserverOSG<CArbitrarySliceVisualizer>, public osg::CActiveObjectBase
    {
    public:
        explicit CArbitrarySliceVisualizer(OSGCanvas *pCanvas, int id);

        ~CArbitrarySliceVisualizer();

    protected:
        void init();

        //!  Called upon updating from the storage
        virtual void updateFromStorage() override;

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

        void onNewDensityData();
        void onArbSliceChanged();

        //! On mode changed - signal response
        void onModeChanged(scene::CAppMode::tMode mode);

        void onDrawingInProgress(bool drawing);

        bool m_drawingInProgress;
        
        osg::ref_ptr<osg::COnOffNode> m_onOffNode;

        osg::ref_ptr<CArbitrarySliceGeometry> m_sliceGeometry;
        osg::ref_ptr<osg::MatrixTransform> m_helpGeom;

        //translate draggers
        osg::ref_ptr<osgManipulator::CDraggerTranslate> m_z_translate_dragger;

        //rotate draggers
        osg::ref_ptr<osgManipulator::CDraggerRotate> m_x_rotate_dragger;
        osg::ref_ptr<osgManipulator::CDraggerRotate> m_y_rotate_dragger;
        osg::ref_ptr<osgManipulator::CDraggerRotate> m_z_rotate_dragger;

        osg::ref_ptr<osgManipulator::CDraggerScale> m_x_scale_dragger;
        osg::ref_ptr<osgManipulator::CDraggerScale> m_y_scale_dragger;

        osg::ref_ptr<osgManipulator::CDraggerBaseComposite> m_translationCompositeDragger;
        osg::ref_ptr<osgManipulator::CDraggerBaseComposite> m_rotationCompositeDragger;
        osg::ref_ptr<osgManipulator::CDraggerBaseComposite> m_scaleCompositeDragger;

        osg::ref_ptr<osgManipulator::CDraggerPlane> m_plane_dragger;

        osg::ref_ptr<osgManipulator::CDraggerBaseComposite> m_3DCompositeDragger;

        osg::ref_ptr<osg::CArbitrarySliceDraggerGeometry> m_xRotateDraggerGeometry;
        osg::ref_ptr<osg::CArbitrarySliceDraggerGeometry> m_yRotateDraggerGeometry;
        osg::ref_ptr<osg::CArbitrarySliceDraggerGeometry> m_zRotateDraggerGeometry;

        geometry::Vec3 m_prevTrans;
        osg::Matrix m_startMatrix;
        osg::Matrix m_prevMatrix;
        double m_sliceWidth;
        double m_sliceHeight;

        //! App mode changed signal connection
        vpl::mod::tSignalConnection m_conAppModeChanged;

        //! Visibility connections
        vpl::mod::tSignalConnection  m_conVis[2];

        vpl::mod::tSignalConnection m_conDrawing;
    };
}

#endif // CARBITRARYSLICEVISUALIZER_H_INCLUDED

