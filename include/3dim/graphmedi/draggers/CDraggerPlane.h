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

#ifndef CDRAGGERPLANE_H_included
#define CDRAGGERPLANE_H_included

#include <draggers/CDraggerBase.h>
#include <osgManipulator/Projector>
#include <osg/PolygonOffset>


namespace osgManipulator
{
class CDraggerPlane : public CDraggerBase < Dragger >
{
public:
    CDraggerPlane(osg::Node* geometry);
    CDraggerPlane(osg::Node* geometry, const osg::Plane& plane);

    void setPlane(const osg::Plane & plane) { _projector = new PlaneProjector(plane); }

    virtual bool handle(const PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

    virtual void accept(osg::NodeVisitor& nv);

    osg::Node* getGeometry();

protected:
    //! Revert plane rotation given by matrix transformations
    virtual void revertTransformsOnPlane();

protected:

    osg::ref_ptr< PlaneProjector >  _projector;

    osg::Vec3d _startProjectedPoint;

    osg::Plane m_initialPlane;

    //! View matrix
    osg::Matrix m_view_matrix;

    osg::ref_ptr<osg::Node> m_geometry;
};

}

#endif