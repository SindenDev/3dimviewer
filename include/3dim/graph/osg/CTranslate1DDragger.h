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

#ifndef CTranslate1DDragger_H
#define CTranslate1DDragger_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osgManipulator/Dragger>
#include <osgManipulator/Projector>
#include <osg/CTwoMaterialsNode.h>
#include <osg/IHoverDragger.h>


namespace osgManipulator
{

///////////////////////////////////////////////////////////////////////////////
//! Dragger for performing 2D translation.

class CTranslate1DDragger : public osg::CTwoMaterialsNode< Dragger >, public osgManipulator::IHoverDragger
{
public:
    CTranslate1DDragger();

    CTranslate1DDragger(const osg::Vec3& s, const osg::Vec3& e);

    /** Handle pick events on dragger and generate TranslateInLine commands. */
    virtual bool handle(const PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

    /** Setup default geometry for dragger. */
    virtual void setupDefaultGeometry();

    inline void setCheckForNodeInNodePath(bool onOff) { _checkForNodeInNodePath = onOff; }

    void onMouseEnter() override;
    void onMouseLeave() override;

protected:
    //! Revert line rotation given by matrix transformations - used for view aligned draggers
    virtual void revertTransformsOnLine() {}

protected:
    virtual ~CTranslate1DDragger();

    osg::ref_ptr< LineProjector >   _projector;
	       
	osg::Vec3d                       _startProjectedPoint;


    bool                            _checkForNodeInNodePath;
};


} // osgManipulator

#endif // CTranslate1DDragger_H
