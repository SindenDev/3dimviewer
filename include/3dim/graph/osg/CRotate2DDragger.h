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

#ifndef CRotate2DDragger_H
#define CRotate2DDragger_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osgManipulator/Dragger>
#include <osgManipulator/Projector>
#include <osgManipulator/Command>

#include <osg/PolygonOffset>
#include <osg/CTwoMaterialsNode.h>


namespace osgManipulator
{

///////////////////////////////////////////////////////////////////////////////
//! Dragger for performing 2D rotation.

class CRotate2DDragger : public osg::CTwoMaterialsNode< Dragger >
{
public:
	CRotate2DDragger();

	CRotate2DDragger(const osg::Plane& plane);

	//! Destructor
    virtual ~CRotate2DDragger();

	/** Handle pick events on dragger and generate TranslateInLine commands. */
	virtual bool handle(const PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	/** Setup default geometry for dragger. */
	void setupDefaultGeometry();

    //! Set dragging plane
    void setPlane( const osg::Plane & plane );

protected:
	//! Compute rotation from point1 to point2 around axis defined by plane (and on plane);
	osg::Quat computeRotation(const osg::Vec3 & point1, const osg::Vec3 & point2);

	//! Modify command
	virtual void updateCommand(MotionCommand & command);

	//! Revert plane rotation given by matrix transformations
	virtual void revertTransformsOnPlane();

protected:
	osg::ref_ptr< PlaneProjector > _projector;

	osg::Vec3d _startProjectedPoint;

	osg::ref_ptr<osg::PolygonOffset> _polygonOffset;

	osg::Plane m_initialPlane;
};


} // namespace osgManipulator

#endif // CRotate2DDragger_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
