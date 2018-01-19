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

#include <graph/widgets/CPositionedWindow.h>

///////////////////////////////////////////////////////////////////////////////
//

void scene::CPositionedWindow::setWindowManager(scene::CSizeableWindowManager * wm)
{
//    VPL_ASSERT( wm );
    if( !wm )
    {
        return;
    }

    // Connect to the signal
    m_conSizeChanged = wm->getSigSizeChanged().connect( this, &scene::CPositionedWindow::onWMSizeChanged );	

    // Update window size
    onWMSizeChanged( wm->getWidth(), wm->getHeight() );
}

///////////////////////////////////////////////////////////////////////////////
//

scene::CPositionedWindow::CPositionedWindow(void)
    : m_widthType( NONE )
    , m_heightType( NONE )
    , m_xType( NONE )
    , m_yType( NONE )
    , m_xOriginType( NONE )
    , m_yOriginType( NONE )
    , m_width( 0.0f )
    , m_height( 0.0f )
    , m_x( 0.0f )
    , m_y( 0.0f )
    , m_ox( 0.0f )
    , m_oy( 0.0f )
{	
}

///////////////////////////////////////////////////////////////////////////////
//

void scene::CPositionedWindow::onWMSizeChanged(int wmw, int wmh)
{
    int w(0), h(0), x(0), y(0), ox(0), oy(0);

    // Call window to update its size/origin information
    this->recomputeSizes();

    // Recompute window size
    this->computeSize( wmw, wmh, w, h );	

    // Compute window origin
    this->computeOrigin( wmw, wmh, w, h, ox, oy );

    // Compute new window position
    this->computePosition( wmw, wmh, ox, oy, x, y );

    // Call update
    updateWindowSizePosition( w, h, x, y );
}

///////////////////////////////////////////////////////////////////////////////
//

void scene::CPositionedWindow::computeSize(int wmw, int wmh, int &w, int &h)
{
    // compute new window width
    switch( m_widthType )
    {
    case NONE: 
    case PROPORTIONAL_WINDOW: break;

    case PROPORTIONAL_WM:
	    w = 0.01 * m_width * wmw;
	    break;

    case FIXED:
	    w = m_width;
	    break;
    }

    // Compute new window height
    switch( m_heightType )
    {
    case NONE: 
    case PROPORTIONAL_WINDOW: break;

    case PROPORTIONAL_WM:
	    h = 0.01 * m_height * wmh;
	    break;

    case FIXED:
	    h = m_height;
	    break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//

void scene::CPositionedWindow::computeOrigin(int wmw, int wmh, int ww, int wh, int &ox, int &oy)
{
    // compute new window x origin
    switch( m_xOriginType )
    {
    case NONE: break;
    case PROPORTIONAL_WINDOW: 
	    ox = 0.01 * m_ox * ww;
	    break;

    case PROPORTIONAL_WM:
	    ox = 0.01 * m_ox * wmw;
	    break;

    case FIXED:
	    ox = m_ox;
	    break;
    }

    // Compute new window y origin
    switch( m_yOriginType )
    {
    case NONE: break; 
    case PROPORTIONAL_WINDOW: 
	    oy = 0.01 * m_oy * wh;
	    break;

    case PROPORTIONAL_WM:
	    oy = 0.01 * m_oy * wmh;
	    break;

    case FIXED:
	    ox = m_ox;
	    break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//

void scene::CPositionedWindow::computePosition(int wmw, int wmh, int ox, int oy, int &x, int &y)
{
    // compute new window x position
    switch( m_xType )
    {
    case NONE: break;
    case PROPORTIONAL_WINDOW: 
	    break;

    case PROPORTIONAL_WM:
	    x = 0.01 * m_x * wmw + ox;
	    break;

    case FIXED:
	    x = m_x + ox;
	    break;
    }

    // Compute new window height
    switch( m_yType )
    {
    case NONE: break; 
    case PROPORTIONAL_WINDOW: 
	    break;

    case PROPORTIONAL_WM:
	    y = 0.01 * m_y * wmh + oy;
	    break;

    case FIXED:
	    y = m_y + oy;
	    break;
    }
}
