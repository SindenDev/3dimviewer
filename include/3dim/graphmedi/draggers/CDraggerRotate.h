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

#ifndef CDRAGGERROTATE_H_included
#define CDRAGGERROTATE_H_included

#include <draggers/CDraggerBase.h>
#include <osgManipulator/Projector>
#include <osg/PolygonOffset>

namespace osg
{
    class CDonutGeometry;

    class CDraggerRotateGeometry : public osg::Group
    {
    public:
        CDraggerRotateGeometry();

        void setSize(double r1, double r2);

    protected:
        osg::ref_ptr<osg::CDonutGeometry> m_donut;
    };
}


namespace osgManipulator
{

class CDraggerRotate : public CDraggerBase < Dragger >
{
public:
    CDraggerRotate(osg::Node* geometry = new osg::CDraggerRotateGeometry());
    CDraggerRotate(const osg::Plane& plane, osg::Node* geometry = new osg::CDraggerRotateGeometry());

    virtual bool handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

    osg::Node* getGeometry();

protected:
    //! \fn                 osg::Quat computeRotation(const osg::Vec3 & point1, const osg::Vec3 & point2)
    //!
    //! \brief              Compute rotation from point1 to point2 around axis defined by plane (and on plane)
    //! \param  point1      First point
    //! \param  point2      Second point.
    osg::Quat computeRotation(const osg::Vec3 & point1, const osg::Vec3 & point2);

    //! \fn     void setPlane(const osg::Plane & plane)
    //!
    //! \brief              Sets projector plane.
    //! \param  plane       The plane to project on.
    void setPlane(const osg::Plane & plane);

    //! \brief Revert plane rotation given by matrix transformations
    virtual void revertTransformsOnPlane();

protected:
    osg::ref_ptr< PlaneProjector > m_projector;

    osg::Vec3d m_startProjectedPoint;

    osg::Plane m_initialPlane;

    osg::ref_ptr<osg::Node> m_geometry;
};

} //namespace osgManipulator

#endif // CDRAGGERROTATE_H_included