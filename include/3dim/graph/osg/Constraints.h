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

#ifndef Constraints_H
#define Constraints_H

#include <osgManipulator/Command>
#include <osgManipulator/Constraint>


namespace osgManipulator
{

///////////////////////////////////////////////////////////////////////////////
//! a Class
//
class CLineConstraint : public Constraint
{
public:
    //! Constructor
    CLineConstraint(osg::Node & refNode) : Constraint(refNode) {}

    virtual bool constrain(TranslateInLineCommand &command) const;
};

class CBoundingBoxConstraint : public Constraint
{
public:
    //! Constructor
    CBoundingBoxConstraint(const osg::BoundingBox &bb = osg::BoundingBox());

    //! Set bounding box in world coordinate system
    void setBoundingBox(const osg::BoundingBox &bb);

//    virtual bool constraint(MotionCommand &command) const { return constraintInternal(command); }
    virtual bool constrain(osgManipulator::TranslateInLineCommand& command) const;
    virtual bool constrain(osgManipulator::TranslateInPlaneCommand& command) const;

protected:
    struct SRay {
    public:
        SRay(osg::Vec3f &o, osg::Vec3f &d)
            : origin(o)
            , direction(d)
        {
            inv_direction = osg::Vec3(1.0 / d.x(), 1.0 / d.y(), 1.0 / d.z());

            sign[0] = (inv_direction.x() < 0) ? 1 : 0;
            sign[1] = (inv_direction.y() < 0) ? 1 : 0;
            sign[2] = (inv_direction.z() < 0) ? 1 : 0;
        }

        osg::Vec3f origin;
        osg::Vec3f direction;
        osg::Vec3f inv_direction;
        int sign[3];
    };

protected:
    //! Get intersections of the line with the bounding box
    bool getLineBBoxIntersections(const SRay &ray, float &t0, float &t1) const;

    //! Test if point is inside bb
    bool pointInBB(const osg::Vec3 &point) const;

    //! Project point back on plain
    osg::Vec3 projectPointOnPlane(const osg::Vec3 &input_point, const osg::Plane &plane) const;

    //! Get point on bounding box that is closest to the given point.
    osg::Vec3 getNearestBBPoint(const osg::Vec3 &point) const;
protected:
    osg::Vec3f m_bb_parameters[2];
    bool m_valid_bb;
};

} // namespace osgManipulator

#endif // Constraints_H
