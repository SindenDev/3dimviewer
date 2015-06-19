///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
//! Clickable label

#ifndef CCLICKABLELABEL_H_
#define CCLICKABLELABEL_H_

#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QMouseEvent>

class CClickableLabel : public QLabel
{
    Q_OBJECT
public:
    CClickableLabel( QWidget * parent = 0 ) : QLabel(parent), m_pSlider(NULL), m_pSpinBox(NULL), m_nDefaultValue(0) {}
    ~CClickableLabel(){}
    void associateWithSlider(QSlider* pSlider, int nDefault) { m_pSlider=pSlider; m_nDefaultValue=nDefault; }
    void associateWithSpinBox(QSpinBox* pSpin, int nDefault) { m_pSpinBox=pSpin; m_nDefaultValue=nDefault; }
signals:
    void clicked();
    void doubleClicked();
protected:
    QSlider*    m_pSlider;
    QSpinBox*   m_pSpinBox;
    int         m_nDefaultValue;
    void mousePressEvent ( QMouseEvent * event ) { emit clicked(); }
    void mouseDoubleClickEvent ( QMouseEvent * event )
    {
        QLabel::mouseDoubleClickEvent(event);
        if (event->button() == Qt::LeftButton)
        {
            if (NULL!=m_pSlider)
                m_pSlider->setValue(m_nDefaultValue);
            if (NULL!=m_pSpinBox)
                m_pSpinBox->setValue(m_nDefaultValue);
            emit doubleClicked();
        }
    }
};

#endif // CCLICKABLELABEL_H_
