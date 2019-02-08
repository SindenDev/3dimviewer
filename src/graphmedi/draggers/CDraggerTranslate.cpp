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

#include "draggers/CDraggerTranslate.h"
#include <osg/CGeometryGenerator.h>

osg::CDraggerTranslateGeometry::CDraggerTranslateGeometry(const osg::Vec3& headRotation)
    : m_offsetHead(0, 0, 0)
    , m_offsetTail(0, 0, 0)
{
    // Create arrows geometry
    osg::ref_ptr<osg::CArrow3DGeometry> arrowHead(new osg::CArrow3DGeometry(32));
    osg::ref_ptr<osg::CArrow3DGeometry> arrowTail(new osg::CArrow3DGeometry(32));

    // Set arrows parameters
    arrowHead->setSize(0.5, 1.0, 1.0, 1.0);
    arrowTail->setSize(0.5, 1.0, 1.0, 1.0);

    arrowHead->update();
    arrowTail->update();

    osg::Quat q1; q1.makeRotate(osg::Vec3(0.0, 0.0, 1.0), headRotation);
    osg::Quat q2; q2.makeRotate(osg::Vec3(0.0, 0.0, 1.0), -headRotation);

    m_defaultMatrixHead = osg::Matrix::rotate(q1);
    m_defaultMatrixTail = osg::Matrix::rotate(q2);

    m_matrixHead = new osg::MatrixTransform(m_defaultMatrixHead);
    m_matrixTail = new osg::MatrixTransform(m_defaultMatrixTail);

    m_matrixHead->addChild(arrowHead);
    m_matrixTail->addChild(arrowTail);

    addChild(m_matrixHead);
    addChild(m_matrixTail);
}

void osg::CDraggerTranslateGeometry::setOffsets(const osg::Vec3& offsetHead, const osg::Vec3& offsetTail)
{
    m_matrixHead->setMatrix(m_defaultMatrixHead * osg::Matrix::translate(offsetHead));
    m_matrixTail->setMatrix(m_defaultMatrixTail * osg::Matrix::translate(offsetTail));

    m_offsetHead = offsetHead;
    m_offsetTail = offsetTail;
}

void osg::CDraggerTranslateGeometry::scale(double scaleFactor)
{
    m_matrixHead->setMatrix(m_defaultMatrixHead * osg::Matrix::scale(scaleFactor, scaleFactor, scaleFactor) * osg::Matrix::translate(m_offsetHead));
    m_matrixTail->setMatrix(m_defaultMatrixTail * osg::Matrix::scale(scaleFactor, scaleFactor, scaleFactor) * osg::Matrix::translate(m_offsetTail));
}



osgManipulator::CDraggerTranslate::CDraggerTranslate()
    : CDraggerTranslate(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(1.0f, 0.0f, 0.0f))
{
}

osgManipulator::CDraggerTranslate::CDraggerTranslate(const osg::Vec3 & s, const osg::Vec3 & e)
{
    m_projector = new LineProjector(s, e);
    m_geometry = new osg::CDraggerTranslateGeometry(m_projector->getLineEnd());
    addChild(m_geometry);
}

osgManipulator::CDraggerTranslate::CDraggerTranslate(osg::Node* geometry)
    : CDraggerTranslate(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(1.0f, 0.0f, 0.0f))
{
}

osgManipulator::CDraggerTranslate::CDraggerTranslate(osg::Node* geometry, const osg::Vec3 & s, const osg::Vec3 & e)
    : m_geometry(geometry)
{
    m_projector = new LineProjector(s, e);
    addChild(geometry);
}

bool osgManipulator::CDraggerTranslate::handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    if (!pointer.contains(this))
    {
        return false;
    }

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

        if (m_projector->project(pointer, m_startProjectedPoint))
        {
            // Generate the motion command.
            osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(m_projector->getLineStart(),
                m_projector->getLineEnd());
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
        //                osg::Vec3 projectedPoint;
        if (m_projector->project(pointer, projectedPoint))
        {
            // Generate the motion command.
            osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(m_projector->getLineStart(),
                m_projector->getLineEnd());
            cmd->setStage(MotionCommand::MOVE);
            cmd->setLocalToWorldAndWorldToLocal(m_projector->getLocalToWorld(), m_projector->getWorldToLocal());
            cmd->setTranslation(projectedPoint - m_startProjectedPoint);

            // Dispatch command.
            dispatch(*cmd);

            aa.requestRedraw();
        }
        return true;
    }

    // Pick finish.
    case (osgGA::GUIEventAdapter::RELEASE):
    {
        osg::Vec3d projectedPoint;
        //                osg::Vec3 projectedPoint;
        if (m_projector->project(pointer, projectedPoint))
        {
            osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(m_projector->getLineStart(),
                m_projector->getLineEnd());

            cmd->setStage(MotionCommand::FINISH);
            cmd->setLocalToWorldAndWorldToLocal(m_projector->getLocalToWorld(), m_projector->getWorldToLocal());

            // Dispatch command.
            dispatch(*cmd);

            // Reset color.
            applyMaterial(osg::FIRST);

            aa.requestRedraw();
        }

        return true;
    }
    default:
        return false;
    }
    return false;
}

osg::Node* osgManipulator::CDraggerTranslate::getGeometry()
{
    return m_geometry;
}