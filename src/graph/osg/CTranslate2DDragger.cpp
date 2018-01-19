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

///////////////////////////////////////////////////////////////////////////////
// include files
#include <osg/CTranslate2DDragger.h>
#include <osgManipulator/Translate2DDragger>
#include <osgManipulator/Command>
#include <osgManipulator/CommandManager>
#include <osg/Material>

#include <osg/dbout.h>

using namespace osgManipulator;

///////////////////////////////////////////////////////////////////////////////
// Constructor - default settings
CTranslate2DDragger::CTranslate2DDragger()
	: m_bNoGeometryTransform( true )
    , m_bTranslating(false)
{
    setName("CTranslate2DDragger");
	m_initialPlane.set(osg::Vec3(0.0, 0.0, 1.0), osg::Vec3(0.0, 0.0, 0.0));
	_projector = new PlaneProjector(m_initialPlane);
	_polygonOffset = new osg::PolygonOffset(-1.0f,-1.0f);
}

///////////////////////////////////////////////////////////////////////////////
// Constructor - user set version
CTranslate2DDragger::CTranslate2DDragger(const osg::Plane& plane)
: m_initialPlane(plane)
, m_bNoGeometryTransform( true )
, m_bTranslating(false)
{
	_projector = new PlaneProjector(m_initialPlane);
	_polygonOffset = new osg::PolygonOffset(-1.0f,-1.0f);
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
CTranslate2DDragger::~CTranslate2DDragger()
{
}

///////////////////////////////////////////////////////////////////////////////
// Handle event
bool CTranslate2DDragger::handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	// Check if the dragger node is in the nodepath.
	if (!pointer.contains(this)) return false;

    if(!m_bTranslating && (ea.getButtonMask() &  osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) == 0)
        return false;

	switch (ea.getEventType())
	{
		// Pick start.
	case (osgGA::GUIEventAdapter::PUSH):
		{
            m_bTranslating = true;

			// Get the LocalToWorld matrix for this node and set it for the projector.
			osg::NodePath nodePathToRoot;
			computeNodePathToRoot(*this,nodePathToRoot);

			revertTransformsOnPlane();

			osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);

            DBOUT_VEC("TD LIP: ", pointer.getLocalIntersectPoint());
            DBOUT_MTX("TD LTW: ", localToWorld);

			_projector->setLocalToWorld(localToWorld);

			if (_projector->project(pointer, _startProjectedPoint))
			{
				// Generate the motion command.
				osg::ref_ptr<TranslateInPlaneCommand> cmd = new TranslateInPlaneCommand(_projector->getPlane());

				cmd->setStage(MotionCommand::START);
				cmd->setReferencePoint(_startProjectedPoint);
				cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
				
				updateCommand(cmd);
				// Dispatch command.
				dispatch( *cmd );

				// Set color to pick color.
				setMaterial( osg::SECOND );
				getOrCreateStateSet()->setAttributeAndModes(_polygonOffset.get(), osg::StateAttribute::ON);

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
				cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
				cmd->setTranslation(projectedPoint - _startProjectedPoint);
				cmd->setReferencePoint(_startProjectedPoint);

				updateCommand(cmd);

				// Dispatch command.
				dispatch( *cmd );

				aa.requestRedraw();
			}
			return true; 
		}

		// Pick finish.
	case (osgGA::GUIEventAdapter::RELEASE):
		{
            m_bTranslating = false;

			osg::ref_ptr<TranslateInPlaneCommand> cmd = new TranslateInPlaneCommand(_projector->getPlane());

			cmd->setStage(MotionCommand::FINISH);
			cmd->setReferencePoint(_startProjectedPoint);
			cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

			updateCommand(cmd);

			// Dispatch command.
			dispatch( *cmd );

			// Reset color.
			setMaterial( osg::FIRST );
			getOrCreateStateSet()->removeAttribute(_polygonOffset.get());

			aa.requestRedraw();

			return true;
		}
	default:
		return false;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Revert plane rotation given by matrix transformations
void CTranslate2DDragger::revertTransformsOnPlane()
{
	osg::Matrix parentMatrix = getParentDragger()->getMatrix();

    DBOUT_MTX("TD PM", parentMatrix);

	parentMatrix.invert(parentMatrix);

    DBOUT_MTX("TD IPM", parentMatrix);

	osg::Quat rotation = parentMatrix.getRotate();
	parentMatrix.makeIdentity();
	parentMatrix.setRotate(rotation);

	osg::Plane plane = m_initialPlane;

    DBOUT("TD plane before: " << plane[0] << ", " << plane[1] << ", " << plane[2] << ", " << plane[3]);

	plane.transform(parentMatrix);

    DBOUT("TD plane after: " << plane[0] << ", " << plane[1] << ", " << plane[2] << ", " << plane[3]);

	_projector->setPlane(plane);

}



