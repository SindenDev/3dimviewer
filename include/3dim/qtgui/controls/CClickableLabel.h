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

///////////////////////////////////////////////////////////////////////////////
//! Clickable label

#ifndef CCLICKABLELABEL_H_
#define CCLICKABLELABEL_H_

#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QMouseEvent>
#include <QPainter>

class CClickableLabel : public QLabel
{
    Q_OBJECT
protected:
    bool m_bHandleDoubleClick;
    bool m_bDeleted;
    bool m_bAdjustSize;
    QPixmap m_pixmapBackup;
public:
    CClickableLabel( QWidget * parent = 0 ) : QLabel(parent), m_pSlider(NULL), m_pSpinBox(NULL), m_nDefaultValue(0), m_bHandleDoubleClick(true), m_bDeleted(false), m_bAdjustSize(false) {  setMouseTracking(true); }
    ~CClickableLabel() { m_pSlider = NULL; m_pSpinBox = NULL; m_bDeleted = false;}
    void associateWithSlider(QSlider* pSlider, int nDefault) { m_pSlider=pSlider; m_nDefaultValue=nDefault; }
    void associateWithSpinBox(QSpinBox* pSpin, int nDefault) { m_pSpinBox=pSpin; m_nDefaultValue=nDefault; }
    void setHandleDoubleClick(bool bHandle) { m_bHandleDoubleClick = bHandle; }
    void setAdjustImageSize(bool bAdjustSize) { m_bAdjustSize = bAdjustSize; }
signals:
    void clicked();
    void doubleClicked();
protected:
    QSlider*    m_pSlider;
    QSpinBox*   m_pSpinBox;
    int         m_nDefaultValue;
    // incomplete implementation, doesn't handle all situations!
    bool checkImagePosition(const QPoint& pos) 
    {
        if (NULL==pixmap())
            return true;
        int w = width();
        int h = height();
        int imgWidth = pixmap()->width();
        int imgHeight = pixmap()->height();
        Qt::Alignment align = alignment();
        QRect roi;
        if (Qt::AlignCenter==align)
        {
            roi.setLeft((w - imgWidth) / 2);
            roi.setTop((h - imgHeight) / 2);
            roi.setWidth(imgWidth);
            roi.setHeight(imgHeight);
        }
        if (!roi.isEmpty())
            return roi.contains(pos);
        return true;
    }
    void mousePressEvent ( QMouseEvent * event ) 
    { 
        if (!checkImagePosition(event->pos()))
            return;
        emit clicked(); 
    }
    void mouseDoubleClickEvent ( QMouseEvent * event )
    {
        QLabel::mouseDoubleClickEvent(event);
        Q_ASSERT(!m_bDeleted); // use deleteLater!
        if (m_bHandleDoubleClick && event->button() == Qt::LeftButton)
        {
            if (NULL!=m_pSlider)
                m_pSlider->setValue(m_nDefaultValue);
            if (NULL!=m_pSpinBox)
                m_pSpinBox->setValue(m_nDefaultValue);
            emit doubleClicked();
        }
    }
    void mouseMoveEvent(QMouseEvent * event)
    {
        if (!isEnabled() || !checkImagePosition(event->pos()))
            setCursor(Qt::ArrowCursor);
        else
            setCursor(Qt::PointingHandCursor);
        QLabel::mouseMoveEvent(event);
    }
protected slots:
    virtual void resizeEvent(QResizeEvent *event)
    {
        if (m_bAdjustSize && NULL!=pixmap())
        {
            if (m_pixmapBackup.isNull())
                m_pixmapBackup = *pixmap();            
            if (width()<m_pixmapBackup.width())
                setPixmap(m_pixmapBackup.scaled(width(),height(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
            else
                setPixmap(m_pixmapBackup);
        }
        QLabel::resizeEvent(event);
    }
};

#endif // CCLICKABLELABEL_H_
