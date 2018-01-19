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
 
#ifndef CCylinderDragger_H_included
#define CCylinderDragger_H_included

#include <osgManipulator/RotateCylinderDragger>
#include <osg/CTwoMaterialsNode.h>

// CylinderPlaneProjector and Cylinder Dragger are based on original OSG code but modify some functionality
namespace osgManipulator
{
/**
 * CylinderPlaneProjector projects a point onto a plane relative to the
 * given cylinder.  For most cases, the plane will be parallel to the
 * cylinder axis oriented towards the eyepoint.  When the eyepoint and
 * cylinder axis are close to parallel, then it will project onto a plane
 * perpendicular to the cylinder.
 */
    class XCylinderPlaneProjector : public osgManipulator::CylinderProjector
    {
        public:

            XCylinderPlaneProjector();

            XCylinderPlaneProjector(osg::Cylinder* cylinder);

            virtual ~XCylinderPlaneProjector();

            /**
             * Calculates the object coordinates (projectedPoint) of a window
             * coordinate (pointToProject) when projected onto the given plane.
             * Returns true on successful projection.
             * \param[in] pi Incoming intersection information
             * \param[out] projectedPoint Point located on the given plane
             * \return bool Whether the projection onto the plane was successful.
             */
            virtual bool project(const osgManipulator::PointerInfo& pi, const osg::Vec3d& eyeDir, osg::Vec3d& projectedPoint) const;

            /**
             * Generates a rotation about the cylinder axis based upon the incoming
             * projected points on the plane computed from project().
             * \param[in] p1 Initial projection point
             * \param[in] p2 Second projection point
             * \return osg::Quat Rotation about cylinder axis
             */
            osg::Quat getRotation(const osg::Vec3d& p1, const osg::Vec3d& p2) const;

        protected:
            mutable osg::Plane _plane;
            mutable osg::Vec3d _planeLineStart, _planeLineEnd;
            mutable bool       _parallelPlane;
    };

	class CCylinderDragger : public osg::CTwoMaterialsNode< RotateCylinderDragger >
	{
    protected:
        osg::ref_ptr<XCylinderPlaneProjector> _projector;
        osg::Vec3                             _realEyeDir;
	public:
		//! Constructor
		CCylinderDragger();

		/** Handle pick events on dragger and generate TranslateInLine commands. */
        virtual bool handle(const PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

	};
}

// CCylinderDragger_H_included
#endif

