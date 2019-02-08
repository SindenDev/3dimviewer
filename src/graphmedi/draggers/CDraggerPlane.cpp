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

#include "draggers/CDraggerPlane.h"

osgManipulator::CDraggerPlane::CDraggerPlane(osg::Node* geometry)
    : CDraggerPlane(geometry, osg::Plane(osg::Vec3(0.0, 0.0, 1.0), osg::Vec3(0.0, 0.0, 0.0)))
{

}

osgManipulator::CDraggerPlane::CDraggerPlane(osg::Node* geometry, const osg::Plane& plane)
    : m_initialPlane(plane)
    , m_geometry(geometry)
{
    _projector = new PlaneProjector(m_initialPlane);
    addChild(geometry);
}

bool osgManipulator::CDraggerPlane::handle(const osgManipulator::PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    // Check if the dragger node is in the nodepath.
    if (!pointer.contains(this)) return false;

    switch (ea.getEventType())
    {
        // Pick start.
    case (osgGA::GUIEventAdapter::PUSH):
    {
        //m_bTranslating = true;

        // Get the LocalToWorld matrix for this node and set it for the projector.
        osg::NodePath nodePathToRoot;
        computeNodePathToRoot(*this, nodePathToRoot);

        revertTransformsOnPlane();

        osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);

        _projector->setLocalToWorld(localToWorld);

        if (_projector->project(pointer, _startProjectedPoint))
        {
            // Generate the motion command.
            osg::ref_ptr<TranslateInPlaneCommand> cmd = new TranslateInPlaneCommand(_projector->getPlane());

            cmd->setStage(MotionCommand::START);
            cmd->setReferencePoint(_startProjectedPoint);
            cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(), _projector->getWorldToLocal());

            updateCommand(*cmd);

            // Dispatch command.
            dispatch(*cmd);

            // Set color to pick color.
            applyMaterial(osg::SECOND);

            aa.requestRedraw();
        }
        return true;
    }

    // Pick move.
    case (osgGA::GUIEventAdapter::DRAG):
    {
        osg::Vec3d projectedPoint;
        osg::Plane p(_projector->getPlane());
        //			osg::Vec3 projectedPoint;
        if (_projector->project(pointer, projectedPoint))
        {
            // Generate the motion command.
            osg::ref_ptr<TranslateInPlaneCommand> cmd = new TranslateInPlaneCommand(_projector->getPlane());

            cmd->setStage(MotionCommand::MOVE);
            cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(), _projector->getWorldToLocal());
            cmd->setTranslation(projectedPoint - _startProjectedPoint);
            cmd->setReferencePoint(_startProjectedPoint);

            updateCommand(*cmd);

            // Dispatch command.
            dispatch(*cmd);

            aa.requestRedraw();
        }
        return true;
    }

    // Pick finish.
    case (osgGA::GUIEventAdapter::RELEASE):
    {
        //m_bTranslating = false;

        osg::ref_ptr<TranslateInPlaneCommand> cmd = new TranslateInPlaneCommand(_projector->getPlane());

        cmd->setStage(MotionCommand::FINISH);
        cmd->setReferencePoint(_startProjectedPoint);
        cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(), _projector->getWorldToLocal());

        updateCommand(*cmd);

        // Dispatch command.
        dispatch(*cmd);

        // Reset color.
        applyMaterial(osg::FIRST);

        aa.requestRedraw();

        return true;
    }
    default:
        return false;
    }

    return false;
}

void osgManipulator::CDraggerPlane::accept(osg::NodeVisitor & nv)
{
    if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        osg::CullStack* cs = dynamic_cast<osg::CullStack*>(&nv);
        if (cs)
        {
            osg::Vec3d translation;
            osg::Quat rotation;
            osg::Vec3d scale;
            osg::Quat so;

            m_view_matrix = *cs->getModelViewMatrix();
            nv.getNodePath();

        }
    }

    Dragger::accept(nv);
}

void osgManipulator::CDraggerPlane::revertTransformsOnPlane()
{
    osg::Vec3d translation;
    osg::Quat rotation;
    osg::Vec3d scale;
    osg::Quat so;

    m_view_matrix.decompose(translation, rotation, scale, so);

    osg::Matrix m(osg::Matrix::rotate(rotation.inverse()));

    osg::Plane plane(0.0, 0.0, -1.0, 0.0);
    plane.transform(m);

    _projector->setPlane(plane);
}

osg::Node* osgManipulator::CDraggerPlane::getGeometry()
{
    return m_geometry;
}