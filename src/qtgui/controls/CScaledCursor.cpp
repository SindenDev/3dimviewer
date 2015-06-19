///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2015 3Dim Laboratory s.r.o.
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

#include <controls/CScaledCursor.h>
#include <QPainter>

#if __APPLE__
#define MAX_CURSOR_PIXMAP_SIZE 64
#else
#define MAX_CURSOR_PIXMAP_SIZE 128
#endif

#define MIN_CURSOR_PIXMAP_SIZE 4

CScaledCursor::CScaledCursor(size_t min_size, size_t max_size)
{
    // Store cursor pixmap limits
    m_min_size = std::max<size_t>(MIN_CURSOR_PIXMAP_SIZE, std::min<size_t>(min_size, MAX_CURSOR_PIXMAP_SIZE));
    m_max_size = std::max<size_t>(MIN_CURSOR_PIXMAP_SIZE, std::min<size_t>(max_size, MAX_CURSOR_PIXMAP_SIZE));
	m_pen_size = 3;
    m_cursor = new QCursor;
    m_color = QColor(230, 230, 0);
	
}

void CScaledCursor::setPenSize(int size)
{
	m_pen_size = size;
}

void CScaledCursor::resize(float new_size)
{
    if(new_size < 0.0)
        return;

    // Compute new used size
    size_t pixmap_size(new_size+0.5);
    pixmap_size = std::max<size_t>(m_min_size, std::min<size_t>(pixmap_size, m_max_size));

    // Render cursor
    render(pixmap_size);
}


void CScaledCursor::render(size_t pixmap_size)
{
    // Create pixmap
    QPixmap pixmap(pixmap_size, pixmap_size);
    pixmap.fill(QColor(0, 0, 0, 0));

    int pen_width(m_pen_size);
    int hpw(pen_width/2);
    // Set pen and brush
    QPen pen(m_color);
    pen.setWidth(pen_width);

    // Draw circle
    QPainter painter(&pixmap);
    painter.setPen(pen);
    painter.drawArc(hpw, hpw, pixmap_size-pen_width, pixmap_size-pen_width, 0, 16*360);

    // Create cursor
    *m_cursor = QCursor(pixmap);
}

CScaledCursor::~CScaledCursor()
{
    if(m_cursor != 0)
        delete m_cursor;
}
