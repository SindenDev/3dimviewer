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

#include "draggers/CDraggerRotate.h"
#include <osg/CGeometryGenerator.h>

osg::CDraggerRotateGeometry::CDraggerRotateGeometry()
{
    m_donut = new osg::CDonutGeometry(64, 5);
    addChild(m_donut);
}

void osg::CDraggerRotateGeometry::setSize(double r1, double r2)
{
    m_donut->setSize(r1, r2);
    m_donut->update();
}



osgManipulator::CDraggerRotate::CDraggerRotate(osg::Node* geometry)
    : CDraggerRotate(osg::Plane(osg::Vec3(0.0, 0.0, 1.0), osg::Vec3(0.0, 0.0, 0.0)), geometry)
{
}

osgManipulator::CDraggerRotate::CDraggerRotate(const osg::Plane & plane, osg::Node* geometry)
    : m_initialPlane(plane)
    , m_geometry(geometry)
{
    m_projector = new PlaneProjector(m_initialPlane);
    addChild(geometry);
}

bool osgManipulator::CDraggerRotate::handle(const osgManipulator::PointerInfo &pi, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    // Check if the dragger node is in the nodepath.
    if (!pi.contains(this)) return false;

    switch (ea.getEventType())
    {
        // Pick start.
    case (osgGA::GUIEventAdapter::PUSH):
    {
        // Get the LocalToWorld matrix for this node and set it for the projector.
        osg::NodePath nodePathToRoot;
        computeNodePathToRoot(*this, nodePathToRoot);

        osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);
        m_projector->setLocalToWorld(localToWorld);

        if (m_projector->project(pi, m_startProjectedPoint))
        {

            // Generate the motion command
            osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand;

            cmd->setStage(MotionCommand::START);
            cmd->setLocalToWorldAndWorldToLocal(m_projector->getLocalToWorld(), m_projector->getWorldToLocal());

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
        //			osg::Vec3 projectedPoint;
        if (m_projector->project(pi, projectedPoint))
        {
            // Generate the motion command
            osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand;

            cmd->setStage(MotionCommand::MOVE);
            cmd->setLocalToWorldAndWorldToLocal(m_projector->getLocalToWorld(), m_projector->getWorldToLocal());
            osg::Quat rotation(computeRotation(m_startProjectedPoint, projectedPoint));
            double angle; osg::Vec3 axis;

            rotation.getRotate(angle, axis);
            //if (angle < osg::DegreesToRadians(0.01))
            //    return true;

            cmd->setRotation(rotation);

            // Dispatch command.
            dispatch(*cmd);

            aa.requestRedraw();
        }
        return true;
    }

    // Pick finish.
    case (osgGA::GUIEventAdapter::RELEASE):
    {
        // Generate the motion command
        osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand;

        cmd->setStage(MotionCommand::FINISH);
        cmd->setLocalToWorldAndWorldToLocal(m_projector->getLocalToWorld(), m_projector->getWorldToLocal());

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

osg::Node* osgManipulator::CDraggerRotate::getGeometry()
{
    return m_geometry;
}

osg::Quat osgManipulator::CDraggerRotate::computeRotation(const osg::Vec3 & point1, const osg::Vec3 & point2)
{
    osg::Quat quat;
    quat.makeRotate(point1, point2);

    return quat;
}

void osgManipulator::CDraggerRotate::revertTransformsOnPlane()
{

    osg::Matrix parentMatrix = getParentDragger()->getMatrix();

    parentMatrix.invert(parentMatrix);

    osg::Quat rotation = parentMatrix.getRotate();
    parentMatrix.makeIdentity();
    parentMatrix.setRotate(rotation);

    osg::Plane plane = m_initialPlane;
    plane.transform(parentMatrix);

    m_projector->setPlane(plane);
}


void osgManipulator::CDraggerRotate::setPlane(const osg::Plane & plane)
{
    m_projector->setPlane(plane);
}
