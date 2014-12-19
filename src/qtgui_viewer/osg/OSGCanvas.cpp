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

/* NOTE from http://developer.qt.nokia.com/doc/qt-4.8/qglwidget.html
Note that under Windows, the QGLContext belonging to a QGLWidget has to be recreated
when the QGLWidget is reparented. This is necessary due to limitations on the Windows
platform. This will most likely cause problems for users that have subclassed and
installed their own QGLContext on a QGLWidget. It is possible to work around this
issue by putting the QGLWidget inside a dummy widget and then reparenting the dummy
widget, instead of the QGLWidget. This will side-step the issue altogether, and is
what we recommend for users that need this kind of functionality.

On Mac OS X, when Qt is built with Cocoa support, a QGLWidget can't have any sibling
widgets placed ontop of itself. This is due to limitations in the Cocoa API and is not
supported by Apple.
*/
#ifdef __APPLE__
    #include <glew.h>
#else
    #include <GL/glew.h>
#endif

#include "cvolumerendererwindow.h"

//#include <VPL/Base/Logging.h>

#include <base/Macros.h>
#include <osg/OSGCanvas.h>

#include <data/CAppSettings.h>

#include <osg/CAppMode.h>
#include <osg/CSceneManipulator.h>
#include <osg/CSceneOSG.h>
#include <osg/CScreenshot.h>
#include <osg/Version>

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

#include <QMouseEvent>
#include <QApplication>


//#define BASE_GRAPHICS_WINDOW  // for debugging

OSGwxGraphicsWindow::OSGwxGraphicsWindow(Traits *traits, QGLWidget *widget) :
#ifdef USE_OSGQT
    XGraphicsWindow(traits)
#else
    XGraphicsWindow()
#endif
{
    m_glCanvas = widget;
//    m_qglContext = new QGLContext(widget->format());

    // set osgViewer::GraphicsWindow traits
#ifndef USE_OSGQT
    _traits = traits;
#endif

    init();
}

void OSGwxGraphicsWindow::init()
{
    if (valid())
    {
        setState( new osg::State );
        getState()->setGraphicsContext(this);

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
		if (_traits.valid() && _traits->sharedContext.get())
#else
        if (_traits.valid() && _traits->sharedContext)
#endif
        {
            getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( getState()->getContextID() );
        }
        else
        {
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }
    }
}

bool OSGwxGraphicsWindow::makeCurrentImplementation()
{
    if (m_glCanvas && m_glCanvas->isVisible())
    {
        lock();
        m_glCanvas->makeCurrent();
        return true;
    }
    return false;
}

void OSGwxGraphicsWindow::swapBuffersImplementation()
{
    return;
    if (m_glCanvas)
        m_glCanvas->swapBuffers();
}

bool OSGwxGraphicsWindow::releaseContextImplementation()
{
    if (m_glCanvas)
    {
        unlock();
        return true;
    }
    return false;
}

void OSGwxGraphicsWindow::closeImplementation()
{
    if (m_glCanvas)
        m_glCanvas->hide();
}

bool OSGwxGraphicsWindow::realizeImplementation()
{
    if( m_glCanvas )
    {
        m_glCanvas->show();
        return true;
    }
    return false;
}

bool OSGwxGraphicsWindow::isRealizedImplementation() const
{
    return (m_glCanvas) ? m_glCanvas->isVisible() : false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

OSGCanvas::OSGCanvas(QWidget *parent) :
#ifdef USE_OSGQT
    XGLWidget(parent,0,0,true)
#else
    XGLWidget(parent)
#endif
{
    // Sets the widget's clear color
    data::CObjectPtr<data::CAppSettings> settings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
    osg::Vec4 color( settings->getClearColor() );
    init(parent,color);
}

OSGCanvas::OSGCanvas(QWidget *parent, const osg::Vec4 &bgColor):
#ifdef USE_OSGQT
    XGLWidget(parent,0,0,true)
#else
    XGLWidget(parent)
#endif
{
    init(parent,bgColor);
}

void OSGCanvas::init(QWidget *parent, const osg::Vec4 &bgColor)
{
    m_lastRenderingTime = 0;
    m_bRestoreModeOnMouseRelease = false;
    
    QSize size(100,100);
    // creating OSGwxGraphicsWindow object for scene OpenGL display
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(osg::DisplaySettings::instance().get());
    traits->width = size.width();
    traits->height = size.height();
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
#ifdef BASE_GRAPHICS_WINDOW
	m_graphic_window = new osgViewer::GraphicsWindowEmbedded(0, 0, size.width(), size.height());
#else
    m_graphic_window = new OSGwxGraphicsWindow(traits,this);
#endif
    

    // creating osgViewer::Viewer object for scene manipulation controling
    m_view = new osgViewer::Viewer;

    m_view->setRunFrameScheme(osgViewer::ViewerBase::ON_DEMAND);

    // setting necessary viewer attributes
    m_view->getCamera()->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
    m_view->getCamera()->setViewport(0, 0, size.width(), size.height());
    m_view->getCamera()->setProjectionMatrixAsPerspective( 30.0f, static_cast<double>( size.width() )/ static_cast<double>( size.height() ), 1.0f, 10000.0f );
    m_view->getCamera()->setGraphicsContext(m_graphic_window.get());

    // monitor gpu to adjust frame rate
    osg::ref_ptr<osg::Stats> pStats = m_view->getCamera()->getStats();
    pStats = m_view->getCamera()->getStats();
    pStats->collectStats("gpu",true);

#ifdef USE_OSGQT
    this->setGraphicsWindow(m_graphic_window.get());
#endif
    //m_view->addEventHandler(new osgViewer::StatsHandler);
    m_view->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    m_view->setCameraManipulator(new osg::CSceneManipulator);
    m_view->setKeyEventSetsDone(0); // if not called then escape key "breaks" view
    //m_view->setCameraManipulator(new osgGA::TrackballManipulator);

    //osg::ref_ptr<osg::Node> loadedModel;
    //loadedModel = osgDB::readNodeFile("../data/cow.osg");
    //m_view->setSceneData(loadedModel.get());

    // Sets the widget's clear color
    this->setBackgroundColor(bgColor);

    // TODO: fix garbage in background when switching tabs with GLWidgets
    setAttribute(Qt::WA_OpaquePaintEvent );
    //setAutoFillBackground(false);

    // we need to set focus policy to be able to handle keyboard input (Mouse mode modifiers)
    setFocusPolicy(Qt::StrongFocus);
    // set mouse tracking so we can extract continously density from a point under cursor
    setMouseTracking(true);

    m_Connection = APP_MODE.getModeChangedSignal().connect(this, &OSGCanvas::setCursorX);
}

OSGCanvas::~OSGCanvas()
{
    APP_MODE.getModeChangedSignal().disconnect(m_Connection);
#ifdef USE_OSGQT
    setGraphicsWindow(NULL);
#endif
}

void OSGCanvas::initializeGL()
{
    /*
    m_graphic_window->setClearMask( GL_COLOR_BUFFER_BIT |
                                    GL_DEPTH_BUFFER_BIT );
    m_graphic_window->clear();
    m_graphic_window->setClearMask( 0 );
    */
    if (glewInit() != GLEW_OK)
    {
        bool damn= true;
    }
}

void OSGCanvas::resizeGL(int width, int height)
{
    if (m_graphic_window.valid())
    {
        m_graphic_window->resized(m_graphic_window->getTraits()->x, m_graphic_window->getTraits()->y, width, height);
        m_graphic_window->getEventQueue()->windowResize(m_graphic_window->getTraits()->x, m_graphic_window->getTraits()->y, width, height);
    }
}

void OSGCanvas::paintGL()
{
    if (m_view.valid()
            /*&& m_view->checkNeedToDoFrame()*/ // this doesn't work properly with canvases in volume crop
            )
    {
        m_view->frame();
        // get statistics
        double value = 0;
        osg::ref_ptr<osg::Stats> pStats = m_view->getCamera()->getStats();
        int frame = pStats->getLatestFrameNumber();
        // do not take last two frames as these might not be finished yet
        // when gpu is overloaded it might be even more - that's why getAveragedAttribute is better
        if (frame>5)
            pStats->getAveragedAttribute(frame-5, frame-2, "GPU draw time taken",value);
        else if (frame>2)
            pStats->getAttribute(frame-2,"GPU draw time taken",value);
        m_lastRenderingTime = value*1000;
    }
}

void OSGCanvas::glDraw()
{
    QGLWidget::glDraw();
}

void OSGCanvas::setScene(osg::Node * scene)
{
    if( m_view.get() )
    {
        m_view->setSceneData(scene);
    }
}

void OSGCanvas::addEventHandler(osgGA::GUIEventHandler * handler)
{
    if( !handler || !(m_view.get()) ) return;

    m_view->addEventHandler(handler);
}

void OSGCanvas::addEventHandlerFront(osgGA::GUIEventHandler * handler)
{
    if( !handler || !(m_view.get()) ) return;

    m_view->getEventHandlers().push_front(handler);
}

void OSGCanvas::centerAndScale()
{
    m_view->getCameraManipulator()->computeHomePosition();
    m_view->getCameraManipulator()->home(0.0);
    Refresh(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Center and scale. 
//!
//!\param   box The box. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void OSGCanvas::centerAndScale( const osg::BoundingBox & box )
{
    // Try to get camera manipulator
    osg::CSceneManipulator * sm( dynamic_cast< osg::CSceneManipulator * >( m_view->getCameraManipulator() ) );

    if( sm != 0 )
    {
        sm->storeBB( box );
        sm->useStoredBox( true );

        m_view->getCameraManipulator()->home(0.0);
        Refresh(false);
    }
}

void OSGCanvas::Refresh(bool bEraseBackground)
{
    update(); // or repaint for immediate repaint
              // or UpdateGL?
}


void OSGCanvas::setBackgroundColor(const osg::Vec4 &color)
{
    m_view->getCamera()->setClearColor( color );
    //setBackgroundRole(QPalette::Mid);
    //setForegroundRole(QPalette::Mid);
}

void OSGCanvas::restoreMouseMode(bool bForce)
{
    if (bForce || APP_MODE.isTempMode())
    {
        APP_MODE.restore();
        if (0!=(APP_MODE.get() & scene::CAppMode::COMMAND_MODE))
            APP_MODE.setDefault();
        APP_MODE.enableHighlight(true);
    }
    m_bRestoreModeOnMouseRelease = false;
}

void OSGCanvas::keyPressEvent( QKeyEvent* event )
{
    switch (event->key())
    {
    case Qt::Key_Control: 
        restoreMouseMode();
        APP_MODE.storeAndSet( scene::CAppMode::MODE_SLICE_MOVE ); 
        break;
    case Qt::Key_Alt:
        restoreMouseMode();
        APP_MODE.storeAndSet( scene::CAppMode::MODE_DENSITY_WINDOW );         
        break;
    case Qt::Key_Shift:
        if (APP_MODE.areDrawingHandlers())
        {
            restoreMouseMode();
            if (dynamic_cast<CVolumeRendererWindow *>(this) != NULL)
            //if (NULL!=dynamic_cast<scene::CScene3D*>(m_view->getSceneData()))
                APP_MODE.storeAndSet( scene::CAppMode::COMMAND_DRAW_WINDOW ); // surface culling
            else
                APP_MODE.storeAndSet( scene::CAppMode::COMMAND_DRAW_GEOMETRY );            
        }
        break;
    case Qt::Key_Escape:
        restoreMouseMode();
        return;
    default:
        XGLWidget::keyPressEvent(event);
    }
}

bool OSGCanvas::canPostponeMouseModeRestore()
{
    scene::CAppMode::tMode mode = APP_MODE.get();
    if (scene::CAppMode::COMMAND_DRAW_WINDOW==mode ||
        scene::CAppMode::COMMAND_DRAW_GEOMETRY == mode)
        return true;
    return false;
}

void OSGCanvas::keyReleaseEvent( QKeyEvent* event )
{
    if (APP_MODE.isTempMode())
    {
        if (canPostponeMouseModeRestore() && Qt::NoButton!=QApplication::mouseButtons())
        {
            m_bRestoreModeOnMouseRelease = true;
        }
        else
        {
            APP_MODE.restore(); // does nothing when there is no stored mode
            m_bRestoreModeOnMouseRelease = false;
        }
    }            
    XGLWidget::keyReleaseEvent(event);
}

void OSGCanvas::wheelEvent ( QWheelEvent * event )
{
    XGLWidget::wheelEvent(event);
    Refresh(false);
}

void OSGCanvas::mouseReleaseEvent ( QMouseEvent * event )
{
    XGLWidget::mouseReleaseEvent(event);
    if (APP_MODE.isTempMode() && m_bRestoreModeOnMouseRelease)
    {
        APP_MODE.restore(); // does nothing when there is no stored mode
        m_bRestoreModeOnMouseRelease = false;
    }    
}

void OSGCanvas::mouseMoveEvent ( QMouseEvent * event )
{
    XGLWidget::mouseMoveEvent(event);
    if (Qt::NoButton!=event->buttons())
        Refresh(false);
}

void OSGCanvas::enterEvent ( QEvent * event )
{
    XGLWidget::enterEvent(event);
    if (!hasFocus())
        setFocus(Qt::MouseFocusReason);
    // call refresh so the osg items can receive the information too
    Refresh(false);
}

void OSGCanvas::leaveEvent ( QEvent * event )
{
    XGLWidget::leaveEvent(event);
    // call refresh so the osg items can receive the information too
    Refresh(false);
}

void OSGCanvas::setCursorX(int appmode)
{
    switch(appmode)
    {
    case scene::CAppMode::MODE_TRACKBALL:
        setCursor(Qt::ArrowCursor);
        break;
    case scene::CAppMode::MODE_SLICE_MOVE:
        setCursor(Qt::PointingHandCursor);
        break;
    case scene::CAppMode::MODE_DENSITY_WINDOW:
        setCursor(Qt::SizeAllCursor);
        break;
    case scene::CAppMode::COMMAND_SCENE_ZOOM:
        {
            // TODO: add better magnifier cursor
            QCursor magCursor(QPixmap(":/icons/magnifier.png"));
            setCursor(magCursor);
        }
        break;
    case scene::CAppMode::COMMAND_SCENE_HIT:
    case scene::CAppMode::COMMAND_ADD_IMPLANT:
    case scene::CAppMode::COMMAND_REMOVE_IMPLANT:
    case scene::CAppMode::COMMAND_REPLACE_IMPLANT:
    case scene::CAppMode::COMMAND_IMPLANT_INFO:
    case scene::CAppMode::COMMAND_DENSITY_MEASURE:
    case scene::CAppMode::COMMAND_DISTANCE_MEASURE:
    case scene::CAppMode::COMMAND_DRAW_WINDOW:
    case scene::CAppMode::COMMAND_DRAW_GEOMETRY:
        setCursor(Qt::CrossCursor); // cross instead of pencil
        break;
    default:
        setCursor(QCursor());
        break;
    }
}

void OSGCanvas::enableMultiThreaded()
{
    if (!m_view) return;
#ifdef BASE_GRAPHICS_WINDOW
	return;
#endif
    m_view->setThreadingModel(osgViewer::Viewer::CullDrawThreadPerContext);
    m_view->setThreadSafeReferenceCounting(true); // sets the default but doesn't enable it for the object (static function)
    m_view->setThreadSafeRefUnref(true); // enables thread safety for the object
    m_view->setEndBarrierPosition(osgViewer::ViewerBase::AfterSwapBuffers);
}

////////////////////////////////////////////////////////////
//

void OSGCanvas::screenShot( osg::Image * img, unsigned int scalePercent, bool bWantWidgets )
{
    osg::ref_ptr< osg::Image > p_Image = img;
    osg::ref_ptr< osg::Camera > p_OrigCam = m_view->getCamera();
    osg::ref_ptr< osg::Viewport > p_OrigView = p_OrigCam->getViewport();

    osg::Matrix m = m_view->getCameraManipulator()->getMatrix();

//    double ratio = p_OrigView->width() / (double) p_OrigView->height();

    int w, h;
    w = p_OrigView->width();
    h = p_OrigView->height();
#define max(x,y) (x)>(y)?(x):(y)
    w = max(1,(int)(w*scalePercent/100.0));
    h = max(1,(int)(h*scalePercent/100.0));
    /*
    if ( maxSide == 0)
    {
        w = p_OrigView->width();
        h = p_OrigView->height();
    }
    else
    {
        if (p_OrigView->width()>=p_OrigView->height())
        {
            w = maxSide;
            h = maxSide / ratio;
        }
        else
        {
            w = maxSide * ratio;
            h = maxSide;
        }
    }*/


    if(bWantWidgets)
    {
        // resize canvas to desired size
        QSize sizeBackup=this->size();
        if (100!=scalePercent)
            resize(w,h);

        // create a new "screenshot" HUD camera
        osg::ref_ptr< osg::CCaptureCamera > p_Camera = new osg::CCaptureCamera();
        osg::ref_ptr< osg::Node >   scene = m_view->getSceneData();

        // setup camera
        p_Camera->setProjectionMatrixAsOrtho2D(0,w,0,h);
        p_Camera->setViewMatrix(osg::Matrix::identity());
        p_Camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
        p_Camera->setViewport( 0, 0, w, h );
        p_Camera->setRenderOrder(osg::Camera::POST_RENDER);
        p_Camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        p_Camera->setAllowEventFocus(false);

        // setup post draw callback for screenshot capture
        osg::ref_ptr< osg::CScreenshotCapture > p_Capture = new osg::CScreenshotCapture();
        p_Capture->enable(true);
        p_Camera->setPostDrawCallback(p_Capture.get());

        // set camera as scene child
        scene->asGroup()->addChild(p_Camera);
        p_Camera->setMode(osg::CCaptureCamera::MODE_SINGLE);

        // redraw the scene
        m_view->frame();

        // get screenshot
        p_Capture->getImage(img);

        // remove camera from the scene
        scene->asGroup()->removeChild(p_Camera);

        // resize the scene back to its original size
        if (100!=scalePercent)
            resize(sizeBackup);
        return;
    }

    // allocate image
    p_Image->allocateImage( w, h, 24, GL_RGB, GL_UNSIGNED_BYTE );
    p_Image->setPixelFormat( GL_RGB );

    // create a new camera and attach the image as color buffer
    osg::ref_ptr< osg::Camera > p_Cam = new osg::Camera();
    p_Cam->setProjectionMatrix( p_OrigCam->getProjectionMatrix() );
    p_Cam->setViewMatrix( p_OrigCam->getViewMatrix() );
    p_Cam->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    p_Cam->setViewport( 0, 0, w, h );
    p_Cam->setRenderOrder( osg::Camera::PRE_RENDER );
    p_Cam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    p_Cam->attach( osg::Camera::COLOR_BUFFER, p_Image.get() );

    // set new custom camera to view
    osg::ref_ptr< osg::Node >   scene = m_view->getSceneData();
    p_Cam->addChild( scene.get() );
    m_view->setSceneData( p_Cam.get() );

    // redraw scene
    m_view->frame();

    p_Cam->setViewport( p_OrigView.get() );

    // restore settings and redraw scene
    m_view->setSceneData( scene.get() );
    m_view->getCameraManipulator()->setByMatrix( m );
    m_view->frame();
}

////////////////////////////////////////////////////////////
// adopted from scene::CBDRulerWidget
void OSGCanvas::getDistances(float & dx, float & dy)
{
    osg::ref_ptr< osg::Camera > p_OrigCam = m_view->getCamera();
    osg::ref_ptr< osg::Viewport > viewport = p_OrigCam->getViewport();

    // Compute transformation matrix
    osg::Matrixd ipm, pm( m_view->getCamera()->getProjectionMatrix() );
    ipm.invert(pm);

    osg::Matrixd iwm, wm( viewport->computeWindowMatrix() );
    iwm.invert(wm);

    /*
        This is really ugly and dirty hack. Update traversal is called BEFORE
        the viewer manipulator updates the camera view matrix. So if we use
        current view matrix, we are using one step backward matrix in fact.
        This hack gets around this harsh reality.
    */
    osg::Matrixd ivm, vm( m_view->getCameraManipulator()->getMatrix() );
    ivm.invert(vm);

    osg::Matrixd matrix = iwm*ipm*ivm;

    // Compute transformations of the unit vectors
    osg::Vec3 vdx( 1.0, 0.0, 0.5 );
    osg::Vec3 vdy( 0.0, 1.0, 0.5 );
    osg::Vec3 vo( 0.0, 0.0, 0.5 );

    vdx = vdx * matrix;
    vdy = vdy * matrix;
    vo = vo * matrix;

    vdx = vdx - vo;
    vdy = vdy - vo;

    dx = vdx.length();
    dy = vdy.length();
}
