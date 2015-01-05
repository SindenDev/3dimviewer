///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#include <graph/widgets/CSizeableWindowManager.h>

///////////////////////////////////////////////////////////////////////////////
//

void scene::CVMSizeChangedCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    // Try to get window manager
    CSizeableWindowManager* wm = dynamic_cast< CSizeableWindowManager* >( node );
    if( wm )
    {
        // Call callback
        wm->onSizeChanged();
    }

    traverse( node, nv );
}


///////////////////////////////////////////////////////////////////////////////
//

const unsigned int MASK_2D = 0xFFFFFFFF; //0xF0000000;
scene::CSizeableWindowManager::CSizeableWindowManager(osgViewer::Viewer *View, int initial_width, int initial_height)
    : osgWidget::WindowManager( View, initial_width, initial_height, MASK_2D,
                                osgWidget::WindowManager::WM_USE_RENDERBINS
                                //osgWidget::WindowManager::WM_PICK_DEBUG 
                                )
    , m_view( View )
    , m_width( 0 )
    , m_height( 0 )
{
    // Initialize and set resizing callback
    m_size_callback = new CVMSizeChangedCallback( );
    setUpdateCallback( m_size_callback.get() );
}

///////////////////////////////////////////////////////////////////////////////
//

void scene::CSizeableWindowManager::repositAllWindows(void)
{
    osg::NodeList::iterator node;
    osgWidget::Window * window;
    // For all chldren
    for( node = this->_children.begin(); node != this->_children.end(); ++node )
    {
        // If it is a window
        window = dynamic_cast< osgWidget::Window * >( node->get() );
        if( window != 0 )
        {
            window->update();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//

void scene::CSizeableWindowManager::showWidgets(bool bShow)
{
    osg::NodeList::iterator node;
    osgWidget::Window * window;
    // For all children
    for( node = this->_children.begin(); node != this->_children.end(); ++node )
    {
        // If it si window
        window = dynamic_cast< osgWidget::Window * >( node->get() );
        if( window != 0 )
        {
            if( bShow )
                window->show();
            else
                window->hide();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//

void scene::CSizeableWindowManager::onSizeChanged(void)
{
    if( !m_view )
    {
        return;
    }

    // Get window size
    osg::Viewport * vp( m_view->getCamera()->getViewport() );
    int width = vp->width();
    int height = vp->height();

    if( ! sizeChanged( width, height ) )
    {
        return;
    }

    m_width = width;
    m_height = height;

    setWindowSize( width, height );
    setSize( width, height );

    // Update windows
    resizeAllWindows();
    repositAllWindows();

    // Call signal
    m_sigSizeChanged.invoke( width, height );
}
