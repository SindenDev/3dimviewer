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


#include "render/cvolumerendererwindow.h"


CVolumeRendererWindow::CVolumeRendererWindow(QWidget *parent)
#if defined(USE_PSVR)
    : tBase( parent )
#endif
{
#ifdef USE_PSVR
    m_Renderer.setCanvas(this);
    //m_Renderer.init();
#endif // USE_PSVR
}

CVolumeRendererWindow::~CVolumeRendererWindow()
{
    //m_Renderer.release();
}

void CVolumeRendererWindow::showEvent(QShowEvent *event)
{
    m_Renderer.init();
    tBase::showEvent(event);	
}

void CVolumeRendererWindow::resizeEvent(QResizeEvent *event)
{
    m_Renderer.updateRenderTargets();
    tBase::resizeEvent(event);
}

void CVolumeRendererWindow::mousePressEvent(QMouseEvent *event)
{
    m_Renderer.setMouseMode(true);
    m_Renderer.setMousePressed(true);
    tBase::mousePressEvent(event);
}

void CVolumeRendererWindow::mouseMoveEvent(QMouseEvent *event) {
	
	//and pass it along to base class
	tBase::mouseMoveEvent(event);

}


void CVolumeRendererWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_Renderer.setMouseMode(false);
    m_Renderer.setMousePressed(false);
    tBase::mouseReleaseEvent(event);
}

void CVolumeRendererWindow::leaveEvent(QEvent *event)
{
    m_Renderer.setMouseMode(false);
    m_Renderer.setMousePressed(false);
    tBase::leaveEvent(event);
}

void CVolumeRendererWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_Renderer.setMouseMode(false);
    m_Renderer.setMousePressed(false);
    tBase::dragLeaveEvent(event);
}
