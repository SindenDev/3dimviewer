///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#include <osg/CSliceDragger.h>

#include <osgManipulator/Command>
#include <osgManipulator/CommandManager>

#include <iostream>

//-----------------------------------------------------------------------------
osgManipulator::CSliceDragger::CSliceDragger(const osg::Vec3 &s, const osg::Vec3 &e)
    : Translate1DDragger(s, e)
{ 
	setColor(osg::Vec4(1,1,1,1));
}

//-----------------------------------------------------------------------------
bool osgManipulator::CSliceDragger::handle(const PointerInfo &pointer, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    // Check if the dragger node is in the nodepath.
    if (_checkForNodeInNodePath)
    {
        if (!pointer.contains(this))
        {
            return false;
        }
    }

    switch (ea.getEventType())
    {
        // Pick start.
        case (osgGA::GUIEventAdapter::PUSH):
            {
                // Get the LocalToWorld matrix for this node and set it for the projector.
                osg::NodePath nodePathToRoot;
                computeNodePathToRoot(*this->getParent(0), nodePathToRoot);
                osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);
                _projector->setLocalToWorld(localToWorld);

                osg::LineSegment::vec_type lineStart = _projector->getLineStart();
                osg::LineSegment::vec_type lineEnd = _projector->getLineEnd();
                lineEnd = lineEnd - lineStart; // relative position of end to start

                lineStart = pointer.getLocalIntersectPoint();
                lineEnd = lineStart + lineEnd;

                // modify projector line so that intersection lies on it (this makes dragger behavior better than the default Translate1DDragger)
                _projector->setLine(lineStart, lineEnd);

                if (_projector->project(pointer, _startProjectedPoint))
                {
                    // Generate the motion command.
                    osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(_projector->getLineStart(), _projector->getLineEnd());
                    cmd->setStage(MotionCommand::START);
                    cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

                    // Dispatch command.
                    dispatch(*cmd);

                    // Set color to pick color.
                    setMaterialColor(_pickColor,*this);

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
                    // Generate the motion command.
                    osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(_projector->getLineStart(), _projector->getLineEnd());
                    cmd->setStage(MotionCommand::MOVE);
                    cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
                    cmd->setTranslation(projectedPoint - _startProjectedPoint);

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
                if (_projector->project(pointer, projectedPoint))
                {
                    osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(_projector->getLineStart(), _projector->getLineEnd());

                    cmd->setStage(MotionCommand::FINISH);
                    cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

                    // Dispatch command.
                    dispatch(*cmd);

                    // Reset color.
                    setMaterialColor(_color,*this);

                    aa.requestRedraw();
                }

                return true;
            }
        default:
            return false;
    }
}
