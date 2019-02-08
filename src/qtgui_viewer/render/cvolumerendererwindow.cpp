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


CVolumeRendererWindow::CVolumeRendererWindow(QWidget* parent) : OSGCanvas(parent)
{
    m_Renderer.setCanvas(this);
}

void CVolumeRendererWindow::showEvent(QShowEvent *event)
{
    m_Renderer.init();
    OSGCanvas::showEvent(event);
}

void CVolumeRendererWindow::resizeEvent(QResizeEvent *event)
{
    m_Renderer.updateRenderTargets();
    OSGCanvas::resizeEvent(event);
}

void CVolumeRendererWindow::mousePressEvent(QMouseEvent *event)
{
    m_Renderer.setMouseMode(true);
    m_Renderer.setMousePressed(true);
    OSGCanvas::mousePressEvent(event);
}

void CVolumeRendererWindow::mouseMoveEvent(QMouseEvent *event) {
	
	//and pass it along to base class
    OSGCanvas::mouseMoveEvent(event);

}


void CVolumeRendererWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_Renderer.setMouseMode(false);
    m_Renderer.setMousePressed(false);
    OSGCanvas::mouseReleaseEvent(event);
}

void CVolumeRendererWindow::leaveEvent(QEvent *event)
{
    m_Renderer.setMouseMode(false);
    m_Renderer.setMousePressed(false);
    OSGCanvas::leaveEvent(event);
}

void CVolumeRendererWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_Renderer.setMouseMode(false);
    m_Renderer.setMousePressed(false);
    OSGCanvas::dragLeaveEvent(event);
}
