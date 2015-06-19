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

#ifndef OSGCanvas_H
#define OSGCanvas_H

#include <osgViewer/Viewer>
#include <osgManipulator/Dragger>

#include <VPL/Base/BasePtr.h>
#include <VPL/Base/Lock.h>
#include <VPL/Module/Signal.h>
#include <VPL/System/Mutex.h>

#include <QGLWidget>

// STL
#include <string>

#define USE_OSGQT

#ifdef USE_OSGQT
    #include <osgQt/GraphicsWindowQt>
    #define XGraphicsWindow     osgQt::GraphicsWindowQt
    #define XGLWidget           osgQt::GLWidget
//    #define XGLWidget           QGLWidget
    using namespace osgQt;
#else
    #define XGraphicsWindow     osgViewer::GraphicsWindow
    #define XGLWidget           QGLWidget
#endif

using namespace osgViewer;

class OSGCanvas;

// empty class at the moment
class OSGwxGraphicsWindow
        : public XGraphicsWindow
        , public vpl::base::CLockableObject<OSGwxGraphicsWindow>
{
    //Q_OBJECT
private:
    // Pointer to parent/hosted wxGLCanvas window.
    //OSGCanvas *m_wxglcanvas;
public:
    OSGwxGraphicsWindow(GraphicsContext::Traits* traits, QGLWidget *widget);
    void init();
    virtual bool valid() const { return true; }
    virtual bool makeCurrentImplementation();
    virtual void swapBuffersImplementation();
    virtual bool releaseContextImplementation();
    virtual void closeImplementation();
    virtual bool realizeImplementation();
    virtual bool isRealizedImplementation() const;

    //vpl::base::CScopedPtr<QGLContext> m_glcontext;
    QGLWidget *m_glCanvas;
};




class OSGCanvas
    : public XGLWidget
    , public vpl::base::CLockableObject<OSGCanvas>
{
    Q_OBJECT

public:
    // Mutual access.
    typedef vpl::base::CLockableObject<OSGCanvas>::CLock tLock;

    // Default constructor
    explicit OSGCanvas(QWidget *parent = NULL);
    explicit OSGCanvas(QWidget *parent, const osg::Vec4& bgColor);
    // Destructor.
    virtual ~OSGCanvas();
protected:
    // Smart pointer to OSG view.
    osg::ref_ptr<osgViewer::Viewer>                         m_view;
    // Smart pointer to OSGwxGraphicsWindow.
    osg::ref_ptr<GraphicsWindow>                       m_graphic_window;
    //! Restore mode on mouse button release?
    bool                m_bRestoreModeOnMouseRelease;    
	//! true when shortcut event came but no key press
	bool				m_bShortcut;
    //! Last rendering time in msecs
    int                 m_lastRenderingTime;
    //! Custom cursor
    QCursor*			m_customCursor;
private:
    void                init(QWidget *parent, const osg::Vec4& bgColor);

    void                initializeGL();
    void                resizeGL(int width, int height);
    void                paintGL();

public:
    //! we override glDraw as GLWidget replaces original implementation with one that we don't want
    virtual void glDraw();

    //! Signal connection (appmode changed).
     vpl::mod::tSignalConnection m_Connection;

    //! Sets mouse cursor according to a given application mode.
    void                setCursorX ( int appmode );

    //! Set custom cursor for drawing mode
    void setCustomCursor(QCursor *cursor) { m_customCursor = cursor; }

    //! Get custom cursor for drawing mode
    QCursor *getCustomCursor() {return m_customCursor; }

    //! Set scene data.
    void                setScene(osg::Node * scene) ;

	//! Get scene data.
    osg::Node *			getScene() ;

    //! Returns pointer to viewer
    osgViewer::Viewer * getView() { return m_view.get(); }

    //! Returns pointer to graphic window
    GraphicsWindow *    getGraphicWindow() { return m_graphic_window.get(); }

    //! Add event handler
    void                addEventHandler(osgGA::GUIEventHandler * handler);

    //! Add event handler to the front of the list
    void                addEventHandlerFront(osgGA::GUIEventHandler * handler);

    //! Centers and acales the scene.
    void                centerAndScale();

    //! Center and scale - given bounding box
    virtual void centerAndScale( const osg::BoundingBox & box );

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
    
    //! Check current mouse mode whether should be restored later when mouse button is down
    bool canPostponeMouseModeRestore();    

    //! Restore mouse mode
    void restoreMouseMode(bool bForce=false);

    //! event handlers
    virtual void        keyPressEvent( QKeyEvent* event );
    virtual void        keyReleaseEvent( QKeyEvent* event );
    virtual void        wheelEvent ( QWheelEvent * event );
    virtual void        mouseMoveEvent ( QMouseEvent * event );
    virtual void        mouseReleaseEvent ( QMouseEvent * event );        
    virtual void        enterEvent ( QEvent * event ) ;
    virtual void        leaveEvent ( QEvent * event ) ;
	virtual bool		event(QEvent *event);
};


#endif // OSGCanvas_H
