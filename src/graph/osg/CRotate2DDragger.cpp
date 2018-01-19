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

#include <osg/CRotate2DDragger.h>
#include <osgManipulator/CommandManager>

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/ShadeModel>

// namespace used
using namespace osgManipulator;

///////////////////////////////////////////////////////////////////////////////
// Constructor - default settings
CRotate2DDragger::CRotate2DDragger()
{
	m_initialPlane.set(osg::Vec3(0.0, 0.0, 1.0), osg::Vec3(0.0, 0.0, 0.0));
	_projector = new PlaneProjector(m_initialPlane);
	_polygonOffset = new osg::PolygonOffset(-1.0f,-1.0f);
}


///////////////////////////////////////////////////////////////////////////////
// Constructor - user set version
CRotate2DDragger::CRotate2DDragger(const osg::Plane &plane)
: m_initialPlane(plane)
{
	_projector = new PlaneProjector(m_initialPlane);
	_polygonOffset = new osg::PolygonOffset(-1.0f,-1.0f);
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
CRotate2DDragger::~CRotate2DDragger()
{
}

///////////////////////////////////////////////////////////////////////////////
// Handle event

bool CRotate2DDragger::handle(const osgManipulator::PointerInfo &pi, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
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
			computeNodePathToRoot(*this,nodePathToRoot);

			revertTransformsOnPlane();

			osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);
			_projector->setLocalToWorld(localToWorld);

			if (_projector->project(pi, _startProjectedPoint))
			{

				// Generate the motion command
				osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand;

				cmd->setStage(MotionCommand::START);
				cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
				updateCommand(*cmd);

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
//			osg::Vec3 projectedPoint;
			if (_projector->project(pi, projectedPoint))
			{
				// Generate the motion command
				osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand;

				cmd->setStage(MotionCommand::MOVE);
				cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
				osg::Quat rotation(computeRotation(_startProjectedPoint, projectedPoint));
                double angle; osg::Vec3 axis;

                rotation.getRotate(angle, axis);
                if(angle < osg::DegreesToRadians(0.01))
                    return true;

                cmd->setRotation(rotation);

				
				updateCommand(*cmd);

				// Dispatch command.
				dispatch( *cmd );

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
			cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

			updateCommand(*cmd);

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

///////////////////////////////////////////////////////////////////////////////
// Setup default geometry
void CRotate2DDragger::setupDefaultGeometry()
{

}

///////////////////////////////////////////////////////////////////////////////
// Compute rotation
osg::Quat CRotate2DDragger::computeRotation(const osg::Vec3 & point1, const osg::Vec3 & point2)
{
	osg::Quat quat;
	quat.makeRotate(point1, point2);

	return quat;
}

///////////////////////////////////////////////////////////////////////////////
// Revert transformations
void CRotate2DDragger::revertTransformsOnPlane()
{

	osg::Matrix parentMatrix = getParentDragger()->getMatrix();

	parentMatrix.invert(parentMatrix);

	osg::Quat rotation = parentMatrix.getRotate();
	parentMatrix.makeIdentity();
	parentMatrix.setRotate(rotation);
	
	osg::Plane plane = m_initialPlane;
	plane.transform(parentMatrix);

	_projector->setPlane(plane);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Sets a plane. 
//!
//!\param   plane   The plane. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRotate2DDragger::setPlane( const osg::Plane & plane )
{
    _projector->setPlane( plane );
}


///////////////////////////////////////////////////////////////////////////////
//
void CRotate2DDragger::updateCommand(MotionCommand & command)
{
}
