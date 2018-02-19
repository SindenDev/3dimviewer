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

#ifndef CScreenshot_H
#define CScreenshot_H

#include <osg/Camera>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/FrameBufferObject>

#include <VPL/Module/Signal.h>

namespace osg
{

    ///////////////////////////////////////////////////////////////////////////////
    //! CLASS CScrrenshotCapture - draw callback

    class CScreenshotCapture : public osg::Camera::DrawCallback
    {
    public:
        //! Constructor
        CScreenshotCapture();

        //! Callback operator
        virtual void operator () (osg::RenderInfo& renderInfo) const;

        //! Get image copy
        void getImage(osg::Image * image) const;

    protected:
        //! Image
        osg::ref_ptr<osg::Image> m_image;
        osg::ref_ptr< osg::Texture2D > m_texture;
        osg::ref_ptr<osg::FrameBufferObject> m_fbo;
    };


    ///////////////////////////////////////////////////////////////////////////////
    //! CLASS CCaptureCamera

    class CCaptureCamera : public osg::Camera
    {
    public:
        //! Capturing mode
        enum ECaptureMode
        {
            MODE_DISABLED,	// disable capturing
            MODE_SINGLE,	// capture one frame only
            MODE_CONTINUOUS	// captura continuously
        };

        //! final draw callback - copy image

    public:
        //! Constructor
        CCaptureCamera();

        //! Set capturing mode
        void setMode(ECaptureMode mode);

        // Set masking mode
        void setMask(osg::Node::NodeMask mask);

    protected:
        //! Render buffer
        osg::ref_ptr< osg::Texture2D > m_renderBuffer;

        //! currently used rendering node mask
        osg::Node::NodeMask m_mask;

        //! Single mode on?
        bool m_single;

    };


} // namespace osg

#endif // CScreenshot_H
