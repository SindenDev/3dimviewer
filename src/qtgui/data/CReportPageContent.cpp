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

#if defined(REPORTDEBUG)
#include "CReportPageContent.h"
#include "CReportTable.h"
#else
#include "data/CReportPageContent.h"
#include "data/CReportTable.h"
#endif

#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

//TEST
#include <QDebug>

//TODO: comments & clean up

CReportPageContent::CReportPageContent()
    :m_contentType(ECT_UNUSED)
    , m_imageScale(1.0)
    , m_centered(false)
    , m_stretchImage(false)
    , m_scalingEnabled(true)
    , m_overlay(false)
    , m_aspectRatioMode(Qt::KeepAspectRatio)

{
}
CReportPageContent::CReportPageContent(SReportPageContentSettings settings)
{
    m_overallRect = settings.overallRect;
    m_contentRect = settings.contentRect;
    m_imageRect = settings.imageRect;
    m_captionRect = settings.captionRect;
    m_marginBottom = settings.marginBottom;
    m_marginRight = settings.marginRight;

    m_text = settings.text;
    m_HTML = settings.HTML;
    m_contentType = settings.contentType;
    m_color = settings.color;
    m_font = settings.font;
    m_textOptions = settings.textOptions;
    m_scalingEnabled = settings.scalingEnabled;

    m_img = settings.img;
    m_imageScale = settings.imageScale;
    m_centered = settings.centered;
    m_stretchImage = settings.stretchImage;
    m_caption = settings.caption;
    m_captionInsideImage = settings.captionInsideImage;
    m_captionPos = settings.captionPos;
    m_overlay = settings.overlay;
    m_aspectRatioMode = settings.aspectRatioMode;
}

CReportPageContent::~CReportPageContent()
{
}

void CReportPageContent::setText(QString text)
{
    m_text = text;
    m_contentType = ECT_TEXT;
}

void CReportPageContent::setTable(CReportTable table)
{
    m_HTML = table.createHTMLTable();
    m_contentType = ECT_HTML;
}

void CReportPageContent::setImage(QImage img)
{
    m_img = img;
    m_contentType = ECT_IMG;
}

void CReportPageContent::setTextOptions(QTextOption options)
{
    m_textOptions = options;
}

QTextOption CReportPageContent::getTextOptions()
{
    return m_textOptions;
}

double CReportPageContent::getImageScaleFactor()
{    
    if (m_contentType != ECT_IMG)
        return 0.0;

    QSize scaledImageSize = m_img.size();
    scaledImageSize.scale(m_imageRect.size(), m_aspectRatioMode);

    double scaleX = scaledImageSize.width() / (double)m_img.width();
    double scaleY = scaledImageSize.height() / (double)m_img.height();
    double scale = qMin(scaleX, scaleY);

    return scale;
}

double CReportPageContent::getImageWidthScaleFactor()
{
    if (m_contentType != ECT_IMG)
        return 0.0;

    QSize scaledImageSize = m_img.size();
    scaledImageSize.scale(m_imageRect.size(), m_aspectRatioMode);

    double scaleX = scaledImageSize.width() / (double)m_img.width();

    return scaleX;
}

double CReportPageContent::getImageHeightScaleFactor()
{
    if (m_contentType != ECT_IMG)
        return 0.0;

    QSize scaledImageSize = m_img.size();
    scaledImageSize.scale(m_imageRect.size(), m_aspectRatioMode);

    double scaleY = scaledImageSize.height() / (double)m_img.height();

    return scaleY;
}

void CReportPageContent::setContent(SReportPageContentSettings settings)
{
    if (!settings.overallRect.isEmpty())
        m_overallRect = settings.overallRect;
    if (!settings.contentRect.isEmpty())
        m_contentRect = settings.contentRect;
    if (!settings.imageRect.isEmpty())
        m_imageRect = settings.imageRect;
    if (!settings.captionRect.isEmpty())
        m_captionRect = settings.captionRect;

    m_marginBottom = settings.marginBottom;
    m_marginRight = settings.marginRight;

    m_text = settings.text;
    m_HTML = settings.HTML;
    m_contentType = settings.contentType;
    m_color = settings.color;
    m_font = settings.font;
    m_textOptions = settings.textOptions;
    m_scalingEnabled = settings.scalingEnabled;

    m_img = settings.img;
    m_imageScale = settings.imageScale;
    m_centered = settings.centered;
    m_stretchImage = settings.stretchImage;
    m_caption = settings.caption;
    m_captionInsideImage = settings.captionInsideImage;
    m_captionPos = settings.captionPos;
    m_overlay = settings.overlay;
    m_aspectRatioMode = settings.aspectRatioMode;
}

void CReportPageContent::print(QPainter * painter)
{
    painter->save();
    painter->setFont(m_font);

    QPen tmpPen = painter->pen();
    tmpPen.setColor(m_color);
    painter->setPen(tmpPen);

    /*painter->drawRect(m_rect);*/
    switch (m_contentType)
    {
    case ECT_UNUSED:
        break;
    case ECT_TEXT:
        printPlainText(painter);
        break;

    case ECT_IMG:
        printImage(painter);
        break;

    case ECT_HTML:
        printHTMLText(painter);
        break;
    default:
        break;
    }

    painter->restore();

}

void CReportPageContent::moveTop(int value)
{
    m_overallRect.moveTop(m_overallRect.top() + value);
    m_contentRect.moveTop(m_contentRect.top() + value);
    m_imageRect.moveTop(m_imageRect.top() + value);
    m_captionRect.moveTop(m_captionRect.top() + value);

}

void CReportPageContent::moveLeft(int value)
{
    m_overallRect.moveLeft(m_overallRect.left() + value);
    m_contentRect.moveLeft(m_contentRect.left() + value);
    m_imageRect.moveLeft(m_imageRect.left() + value);
    m_captionRect.moveLeft(m_captionRect.left() + value);
}

QRect CReportPageContent::getOverallRect()
{
    return m_overallRect;
}

QRect CReportPageContent::getContentRect()
{
    return m_contentRect;
}

EContentType CReportPageContent::getContentType()
{
    return  m_contentType;
}

void CReportPageContent::setOverallRect(QRect rect)
{
    m_overallRect = rect;
}

void CReportPageContent::setContentRect(QRect rect)
{
    m_contentRect = rect;
}

void CReportPageContent::setColor(QColor col)
{
    m_color = col;
}

QColor CReportPageContent::getColor()
{
    return m_color;
}

void CReportPageContent::setFont(QFont font)
{
    m_font = font;
}

QFont CReportPageContent::getFont()
{
    return m_font;
}

void CReportPageContent::printHTMLText(QPainter *painter)
{

    QFont painterFont(painter->font());
    QTextDocument td;
    td.setDefaultFont(painterFont);
    td.setPageSize(m_contentRect.size());
    int fontSize = painterFont.pointSize();
    td.setHtml(m_HTML);

    //translate to desired location
    painter->translate(m_contentRect.topLeft());

    //content does not fit in desired rectangle -> need to scale down
    while (m_scalingEnabled && (td.documentLayout()->documentSize().height() > m_contentRect.height() || td.documentLayout()->documentSize().width() > m_contentRect.width()))
    {
        td.setPlainText("");
        td.clear();

        --fontSize;
        painterFont.setPointSizeF(fontSize);
        td.setDefaultFont(painterFont);
        td.setHtml(m_HTML);

        if (fontSize <= 0)
            break;

    }

    //set text color
    QAbstractTextDocumentLayout::PaintContext context;
    context.palette.setColor(QPalette::Text, m_color);

    //no need for scaling -> print directly
    td.documentLayout()->draw(painter, context);
    painter->resetTransform();
}

void CReportPageContent::printPlainText(QPainter * painter)
{

    QFont painterFont(painter->font());
    QTextDocument td;
    td.setDefaultFont(painterFont);
    td.setPageSize(m_contentRect.size());
    int fontSize = painterFont.pointSize();
    td.setPlainText(m_text);

    //content does not fit in desired rectangle -> need to scale down
    while (m_scalingEnabled && (td.documentLayout()->documentSize().height() > m_contentRect.height() || td.documentLayout()->documentSize().width() > m_contentRect.width()))
    {
        td.setPlainText("");
        td.clear();

        --fontSize;
        painterFont.setPointSizeF(fontSize);
        td.setDefaultFont(painterFont);
        td.setPlainText(m_text);

        if (fontSize <= 0)
            break;

    }

    //set scaled font
    painter->setFont(painterFont);
    //draw text
    painter->drawText(m_contentRect, m_text, m_textOptions);

    /*QPen p = painter->pen();

    p.setColor(Qt::red);
    painter->setPen(p);
    painter->drawRect(m_overallRect);

    p.setColor(Qt::blue);
    painter->setPen(p);
    painter->drawRect(m_contentRect);

    p.setColor(Qt::green);
    painter->setPen(p);
    painter->drawRect(m_imageRect);

    p.setColor(Qt::black);
    painter->setPen(p);
    painter->drawRect(m_captionRect);*/
}

void CReportPageContent::printImage(QPainter * painter)
{
    painter->resetTransform();

    //scale image based on aspect ratio mode
    QImage scaledImage = m_img.scaled(m_imageRect.size(), m_aspectRatioMode);
    QRect scaledImageRect(m_imageRect);
    scaledImageRect.setSize(scaledImage.size());
    scaledImageRect.moveCenter(m_imageRect.center());

    double scaleX = scaledImage.width() / (double)m_img.width();
    double scaleY = scaledImage.height() / (double)m_img.height();
    int scale = (int)(qMin(scaleX, scaleY) * 100);

    //draw image
    painter->drawImage(scaledImageRect, scaledImage);


    //QPen p = painter->pen();
    //
    //p.setColor(Qt::red);
    //painter->setPen(p);
    //painter->drawRect(m_overallRect);
    //
    //p.setColor(Qt::blue);
    //painter->setPen(p);
    //painter->drawRect(m_contentRect);
    //
    //p.setColor(Qt::green);
    //painter->setPen(p);
    //painter->drawRect(m_imageRect);

    //p.setColor(Qt::black);
    //painter->setPen(p);
    //painter->drawRect(m_captionRect);

    //caption
    if (!m_caption.isEmpty())
    {
        QString caption = m_caption;
        caption.replace(QString("%scaleFactor"), QString::number(scale) + QString("%"));
        painter->drawText(m_captionRect, caption, m_textOptions);
    }

}