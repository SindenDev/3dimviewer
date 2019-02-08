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

#ifndef CDRAGGERTRANSLATE_H_included
#define CDRAGGERTRANSLATE_H_included

#include <draggers/CDraggerBase.h>
#include <osgManipulator/Projector>

namespace osg
{
    class CDraggerTranslateGeometry : public osg::Group
    {
    public:
        CDraggerTranslateGeometry(const osg::Vec3& headRotation);

        void setOffsets(const osg::Vec3& offsetHead, const osg::Vec3& offsetTail);

        void scale(double scaleFactor);

    protected:
        osg::ref_ptr<osg::MatrixTransform> m_matrixHead;
        osg::ref_ptr<osg::MatrixTransform> m_matrixTail;

        osg::Matrix m_defaultMatrixHead;
        osg::Matrix m_defaultMatrixTail;

        osg::Vec3 m_offsetHead;
        osg::Vec3 m_offsetTail;
    };
}


namespace osgManipulator
{

class CDraggerTranslate : public CDraggerBase< Dragger >
{
public:
    // constructors, that create default geometry
    // these are necessary, because of the parameter of CDraggerTranslateGeometry constructor
    CDraggerTranslate();
    CDraggerTranslate(const osg::Vec3& s, const osg::Vec3& e);

    // constructors, that set custom geometry
    CDraggerTranslate(osg::Node* geometry);
    CDraggerTranslate(osg::Node* geometry, const osg::Vec3& s, const osg::Vec3& e);

    virtual bool handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

    osg::Node* getGeometry();

protected:
    osg::ref_ptr<LineProjector> m_projector;
    osg::Vec3d m_startProjectedPoint;

    osg::ref_ptr<osg::Node> m_geometry;
};

} //namespace osgManipulator

#endif // CDRAGGERTRANSLATE_H_included