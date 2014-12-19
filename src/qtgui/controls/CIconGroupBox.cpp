///////////////////////////////////////////////////////////////////////////////
// $Id:$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2014 3Dim Laboratory s.r.o.
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

#include "qtgui/controls/CIconGroupBox.h"
#include <QPainter>
#include <QPalette>

CIconGroupBox::CIconGroupBox(const QIcon &icon, const QSize &iconSize, QWidget *parent)
    : QGroupBox(parent)
    , m_icon(icon)
    , m_iconSize(iconSize)
{ }

CIconGroupBox::~CIconGroupBox()
{ }

void CIconGroupBox::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

QIcon CIconGroupBox::icon() const
{
    return m_icon;
}

void CIconGroupBox::setIconSize(const QSize &size)
{
    m_iconSize = size;
}

QSize CIconGroupBox::iconSize() const
{
    return m_iconSize;
}

void CIconGroupBox::paintEvent(QPaintEvent *e)
{
    QGroupBox::paintEvent(e);

    QPainter painter(this);

    QRect rectangle = rect();

    int x, y, w, h;
    x = std::max(rectangle.x(), rectangle.x() + rectangle.width() - m_iconSize.width() - 8 - 4 - 4);
    y = rectangle.y();
    w = std::min(rectangle.width(), m_iconSize.width() + 4 + 4);
    h = std::min(rectangle.height(), m_iconSize.height());

    QColor bgColor = palette().color(QPalette::Background);

    painter.setPen(bgColor);
    painter.setBrush(QBrush(bgColor));
    painter.drawRect(x, y, w, h);

    x = x + 4;
    w = w - 4 - 4;

    m_icon.paint(&painter, x, y, w, h);
}
