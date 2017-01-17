///////////////////////////////////////////////////////////////////////////////
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

#ifndef QtOSGCanvas_H
#define QtOSGCanvas_H

#include <osgViewer/Viewer>
#include <osgManipulator/Dragger>
#include <osgQt/GraphicsWindowQt>
#include <osg/Version>
#include <osg/FrameBufferObject>

#include <VPL/Base/BasePtr.h>
#include <VPL/Base/Lock.h>
#include <VPL/Module/Signal.h>
#include <VPL/System/Mutex.h>

// STL
#include <string>

#include <qglobal.h>
#include <QInputEvent>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    #include <QOpenGLWidget>    
    #include <QGLWidget>
    //#define BASEGLWidget     QGLWidget      
    #define BASEGLWidget     QOpenGLWidget          
#else
    #include <QGLWidget>
    #define BASEGLWidget      QGLWidget      
#endif


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

// camera pre draw callback
class BindFboPreDrawCallback : public osg::Camera::DrawCallback
{
protected:
    osg::ref_ptr<osg::FrameBufferObject> _fbo;
public:
    BindFboPreDrawCallback(osg::FrameBufferObject* fbo) : _fbo(fbo) {}
    virtual void operator () (osg::RenderInfo& renderInfo) const
    {
        if (NULL != _fbo)
            _fbo->apply(*renderInfo.getState()); //bind FBO 
    }
};

// camera post draw callback to correctly restore fbo (osg restores to zero)
class UnBindFboPostDrawCallback : public osg::Camera::DrawCallback
{
public:
    virtual void operator () (osg::RenderInfo& renderInfo) const
    {
        osg::State* pState = renderInfo.getState();
#if OSG_VERSION_GREATER_OR_EQUAL(3,4,0)
        osg::GLExtensions* ext = renderInfo.getState()->get<osg::GLExtensions>();
        bool fbo_supported = ext && ext->isFrameBufferObjectSupported;
        if (NULL != ext)
            ext->glBindFramebuffer(GL_FRAMEBUFFER_EXT, ((StateEx*)(pState))->getDefaultFbo());
#else
        osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(pState->getContextID(), true);
        bool fbo_supported = fbo_ext && fbo_ext->isSupported();
        if (NULL != fbo_ext)
            fbo_ext->glBindFramebuffer(osg::FrameBufferObject::READ_DRAW_FRAMEBUFFER, ((StateEx*)(pState))->getDefaultFbo());
#endif
    }
};




// OSG Graphics Window for Qt - simple safer implementation for single threaded use 
class QtOSGGraphicsWindow
        : public osgViewer::GraphicsWindowEmbedded // this type of window lets Qt do the context switching
        , public vpl::base::CLockableObject<QtOSGGraphicsWindow>
{
protected:
    BASEGLWidget *m_glCanvas;
    bool          m_bContextX; // when context was made current by this class
public:
    //! Constructor
    QtOSGGraphicsWindow(GraphicsContext::Traits* traits, BASEGLWidget *widget);
    //! Destructor
    ~QtOSGGraphicsWindow();
    //! Initialization method
    void init();
    //! Canvas access method
    BASEGLWidget *getCanvas() const { return m_glCanvas; }

    // for single threaded rendering in qt these are just dummy implementations assuming that context is *always* current and valid
    virtual bool valid() const;
    virtual bool realizeImplementation();
    virtual bool isRealizedImplementation() const;
    virtual void closeImplementation();
    virtual bool makeCurrentImplementation();
    virtual bool releaseContextImplementation();
    virtual void swapBuffersImplementation();
    virtual void grabFocus();
    virtual void grabFocusIfPointerInWindow();
    virtual void raiseWindow();
};

// OSG Graphics Window for Qt - for multithreaded opengl, has known issues on AMD cards when screenshots are taken
class QtOSGGraphicsWindowMT
    : public osgQt::GraphicsWindowQt
    , public vpl::base::CLockableObject<QtOSGGraphicsWindowMT>
{
protected:
    BASEGLWidget *m_glCanvas;
public:
    //! Constructor
    QtOSGGraphicsWindowMT(GraphicsContext::Traits* traits, BASEGLWidget *widget);
    //! Destructor
    ~QtOSGGraphicsWindowMT();
    //! initialization method
    void init();
    //! Canvas access method
    BASEGLWidget *getCanvas() const { return m_glCanvas; }
    //! Overrides
    virtual bool valid() const;
    virtual bool makeCurrentImplementation();
    virtual void swapBuffersImplementation();
    virtual bool releaseContextImplementation();
    virtual void closeImplementation();
    virtual bool realizeImplementation();
    virtual bool isRealizedImplementation() const;
};


//! Customized OSG viewer
class QtOSGViewer : public osgViewer::Viewer
{
public:
    //! override viewerBase setUpThreading method to be able to adjust thread affinity
    virtual void setUpThreading();
};


//! Qt-OSG adaptation layer
class QGLOSGWidget
    : public BASEGLWidget
    , public vpl::base::CLockableObject<QGLOSGWidget>
{
    Q_OBJECT
public:
    // Mutual access.
    typedef vpl::base::CLockableObject<QGLOSGWidget>::CLock tLock;
    // Default constructor
    explicit QGLOSGWidget(QWidget *parent = NULL);
    explicit QGLOSGWidget(QWidget *parent, const osg::Vec4& bgColor, bool bAntialiasing);
    // Destructor.
    virtual ~QGLOSGWidget();
protected:
    // Smart pointer to OSG view.
    osg::ref_ptr<osgViewer::Viewer>                         m_view;
    // Smart pointer to QtOSGGraphicsWindow.
    osg::ref_ptr<osgViewer::GraphicsWindow>                 m_graphic_window;
    //! Last rendering time in msecs
    int                 m_lastRenderingTime;
    //! Use antialiasing
    bool                m_bAntialiasing;
protected:
    virtual void        init(QWidget *parent, const osg::Vec4& bgColor);
    virtual void        initializeGL();
    virtual void        resizeGL(int width, int height);
    virtual void        paintGL();
    virtual void        paintEvent( QPaintEvent* paintEvent );
    virtual void        onHome();
    virtual void        onResize( int width, int height );
    osgGA::EventQueue*  getEventQueue() const;
public:
    //! we override glDraw as GLWidget replaces original implementation with one that we don't want
    virtual void        glDraw();
    //! make glinit accessible for graphics window
    virtual void        glInit();

    //! Set scene data.
    void                setScene(osg::Node * scene) ;

	//! Get scene data.
    osg::Node *			getScene() ;

    //! Returns pointer to viewer
    osgViewer::Viewer * getView() { return m_view.get(); }

    //! Returns pointer to graphic window
    osgViewer::GraphicsWindow *    getGraphicWindow() { return m_graphic_window.get(); }

    //! Add event handler
    void                addEventHandler(osgGA::GUIEventHandler * handler);

    //! Add event handler to the front of the list
    void                addEventHandlerFront(osgGA::GUIEventHandler * handler);

    //! Remove event handler
    void                removeEventHandler(osgGA::GUIEventHandler *handler);

    //! Checks presence of event handler
    bool                findEventHandler(osgGA::GUIEventHandler * handler);

    //! Centers and scales the scene.
    virtual void        centerAndScale();

    //! Center and scale - given bounding box
    virtual void        centerAndScale( const osg::BoundingBox & box );

    //! calls update (asynchronous redraw)
    void                Refresh(bool bEraseBackground=true);

    //! Set background color
    void                setBackgroundColor(const osg::Vec4 &color);

    //! Render a screenshot into a file
    void screenShot( osg::Image * img, unsigned int maxSide = 0, bool bWantWidgets = true );

    //! Get real distance/size
    void getDistances(float & dx, float & dy);

    //! Setups the multithreaded mode.
    void enableMultiThreaded();

    //! Returns durating in msecs of last frame() call
    int getLastRenderingTime() const { return m_lastRenderingTime; }

    //! calls view->frame including context switch
    virtual void        frame();

    //! Set keyboard modifiers when passing qt to osg
    virtual void        setKeyboardModifiers( QInputEvent* event );
    
    //! event handlers
    virtual void        keyPressEvent( QKeyEvent* event );
    virtual void        keyReleaseEvent( QKeyEvent* event );
    virtual void        wheelEvent ( QWheelEvent * event );
    virtual void        mouseMoveEvent ( QMouseEvent * event );
    virtual void        mousePressEvent ( QMouseEvent * event );
    virtual void        mouseReleaseEvent ( QMouseEvent * event );   
    virtual void        mouseDoubleClickEvent( QMouseEvent* event );
    virtual void        enterEvent ( QEvent * event ) ;
    virtual void        leaveEvent ( QEvent * event ) ;
	virtual bool		event(QEvent *event);
};


#endif // QtOSGCanvas_H
