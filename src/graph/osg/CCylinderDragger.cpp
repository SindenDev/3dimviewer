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

#include <osg/CCylinderDragger.h>
#include <osgViewer/View>

// When the squared magnitude (length2) of the cross product of 2
// angles is less than this tolerance, they are considered parallel.
// osg::Vec3 a, b; (a ^ b).length2()
#define CROSS_PRODUCT_ANGLE_TOLERANCE (0.005)

namespace osgManipulator
{

bool computeClosestPointOnLine(const osg::Vec3d& lineStart, const osg::Vec3d& lineEnd,
                               const osg::Vec3d& fromPoint, osg::Vec3d& closestPoint)
{
    osg::Vec3d v = lineEnd - lineStart;
    osg::Vec3d w = fromPoint - lineStart;

    double c1 = w * v;
    double c2 = v * v;

    double almostZero = 0.000001;
    if (c2 < almostZero) return false;

    double b = c1 / c2;
    closestPoint = lineStart + v * b;

    return true;
}

bool getPlaneLineIntersection(const osg::Vec4d& plane,
                              const osg::Vec3d& lineStart, const osg::Vec3d& lineEnd,
                              osg::Vec3d& isect)
{
    const double deltaX = lineEnd.x() - lineStart.x();
    const double deltaY = lineEnd.y() - lineStart.y();
    const double deltaZ = lineEnd.z() - lineStart.z();

    const double denominator = (plane[0]*deltaX + plane[1]*deltaY + plane[2]*deltaZ);
    if (! denominator) return false;

    const double C = (plane[0]*lineStart.x() + plane[1]*lineStart.y() + plane[2]*lineStart.z() + plane[3]) / denominator;

    isect.x() = lineStart.x() - deltaX * C;
    isect.y() = lineStart.y() - deltaY * C;
    isect.z() = lineStart.z() - deltaZ * C;

    return true;
}

osg::Vec3d getLocalEyeDirection(const osg::Vec3d& eyeDir, const osg::Matrix& localToWorld)
{
    // To take a normal from world to local you need to transform it by the transpose of the inverse of the
    // world to local matrix. Pre-multiplying is equivalent to doing a post-multiplication of the transpose.
    osg::Vec3d localEyeDir = localToWorld * eyeDir;
    localEyeDir.normalize();
    return localEyeDir;
}

// Computes a plane to be used as a basis for determining a displacement.  When eyeDir is close
// to the cylinder axis, then the plane will be set to be perpendicular to the cylinder axis.
// Otherwise it will be set to be parallel to the cylinder axis and oriented towards eyeDir.
osg::Plane computeIntersectionPlane(const osg::Vec3d& realEyeDir, const osg::Vec3d& eyeDir, const osg::Matrix& localToWorld,
                                    const osg::Vec3d& axisDir, const osg::Cylinder& cylinder,
                                    osg::Vec3d& planeLineStart, osg::Vec3d& planeLineEnd,
                                    bool& parallelPlane, bool front)
{
    osg::Plane plane;

    osg::Vec3d unitAxisDir = axisDir;
    unitAxisDir.normalize();
    osg::Vec3d perpDir = unitAxisDir ^ getLocalEyeDirection(realEyeDir, localToWorld);

//	double length = perpDir.length2();
    // Check to make sure eye and cylinder axis are not too close
    //if(perpDir.length2() > CROSS_PRODUCT_ANGLE_TOLERANCE)
    if(perpDir.length2() < (1-CROSS_PRODUCT_ANGLE_TOLERANCE)) // Note: I switched the condition
    {
        // Too close, so instead return plane perpendicular to cylinder axis.
        plane.set(unitAxisDir, cylinder.getCenter());
        parallelPlane = false;
        return plane;
    }

    // Otherwise compute plane along axisDir oriented towards eye
    perpDir = unitAxisDir ^ getLocalEyeDirection(eyeDir, localToWorld);
    osg::Vec3d planeDir = perpDir ^ axisDir;
    planeDir.normalize();
    if (! front)
        planeDir = -planeDir;

    osg::Vec3d planePoint = planeDir * cylinder.getRadius() + axisDir;
    plane.set(planeDir, planePoint);

    planeLineStart = planePoint;
    planeLineEnd = planePoint + axisDir;
    parallelPlane = true;
    return plane;
}

XCylinderPlaneProjector::XCylinderPlaneProjector()
{
}

XCylinderPlaneProjector::XCylinderPlaneProjector(osg::Cylinder* cylinder) : CylinderProjector(cylinder), _parallelPlane(false)
{
}

XCylinderPlaneProjector::~XCylinderPlaneProjector()
{
}

bool XCylinderPlaneProjector::project(const osgManipulator::PointerInfo& pi, const osg::Vec3d &eyeDir, osg::Vec3d &projectedPoint) const
{
    if (!_cylinder.valid())
    {
        OSG_WARN << "Warning: Invalid cylinder. CylinderProjector::project() failed."
                               << std::endl;
        return false;
    }

    // Get the near and far points for the mouse point.
    osg::Vec3d nearPoint, farPoint;
    pi.getNearFarPoints(nearPoint,farPoint);

    // Transform these points into local coordinates.
    osg::Vec3d objectNearPoint, objectFarPoint;
    objectNearPoint = nearPoint * getWorldToLocal();
    objectFarPoint  = farPoint * getWorldToLocal();

    // Computes either a plane parallel to cylinder axis oriented to the eye or the plane
    // perpendicular to the cylinder axis if the eye-cylinder angle is close.
    _plane = computeIntersectionPlane(eyeDir, pi.getEyeDir(), getLocalToWorld(), _cylinderAxis,
                                      *_cylinder, _planeLineStart, _planeLineEnd,
                                     _parallelPlane, _front);

    // Now find the point of intersection on our newly-calculated plane.
    getPlaneLineIntersection(_plane.asVec4(), objectNearPoint, objectFarPoint, projectedPoint);
    return true;
}

osg::Quat XCylinderPlaneProjector::getRotation(const osg::Vec3d& p1, const osg::Vec3d& p2) const
{
    if(_parallelPlane)
    {
        osg::Vec3d closestPointToPlaneLine1, closestPointToPlaneLine2;
        computeClosestPointOnLine(_planeLineStart, _planeLineEnd,
                                  p1, closestPointToPlaneLine1);
        computeClosestPointOnLine(_planeLineStart, _planeLineEnd,
                                  p2, closestPointToPlaneLine2);

        osg::Vec3d v1 = p1 - closestPointToPlaneLine1;
        osg::Vec3d v2 = p2 - closestPointToPlaneLine2;

        osg::Vec3d diff = v2 - v1;
        double d = diff.length();

        // The amount of rotation is inversely proportional to the size of the cylinder
        double angle = (getCylinder()->getRadius() == 0.0) ? 0.0 : (d / getCylinder()->getRadius());
        osg::Vec3d rotAxis = _plane.getNormal() ^ v1;

        if (v2.length() > v1.length())
           return osg::Quat(angle, rotAxis);
        else
           return osg::Quat(-angle, rotAxis);
    }
    else
    {
        osg::Vec3d v1 = p1 - getCylinder()->getCenter();
        osg::Vec3d v2 = p2 - getCylinder()->getCenter();

        double cosAngle = v1 * v2 / (v1.length() * v2.length());

        if (cosAngle > 1.0 || cosAngle < -1.0)
            return osg::Quat();

        double angle = acosf(cosAngle);
        osg::Vec3d rotAxis = v1 ^ v2;

        return osg::Quat(angle, rotAxis);
    }
}

} // namespace osgManipulator

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Default constructor. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osgManipulator::CCylinderDragger::CCylinderDragger(): _realEyeDir()
{
    _projector = new osgManipulator::XCylinderPlaneProjector();
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Handles. 
//!
//!\param	pi				The pi. 
//!\param	ea				The ea. 
//!\param [in,out]	us	The us. 
//!
//!\return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////

bool osgManipulator::CCylinderDragger::handle( const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
	// Check if the dragger node is in the nodepath.
    if (!pointer.contains(this)) return false;

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

			_startLocalToWorld = _projector->getLocalToWorld();
			_startWorldToLocal = _projector->getWorldToLocal();

			if (_projector->isPointInFront(pointer, _startLocalToWorld))
				_projector->setFront(true);
			else
				_projector->setFront(false);

            {   // because pointer.getEyeDir() which was used in the original code
                // will return direction which doesn't reflect perspective, we get our own
                // and on push save it for further dragging so we have "stable conditions"

                // Get the near and far points for the mouse point.
                osg::Vec3d nearPoint, farPoint;
                pointer.getNearFarPoints(nearPoint,farPoint);
                // get "real" eye vector
                _realEyeDir = nearPoint-farPoint;
                _realEyeDir.normalize();
            }

			osg::Vec3d projectedPoint;
            if (_projector->project(pointer, _realEyeDir, projectedPoint))
			{
				// Generate the motion command.
				osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();
				cmd->setStage(MotionCommand::START);
				cmd->setLocalToWorldAndWorldToLocal(_startLocalToWorld,_startWorldToLocal);

				// Dispatch command.
				dispatch(*cmd);

				// Set color to pick color.
				setMaterial(osg::SECOND);

				_prevWorldProjPt = projectedPoint * _projector->getLocalToWorld();
				_prevRotation = osg::Quat();

				aa.requestRedraw();
			}
			return true; 
		}

		// Pick move.
	case (osgGA::GUIEventAdapter::DRAG):
		{
			// Get the LocalToWorld matrix for this node and set it for the projector.
			osg::Matrix localToWorld = osg::Matrix(_prevRotation) * _startLocalToWorld;
			_projector->setLocalToWorld(localToWorld);

			osg::Vec3d projectedPoint;
            if (_projector->project(pointer, _realEyeDir, projectedPoint))
			{
				osg::Vec3d prevProjectedPoint = _prevWorldProjPt * _projector->getWorldToLocal();
                osg::Quat  deltaRotation = _projector->getRotation(prevProjectedPoint, projectedPoint);
				osg::Quat rotation = deltaRotation * _prevRotation;

				// Generate the motion command.
				osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();
				cmd->setStage(MotionCommand::MOVE);
				cmd->setLocalToWorldAndWorldToLocal(_startLocalToWorld,_startWorldToLocal);
				cmd->setRotation(rotation);

				// Dispatch command.
				dispatch(*cmd);

				_prevWorldProjPt = projectedPoint * _projector->getLocalToWorld();
				_prevRotation = rotation;
				aa.requestRedraw();
			}
			return true; 
		}

		// Pick finish.
	case (osgGA::GUIEventAdapter::RELEASE):
		{
			osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();

			cmd->setStage(MotionCommand::FINISH);
			cmd->setLocalToWorldAndWorldToLocal(_startLocalToWorld,_startWorldToLocal);

			// Dispatch command.
			dispatch(*cmd);

			// Reset color.
			setMaterial(osg::FIRST);

			aa.requestRedraw();

			return true;
		}
	default:
		return false;
	}
}

