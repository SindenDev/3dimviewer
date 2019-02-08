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

#include "draggers/CDraggerScale.h"

osg::CDraggerScaleGeometry::CDraggerScaleGeometry()
    : m_offsetHead(0, 0, 0)
    , m_offsetTail(0, 0, 0)
{
    // Create geometries
    osg::ref_ptr<osg::CSphereGeometry> sphereHead(new osg::CSphereGeometry());
    osg::ref_ptr<osg::CSphereGeometry> sphereTail(new osg::CSphereGeometry());

    sphereHead->setRadius(1.0);
    sphereTail->setRadius(1.0);

    sphereHead->update();
    sphereTail->update();

    // Create shifting matrix transforms
    osg::Vec3 end(0.0, 0.0, 1.0);// = _projector->getLineEnd();
    osg::Quat q1; q1.makeRotate(osg::Vec3(0.0, 0.0, 1.0), end);
    osg::Quat q2; q2.makeRotate(osg::Vec3(0.0, 0.0, 1.0), -end);
    osg::Matrix shiftHead = osg::Matrix::rotate(q1);
    osg::Matrix shiftTail = osg::Matrix::rotate(q2);

    m_defaultMatrixHead = osg::Matrix::rotate(q1);
    m_defaultMatrixTail = osg::Matrix::rotate(q2);

    m_matrixHead = new osg::MatrixTransform(m_defaultMatrixHead);
    m_matrixTail = new osg::MatrixTransform(m_defaultMatrixTail);

    m_matrixHead->addChild(sphereHead);
    m_matrixTail->addChild(sphereTail);

    addChild(m_matrixHead);
    addChild(m_matrixTail);
}

void osg::CDraggerScaleGeometry::setOffsets(const osg::Vec3& offsetHead, const osg::Vec3& offsetTail)
{
    m_matrixHead->setMatrix(m_defaultMatrixHead * osg::Matrix::translate(offsetHead));
    m_matrixTail->setMatrix(m_defaultMatrixTail * osg::Matrix::translate(offsetTail));

    m_offsetHead = offsetHead;
    m_offsetTail = offsetTail;
}

void osg::CDraggerScaleGeometry::scale(double scaleFactor)
{
    m_matrixHead->setMatrix(m_defaultMatrixHead * osg::Matrix::scale(scaleFactor, scaleFactor, scaleFactor) * osg::Matrix::translate(m_offsetHead));
    m_matrixTail->setMatrix(m_defaultMatrixTail * osg::Matrix::scale(scaleFactor, scaleFactor, scaleFactor) * osg::Matrix::translate(m_offsetTail));
}



osgManipulator::CDraggerScale::CDraggerScale(osg::Node* geometry)
    : CDraggerScale(CModifiedScaleCommand::SCALE_UNIFORM, osg::Vec3(1.0f, 0.0f, 0.0f), geometry)
{
}

osgManipulator::CDraggerScale::CDraggerScale(CModifiedScaleCommand::EScalingMode mode, osg::Vec3 scaleVector, osg::Node* geometry)
    : m_scaleVector(scaleVector)
    , m_inverseScalingEnabled(true)
    , m_mode(mode)
    , m_referencePoint(0.0)
    , m_geometry(geometry)
{
    _projector = new LineProjector(osg::Vec3(0.0, 0.0, 0.0), scaleVector);
    addChild(geometry);
}

void osgManipulator::CDraggerScale::enableInverseScaling(bool enable)
{
    m_inverseScalingEnabled = enable;
}

bool osgManipulator::CDraggerScale::inverseScalingEnabled()
{
    return m_inverseScalingEnabled;
}
bool osgManipulator::CDraggerScale::handle(const PointerInfo & pointer, const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa)
{
    if (!pointer.contains(this)) return false;

    switch (ea.getEventType())
    {
        // Pick start.
    case (osgGA::GUIEventAdapter::PUSH):
    {
        // Get the LocalToWorld matrix for this node and set it for the projector.
        osg::NodePath nodePathToRoot;
        computeNodePathToRoot(*this, nodePathToRoot);
        osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);
        _projector->setLocalToWorld(localToWorld);
        _projector->setLine(osg::Vec3(0.0, 0.0, 0.0), m_scaleVector);

        if (_projector->project(pointer, _startProjectedPoint))
        {
            // Generate the motion command.
            osg::ref_ptr<CModifiedScaleCommand> cmd = new CModifiedScaleCommand(m_mode);
            cmd->setStage(MotionCommand::START);
            cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(), _projector->getWorldToLocal());

            //compute reference point
            osg::Vec3 point(_startProjectedPoint);
            point[0] *= m_scaleVector[0];
            point[1] *= m_scaleVector[1];
            point[2] *= m_scaleVector[2];
            m_referencePoint = point[0] + point[1] + point[2];

            cmd->setReferencePoint(m_referencePoint);

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
        if (_projector->project(pointer, projectedPoint))
        {            
            // Compute scale.
            double scale = computeScale(_startProjectedPoint, projectedPoint, _scaleCenter);

            if (scale < getMinScale())
                scale = getMinScale();

            // Generate the motion command.
            osg::ref_ptr<CModifiedScaleCommand> cmd = new CModifiedScaleCommand(m_mode);

            cmd->setStage(MotionCommand::MOVE);
            cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(), _projector->getWorldToLocal());
            cmd->setScale(scale);
            cmd->setScaleCenter(m_scaleVector * _scaleCenter);
            cmd->setReferencePoint(m_referencePoint);
            
            // Dispatch command.
            dispatch(*cmd);

            aa.requestRedraw();
        }
        return true;
    }

    // Pick finish.
    case (osgGA::GUIEventAdapter::RELEASE):
    {
        osg::ref_ptr<CModifiedScaleCommand> cmd = new CModifiedScaleCommand(m_mode);

        cmd->setStage(MotionCommand::FINISH);
        cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(), _projector->getWorldToLocal());

        // Dispatch command.
        dispatch(*cmd);

        applyMaterial(osg::FIRST);

        aa.requestRedraw();

        return true;

    }
    default:
        return false;
    }
    return false;
}

double osgManipulator::CDraggerScale::computeScale(const osg::Vec3d& startProjectedPoint, const osg::Vec3d& projectedPoint, double scaleCenter)
{
    double denom = startProjectedPoint.length() - scaleCenter;
    double scale = denom ? (projectedPoint.length() - scaleCenter) / denom : 1.0;

    //if we dont want inverse scaling
    if (!m_inverseScalingEnabled)
    {
        osg::Vec3 startPoint(startProjectedPoint);
        startPoint[0] *= m_scaleVector[0];
        startPoint[1] *= m_scaleVector[1];
        startPoint[2] *= m_scaleVector[2];

        osg::Vec3 point(projectedPoint);
        point[0] *= m_scaleVector[0];
        point[1] *= m_scaleVector[1];
        point[2] *= m_scaleVector[2];

        //sign change indicates, that user has crossed scale center, and scaling will be inversed
        float startSign = startPoint[0] + startPoint[1] + startPoint[2];
        float pointSign = point[0] + point[1] + point[2];

        //sign has changed
        if (startSign* pointSign < 0)
        {
            return getMinScale();
        }
    }

    return scale;
}

osg::Node* osgManipulator::CDraggerScale::getGeometry()
{
    return m_geometry;
}