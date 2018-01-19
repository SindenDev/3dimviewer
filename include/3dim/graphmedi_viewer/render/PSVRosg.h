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

#ifndef PSVRosg_H
#define PSVRosg_H

#include <configure.h>

// Check predefined type of volume rendering algorithm
#ifdef USE_PSVR

///////////////////////////////////////////////////////////////////////////////
// includes

#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/BlendFunc>
#include <osgDB/ReadFile>
#include <osg/Version>

#include <osg/CObjectObserverOSG.h>
#include <data/CActiveDataSet.h>


namespace PSVR
{

///////////////////////////////////////////////////////////////////////////////
// forward declarations
class PSVolumeRendering;


///////////////////////////////////////////////////////////////////////////////
//! OSG drawable implementing volume rendering.
class osgPSVolumeRendering : public osg::Drawable
{
public:
    //! Default constructor.
    osgPSVolumeRendering();

    //! Copy constructor using CopyOp to manage deep vs shallow copy.
    osgPSVolumeRendering(const osgPSVolumeRendering& volume, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

    //! Destructor.
    virtual ~osgPSVolumeRendering();

    META_Object(3DimAdvLib, osgPSVolumeRendering);

    //! Returns pointer to the renderer.
    PSVolumeRendering * getRenderer() { return m_pRenderer; }

    //! Sets pointer to the volume rendering.
    osgPSVolumeRendering& setRenderer(PSVolumeRendering * pRenderer)
    {
        m_pRenderer = pRenderer;
        return *this;
    }

    // the draw immediate mode method is where the OSG wraps up the drawing of
    // of OpenGL primitives.
    virtual void drawImplementation(osg::RenderInfo& ri) const;

    //! Computes the bounding box.
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    virtual osg::BoundingBox computeBoundingBox() const;
#else
    virtual osg::BoundingBox computeBound() const;
#endif

protected:
    //! Pointer to the volume rendering.
    PSVolumeRendering * m_pRenderer;
};


///////////////////////////////////////////////////////////////////////////////
//! OSG geode implementing volume rendering
class osgPSVolumeRenderingGeode : public osg::Geode, public scene::CObjectObserverOSG<data::CActiveDataSet>
{
public:
    //! Default constructor.
    //! - Please, remember to set pointer to the canvas.
    osgPSVolumeRenderingGeode();

    //! Destructor.
    virtual ~osgPSVolumeRenderingGeode();

    //! Method called on OSG update callback.
    virtual void updateFromStorage();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//\class CPSVPlaneUpdateCallback
//
//\brief Plane update callback. 
////////////////////////////////////////////////////////////////////////////////////////////////////
class CPSVPlaneUpdateCallback : public osg::Drawable::UpdateCallback
{
public:
   //! Update operator
   virtual void update(osg::NodeVisitor* nv, osg::Drawable * node);
}; // class CPSVPlaneUpdateCallback 

} // namespace PSVR

#endif // USE_PSVR
#endif // PSVRosg_H
