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

#ifndef OSGCanvas_H
#define OSGCanvas_H

#include <osgViewer/Viewer>
#include <osgManipulator/Dragger>

#include <VPL/Base/BasePtr.h>
#include <VPL/Base/Lock.h>
#include <VPL/Module/Signal.h>
#include <VPL/System/Mutex.h>

#include <osgQt/QtOsg.h>

// STL
#include <string>

//! Qt-OSG Canvas
class OSGCanvas
    : public QGLOSGWidget
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
    //! Restore mode on mouse button release?
    bool                m_bRestoreModeOnMouseRelease;    
	//! true when shortcut event came but no key press
	bool				m_bShortcut;
    //! Custom cursor
    QCursor*			m_customCursor;
private:
    virtual void        init(QWidget *parent, const osg::Vec4& bgColor);
public:
    //! Signal connection (appmode changed).
     vpl::mod::tSignalConnection m_Connection;

    //! Sets mouse cursor according to a given application mode.
    void                setCursorX ( int appmode );

    //! Set custom cursor for drawing mode
    void setCustomCursor(QCursor *cursor) { m_customCursor = cursor; }

    //! Get custom cursor for drawing mode
    QCursor *getCustomCursor() {return m_customCursor; }

    //! Centers and scales the scene.
    virtual void        centerAndScale();

    //! Center and scale - given bounding box
    virtual void centerAndScale( const osg::BoundingBox & box );

    //! Check current mouse mode whether should be restored later when mouse button is down
    bool canPostponeMouseModeRestore();    

    //! Restore mouse mode
    void restoreMouseMode(bool bForce=false);

    //! event handlers
    virtual void        keyPressEvent( QKeyEvent* event );
    virtual void        keyReleaseEvent( QKeyEvent* event );
    virtual void        wheelEvent ( QWheelEvent * event );
    virtual void        mouseMoveEvent ( QMouseEvent * event );
    virtual void        mousePressEvent ( QMouseEvent * event );
    virtual void        mouseReleaseEvent ( QMouseEvent * event );        
    virtual void        enterEvent ( QEvent * event ) ;
    virtual void        leaveEvent ( QEvent * event ) ;
	virtual bool		event(QEvent *event);
    virtual void        focusInEvent ( QFocusEvent * event ) override;
    virtual void        focusOutEvent ( QFocusEvent * event ) override;
};


#endif // OSGCanvas_H
