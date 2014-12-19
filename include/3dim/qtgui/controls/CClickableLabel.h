///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// BlueSkyPlan version 3.x
// Diagnostic and implant planning software for dentistry.
//
// Copyright 2012 Blue Sky Bio, LLC
// All rights reserved
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
