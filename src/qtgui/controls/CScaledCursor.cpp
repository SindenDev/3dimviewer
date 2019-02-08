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

void CScaledCursor::setMinSize(size_t min_size)
{
    m_min_size = min_size;
}

void CScaledCursor::setMaxSize(size_t max_size)
{
    m_max_size = max_size;
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




CScaledCursorDoubleCircle::CScaledCursorDoubleCircle(size_t min_size, size_t max_size)
{
    // Store cursor pixmap limits
    m_min_size = std::max<size_t>(MIN_CURSOR_PIXMAP_SIZE, std::min<size_t>(min_size, MAX_CURSOR_PIXMAP_SIZE));
    m_max_size = std::max<size_t>(MIN_CURSOR_PIXMAP_SIZE, std::min<size_t>(max_size, MAX_CURSOR_PIXMAP_SIZE));
    m_pen_size_outer = 3;
    m_pen_size_inner = 2;
    m_cursor = new QCursor;
    m_color = QColor(230, 230, 0);

}

void CScaledCursorDoubleCircle::setMinSize(size_t min_size)
{
    m_min_size = min_size;
}

void CScaledCursorDoubleCircle::setMaxSize(size_t max_size)
{
    m_max_size = max_size;
}

void CScaledCursorDoubleCircle::setPenSize(int size_outer, int size_inner)
{
    m_pen_size_outer = size_outer;
    m_pen_size_inner = size_inner;
}

void CScaledCursorDoubleCircle::resize(float new_size_outer, float new_size_inner)
{
    if (new_size_outer < 0.0 || new_size_inner < 0.0)
        return;

    // Compute new used size
    size_t pixmap_size(std::max(new_size_outer, new_size_inner) + 0.5);
    pixmap_size = std::max<size_t>(m_min_size, std::min<size_t>(pixmap_size, m_max_size));

    // Render cursor
    render(pixmap_size, new_size_outer, new_size_inner);
}


void CScaledCursorDoubleCircle::render(size_t pixmap_size, size_t size_outer, size_t size_inner)
{
    // Create pixmap
    QPixmap pixmap(pixmap_size, pixmap_size);
    pixmap.fill(QColor(0, 0, 0, 0));

    int pen_width_outer(m_pen_size_outer);
    int pen_width_inner(m_pen_size_inner);
    int hpw_outer(pen_width_outer / 2);
    int hpw_inner(pen_width_inner / 2);

    // Set pen and brush
    QPen pen_outer(m_color);
    pen_outer.setWidth(pen_width_outer);

    QPen pen_inner(m_color);
    pen_inner.setWidth(pen_width_inner);

    size_t final_size_outer = std::min(size_outer, pixmap_size);
    size_t final_size_inner = std::min(size_inner, pixmap_size);

    // Draw circle
    QPainter painter(&pixmap);
    painter.setPen(pen_outer);
    painter.drawArc(hpw_outer + (pixmap_size - final_size_outer) * 0.5, hpw_outer + (pixmap_size - final_size_outer) * 0.5, final_size_outer - pen_width_outer, final_size_outer - pen_width_outer, 0, 16 * 360);
    painter.setPen(pen_inner);
    painter.drawArc(hpw_inner + (pixmap_size - final_size_inner) * 0.5, hpw_inner + (pixmap_size - final_size_inner) * 0.5, final_size_inner - pen_width_inner, final_size_inner - pen_width_inner, 0, 16 * 360);

    // Create cursor
    *m_cursor = QCursor(pixmap);
}

CScaledCursorDoubleCircle::~CScaledCursorDoubleCircle()
{
    if (m_cursor != 0)
        delete m_cursor;
}
