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

#ifndef CSceneVolumeRendering_H
#define CSceneVolumeRendering_H

#include <osg/CSceneOSG.h>

// Clipping support
//#include <osg/ClipNode>

////////////////////////////////////////////////////////////////////////////////////////////////////
// forward declarations
namespace PSVR { class PSVolumeRendering; }

namespace scene
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//! Scene volume rendering - base class
    class CSceneVolumeRendering : public CSceneBase
{
public:
    //! Constructor
    CSceneVolumeRendering(OSGCanvas *pCanvas);

    //! Sets the renderer.
    void setRenderer(PSVR::PSVolumeRendering *pRenderer);

    //! Enable/disable renderer
    void enableRenderer(bool enable);

    osg::Geode *getVRGeode();

protected:
    // Update scene from the storage
    virtual void updateFromStorage();

protected:
    //! Renderer scene group 
    osg::ref_ptr<osg::MatrixTransform> m_renderedGroup;

    //! Drawable...
    osg::ref_ptr<osg::Drawable> m_vrDrawable;

    //! Geode...
    osg::ref_ptr<osg::Geode> m_vrGeode;

    //! Rendered group state set
    osg::ref_ptr<osg::StateSet> m_renderedSS;

    //! Clipping node
    //osg::ref_ptr<osg::ClipNode> m_clipNode;

    //! Clipping plane
    //osg::ref_ptr<osg::ClipPlane> m_clipPlane;
};

} // namespace scene

#endif // CSceneVolumeRendering_H
