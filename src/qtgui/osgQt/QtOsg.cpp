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

#include <glad/glad.h>
// needed for mac:
#ifndef GL_GLEXT_FUNCTION_POINTERS
  #define GL_GLEXT_FUNCTION_POINTERS
#endif

#include "render/cvolumerendererwindow.h"
#include <render/glErrorReporting.h>

//#include <VPL/Base/Logging.h>

#include <base/Macros.h>

#include <osg/CSceneManipulator.h>
#include <osg/CSceneOSG.h>
#include <osg/CScreenshot.h>
#include <osg/Version>

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Renderer>

#include <QMouseEvent>
#include <QApplication>
#include <QThread>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QOpenGLContext>

#include <osgQt/QtOsg.h>

//#define DEBUG_CONTEXT_SWITCH

///////////////////////////////////////////////////////////////////////////////
// state which is able to save default frame buffer, because osg doesn't restore it ok
// see http://forum.openscenegraph.org/viewtopic.php?t=15097

class StateEx : public osg::State
{
public:
    //! Constructor
    StateEx() : defaultFbo(0)
    {
    }

    void setDefaultFbo(GLuint fbo)
    {
        defaultFbo = fbo;
    }
    GLuint getDefaultFbo() const
    {
        return defaultFbo;
    }

protected:
    GLuint defaultFbo;
};

void UnBindFboPostDrawCallback::operator () (osg::RenderInfo& renderInfo) const
{
    tridimGlR("glBindFramebuffer", glBindFramebuffer(GL_FRAMEBUFFER, ((StateEx*)(renderInfo.getState()))->getDefaultFbo()));
}

///////////////////////////////////////////////////////////////////////////////
// High DPI handling helper

int getDpiFactor(QGLOSGWidget* pWidget)
{
    // enable hi dpi support
    // https://vicrucann.github.io/tutorials/osg-qt-high-dpi/
    if (QApplication::testAttribute(Qt::AA_EnableHighDpiScaling))
    {
        return pWidget->devicePixelRatio();
        int ratio = QApplication::desktop()->devicePixelRatio();
        int sn = QApplication::desktop()->screenNumber(pWidget);
        QWidget* pScreen = qobject_cast<QWidget*>(QApplication::desktop()->screen(sn));
        //qDebug() << "sn " << sn << " p " << pScreen;
        if (NULL != pScreen)
        {
            ratio = pScreen->devicePixelRatio();
            //qDebug() << "r " << ratio;
            return ratio;
        }
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// mapping of special keys

class QtKeyboardMap
{

public:
    QtKeyboardMap()
    {
        mKeyMap[Qt::Key_Escape     ] = osgGA::GUIEventAdapter::KEY_Escape;
        mKeyMap[Qt::Key_Delete   ] = osgGA::GUIEventAdapter::KEY_Delete;
        mKeyMap[Qt::Key_Home       ] = osgGA::GUIEventAdapter::KEY_Home;
        mKeyMap[Qt::Key_Enter      ] = osgGA::GUIEventAdapter::KEY_KP_Enter;
        mKeyMap[Qt::Key_End        ] = osgGA::GUIEventAdapter::KEY_End;
        mKeyMap[Qt::Key_Return     ] = osgGA::GUIEventAdapter::KEY_Return;
        mKeyMap[Qt::Key_PageUp     ] = osgGA::GUIEventAdapter::KEY_Page_Up;
        mKeyMap[Qt::Key_PageDown   ] = osgGA::GUIEventAdapter::KEY_Page_Down;
        mKeyMap[Qt::Key_Left       ] = osgGA::GUIEventAdapter::KEY_Left;
        mKeyMap[Qt::Key_Right      ] = osgGA::GUIEventAdapter::KEY_Right;
        mKeyMap[Qt::Key_Up         ] = osgGA::GUIEventAdapter::KEY_Up;
        mKeyMap[Qt::Key_Down       ] = osgGA::GUIEventAdapter::KEY_Down;
        mKeyMap[Qt::Key_Backspace  ] = osgGA::GUIEventAdapter::KEY_BackSpace;
        mKeyMap[Qt::Key_Tab        ] = osgGA::GUIEventAdapter::KEY_Tab;
        mKeyMap[Qt::Key_Space      ] = osgGA::GUIEventAdapter::KEY_Space;
        mKeyMap[Qt::Key_Delete     ] = osgGA::GUIEventAdapter::KEY_Delete;
        mKeyMap[Qt::Key_Alt      ] = osgGA::GUIEventAdapter::KEY_Alt_L;
        mKeyMap[Qt::Key_Shift    ] = osgGA::GUIEventAdapter::KEY_Shift_L;
        mKeyMap[Qt::Key_Control  ] = osgGA::GUIEventAdapter::KEY_Control_L;
        mKeyMap[Qt::Key_Meta     ] = osgGA::GUIEventAdapter::KEY_Meta_L;

        mKeyMap[Qt::Key_F1             ] = osgGA::GUIEventAdapter::KEY_F1;
        mKeyMap[Qt::Key_F2             ] = osgGA::GUIEventAdapter::KEY_F2;
        mKeyMap[Qt::Key_F3             ] = osgGA::GUIEventAdapter::KEY_F3;
        mKeyMap[Qt::Key_F4             ] = osgGA::GUIEventAdapter::KEY_F4;
        mKeyMap[Qt::Key_F5             ] = osgGA::GUIEventAdapter::KEY_F5;
        mKeyMap[Qt::Key_F6             ] = osgGA::GUIEventAdapter::KEY_F6;
        mKeyMap[Qt::Key_F7             ] = osgGA::GUIEventAdapter::KEY_F7;
        mKeyMap[Qt::Key_F8             ] = osgGA::GUIEventAdapter::KEY_F8;
        mKeyMap[Qt::Key_F9             ] = osgGA::GUIEventAdapter::KEY_F9;
        mKeyMap[Qt::Key_F10            ] = osgGA::GUIEventAdapter::KEY_F10;
        mKeyMap[Qt::Key_F11            ] = osgGA::GUIEventAdapter::KEY_F11;
        mKeyMap[Qt::Key_F12            ] = osgGA::GUIEventAdapter::KEY_F12;
        mKeyMap[Qt::Key_F13            ] = osgGA::GUIEventAdapter::KEY_F13;
        mKeyMap[Qt::Key_F14            ] = osgGA::GUIEventAdapter::KEY_F14;
        mKeyMap[Qt::Key_F15            ] = osgGA::GUIEventAdapter::KEY_F15;
        mKeyMap[Qt::Key_F16            ] = osgGA::GUIEventAdapter::KEY_F16;
        mKeyMap[Qt::Key_F17            ] = osgGA::GUIEventAdapter::KEY_F17;
        mKeyMap[Qt::Key_F18            ] = osgGA::GUIEventAdapter::KEY_F18;
        mKeyMap[Qt::Key_F19            ] = osgGA::GUIEventAdapter::KEY_F19;
        mKeyMap[Qt::Key_F20            ] = osgGA::GUIEventAdapter::KEY_F20;

        mKeyMap[Qt::Key_hyphen         ] = '-';
        mKeyMap[Qt::Key_Equal         ] = '=';

        mKeyMap[Qt::Key_division      ] = osgGA::GUIEventAdapter::KEY_KP_Divide;
        mKeyMap[Qt::Key_multiply      ] = osgGA::GUIEventAdapter::KEY_KP_Multiply;
        mKeyMap[Qt::Key_Minus         ] = '-';
        mKeyMap[Qt::Key_Plus          ] = '+';
        //mKeyMap[Qt::Key_H              ] = osgGA::GUIEventAdapter::KEY_KP_Home;
        //mKeyMap[Qt::Key_                    ] = osgGA::GUIEventAdapter::KEY_KP_Up;
        //mKeyMap[92                    ] = osgGA::GUIEventAdapter::KEY_KP_Page_Up;
        //mKeyMap[86                    ] = osgGA::GUIEventAdapter::KEY_KP_Left;
        //mKeyMap[87                    ] = osgGA::GUIEventAdapter::KEY_KP_Begin;
        //mKeyMap[88                    ] = osgGA::GUIEventAdapter::KEY_KP_Right;
        //mKeyMap[83                    ] = osgGA::GUIEventAdapter::KEY_KP_End;
        //mKeyMap[84                    ] = osgGA::GUIEventAdapter::KEY_KP_Down;
        //mKeyMap[85                    ] = osgGA::GUIEventAdapter::KEY_KP_Page_Down;
        mKeyMap[Qt::Key_Insert        ] = osgGA::GUIEventAdapter::KEY_KP_Insert;
        //mKeyMap[Qt::Key_Delete        ] = osgGA::GUIEventAdapter::KEY_KP_Delete;
    }

    ~QtKeyboardMap()
    {
    }

    int remapKey(QKeyEvent* event)
    {
        KeyMap::iterator itr = mKeyMap.find(event->key());
        if (itr == mKeyMap.end())
        {
#if QT_VERSION < 0x050000
            return int(*(event->text().toAscii().data()));
#else
            return int(*(event->text().toLatin1().data()));
#endif
        }
        else
            return itr->second;
    }

    private:
    typedef std::map<unsigned int, int> KeyMap;
    KeyMap mKeyMap;
};

static QtKeyboardMap s_QtKeyboardMap;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// source code taken from osg 3.3.2
// modifications based on http://forum.openscenegraph.org/viewtopic.php?t=15097
// and on http://polar.inria.fr/downloadfaq/download/

// Custom FrameBufferObject set via camera default stateset, restores fbo to the one used by QOpenGLWidget when necessary
class CExtraFBO : public osg::FrameBufferObject
{
public:
    virtual void apply(osg::State &state) const
    {
        apply(state, READ_DRAW_FRAMEBUFFER);
    }
    virtual void apply(osg::State &state, BindTarget target) const
    {
        if (getAttachmentMap().empty())
        {
            tridimGlR("glBindFramebuffer", glBindFramebuffer(GL_FRAMEBUFFER, ((StateEx*)(&state))->getDefaultFbo()));
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
// Graphics window for single threaded use

QtOSGGraphicsWindow::QtOSGGraphicsWindow(Traits *traits, QOpenGLWidget *widget) :
    osgViewer::GraphicsWindowEmbedded(traits)
{
    m_glCanvas = widget;
    m_bContextX = false;
    // set osgViewer::GraphicsWindow traits
    _traits = traits;    
    init();
}

QtOSGGraphicsWindow::~QtOSGGraphicsWindow()
{
    close();
}

void QtOSGGraphicsWindow::init()
{
    if (valid())
    {
        setState( new StateEx);
        getState()->setGraphicsContext(this);

		if (_traits.valid() && _traits->sharedContext.get())
        {
            getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
            incrementContextIDUsageCount( getState()->getContextID() );
        }
        else
        {
            getState()->setContextID( osg::GraphicsContext::createNewContextID() );
        }

        getEventQueue()->syncWindowRectangleWithGraphicsContext();
    }
}

// dummy implementations, assume that graphics context is *always* current and valid.
bool QtOSGGraphicsWindow::valid() const 
{ 
    return true; 
}

bool QtOSGGraphicsWindow::realizeImplementation() 
{ 
    return true; 
}

bool QtOSGGraphicsWindow::isRealizedImplementation() const  
{ 
    return true; 
}

void QtOSGGraphicsWindow::closeImplementation() 
{
}

bool QtOSGGraphicsWindow::makeCurrentImplementation() 
{ 
#ifdef DEBUG_CONTEXT_SWITCH
    if (dynamic_cast<CVolumeRendererWindow *>(m_glCanvas) != NULL)
        qDebug() << "makeCurrent " << QThread::currentThreadId() << " " << wglGetCurrentContext();
#endif
    if (m_glCanvas->context()!=QOpenGLContext::currentContext())
    {
        m_glCanvas->makeCurrent();
        m_bContextX = true;
        qDebug() << "QtOSGGraphicsWindow::makeCurrentImplementation: fixing bad context";
    }

    return true; 
}

bool QtOSGGraphicsWindow::releaseContextImplementation()
{ 
#ifdef DEBUG_CONTEXT_SWITCH
    if (dynamic_cast<CVolumeRendererWindow *>(m_glCanvas) != NULL)
        qDebug() << "doneCurrent " << QThread::currentThreadId();
#endif
    if (m_bContextX)
        m_glCanvas->doneCurrent();

    m_bContextX = false;
    return true; 
}

void QtOSGGraphicsWindow::swapBuffersImplementation() 
{
}

void QtOSGGraphicsWindow::grabFocus() 
{
}

void QtOSGGraphicsWindow::grabFocusIfPointerInWindow() 
{
}

void QtOSGGraphicsWindow::raiseWindow() 
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics window for multithreaded opengl with known issues on AMD cards

QtOSGGraphicsWindowMT::QtOSGGraphicsWindowMT(Traits *traits, QOpenGLWidget *widget) :
	osgViewer::GraphicsWindowEmbedded(traits)
{
    m_glCanvas = widget;
    // set osgViewer::GraphicsWindow traits
    _traits = traits;
    init();
}

QtOSGGraphicsWindowMT::~QtOSGGraphicsWindowMT()
{
}

void QtOSGGraphicsWindowMT::init()
{
    if (valid())
    {
        setState( new StateEx );
        getState()->setGraphicsContext(this);

		if (_traits.valid() && _traits->sharedContext.get())
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

bool QtOSGGraphicsWindowMT::valid() const 
{ 
    if (m_glCanvas && m_glCanvas->isValid())
        return true;
    return false; 
}

bool QtOSGGraphicsWindowMT::makeCurrentImplementation()
{
#ifdef DEBUG_CONTEXT_SWITCH
    if (dynamic_cast<CVolumeRendererWindow *>(m_glCanvas) != NULL)
        qDebug() << "makeCurrent " << QThread::currentThreadId() << " " << wglGetCurrentContext();
#endif
    if (m_glCanvas && m_glCanvas->isVisible())
    {
        lock();
        m_glCanvas->makeCurrent();
        return true;
    }
    return false;
}

void QtOSGGraphicsWindowMT::swapBuffersImplementation()
{
}

bool QtOSGGraphicsWindowMT::releaseContextImplementation()
{
#ifdef DEBUG_CONTEXT_SWITCH
    if (dynamic_cast<CVolumeRendererWindow *>(m_glCanvas) != NULL)
        qDebug() << "doneCurrent " << QThread::currentThreadId();
#endif
    if (m_glCanvas)
    {
        m_glCanvas->doneCurrent();
        unlock();
        return true;
    }
    return false;
}

void QtOSGGraphicsWindowMT::closeImplementation()
{
    if (m_glCanvas)
        m_glCanvas->hide();
}

bool QtOSGGraphicsWindowMT::realizeImplementation()
{
    if( m_glCanvas )
    {
        m_glCanvas->show();
        return true;
    }
    return false;
}

bool QtOSGGraphicsWindowMT::isRealizedImplementation() const
{
    return (m_glCanvas) ? m_glCanvas->isVisible() : false;
}

///////////////////////////////////////////////////////////////////////////////
// Viewer implementation

void QtOSGViewer::setUpThreading()
{
    // override default implementation which sets thread affinity
    // osgViewer::Viewer::setUpThreading();
    Contexts contexts;
    getContexts(contexts);
    if(_threadingModel == SingleThreaded)
    {
        if(_threadsRunning)
            stopThreading();
    }
    else 
    {
        if(!_threadsRunning)
            startThreading();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Qt-OSG GLWidget implementation

QGLOSGWidget::QGLOSGWidget(QWidget *parent) : QGLOSGWidget(parent, osg::Vec4(0.0, 0.0, 0.0, 0.0))
{

}

QGLOSGWidget::QGLOSGWidget(QWidget *parent, const osg::Vec4& bgColor)
    : QOpenGLWidget(parent)
    , m_lastRenderingTime(0)
    , m_bInitialized(false)
{
    init(parent,bgColor);
}

void QGLOSGWidget::init(QWidget *parent, const osg::Vec4 &bgColor)
{    
    QSize size(100,100);
    // creating graphics window object for scene OpenGL display
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(osg::DisplaySettings::instance());
    traits->width = size.width();
    traits->height = size.height();
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
//	m_graphic_window = new osgViewer::GraphicsWindowEmbedded(0, 0, size.width(), size.height());
    m_graphic_window = new QtOSGGraphicsWindow(traits,this);
    //m_graphic_window = new QtOSGGraphicsWindowMT(traits,this);

    // creating osgViewer::Viewer object for scene manipulation controling
    m_view = new QtOSGViewer;
    m_view->setRunFrameScheme(osgViewer::ViewerBase::ON_DEMAND);
    //m_view->setReleaseContextAtEndOfFrameHint(false); // doesn't work with screenshots - causes driver crash

    // setting necessary viewer attributes
    m_view->getCamera()->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
    m_view->getCamera()->setViewport(0, 0, size.width(), size.height());
    m_view->getCamera()->setProjectionMatrixAsPerspective( 30.0f, static_cast<double>( size.width() )/ static_cast<double>( size.height() ), 1.0f, 10000.0f );
    m_view->getCamera()->setGraphicsContext(m_graphic_window.get());

    // to correctly restore fbo after osg::FrameBufferObject was set via stateset change during draw we change default camera stateset to a new one with our customized 
    // fbo which restores fbo to the one used by QOpenGLWidget
    m_view->getCamera()->getOrCreateStateSet()->setGlobalDefaults();
    m_view->getCamera()->getOrCreateStateSet()->setAttributeAndModes(new CExtraFBO);

    // monitor gpu to adjust frame rate
    osg::ref_ptr<osg::Stats> pStats = m_view->getCamera()->getStats();
    pStats = m_view->getCamera()->getStats();    
    pStats->collectStats("gpu",true);

    //m_view->addEventHandler(new osgViewer::StatsHandler);
    m_view->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    m_view->setCameraManipulator(new osgGA::TrackballManipulator); // should be osg::SceneManipulator, but because implementation differs between apps, their osgcanvas handles it
    m_view->setKeyEventSetsDone(0); // if not called then escape key "breaks" view    

    // Sets the widget's clear color
    setBackgroundColor(bgColor);

    // TODO: fix garbage in background when switching tabs with GLWidgets
    setAttribute(Qt::WA_OpaquePaintEvent );
    //setAutoFillBackground(false);

    // we need to set focus policy to be able to handle keyboard input (Mouse mode modifiers)
    setFocusPolicy(Qt::StrongFocus);
    // set mouse tracking so we can extract continously density from a point under cursor
    setMouseTracking(true);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();

    int samples = osg::DisplaySettings::instance()->getNumMultiSamples();
    bool debugContext = osg::DisplaySettings::instance()->getGLContextFlags() & GL_CONTEXT_FLAG_DEBUG_BIT;

    format.setSamples(samples);
    format.setOption(QSurfaceFormat::DebugContext, debugContext);

    setFormat(format);

    if (samples > 0)
    {
        m_view->getCamera()->getOrCreateStateSet()->setMode(GL_MULTISAMPLE, osg::StateAttribute::ON);
    }

    if (debugContext)
    {
#ifdef _WIN32
        osg::setNotifyHandler(new osg::WinDebugNotifyHandler());
#endif
        osg::setNotifyLevel(osg::WARN);
    }
}

QGLOSGWidget::~QGLOSGWidget()
{
    VPL_LOG_INFO("Destructing GL widget " << this->objectName().toStdString());
    // disconnect this signal now because it would be called later on a half deleted object
    if (nullptr!= context())
        disconnect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &QGLOSGWidget::contextCleanup);

    // disabled as it has problems with context on mac, examine later
    //if (m_graphic_window.get())
    //    m_graphic_window->close();
}

void APIENTRY openglCallbackFunction(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    std::stringstream sstream;

    sstream << "OpenGL DEBUG: ";

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        sstream << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        sstream << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        sstream << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        sstream << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        sstream << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_MARKER:
        sstream << "MARKER";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        sstream << "PUSH_GROUP";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        sstream << "POP_GROUP";
        break;
    case GL_DEBUG_TYPE_OTHER:
        sstream << "OTHER";
        break;
    }

    sstream << " " << "id: " << id << " " << "severity ";

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_LOW:
        sstream << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        sstream << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        sstream << "HIGH";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        sstream << "NOTIFICATION";
        break;
    }

    sstream << ": " << message;

    const auto outputStr = sstream.str();   

#ifdef _WIN32
    OutputDebugStringA((sstream.str() + '\n').c_str());
#endif

    VPL_LOG_INFO(outputStr);
}

void QGLOSGWidget::initializeGL()
{
    Q_ASSERT(!m_bInitialized); // reinitialization can occur when reparenting opengl window - see QOpenGLWidget documentation
    if (!m_bInitialized)
    {
        VPL_LOG_INFO("Initializing GL window " << this->objectName().toStdString());
        m_bInitialized = true;

        if (!gladLoadGL())
        {
            m_bInitialized = false;
        }

        if (isDebugMessageCallbackAvailable() && isDebugContextEnabled())
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#ifndef __APPLE__
            glDebugMessageCallback(openglCallbackFunction, nullptr);

            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
#endif
        }
    }
    else
    {
        VPL_LOG_INFO("initializeGL called for second time, application is about to crash soon " << this->objectName().toStdString());
    }
    // context() and QOpenGLContext::currentContext() are equivalent when called from initializeGL or paintGL
    if (nullptr!= context())
        connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &QGLOSGWidget::contextCleanup);
}

void QGLOSGWidget::contextCleanup()
{
    VPL_LOG_INFO("Context about to be destroyed " << this->objectName().toStdString());
    makeCurrent();
    // todo: perform cleanup
    doneCurrent();
}

void QGLOSGWidget::resizeGL(int width, int height)
{
    int ratio = getDpiFactor(this);
    if (ratio > 1)
    {
        width *= ratio;
        height *= ratio;
    }
    if (m_graphic_window.valid())
    {
        getEventQueue()->windowResize(/*m_graphic_window->getTraits()*/x(),y(), width, height);
        m_graphic_window->resized(x(), y(), width, height);        
    }
    onResize(width,height);
}

void QGLOSGWidget::frame()
{
    if (!m_bInitialized) 
        return;
    if (m_view.valid())
    {
        if (property("DrawingFrame").toBool())
        {
            qDebug() << "Draw call within draw - skipping";
            return;
        }
        setProperty("DrawingFrame", true);
        makeCurrent();

        static_cast<StateEx *>(m_graphic_window->getState())->setDefaultFbo(defaultFramebufferObject());

        glClear(m_view->getCamera()->getClearMask());
        m_view->frame();
        doneCurrent();
        setProperty("DrawingFrame", false);
    }
}

void QGLOSGWidget::paintGL()
{
    if (m_view.valid())
    {
        if (property("DrawingFrame").toBool())
        {
            qDebug() << "Draw call within draw - skipping";
            return;
        }
        setProperty("DrawingFrame", true);

        static_cast<StateEx *>(m_graphic_window->getState())->setDefaultFbo(defaultFramebufferObject());        

        glClear(m_view->getCamera()->getClearMask());
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

        setProperty("DrawingFrame", false);
    }
}

void QGLOSGWidget::glDraw()
{

}

void QGLOSGWidget::glInit()
{

}

void QGLOSGWidget::setScene(osg::Node * scene)
{
    if( m_view.get() )
        m_view->setSceneData(scene);
}

osg::Node *	QGLOSGWidget::getScene() 
{
    if( m_view.get() )
        return m_view->getSceneData();
	return NULL;
}

void QGLOSGWidget::addEventHandler(osgGA::GUIEventHandler * handler)
{
    if( !handler || !(m_view.get()) ) return;

    m_view->addEventHandler(handler);
}

void QGLOSGWidget::addEventHandlerFront(osgGA::GUIEventHandler * handler)
{
    if( !handler || !(m_view.get()) ) return;

    m_view->getEventHandlers().push_front(handler);
}

bool QGLOSGWidget::findEventHandler(osgGA::GUIEventHandler * handler)
{
    if( !handler || !(m_view.get()) ) return false;
    osgViewer::View::EventHandlers ehs = m_view->getEventHandlers();
    osgViewer::View::EventHandlers::iterator findIter = std::find(ehs.begin(), ehs.end(), handler);
    return findIter!=ehs.end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Removes the event handler described by handler.
//!
//!\param [in,out]  handler If non-null, the handler. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void QGLOSGWidget::removeEventHandler( osgGA::GUIEventHandler *handler )
{
    if(handler != 0)
        m_view->removeEventHandler(handler);
}

void QGLOSGWidget::centerAndScale()
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
void QGLOSGWidget::centerAndScale( const osg::BoundingBox & box )
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

void QGLOSGWidget::Refresh(bool bEraseBackground)
{
    update(); // or repaint for immediate repaint or UpdateGL?
}


void QGLOSGWidget::setBackgroundColor(const osg::Vec4 &color)
{
    m_bgColor = color;
    m_view->getCamera()->setClearColor( color );
    //setBackgroundRole(QPalette::Mid);
    //setForegroundRole(QPalette::Mid);
}

const osg::Vec4& QGLOSGWidget::getBackgroundColor() const
{
    return m_bgColor;
}


void QGLOSGWidget::keyPressEvent( QKeyEvent* event )
{
    setKeyboardModifiers(event);
#if(0) // doesn't handle correctly special keys like arrow keys
    QString keyString   = event->text();
    QByteArray arrKey = keyString.toLocal8Bit();
    const char* keyData = arrKey.data();
    this->getEventQueue()->keyPress( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
#else
    int value = s_QtKeyboardMap.remapKey( event );
	getEventQueue()->keyPress( value );
#endif
}

void QGLOSGWidget::keyReleaseEvent( QKeyEvent* event )
{
    setKeyboardModifiers(event);
#if(0)
    QString keyString   = event->text();
    QByteArray arrKey = keyString.toLocal8Bit();
    const char* keyData = arrKey.data();
    this->getEventQueue()->keyRelease( osgGA::GUIEventAdapter::KeySymbol( *keyData ) );
#else
    int value = s_QtKeyboardMap.remapKey( event );
	getEventQueue()->keyRelease( value );
#endif
}

void QGLOSGWidget::wheelEvent ( QWheelEvent * event )
{
    setKeyboardModifiers(event);
    /*event->accept();
    int delta = event->delta();
    osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?   osgGA::GUIEventAdapter::SCROLL_UP
                                                                : osgGA::GUIEventAdapter::SCROLL_DOWN;
    this->getEventQueue()->mouseScroll( motion );
    Refresh(false);*/
    int d = event->delta();
    if (0==d)
        return;
    
	getEventQueue()->mouseScroll(
        event->orientation() == Qt::Vertical ?
            (event->delta()>0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN) :
            (event->delta()>0 ? osgGA::GUIEventAdapter::SCROLL_LEFT : osgGA::GUIEventAdapter::SCROLL_RIGHT) );
}

osgGA::EventQueue* QGLOSGWidget::getEventQueue() const
{
    osgGA::EventQueue* eventQueue = m_graphic_window->getEventQueue();
    if( eventQueue )
        return eventQueue;
    else
        throw std::runtime_error( "Unable to obtain valid event queue");
} 

void QGLOSGWidget::paintEvent( QPaintEvent* event )
{
    QOpenGLWidget::paintEvent(event);
}

void QGLOSGWidget::onHome()
{
    m_view->home();
}

void QGLOSGWidget::onResize( int width, int height )
{
	std::vector<osg::Camera*> cameras;
	m_view->getCameras( cameras );
	if ( cameras.size() == 1 )
		cameras.front()->setViewport( 0, 0, width, height );
}

void QGLOSGWidget::setKeyboardModifiers( QInputEvent* event )
{
    int modkey = event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier);
    unsigned int mask = 0;
    if ( modkey & Qt::ShiftModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
    if ( modkey & Qt::ControlModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_CTRL;
    if ( modkey & Qt::AltModifier ) mask |= osgGA::GUIEventAdapter::MODKEY_ALT;
    getEventQueue()->getCurrentEventState()->setModKeyMask( mask );
}

void QGLOSGWidget::mousePressEvent ( QMouseEvent * event )
{
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button
    unsigned int button = 0;
    switch( event->button() )
    {
    case Qt::LeftButton:      button = 1;      break;
    case Qt::MiddleButton:    button = 2;      break;
    case Qt::RightButton:     button = 3;      break;
    default:      break;
    }
    setKeyboardModifiers( event );
    int ratio = getDpiFactor(this);
    getEventQueue()->mouseButtonPress( static_cast<float>( event->x() * ratio),
                                             static_cast<float>( event->y() * ratio),
                                             button );
}

void QGLOSGWidget::mouseReleaseEvent ( QMouseEvent * event )
{    
    // 1 = left mouse button
    // 2 = middle mouse button
    // 3 = right mouse button

    unsigned int button = 0;

    switch( event->button() )
    {
    case Qt::LeftButton:      button = 1;      break;
    case Qt::MiddleButton:    button = 2;      break;
    case Qt::RightButton:     button = 3;      break;
    default:      break;
    }
    setKeyboardModifiers(event);
    int ratio = getDpiFactor(this);
    getEventQueue()->mouseButtonRelease( static_cast<float>( event->x() * ratio),
                                               static_cast<float>( event->y() * ratio ),
                                               button );
}

void QGLOSGWidget::mouseDoubleClickEvent( QMouseEvent* event )
{
    int button = 0;
    switch ( event->button() )
    {
        case Qt::LeftButton: button = 1; break;
        case Qt::MidButton: button = 2; break;
        case Qt::RightButton: button = 3; break;
        case Qt::NoButton: button = 0; break;
        default: button = 0; break;
    }
    setKeyboardModifiers( event );
    int ratio = getDpiFactor(this);
    getEventQueue()->mouseDoubleButtonPress( event->x() * ratio, event->y() * ratio, button );
}

void QGLOSGWidget::mouseMoveEvent ( QMouseEvent * event )
{
    setKeyboardModifiers( event );
    int ratio = getDpiFactor(this);
    this->getEventQueue()->mouseMotion( static_cast<float>( event->x() * ratio),
                                        static_cast<float>( event->y() * ratio ) );
    //if (Qt::NoButton!=event->buttons())
    //    Refresh(false);
}

void QGLOSGWidget::enterEvent ( QEvent * event )
{
    QOpenGLWidget::enterEvent(event);
    if (!hasFocus())
        setFocus(Qt::MouseFocusReason);
    // call refresh so the osg items can receive the information too
        Refresh(false);
}

void QGLOSGWidget::leaveEvent ( QEvent * event )
{
    QOpenGLWidget::leaveEvent(event);
    // call refresh so the osg items can receive the information too
        Refresh(false);
}

bool QGLOSGWidget::event(QEvent *event)
{
    // VPL_LOG_INFO("context " << context() << " event " << event->type() << " initialized " << m_bInitialized << " " << this->objectName().toStdString());
    if (QEvent::Paint == event->type() && property("BlockPaint").toBool())
    {
        VPL_LOG_INFO("Blocking paint " << objectName().toStdString());
        return false;
    }
    if (QEvent::Paint == event->type() && property("SilentBlockPaint").toBool())
    {
        return false;
    }

    bool handled = QOpenGLWidget::event(event);

    // This ensures that the OSG widget is always going to be repainted after the
    // user performed some interaction. Doing this in the event handler ensures
    // that we don't forget about some event and prevents duplicate code.
    switch( event->type() )
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::Wheel:
            update();
            break;
    default:
            break;
    }
    return handled;
}

void QGLOSGWidget::enableMultiThreaded()
{
    if (!m_view.get() || !m_graphic_window.get()) return;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    return; // Qt5 multithreaded support is broken
    // http://osg-users.openscenegraph.narkive.com/yR4B7TDq/qt-5-4-osg-3-2-1-multithreading
    // http://pastebin.com/W2QJj8bQ#
#endif
    {   // replace safer single threaded graphics window with the multithreaded one
        m_graphic_window = new QtOSGGraphicsWindowMT(const_cast<osg::GraphicsContext::Traits*>(m_graphic_window->getTraits()),this);
        m_view->getCamera()->setGraphicsContext(m_graphic_window.get());
    }
    m_view->setThreadingModel(osgViewer::Viewer::CullDrawThreadPerContext);

    m_view->setThreadSafeRefUnref(true); // enables thread safety for the object
    m_view->setEndBarrierPosition(osgViewer::ViewerBase::AfterSwapBuffers);
}

////////////////////////////////////////////////////////////
//

void QGLOSGWidget::screenShot( osg::Image * img, unsigned int scalePercent, bool bWantWidgets )
{
    osg::ref_ptr< osg::Image > p_Image = img;
    osg::ref_ptr< osg::Camera > p_OrigCam = m_view->getCamera();
    osg::ref_ptr< osg::Viewport > p_OrigView = p_OrigCam->getViewport();

    osg::Matrix m = m_view->getCameraManipulator()->getMatrix();

//    double ratio = p_OrigView->width() / (double) p_OrigView->height();

    int w, h;
    w = p_OrigView->width();
    h = p_OrigView->height();

    w = std::max(1,(int)(w*scalePercent/100.0));
    h = std::max(1,(int)(h*scalePercent/100.0));

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

        p_Camera->setFinalDrawCallback(p_Capture.get());

        // set camera as scene child
        scene->asGroup()->addChild(p_Camera);
        p_Camera->setMode(osg::CCaptureCamera::MODE_SINGLE);

        // redraw the scene
        makeCurrent();
        glClear(m_view->getCamera()->getClearMask());
        m_view->frame();
        doneCurrent();

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
    makeCurrent();
    glClear(m_view->getCamera()->getClearMask());
    m_view->frame();
    doneCurrent();

    p_Cam->setViewport( p_OrigView.get() );

    // restore settings and redraw scene
    m_view->setSceneData( scene.get() );
    m_view->getCameraManipulator()->setByMatrix( m );
    makeCurrent();
    glClear(m_view->getCamera()->getClearMask());
    m_view->frame();
    doneCurrent();
}

////////////////////////////////////////////////////////////
// adopted from scene::CBDRulerWidget
void QGLOSGWidget::getDistances(float & dx, float & dy)
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
