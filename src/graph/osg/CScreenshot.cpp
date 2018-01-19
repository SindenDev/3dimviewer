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

/******************************************************************************
	CLASS CScrrenshotCapture
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Constructor
osg::CScreenshotCapture::CScreenshotCapture()
{
	// create image
	m_image = new osg::Image;
	m_enabled = true;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
osg::CScreenshotCapture::~CScreenshotCapture()
{

}

///////////////////////////////////////////////////////////////////////////////
// Callback operator
void osg::CScreenshotCapture::operator () (osg::RenderInfo& renderInfo) const
{
	if( ! m_enabled )
		return;

	// lock capturer
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock( m_lock );

	// get current graphics context
    osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();

	if( ! gc )
		return;

    if (gc->getTraits())
    {
        GLenum pixelFormat;

		// use alpha channel?
        if (gc->getTraits()->alpha)
            pixelFormat = GL_RGBA;
        else 
            pixelFormat = GL_RGB;

		// get image sizes
        int width = gc->getTraits()->width;
        int height = gc->getTraits()->height;
        
        m_image->readPixels(0, 0, width, height, pixelFormat, GL_UNSIGNED_BYTE);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Get image copy
void osg::CScreenshotCapture::getImage( osg::Image * image ) const
{
	if( !image )
		return;

	if (!m_image->valid()) 
		return;

	// lock capturer
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock( m_lock );

	// copy image
	image->copySubImage( 0, 0, 0, m_image.get() );
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
	m_renderBuffer->setInternalFormat(GL_RGBA);
	m_renderBuffer->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	m_renderBuffer->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

	// set camera rendering mode
	setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	// attach buffer
	attach( osg::Camera::COLOR_BUFFER, m_renderBuffer );

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    this->setPostDrawCallback(new UnBindFboPostDrawCallback);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Get image copy
void osg::CCaptureCamera::getImage( osg::Image * image ) const
{
	if( !image )
		return;

	// lock camera
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock( *( this->getDataChangeMutex() ) );

	// copy image
	image->copySubImage( 0, 0, 0, m_renderBuffer->getImage() );
}

///////////////////////////////////////////////////////////////////////////////
// Set capturing mode
void osg::CCaptureCamera::setMode( ECaptureMode mode )
{
	switch( mode )
	{
	case MODE_DISABLED:
		setNodeMask( 0x0 );
		break;

	case MODE_SINGLE:
		setNodeMask( m_mask );
		m_single = true;
		break;

	case MODE_CONTINUOUS:
		setNodeMask( m_mask );
		m_single = false;
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Set masking mode
void osg::CCaptureCamera::setMask( osg::Node::NodeMask mask )
{
	m_mask = getNodeMask();
}

