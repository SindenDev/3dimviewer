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

#ifndef CVolumeRendererWindow_H
#define CVolumeRendererWindow_H

#include <render/PSVRrenderer.h>
#include <osg/OSGCanvas.h>

#include <QMouseEvent>


//! Window containing volume rendering
class CVolumeRendererWindow : public OSGCanvas
{
    Q_OBJECT

public:
    CVolumeRendererWindow(QWidget* parent);

    //! Returns reference to the volume renderer.
    PSVR::PSVolumeRendering& getRenderer() { return m_Renderer; }

    //! Refresh canvas
    virtual void Refresh(bool bEraseBackground)
    {
        OSGCanvas::Refresh(bEraseBackground);
    }

protected:
    //! Volume renderer.
    PSVR::PSVolumeRendering m_Renderer;

    //! Initialize on show
    virtual void 	showEvent ( QShowEvent * event );

    //! Invalidate renderer's off-screen rendertargets on resize
    virtual void resizeEvent(QResizeEvent *event);

    //! Turns on downsampling during mouse handling
    virtual void mousePressEvent(QMouseEvent *event);

	//! Broadcasts the mouse cordinates through signal.
	virtual void mouseMoveEvent(QMouseEvent *event);


    //! Turns off downsampling when mouse handling is done
    virtual void mouseReleaseEvent(QMouseEvent *event);

    //! Turns off downsampling when mouse leaves window
    virtual void leaveEvent(QEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
};

#endif
