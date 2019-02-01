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

#ifndef CDRAGGERBASE_H_included
#define CDRAGGERBASE_H_included

#include <osg/CTwoMaterialsNode.h>
#include <osg/IHoverDragger.h>
#include <osgManipulator/Dragger>


namespace osgManipulator
{

template<class T>
class CDraggerBase : public osg::CTwoMaterialsNode< T >, public osgManipulator::IHoverDragger
{
public:
    CDraggerBase() {}

    virtual bool handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;

    virtual void updateCommand(osgManipulator::MotionCommand& command) {};

    void onMouseEnter() override
    {
        osg::CTwoMaterialsNode< T >::applyMaterial(osg::SECOND);
    }

    void onMouseLeave() override
    {
        osg::CTwoMaterialsNode< T >::applyMaterial(osg::FIRST);
    }

    void setColor(osg::MaterialNumber num, const osg::Vec3& color)
    {
        osg::CTwoMaterialsNode< T >::setDiffuse(num, color);
    }

    void setColors(const osg::Vec3& firstColor, const osg::Vec3& secondColor)
    {
        osg::CTwoMaterialsNode< T >::setDiffuse(osg::FIRST, firstColor);
        osg::CTwoMaterialsNode< T >::setDiffuse(osg::SECOND, secondColor);
    }
};

} //namespace osgManipulator

#endif // CDRAGGERBASE_H_included
