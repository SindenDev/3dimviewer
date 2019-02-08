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

#include <osg/Constraints.h>
#include <osg/dbout.h>

using namespace osgManipulator;


bool CLineConstraint::constrain(TranslateInLineCommand &command) const
{
	osg::Vec3 direction(command.getLineEnd() - command.getLineStart());

	osg::Vec3d localTranslatedPoint = ((command.getLineStart() + command.getTranslation())
		* command.getLocalToWorld() * getWorldToLocal());

	osg::Vec3 ls(command.getLineStart());
	osg::Vec3 le(command.getLineEnd());

	int i;
	for(i = 0; i < 3; i++)
		if(direction[i] != 0.0)
			break;

	if(direction[i] == 0.0)
		return true;

	double divided(localTranslatedPoint[i] / direction[i]);

	if(divided > 1.0)
		command.setTranslation(le * getLocalToWorld() * command.getWorldToLocal());

	if(divided < 0.0)
		command.setTranslation(ls * getLocalToWorld() * command.getWorldToLocal());

	return true;
}

osgManipulator::CBoundingBoxConstraint::CBoundingBoxConstraint(const osg::BoundingBox &bb)
{
    setBoundingBox(bb);
}


void osgManipulator::CBoundingBoxConstraint::setBoundingBox(const osg::BoundingBox &bb)
{
    m_valid_bb = bb.valid();
    m_bb_parameters[0] = bb._min;
    m_bb_parameters[1] = bb._max;
}

bool osgManipulator::CBoundingBoxConstraint::constrain(osgManipulator::TranslateInLineCommand& command) const
{
    if(!m_valid_bb)
        return false;

    osg::Vec3f line_direction(command.getLineEnd() - command.getLineStart());

    int i;
    for (i = 0; i < 3; i++)
        if (line_direction[i] != 0.0)
            break;

    if (line_direction[i] == 0.0)
        return true;

    osg::Matrix commandToConstraint = command.getLocalToWorld() * getWorldToLocal();
    osg::Matrix constraintToCommand = getLocalToWorld() * command.getWorldToLocal();

    osg::Vec3d localTranslatedPoint = ((command.getLineStart() + command.getTranslation())
        * commandToConstraint);
        
    osg::Vec3f world_direction(line_direction * commandToConstraint - command.getLocalToWorld().getTrans());
    osg::Vec3f world_start(command.getLineStart() * commandToConstraint);

    line_direction.normalize();
    double divided(localTranslatedPoint[i] / line_direction[i]);

    float t0, t1;
    if (!getLineBBoxIntersections(SRay(world_start, world_direction), t0, t1))
        return false;

    if (divided > t1)
        command.setTranslation((command.getLineStart() + line_direction * t1) * constraintToCommand);

    if (divided < t0)
        command.setTranslation((command.getLineStart() + line_direction * t0) * constraintToCommand);

    return true;
}

bool osgManipulator::CBoundingBoxConstraint::constrain(osgManipulator::TranslateInPlaneCommand& command) const
{
    if (!m_valid_bb)
        return false;

    osg::Matrix commandToConstraint = command.getLocalToWorld();// *getWorldToLocal();
    osg::Matrix constraintToCommand = /*getLocalToWorld() **/ command.getWorldToLocal();

    osg::Vec3 translatedPoint = (command.getReferencePoint() + command.getTranslation()) * commandToConstraint;

    if (pointInBB(translatedPoint))
        return true;

    osg::Vec3 nearest_point = getNearestBBPoint(translatedPoint);
    nearest_point = projectPointOnPlane(nearest_point * constraintToCommand, command.getPlane());
    command.setTranslation(nearest_point - command.getReferencePoint());
    return true;
}

bool osgManipulator::CBoundingBoxConstraint::getLineBBoxIntersections(const SRay &ray, float &t0, float &t1) const
{
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (m_bb_parameters[ray.sign[0]].x() - ray.origin.x()) * ray.inv_direction.x();
    tmax = (m_bb_parameters[1 - ray.sign[0]].x() - ray.origin.x()) * ray.inv_direction.x();
    tymin = (m_bb_parameters[ray.sign[1]].y() - ray.origin.y()) * ray.inv_direction.y();
    tymax = (m_bb_parameters[1 - ray.sign[1]].y() - ray.origin.y()) * ray.inv_direction.y();
    if ((tmin > tymax) || (tymin > tmax))
        return false;
    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;
    tzmin = (m_bb_parameters[ray.sign[2]].z() - ray.origin.z()) * ray.inv_direction.z();
    tzmax = (m_bb_parameters[1 - ray.sign[2]].z() - ray.origin.z()) * ray.inv_direction.z();
    if ((tmin > tzmax) || (tzmin > tmax))
        return false;
    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    t0 = tmin;
    t1 = tmax;

    return true;
}

bool osgManipulator::CBoundingBoxConstraint::pointInBB(const osg::Vec3 &point) const
{
    for (int i = 0; i < 3; ++i)
        if (point[i] < m_bb_parameters[0][i] || point[i] > m_bb_parameters[1][i])
            return false;

    return true;
}

osg::Vec3 osgManipulator::CBoundingBoxConstraint::projectPointOnPlane(const osg::Vec3 &input_point, const osg::Plane &plane) const
{
    osg::Plane plane_normalized(plane);
    plane_normalized.makeUnitLength();

    double distance = plane_normalized.distance(input_point);

    return input_point - plane_normalized.getNormal() * distance;
}

osg::Vec3 osgManipulator::CBoundingBoxConstraint::getNearestBBPoint(const osg::Vec3 &point) const
{
    osg::Vec3 out_point(point);
    for (int i = 0; i < 3; ++i)
    {
        if (point[i] < m_bb_parameters[0][i])
            out_point[i] = m_bb_parameters[0][i];

        if (point[i] > m_bb_parameters[1][i])
            out_point[i] = m_bb_parameters[1][i];
    }

    return out_point;
}

