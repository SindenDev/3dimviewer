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

#include <base/Defs.h>

#include <osg/CTranslateOtherLineDragger.h>
#include <osgManipulator/Command>
#include <osgManipulator/CommandManager>
#include <base/Macros.h>
#include <iostream>

using namespace osgManipulator;

bool CTranslateOtherLineDragger::handle (const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
 //   if( ! this->getHandleEvents() )
 //       return false;

    if( !this->getDraggerActive() )
        return false;

    // Check if the dragger node is in the node path.
    if (_checkForNodeInNodePath)
    {
        if (!pointer.contains(this)) return false;
    }

    switch (ea.getEventType())
    {
        // Pick start.
    case (osgGA::GUIEventAdapter::PUSH):
        {

            // Get the LocalToWorld matrix for this node and set it for the projector.
            osg::NodePath nodePathToRoot;
            computeNodePathToRoot(*this,nodePathToRoot);
            osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);
            _projector->setLocalToWorld(localToWorld);
/*
            // Get current transformation
            osg::Quat rotation, so;
            osg::Vec3d translation, scale;
            getMatrix().decompose(translation, rotation, scale, so);

            translation = tpRotation * translation;

            // Transform projection line to simulate dragger geometry movement
            _projector->setLine(m_mouseStartPoint + translation, m_mouseEndPoint + translation);
*/
            if (_projector->project(pointer, _startProjectedPoint))
            {
                // Generate the motion command.
                osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(m_movementStartPoint, m_movementEndPoint);

                cmd->setStage(MotionCommand::START);
                cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

                // Dispatch command.
				dispatch( *cmd );

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
//            osg::Vec3 projectedPoint;
            if (_projector->project(pointer, projectedPoint))
            {
                // Generate the motion command.
                osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(m_movementStartPoint, m_movementEndPoint);

                cmd->setStage(MotionCommand::MOVE);
                cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

                if(!ptRotation.zeroRotation())
                    cmd->setTranslation((ptRotation * (projectedPoint - _startProjectedPoint)) * m_scaleFactor);
                else
                    cmd->setTranslation((projectedPoint - _startProjectedPoint) * m_scaleFactor);
/*                
                osg::Vec3 tr(cmd->getTranslation());

                //_TRACEW(std::endl << "M: " << tr[0] << ", " << tr[1] << ", " << tr[2] << std::endl);
                // Get current transformation
                osg::Quat rotation, so;
                osg::Vec3d translation, scale;
                getMatrix().decompose(translation, rotation, scale, so);
                //_TRACEW(std::endl << "T: " << translation[0] << ", " << translation[1] << ", " << translation[2] << std::endl);

*/
                // Dispatch command.
				dispatch( *cmd );

                aa.requestRedraw();
            }
            return true; 
        }

        // Pick finish.
    case (osgGA::GUIEventAdapter::RELEASE):
        {
            osg::Vec3d projectedPoint;
//            osg::Vec3 projectedPoint;
            if (_projector->project(pointer, projectedPoint))
            {
                osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(m_movementStartPoint, m_movementEndPoint);
/*
                osg::Vec3 tr(cmd->getTranslation());
                //_TRACEW(std::endl << "Release: " << tr[0] << ", " << tr[1] << ", " << tr[2] << std::endl)

                cmd->setStage(MotionCommand::FINISH);
                cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
*/

				cmd->setStage(MotionCommand::FINISH);

                // Dispatch command.
				dispatch( *cmd );

                // Reset color.
                setMaterialColor(_color,*this);

                aa.requestRedraw();
            }

            return true;
        }
    default:
        return false;
    }
    return false;
}