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

#include <osg/CScreenshot.h>
#include <osg/RenderInfo>
#include <osgQt/QtOsg.h>
#include <VPL/Base/Logging.h>
#include <render/glErrorReporting.h>

/******************************************************************************
    CLASS CScrrenshotCapture
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Constructor
osg::CScreenshotCapture::CScreenshotCapture()
    : m_image(new osg::Image())
    , m_texture(new osg::Texture2D())
    , m_fbo(new osg::FrameBufferObject())
{
    m_texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    m_texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
}


///////////////////////////////////////////////////////////////////////////////
// Callback operator
void osg::CScreenshotCapture::operator () (osg::RenderInfo& renderInfo) const
{
    osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();

    int width = gc->getTraits()->width;
    int height = gc->getTraits()->height;

    m_texture->setTextureSize(width, height);
    m_texture->setInternalFormat(gc->getTraits()->alpha ? GL_RGBA : GL_RGB);
    m_texture->dirtyTextureObject();
    m_texture->dirtyTextureParameters();

    m_fbo->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment(m_texture));

    auto stateSet = new osg::StateSet();
    renderInfo.getState()->captureCurrentState(*stateSet);

    m_fbo->apply(*renderInfo.getState(), osg::FrameBufferObject::DRAW_FRAMEBUFFER);

    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    GLenum error = glGetError();
    if (error!=GL_NO_ERROR)
    {
        VPL_LOG_ERROR("Screenshot blit error " << glErrorEnumString(error));
    }
    renderInfo.getState()->applyTextureAttribute(0, m_texture);
    
#ifdef __APPLE__ // has different format than on windows and readImageFromCurrentTexture can't handle it
    // khronos.org/opengl/wiki/GL_EXT_framebuffer_multisample
    m_fbo->apply(*renderInfo.getState(), osg::FrameBufferObject::READ_DRAW_FRAMEBUFFER);
    m_image->readPixels(0,0,width,height,m_texture->getInternalFormat(),GL_UNSIGNED_BYTE);
#else
    m_image->readImageFromCurrentTexture(renderInfo.getContextID(), false);
#endif
    renderInfo.getState()->haveAppliedAttribute(m_fbo);
    renderInfo.getState()->apply(stateSet);
}

///////////////////////////////////////////////////////////////////////////////
// Get image copy
void osg::CScreenshotCapture::getImage(osg::Image * image) const
{
    image->copySubImage(0, 0, 0, m_image);
}

/******************************************************************************
    CLASS CCaptureCamera
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Constructor
osg::CCaptureCamera::CCaptureCamera()
{
    m_mask = (osg::Node::NodeMask)(-1);

    // create render buffer
    m_renderBuffer = new osg::Texture2D;

    // set texture parameters
    m_renderBuffer->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
    m_renderBuffer->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
    m_renderBuffer->setInternalFormat(GL_RGBA);

    // set camera rendering mode
    setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

    // attach buffer
    attach(osg::Camera::COLOR_BUFFER0, m_renderBuffer);

    setPostDrawCallback(new UnBindFboPostDrawCallback);
}

///////////////////////////////////////////////////////////////////////////////
// Set capturing mode
void osg::CCaptureCamera::setMode(ECaptureMode mode)
{
    switch (mode)
    {
    case MODE_DISABLED:
        setNodeMask(0x0);
        break;

    case MODE_SINGLE:
        setNodeMask(m_mask);
        m_single = true;
        break;

    case MODE_CONTINUOUS:
        setNodeMask(m_mask);
        m_single = false;
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set masking mode
void osg::CCaptureCamera::setMask(osg::Node::NodeMask mask)
{
    m_mask = getNodeMask();
}

