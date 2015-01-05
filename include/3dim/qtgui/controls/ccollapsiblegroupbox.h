///////////////////////////////////////////////////////////////////////////////
// $Id$
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

#ifndef CCollapsibleGroupBox_H
#define CCollapsibleGroupBox_H

#include <QGroupBox>
#include <QProxyStyle>

#define GROUPBOX_HEIGHT 22

// Proxy style for group box that replaces checkbox with arrow
class CCollapsibleGroupBoxStyle : public QProxyStyle
{
public:
    virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption * opt, QPainter * p, const QWidget * widget = 0) const
    {
        if (QStyle::PE_IndicatorCheckBox == pe)
        {
            const QGroupBox* groupBox= qobject_cast<const QGroupBox*>(widget);
            if (groupBox)
            {
                QProxyStyle::drawPrimitive(groupBox->isChecked() ? QStyle::PE_IndicatorArrowDown : QStyle::PE_IndicatorArrowRight, opt, p, widget);
                return;
            }
        }
        QProxyStyle::drawPrimitive(pe, opt, p, widget);
    }
};

class CCollapsibleGroupBox : public QObject
{
    Q_OBJECT
private:
    QGroupBox*  m_pGBox;
    int         m_nHeight;
    QString     m_sRegName;
public:
    CCollapsibleGroupBox();    
    ~CCollapsibleGroupBox();
    void setGroupBox(QGroupBox* pGBox, const QString& regName);
private slots:
    void packBox(bool);
};

#define SETUP_COLLAPSIBLE_GROUPBOX(gbName)  \
    gbName->setCheckable(true); \
    gbName->setStyle(new CCollapsibleGroupBoxStyle()); \
    gbName->setProperty("ProperHeight",gbName->sizeHint().height()); \
    gbName->setObjectName("CollapsibleBox"); \
    QObject::connect(gbName,SIGNAL(clicked(bool)),this,SLOT(packGroupBox(bool)));

#endif // CCollapsibleGroupBox_H