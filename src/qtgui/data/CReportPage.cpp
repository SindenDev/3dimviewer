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
#include "CReportPage.h"
#include "CReportTable.h"
#include "CImageGrid.h"

#else
#include "data/CReportPage.h"
#include "data/CReportTable.h"
#include "data/CImageGrid.h"
#endif

#include <vector>

#include <QImage>
#include <QPainter>
#include <QPrinter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QStaticText>
#include <QRegion>


//TEST
#include <QDebug>

//TODO: comments & clean up

//TODO:
// vo funckiach add... nepocitat rectangle, iba pridat content, rectangle vypocitat az tesne pred tlacou, pretoze, ten content sa moze este zmenit?
CReportPage::CReportPage()
    : m_contentMarginBottom(0.0)
    , m_contentMarginRight(0.0)
    , m_floatingContent(false)
    , m_scalingEnabled(true)
{
    m_content.clear();
}

CReportPage::CReportPage(SReportPageSettings settings)
{
    m_pageRect = settings.pageRect;
    m_pageSize = m_pageRect.size();
    m_contentMarginBottom = settings.contentMarginBottom;
    m_contentMarginRight = settings.contentMarginRight;
    m_floatingContent = settings.floatingContent;
    m_color = settings.color;
    m_font = settings.font;
    m_scalingEnabled = settings.scalingEnabled;
    m_pageHeightPixelsPerMM = settings.pageHeightPixelsPerMM;
    m_pageWidthPixelsPerMM = settings.pageWidthPixelsPerMM;

    m_content.clear();
}

CReportPage::~CReportPage()
{
    for (int i = 0; i < m_content.size(); ++i)
    {
        free(m_content.at(i));        
    }
}

int CReportPage::addRectangle(double width, double height)
{
    if (width < 0.0 || width > 1.0 || height < 0.0 || height > 1.0)
        return -1;

    QSize contentSize(width * m_pageSize.width(), height * m_pageSize.height());
    QSize overallSize = contentSize;

    //margins
    overallSize.setHeight(qMin(int(overallSize.height() + m_contentMarginBottom), m_pageRect.height()));
    overallSize.setWidth(qMin(int(overallSize.width() + m_contentMarginRight), m_pageRect.width()));


    QRect rect = findFreeSpace(overallSize);
    //no free space was found
    if (rect.isEmpty())
        return -1;

    rect.setSize(overallSize);

    QRect contentRect(rect);

    //set content size
    contentRect.setSize(contentSize);

    SReportPageContentSettings contentSettings;
    contentSettings.overallRect = rect;
    contentSettings.contentRect = contentRect;

    contentSettings.color = m_color;
    contentSettings.font = m_font;
    contentSettings.scalingEnabled = m_scalingEnabled;

    //CReportPageContent content(contentSettings);
    m_content.push_back(new CReportPageContent(contentSettings));

    //return index to content
    return (m_content.size() - 1);
}

int CReportPage::addRectangle(double width, double height, double x, double y, eRectangleAlignmentPosition alignPosition)
{
    if (width < 0.0 || width > 1.0 || height < 0.0 || height > 1.0 ||
        x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0)
        return -1;

    QRect contentRect(m_pageRect);

    //bottom is moved
    contentRect.setHeight(height * m_pageSize.height());

    //right is moved
    contentRect.setWidth(width * m_pageSize.width());

    QRect overallRect(contentRect);

    //margins
    overallRect.setHeight(qMin(int(overallRect.height() + m_contentMarginBottom), m_pageRect.height()));
    overallRect.setWidth(qMin(int(overallRect.width() + m_contentMarginRight), m_pageRect.width()));

    QPoint point(m_pageRect.left() + x * m_pageSize.width(), m_pageRect.top() + y* m_pageSize.height());

    switch (alignPosition)
    {
    case ERAP_TOPLEFT:
        overallRect.moveTopLeft(point);
        break;
    case ERAP_TOPRIGHT:
        overallRect.moveTopRight(point);
        break;
    case ERAP_BOTTOMLEFT:
        overallRect.moveBottomLeft(point);
        break;
    case ERAP_BOTTOMRIGHT:
        overallRect.moveBottomRight(point);
        break;
    default:
        break;
    }
    contentRect.moveTopLeft(overallRect.topLeft());


    SReportPageContentSettings settings;
    settings.overallRect = overallRect;
    settings.contentRect = contentRect;

    settings.color = m_color;
    settings.font = m_font;
    settings.scalingEnabled = m_scalingEnabled;

    //CReportPageContent content(contentSettings);
    m_content.push_back(new CReportPageContent(settings));

    //return index to content
    return (m_content.size() - 1);
}

int CReportPage::addRectangle(QSize size)
{
    QSize overallSize(size);


    //margins
    overallSize.setHeight(qMin(int(overallSize.height() + m_contentMarginBottom), m_pageRect.height()));
    overallSize.setWidth(qMin(int(overallSize.width() + m_contentMarginRight), m_pageRect.width()));

    QRect rect = findFreeSpace(overallSize);
    //no free space was found
    if (rect.isEmpty())
        return -1;


    rect.setSize(overallSize);

    //this rectable reaches outside page rectangle
    if (rect.bottom() > m_pageRect.bottom())
        return -1;

    QRect contentRect(rect);

    //set content size
    contentRect.setSize(size);

    SReportPageContentSettings settings;
    settings.overallRect = rect;
    settings.contentRect = contentRect;

    settings.color = m_color;
    settings.font = m_font;
    settings.scalingEnabled = m_scalingEnabled;

    //CReportPageContent content(contentSettings);
    m_content.push_back(new CReportPageContent(settings));

    //return index to content
    return (m_content.size() - 1);
}

int CReportPage::addRectangle(QSize size, double x, double y, eRectangleAlignmentPosition alignPosition)
{
    if (x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0)
        return -1;

    QRect contentRect(m_pageRect);
    contentRect.setSize(size);

    QPoint point(m_pageRect.left() + x * m_pageSize.width(), m_pageRect.top() + y* m_pageSize.height());

    QRect overallRect(contentRect);

    //margins
    overallRect.setHeight(qMin(int(overallRect.height() + m_contentMarginBottom), m_pageRect.height()));
    overallRect.setWidth(qMin(int(overallRect.width() + m_contentMarginRight), m_pageRect.width()));

    switch (alignPosition)
    {
    case ERAP_TOPLEFT:
        overallRect.moveTopLeft(point);
        break;
    case ERAP_TOPRIGHT:
        overallRect.moveTopRight(point);
        break;
    case ERAP_BOTTOMLEFT:
        overallRect.moveBottomLeft(point);
        break;
    case ERAP_BOTTOMRIGHT:
        overallRect.moveBottomRight(point);
        break;
    default:
        break;
    }
    contentRect.moveTopLeft(overallRect.topLeft());

    SReportPageContentSettings settings;
    settings.overallRect = overallRect;
    settings.contentRect = contentRect;

    settings.color = m_color;
    settings.font = m_font;
    settings.scalingEnabled = m_scalingEnabled;

    //CReportPageContent content(contentSettings);
    m_content.push_back(new CReportPageContent(settings));

    //return index to content
    return (m_content.size() - 1);
}

int CReportPage::addRectangle(QRect rect)
{
    QRect contentRect = rect;

    QSize overallSize = rect.size();

    //margins
    overallSize.setHeight(qMin(int(overallSize.height() + m_contentMarginBottom), m_pageRect.height()));
    overallSize.setWidth(qMin(int(overallSize.width() + m_contentMarginRight), m_pageRect.width()));

    SReportPageContentSettings contentSettings;
    contentSettings.overallRect = rect;
    contentSettings.contentRect = contentRect;

    contentSettings.color = m_color;
    contentSettings.font = m_font;
    contentSettings.scalingEnabled = m_scalingEnabled;

    //CReportPageContent content(contentSettings);
    m_content.push_back(new CReportPageContent (contentSettings));

    //return index to content
    return (m_content.size() - 1);
}

bool CReportPage::insertText(int index, QString text, SContentSettings settings)
{
    if (index < 0 || m_content.size() <= index)
        return false;

    QFont font = (settings.useFont ? settings.font : m_font);
    QColor color = (settings.useColor ? settings.color : m_color);

    SReportPageContentSettings contentSettings;
    contentSettings.color = color;
    contentSettings.contentType = ECT_TEXT;
    contentSettings.font = font;
    contentSettings.text = text;
    contentSettings.scalingEnabled = m_scalingEnabled;
    contentSettings.textOptions = settings.textOptions;
    contentSettings.overlay = settings.overlay;

    m_content.at(index)->setContent(contentSettings);

    return true;
}


bool CReportPage::insertTable(int index, CReportTable table, SContentSettings settings)
{
    if (index < 0 || m_content.size() <= index)
        return false;

    QFont font = (settings.useFont ? settings.font : m_font);
    QColor color = (settings.useColor ? settings.color : m_color);

    SReportPageContentSettings contentSettings;
    contentSettings.color = color;
    contentSettings.contentType = ECT_HTML;
    contentSettings.font = font;
    contentSettings.HTML = table.createHTMLTable();
    contentSettings.scalingEnabled = m_scalingEnabled;
    contentSettings.overlay = settings.overlay;

    m_content.at(index)->setContent(contentSettings);

    return true;

}

bool CReportPage::insertImage(int index, QImage img, SContentSettings settings)
{
    if (index < 0 || m_content.size() <= index)
        return false;

    QRect contentRect = m_content.at(index)->getContentRect();

    QRect imageRect(contentRect);

    QSize imageSize = img.size();

    if (settings.stretch)
    {
        imageSize = contentRect.size();
    }
    else
    {
        imageSize.scale(img.size() * settings.scaleFactor, settings.aspectRatioMode);
        //image cannot be bigger that assigned content space
        if (imageSize.width() > contentRect.width() || imageSize.height() > contentRect.height())
            imageSize = contentRect.size();

    }

    imageRect.setSize(imageSize);

    if (settings.centered)
        imageRect.moveCenter(contentRect.center());

    QRect captionRect(imageRect);



    QFont captionFont = (settings.useFont ? settings.font : m_font);
    QColor captionColor = (settings.useColor ? settings.color : m_color);

    if (!settings.caption.isEmpty())
    {
        QTextDocument td;
        td.setDefaultFont(captionFont);
        td.setPlainText(settings.caption);
        td.setDefaultTextOption(settings.textOptions);

        qreal textHeight = td.documentLayout()->documentSize().height() + 0.01*m_pageRect.size().height();

        switch (settings.captionPos)
        {

        case EICP_BOTTOM:
            if (settings.captionInsideImage)
            {
                captionRect.setTop(imageRect.bottom() - textHeight);
                //captionRect.setHeight(textHeight);
            }
            else
            {

                //captionRect.setTop(contentRect.bottom() - textHeight);
                //imageRect.setBottom(captionRect.top());
                captionRect.setTop(imageRect.bottom());
                //bottom is moved
                captionRect.setHeight(textHeight);
            }
            break;

        case EICP_TOP:
            if (settings.captionInsideImage)
            {
                captionRect.setHeight(textHeight);
            }
            else
            {
                captionRect.setTop(imageRect.top() - textHeight);
                //bottom is moved
                captionRect.setHeight(textHeight);

                //captionRect.setHeight(textHeight);
                //imageRect.setTop(captionRect.bottom());

            }
            break;

        default:
            break;
        }

        ////image with caption does not fit into page
        //if (captionRect.bottom() > m_pageRect.bottom())
        //	return false;
    }

    SReportPageContentSettings contentSettings;

    //contentSettings.contentRect = contentRect;
    contentSettings.imageRect = imageRect;
    contentSettings.captionRect = captionRect;

    contentSettings.contentType = ECT_IMG;
    contentSettings.img = img;
    contentSettings.imageScale = settings.scaleFactor;
    contentSettings.centered = settings.centered;
    contentSettings.stretchImage = settings.stretch;
    contentSettings.scalingEnabled = m_scalingEnabled;
    contentSettings.overlay = settings.overlay;

    //caption
    contentSettings.caption = settings.caption;
    contentSettings.color = captionColor;
    contentSettings.font = captionFont;
    contentSettings.textOptions = settings.textOptions;
    contentSettings.captionInsideImage = settings.captionInsideImage;
    contentSettings.aspectRatioMode = settings.aspectRatioMode;

    m_content.at(index)->setContent(contentSettings);


    return true;
}

bool CReportPage::addText(QString text, SContentSettings settings, bool fromBottom)
{
    QFont font = (settings.useFont ? settings.font : m_font);
    QColor color = (settings.useColor ? settings.color : m_color);

    QTextDocument td;
    td.setDefaultFont(font);

    // if text is not floating then use the whole page size
    if (!m_floatingContent)
        td.setPageSize(m_pageSize);

    td.setDefaultTextOption(settings.textOptions);
    td.setPlainText(text);

    QSizeF docSize = td.documentLayout()->documentSize();
    QSizeF overallSize = docSize;

    //margins
    overallSize.setHeight(overallSize.height() + m_contentMarginBottom);
    overallSize.setWidth(overallSize.width() + m_contentMarginRight);

    // if text is not floating then size should be whole page width, otherwise setting margins caused problems
    // overall size was bigger that page size
    if (!m_floatingContent)
    {
        overallSize.setWidth(m_pageSize.width());
    }

    //rectangle with all free space
    QRect rect = findFreeSpace(overallSize);

    //no free space was found
    if (rect.isEmpty())
        return false;

    //should rectangle be added in the bottom of the page?
    if (fromBottom)
    {
        QRect tmp(rect);

        //bottom is moved
        rect.setHeight(overallSize.height());

        rect.moveBottom(tmp.bottom());
    }
    else
    {
        //bottom is moved
        rect.setHeight(overallSize.height());
    }

    bool canFit = (m_pageRect.bottom() >= rect.bottom() && m_pageRect.right() >= rect.right());

    for (int i = 0; i < m_content.size(); ++i)
    {
        QRect contentRect = m_content.at(i)->getOverallRect();
        canFit = canFit && !(rect.intersects(contentRect));
    }

    //this rectable does not fit into this page
    if (!canFit)
        return false;

    //right is moved
    rect.setWidth(overallSize.width());

    // if content is not floating, image should be stretched to whole page width
    if (!m_floatingContent)
    {
        rect.setLeft(m_pageRect.left());
        rect.setRight(m_pageRect.right());
    }

    QRect contentRect(rect);

    //set content size
    contentRect.setWidth(docSize.width());
    contentRect.setHeight(docSize.height());

    SReportPageContentSettings contentSettings;
    contentSettings.overallRect = rect;
    contentSettings.contentRect = contentRect;

    contentSettings.text = text;
    contentSettings.contentType = ECT_TEXT;
    contentSettings.color = color;
    contentSettings.font = font;
    contentSettings.textOptions = settings.textOptions;
    contentSettings.scalingEnabled = m_scalingEnabled;
    contentSettings.overlay = settings.overlay;

    //CReportPageContent content(contentSettings);
    m_content.push_back(new CReportPageContent(contentSettings));

    return true;
}

bool CReportPage::addImage(QImage img, SContentSettings settings, bool fromBottom)
{
    QSize imageSize = img.size();
    imageSize.scale(img.size() * settings.scaleFactor, settings.aspectRatioMode);

    QSize contentSize = imageSize;

    qreal textHeight = 0.0;
    QFont captionFont = (settings.useFont ? settings.font : m_font);
    QColor captionColor = (settings.useColor ? settings.color : m_color);

    if (!settings.caption.isEmpty())
    {

        QTextDocument td;
        td.setDefaultFont(captionFont);
        td.setPlainText(settings.caption);
        td.setDefaultTextOption(settings.textOptions);

        textHeight = td.documentLayout()->documentSize().height() + 0.01*m_pageRect.size().height();
        int textWidth = td.documentLayout()->documentSize().width();
        //adjust needed sapce
        if (!settings.captionInsideImage)
        {
            contentSize.setHeight(contentSize.height() + textHeight);
            contentSize.setWidth(qMax(textWidth, contentSize.width()));
        }
    }

    QSize overallSize = contentSize;

    //margins
    overallSize.setHeight(overallSize.height() + m_contentMarginBottom);
    overallSize.setWidth(overallSize.width() + m_contentMarginRight);

    QRect rect = findFreeSpace(overallSize);

    if (rect.isEmpty())
        return false;

    //should rectangle be added in the bottom of the page?
    if (fromBottom)
    {
        QRect tmp(rect);
        rect.setSize(overallSize);
        rect.moveBottom(tmp.bottom());
    }
    else
    {
        //bottom is moved
        rect.setSize(overallSize);
    }

    bool canFit = (m_pageRect.bottom() >= rect.bottom() && m_pageRect.right() >= rect.right());

    for (int i = 0; i < m_content.size(); ++i)
    {
        QRect contentRect = m_content.at(i)->getOverallRect();
        canFit = canFit && !(rect.intersects(contentRect));
    }

    //this rectable does not fit into this page
    if (!canFit)
        return false;

    // if content is not floating, image should be stretched to whole page width
    if (!m_floatingContent)
    {
        rect.setLeft(m_pageRect.left());
        rect.setRight(m_pageRect.right());
    }

    QRect contentRect(rect);

    //set content size
    contentRect.setSize(contentSize);

    //stretch content rectangle to rect size, if floating content is on, it will be stretched to whole page width
    contentRect.setLeft(rect.left());
    contentRect.setRight(rect.right());

    QRect imageRect(contentRect);
    imageRect.setSize(imageSize);
    if (settings.stretch)
        imageRect.setSize(contentRect.size());

    if (settings.centered)
        imageRect.moveCenter(contentRect.center());

    QRect captionRect(imageRect);
    captionRect.setSize(contentSize);
    captionRect.setHeight(textHeight);
    captionRect.moveCenter(imageRect.center());

    if (!settings.caption.isEmpty())
    {
        switch (settings.captionPos)
        {

        case EICP_BOTTOM:
            if (settings.captionInsideImage)
            {
                captionRect.moveBottom(imageRect.bottom());
            }
            else
            {
                captionRect.moveTop(imageRect.bottom());

                //caption reaches outside assigned content rectangle -> need to adjust
                if (captionRect.bottom() > contentRect.bottom())
                {
                    captionRect.moveBottom(contentRect.bottom());
                    imageRect.moveBottom(captionRect.top());
                }
            }
            break;

        case EICP_TOP:
            if (settings.captionInsideImage)
            {
                captionRect.moveTop(imageRect.top());
            }
            else
            {
                captionRect.moveBottom(imageRect.top());

                //caption reaches outside assigned content rectangle -> need to adjust
                if(captionRect.top() < contentRect.top())
                {
                    captionRect.moveTop(contentRect.top());
                    imageRect.moveTop(captionRect.bottom());
                }
            }
            break;

        default:
            break;
        }
    }

    SReportPageContentSettings contentSettings;
    contentSettings.overallRect = rect;
    contentSettings.contentRect = contentRect;
    contentSettings.imageRect = imageRect;
    contentSettings.captionRect = captionRect;
    contentSettings.marginBottom = m_contentMarginBottom;
    contentSettings.marginRight = m_contentMarginRight;

    contentSettings.contentType = ECT_IMG;
    contentSettings.img = img;
    contentSettings.imageScale = settings.scaleFactor;
    contentSettings.centered = settings.centered;
    contentSettings.stretchImage = settings.stretch;
    contentSettings.scalingEnabled = m_scalingEnabled;
    contentSettings.overlay = settings.overlay;

    //caption
    contentSettings.caption = settings.caption;
    contentSettings.color = captionColor;
    contentSettings.font = captionFont;
    contentSettings.textOptions = settings.textOptions;
    contentSettings.captionInsideImage = settings.captionInsideImage;
    contentSettings.captionPos = settings.captionPos;
    contentSettings.aspectRatioMode = settings.aspectRatioMode;

    //CReportPageContent content(contentSettings);
    m_content.push_back(new CReportPageContent(contentSettings));

    return true;
}

bool CReportPage::addImagePhysical(QImage img, Qt::AspectRatioMode aspectRatioMode, SContentSettings settings)
{
    int imgWidth = img.width();
    int imgHeight = img.height();
    int dpmX = img.dotsPerMeterX();
    int dpmY = img.dotsPerMeterY();

    double imgWidthMM = 0.0;
    double imgHeightMM = 0.0;

    if (dpmX>0 && dpmY>0)
    {
        imgWidthMM = 1000 * imgWidth / (double)dpmX;
        imgHeightMM = 1000 * imgHeight / (double)dpmY;
    }
    //physical resolution is not defined in images, add unscaled image
    else
    {
        return addImage(img, settings);
    }

    //scale image acording to physical resolution of printer
    double imgPhysicalWidthInPixels = imgWidthMM * m_pageWidthPixelsPerMM;
    double imgPhysicalHeightInPixels = imgHeightMM * m_pageHeightPixelsPerMM;

    QImage scaledImg = img.scaled(imgPhysicalWidthInPixels, imgPhysicalHeightInPixels, aspectRatioMode);

    return addImage(scaledImg, settings);
}

bool CReportPage::addTable(CReportTable table, SContentSettings settings, bool fromBottom)
{

    QFont font = (settings.useFont ? settings.font : m_font);
    QColor color = (settings.useColor ? settings.color : m_color);

    QTextDocument td;
    td.setDefaultFont(font);

    // if text is not floating then use the whole page size
    // in addtition Qt HTML preprocessor can use page size, so that it shrinks HTML, but
    // does not change font
    if (!m_floatingContent)
        td.setPageSize(m_pageSize);

    QString htmlTalbe = table.createHTMLTable();
    td.setHtml(htmlTalbe);

    QSizeF docSize = td.documentLayout()->documentSize();
    QSizeF overallSize = td.documentLayout()->documentSize();

    //margins
    overallSize.setHeight(overallSize.height() + m_contentMarginBottom);
    overallSize.setWidth(overallSize.width() + m_contentMarginRight);

    // if text is not floating then size should be whole page width, otherwise setting margins caused problems
    // overall size was bigger that page size
    if (!m_floatingContent)
    {
        overallSize.setWidth(m_pageSize.width());
    }


    //rectangle with all free space
    QRect rect = findFreeSpace(overallSize);

    //no free space was found
    if (rect.isEmpty())
        return false;

    //bottom is moved
    rect.setHeight(overallSize.height());

    //right is moved
    rect.setWidth(overallSize.width());

    bool canFit = (m_pageRect.bottom() >= rect.bottom() && m_pageRect.right() >= rect.right());

    for (int i = 0; i < m_content.size(); ++i)
    {
        if (m_content.at(i)->m_overlay)
            continue;

        QRect contentRect = m_content.at(i)->getOverallRect();
        canFit = canFit && !(rect.intersects(contentRect));
    }

    //this rectable does not fit into this page
    if (!canFit)
        return false;

    QRect contentRect(rect);

    //set content size
    contentRect.setWidth(docSize.width());
    contentRect.setHeight(docSize.height());

    SReportPageContentSettings contentSettings;
    contentSettings.overallRect = rect;
    contentSettings.contentRect = contentRect;

    contentSettings.HTML = htmlTalbe;
    contentSettings.contentType = ECT_HTML;
    contentSettings.color = color;
    contentSettings.font = font;
    contentSettings.scalingEnabled = m_scalingEnabled;
    contentSettings.overlay = settings.overlay;

    //CReportPageContent content(contentSettings);
    m_content.push_back(new CReportPageContent(contentSettings));

    return true;
}

bool CReportPage::addHTML(QString html, SContentSettings settings, bool fromBottom)
{
    //TODO: checks
    QFont font = (settings.useFont ? settings.font : m_font);
    QColor color = (settings.useColor ? settings.color : m_color);

    QTextDocument td;
    td.setDefaultFont(font);

    // if text is not floating then use the whole page size
    // in addtition Qt HTML preprocessor can use page size, so that it shrinks HTML, but
    // does not change font
    if (!m_floatingContent)
        td.setPageSize(m_pageSize);

    td.setHtml(html);

    QSizeF docSize = td.documentLayout()->documentSize();
    QSizeF overallSize = docSize;

    //margins
    overallSize.setHeight(overallSize.height() + m_contentMarginBottom);
    overallSize.setWidth(overallSize.width() + m_contentMarginRight);

    // if text is not floating then size should be whole page width, otherwise setting margins caused problems
    // overall size was bigger that page size
    if (!m_floatingContent)
    {
        overallSize.setWidth(m_pageSize.width());
    }

    //rectangle with all free space
    QRect rect = findFreeSpace(overallSize);

    //no free space was found
    if (rect.isEmpty())
        return false;

    //should rectangle be added in the bottom of the page?
    if (fromBottom)
    {
        QRect tmp(rect);

        //bottom is moved
        rect.setHeight(overallSize.height());

        rect.moveBottom(tmp.bottom());
    }
    else
    {
        //bottom is moved
        rect.setHeight(overallSize.height());
    }

    bool canFit = (m_pageRect.bottom() >= rect.bottom() && m_pageRect.right() >= rect.right());

    for (int i = 0; i < m_content.size(); ++i)
    {
        if (m_content.at(i)->m_overlay)
            continue;

        QRect contentRect = m_content.at(i)->getOverallRect();
        canFit = canFit && !(rect.intersects(contentRect));
    }

    //this rectable does not fit into this page
    if (!canFit)
        return false;

    QRect contentRect(rect);

    //set content size
    contentRect.setWidth(docSize.width());
    contentRect.setHeight(docSize.height());

    SReportPageContentSettings contentSettings;
    contentSettings.overallRect = rect;
    contentSettings.contentRect = contentRect;

    contentSettings.HTML = html;
    contentSettings.contentType = ECT_HTML;
    contentSettings.color = color;
    contentSettings.font = font;
    contentSettings.scalingEnabled = m_scalingEnabled;
    contentSettings.overlay = settings.overlay;

    //CReportPageContent content(contentSettings);
    m_content.push_back(new CReportPageContent (contentSettings));

    return true;
}

void CReportPage::setFont(QFont font)
{
    m_font = font;
}

QFont CReportPage::getFont()
{
    return m_font;
}

void CReportPage::setColor(QColor color)
{
    m_color = color;
}

void CReportPage::setColor(Qt::GlobalColor color)
{
    m_color = QColor(color);
}

QColor CReportPage::getColor()
{
    return m_color;
}

void CReportPage::setFloatingContent(bool b)
{
    m_floatingContent = b;
}

bool CReportPage::getFloatingContent()
{
    return m_floatingContent;
}

void CReportPage::setScalingEnabled(bool b)
{
    m_scalingEnabled = b;
}

bool CReportPage::getScalingEnabled()
{
    return m_scalingEnabled;
}

QSize CReportPage::getPageSize()
{
    return m_pageSize;
}

void CReportPage::setMargins(double left, double right, double top, double bottom)
{
    if (left < 0.0 || left > 1.0 || right < 0.0 || right > 1.0 ||
        top < 0.0 || top > 1.0 || bottom < 0.0 || bottom > 1.0)
        return;

    m_pageSize = m_pageRect.size();
    //page rect margin
    double pageLeftMargin = m_pageSize.width()*left;
    double pageRightMargin = m_pageSize.width()*right;
    double pageTopMargin = m_pageSize.height()*top;
    double pageBottomMargin = m_pageSize.height()*bottom;

    m_pageRect.setLeft(m_pageRect.left() + pageLeftMargin);
    m_pageRect.setRight(m_pageRect.right() - pageRightMargin);
    m_pageRect.setTop(m_pageRect.top() + pageTopMargin);
    m_pageRect.setBottom(m_pageRect.bottom() - pageBottomMargin);

    m_pageSize = m_pageRect.size();
}

void CReportPage::setContentMargins(double right, double bottom)
{
    if (right < 0.0 || right > 1.0 || right < 0.0 || right > 1.0)
        return;

    m_contentMarginBottom = bottom * m_pageSize.height();
    m_contentMarginRight = right * m_pageSize.width();
}

bool CReportPage::canFitContentWithoutScaling(QString text, QFont font)
{
    QTextDocument td;
    td.setDefaultFont(font);

    // if text is not floating then use the whole page size
    if (!m_floatingContent)
        td.setPageSize(m_pageSize);

    td.setPlainText(text);

    QSizeF overallSize = td.documentLayout()->documentSize();

    //margins
    overallSize.setHeight(overallSize.height() + m_contentMarginBottom);
    overallSize.setWidth(overallSize.width() + m_contentMarginRight);

    //disabled scaling
    bool scalingEnabled = m_scalingEnabled;
    m_scalingEnabled = false;

    //rectangle with all free space
    QRect rect = findFreeSpace(overallSize);

    //set previous scaling value
    m_scalingEnabled = scalingEnabled;

    //no free space was found
    if (rect.isEmpty())
        return false;

    //bottom is moved
    rect.setHeight(overallSize.height());

    //right is moved
    rect.setWidth(overallSize.width());

    return !(rect.bottom() > m_pageRect.bottom());
}

bool CReportPage::canFitContentWithoutScaling(QString text)
{
    return canFitContentWithoutScaling(text, m_font);
}

bool CReportPage::canFitContentWithoutScaling(CReportTable table)
{
    return canFitContentWithoutScaling(table, m_font);
}

bool CReportPage::canFitContentWithoutScaling(CReportTable table, QFont font)
{
    QTextDocument td;
    td.setDefaultFont(font);

    // if text is not floating then use the whole page size
    if (!m_floatingContent)
        td.setPageSize(m_pageSize);

    QString htmlTalbe = table.createHTMLTable();
    td.setHtml(htmlTalbe);

    QSizeF overallSize = td.documentLayout()->documentSize();

    //margins
    overallSize.setHeight(overallSize.height() + m_contentMarginBottom);
    overallSize.setWidth(overallSize.width() + m_contentMarginRight);

    //disabled scaling
    bool scalingEnabled = m_scalingEnabled;
    m_scalingEnabled = false;

    //rectangle with all free space
    QRect rect = findFreeSpace(overallSize);

    //set previous scaling value
    m_scalingEnabled = scalingEnabled;

    //no free space was found
    if (rect.isEmpty())
        return false;

    //bottom is moved
    rect.setHeight(overallSize.height());

    //right is moved
    rect.setWidth(overallSize.width());

    return !(rect.bottom() > m_pageRect.bottom());
}

bool CReportPage::canFitContentWithoutScaling(QImage img, SContentSettings settings)
{
    QSize spaceSize = img.size() * settings.scaleFactor;
    qreal textHeight = 0.0;
    QFont captionFont = (settings.useFont ? settings.font : m_font);
    QColor captionColor = (settings.useColor ? settings.color : m_color);

    QSize overallSize = spaceSize;

    if (!settings.caption.isEmpty())
    {

        QTextDocument td;
        td.setDefaultFont(captionFont);
        td.setPlainText(settings.caption);
        td.setDefaultTextOption(settings.textOptions);

        textHeight = td.documentLayout()->documentSize().height() + 0.01*m_pageRect.size().height();

        //adjust needed sapce
        if (!settings.captionInsideImage)
            overallSize.setHeight(overallSize.height() + textHeight);
    }

    //margins
    overallSize.setHeight(overallSize.height() + m_contentMarginBottom);
    overallSize.setWidth(overallSize.width() + m_contentMarginRight);

    //disabled scaling
    bool scalingEnabled = m_scalingEnabled;
    m_scalingEnabled = false;

    QRect rect = findFreeSpace(overallSize);

    //set previous scaling value
    m_scalingEnabled = scalingEnabled;

    //no free space was found
    if (rect.isEmpty())
        return false;

    rect.setSize(overallSize);

    bool canFit = (m_pageRect.bottom() >= rect.bottom() && m_pageRect.right() >= rect.right());

    for (int i = 0; i < m_content.size(); ++i)
    {
        QRect contentRect = m_content.at(i)->getOverallRect();
        canFit = canFit && !(rect.intersects(contentRect));
    }

    return canFit;
}

bool CReportPage::canFitContentWithoutScaling(QImage img, QRect rect, SContentSettings settings)
{
    QSize spaceSize = img.size() * settings.scaleFactor;
    qreal textHeight = 0.0;
    QFont captionFont = (settings.useFont ? settings.font : m_font);
    QColor captionColor = (settings.useColor ? settings.color : m_color);

    QSize overallSize = spaceSize;

    if (!settings.caption.isEmpty())
    {

        QTextDocument td;
        td.setDefaultFont(captionFont);
        td.setPlainText(settings.caption);
        td.setDefaultTextOption(settings.textOptions);

        textHeight = td.documentLayout()->documentSize().height() + 0.01*m_pageRect.size().height();

        //adjust needed sapce
        if (!settings.captionInsideImage)
            overallSize.setHeight(overallSize.height() + textHeight);
    }

    //margins
    overallSize.setHeight(overallSize.height() + m_contentMarginBottom);
    overallSize.setWidth(overallSize.width() + m_contentMarginRight);

    return overallSize.width() < rect.width() && overallSize.height() < rect.height();
}

bool CReportPage::isEmpty()
{
    return m_content.empty();
}

int CReportPage::getContentCount()
{
    return m_content.size();
}

CReportPageContent * CReportPage::getContent(int index)
{
    if (index >= m_content.size() || index < 0)
        return nullptr;

    return (m_content.at(index));
}

QVector<QRect> CReportPage::getFreeSpace()
{
    //if (m_content.empty())
    //    return m_pageRect;

    //found free space -> initialized to whole page
    QRegion freeSpace(m_pageRect);

    //iterate through all content rectangles and substract them from free space
    for (int i = 0; i < m_content.size(); ++i)
    {
        QRect rect = m_content.at(i)->getOverallRect();

        //substraction
        QRegion tmpRegion = freeSpace.subtracted(rect);

        freeSpace = tmpRegion;
    }


    return freeSpace.rects();
}

QRect CReportPage::getBiggestFreeRectangle()
{
    //get remaining free space
    QVector<QRect> freeSpaces = getFreeSpace();
    //should not happen, but just to be sure
    if (freeSpaces.empty())
    {
        return QRect();
    }

    //largest free space
    QRect freeSpace = QRect();

    for (int i = 0; i < freeSpaces.size(); ++i)
    {
        QRect tmp = freeSpaces.at(i);
        if (freeSpace.size().width() < tmp.size().width() && freeSpace.size().height() < tmp.size().height())
            freeSpace = tmp;
    }

    return freeSpace;
}

QImage CReportPage::getScaledImage(QImage image, QSize desiredSize, Qt::AspectRatioMode aspectRatioMode, SContentSettings settings)
{
    QSize imageSize = desiredSize;
    qreal textHeight = 0.0;
    QFont captionFont = (settings.useFont ? settings.font : m_font);
    QColor captionColor = (settings.useColor ? settings.color : m_color);

    if (!settings.caption.isEmpty())
    {
        QTextDocument td;
        td.setDefaultFont(captionFont);
        td.setPlainText(settings.caption);
        td.setDefaultTextOption(settings.textOptions);

        textHeight = td.documentLayout()->documentSize().height() + 0.01*m_pageRect.size().height();

        //adjust needed sapce
        if (!settings.captionInsideImage)
            imageSize.setHeight(imageSize.height() - textHeight);
    }

    //margins
    imageSize.setHeight(imageSize.height() - m_contentMarginBottom);
    imageSize.setWidth(imageSize.width() - m_contentMarginRight);

    QImage scaledImage = image.scaled(imageSize, aspectRatioMode);

    return scaledImage;
}

QSize CReportPage::getOverallSizeNeeded(QImage image, SContentSettings settings)
{
    QSize imageSize = image.size() * settings.scaleFactor;
    QSize contentSize = imageSize;

    qreal textHeight = 0.0;
    QFont captionFont = (settings.useFont ? settings.font : m_font);
    QColor captionColor = (settings.useColor ? settings.color : m_color);

    if (!settings.caption.isEmpty())
    {

        QTextDocument td;
        td.setDefaultFont(captionFont);
        td.setPlainText(settings.caption);
        td.setDefaultTextOption(settings.textOptions);

        textHeight = td.documentLayout()->documentSize().height() + 0.01*m_pageRect.size().height();

        //adjust needed sapce
        if (!settings.captionInsideImage)
            contentSize.setHeight(contentSize.height() + textHeight);
    }

    QSize overallSize = contentSize;

    //margins
    overallSize.setHeight(overallSize.height() + m_contentMarginBottom);
    overallSize.setWidth(overallSize.width() + m_contentMarginRight);

    return overallSize;
}

CImageGrid* CReportPage::createImageGrid(int rows, int cols)
{
    SReportPageSettings settings;
    settings.pageRect = m_pageRect;
    settings.font = m_font;
    settings.contentMarginBottom = m_contentMarginBottom;
    settings.contentMarginRight = m_contentMarginRight;

    CImageGrid* ptr = new CImageGrid(rows, cols, getBiggestFreeRectangle(), settings);
    m_content.push_back(ptr);

    return ptr;

}

void CReportPage::print(QPainter *painter)
{
    for (int i = 0; i < m_content.size(); ++i)
    {
            m_content.at(i)->print(painter);
    }

}

QRect CReportPage::findFreeSpace(QSizeF desiredSize)
{
    if (m_content.empty())
        return m_pageRect;

    //found free space -> initialized to whole page
    QRegion freeSpace(m_pageRect);

    //iterate through all content rectangles and substract them from free space
    for (int i = 0; i < m_content.size(); ++i)
    {
        //ignore overlay
        if (m_content.at(i)->m_overlay)
            continue;

        QRect rect = m_content.at(i)->getOverallRect();

        //stretch on whole page size, just to be sure
        //this prohibits inserting content next to rectangles with smaller width than page width
        //in other words, free space will be always found UNDER current rectangle, not next to it
        if (!m_floatingContent)
        {
            rect.setLeft(m_pageRect.left());
            rect.setRight(m_pageRect.right());
        }


        //substraction
        QRegion tmpRegion = freeSpace.subtracted(rect);

        freeSpace = tmpRegion;
    }

    QRect biggestRect;
    //iterate through all free rectangles on page and return the first suitable
    QVector<QRect> rects = freeSpace.rects();

    for (int i = 0; i < rects.size(); ++i)
    {
        QRect rect = rects.at(i);

        //apply margins
        //rect.setBottom(rect.bottom() - m_contentMarginBottom);
        //rect.setRight(rect.right() - m_contentMarginRight);

        //this rectable reaches outside page rectangle -> dont consider it
        if (rect.bottom() > m_pageRect.bottom())
            continue;

        //rect with desired size found
        if (rect.size().height() >= desiredSize.height() && rect.size().width() >= desiredSize.width())
        {
            return rect;
        }

        if (biggestRect.width() < rect.width() || biggestRect.height() < rect.height())
            biggestRect = rect;
    }

    //if no rectangle big enough was found, but scaling is enabled, return the biggest one
    if (m_scalingEnabled)
        return biggestRect;


    //return empty rect, if no suitable rect was found
    return QRect();
}