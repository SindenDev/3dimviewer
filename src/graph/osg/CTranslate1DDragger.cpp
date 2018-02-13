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

#include <osg/CTranslate1DDragger.h>
#include <osgManipulator/Command>
#include <osgManipulator/CommandManager>

#include <osg/ShapeDrawable>
#include <osg/Geometry>

namespace osgManipulator
{
    CTranslate1DDragger::CTranslate1DDragger() : CTranslate1DDragger(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(1.0f, 0.0f, 0.0f))
    {

    }

    CTranslate1DDragger::CTranslate1DDragger(const osg::Vec3& s, const osg::Vec3& e) : _projector(new LineProjector(s, e)), _checkForNodeInNodePath(true)
    {
    }

    CTranslate1DDragger::~CTranslate1DDragger()
    {
    }

    void osgManipulator::CTranslate1DDragger::onMouseEnter()
    {
        applyMaterial(osg::SECOND);
    }

    void osgManipulator::CTranslate1DDragger::onMouseLeave()
    {
        applyMaterial(osg::FIRST);
    }

    bool CTranslate1DDragger::handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        // Check if the dragger node is in the nodepath.
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
            computeNodePathToRoot(*this, nodePathToRoot);
            osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);
            _projector->setLocalToWorld(localToWorld);

            if (_projector->project(pointer, _startProjectedPoint))
            {
                // Generate the motion command.
                osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(_projector->getLineStart(),
                    _projector->getLineEnd());
                cmd->setStage(MotionCommand::START);
                cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(), _projector->getWorldToLocal());

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
            if (_projector->project(pointer, projectedPoint))
            {
                // Generate the motion command.
                osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(_projector->getLineStart(),
                    _projector->getLineEnd());
                cmd->setStage(MotionCommand::MOVE);
                cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(), _projector->getWorldToLocal());
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
            //                osg::Vec3 projectedPoint;
            if (_projector->project(pointer, projectedPoint))
            {
                osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(_projector->getLineStart(),
                    _projector->getLineEnd());

                cmd->setStage(MotionCommand::FINISH);
                cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(), _projector->getWorldToLocal());

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Sets up the default geometry. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void CTranslate1DDragger::setupDefaultGeometry()
    {
        // Get the line length and direction.
        osg::Vec3 lineDir = _projector->getLineEnd() - _projector->getLineStart();
        float lineLength = lineDir.length();
        lineDir.normalize();

        osg::Geode* geode = new osg::Geode;
        // Create a left cone.
        {
            osg::Cone* cone = new osg::Cone(_projector->getLineStart(), 0.025f * lineLength, 0.10f * lineLength);
            osg::Quat rotation;
            rotation.makeRotate(lineDir, osg::Vec3(0.0f, 0.0f, 1.0f));
            cone->setRotation(rotation);

            geode->addDrawable(new osg::ShapeDrawable(cone));
        }

        // Create a right cone.
        {
            osg::Cone* cone = new osg::Cone(_projector->getLineEnd(), 0.025f * lineLength, 0.10f * lineLength);
            osg::Quat rotation;
            rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), lineDir);
            cone->setRotation(rotation);

            geode->addDrawable(new osg::ShapeDrawable(cone));
        }

        // Create an invisible cylinder for picking the line.
        {
            osg::Cylinder* cylinder = new osg::Cylinder((_projector->getLineStart() + _projector->getLineEnd()) / 2, 0.015f * lineLength, lineLength);
            osg::Quat rotation;
            rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), lineDir);
            cylinder->setRotation(rotation);
            osg::Drawable* cylinderGeom = new osg::ShapeDrawable(cylinder);

            setDrawableToAlwaysCull(*cylinderGeom);

            geode->addDrawable(cylinderGeom);
        }

        osg::Geode* lineGeode = new osg::Geode;
        // Create a line.
        {
            osg::Geometry* geometry = new osg::Geometry();

            osg::Vec3Array* vertices = new osg::Vec3Array(2);
            (*vertices)[0] = _projector->getLineStart();
            (*vertices)[1] = _projector->getLineEnd();

            geometry->setVertexArray(vertices);
            geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));

            lineGeode->addDrawable(geometry);
        }

        // Add line and cones to the scene.
        addChild(lineGeode);
        addChild(geode);
    }
}