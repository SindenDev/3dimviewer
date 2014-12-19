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

///////////////////////////////////////////////////////////////////////////////
//! Group box with icon in title

#ifndef CIconGroupBox_H
#define CIconGroupBox_H

#include <QGroupBox>
#include <QIcon>

class CIconGroupBox : public QGroupBox
{
    Q_OBJECT

protected:
    QIcon m_icon;
    QSize m_iconSize;

public:
    CIconGroupBox(const QIcon &icon, const QSize &iconSize, QWidget *parent = NULL);
    virtual ~CIconGroupBox();

public:
    void setIcon(const QIcon &icon);
    QIcon icon() const;

    void setIconSize(const QSize &size);
    QSize iconSize() const;

protected slots:
    virtual void paintEvent(QPaintEvent *e);
};

#endif // CIconGroupBox_H
