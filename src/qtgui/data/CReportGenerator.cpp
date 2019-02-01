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
#include "CReportGenerator.h"
#include "CReportPage.h"
#include "CReportTable.h"
#else
#include "data/CReportGenerator.h"
#include "data/CReportPage.h"
#include "data/CReportTable.h"
#include "data/CImageGrid.h"
#endif

#include <QPrinter>
#include <QMessageBox>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

//TEST
#include <QDebug>
//TODO: comments & clean up



CReportGenerator::CReportGenerator(QPrinter& printer)
    : m_printer(printer)
    , m_contentMarginBottom(0.0)
    , m_contentMarginRight(0.0)
    , m_floatingContent(false)
    , m_pageNumberingOn(false)
    , m_scalingEnabled(true)
{
    m_printer.setOutputFormat(QPrinter::PdfFormat);

    //setting default font: nasty trick
    QPainter tmpPainter;
    tmpPainter.begin(&m_printer);
    m_font = tmpPainter.font();
    m_font.setPointSize(14);
    tmpPainter.end();

    //no margins by default
    m_printer.setPageMargins(0, 0, 0, 0, QPrinter::Point);
    //default page rect
    m_pageRect = m_printer.pageRect();

    QRectF pageRectMM = m_printer.pageRect(QPrinter::Millimeter);

    int pageHeight = m_pageRect.height();
    int pageWidth = m_pageRect.width();

    double pageHeightMM = pageRectMM.height();
    double pageWidthMM = pageRectMM.width();

    m_pageHeightPixelsPerMM = (double)pageHeight / pageHeightMM;
    m_pageWidthPixelsPerMM = (double)pageWidth / pageWidthMM;

}



CReportGenerator::~CReportGenerator()
{
}

CReportPage& CReportGenerator::newPage()
{  
    if (m_usePageTemplate)
    {
        m_pages.push_back(CReportPage(m_templatePage));
    }
    else
    {
        SReportPageSettings settings;
        settings.contentMarginBottom = m_contentMarginBottom;
        settings.contentMarginRight = m_contentMarginRight;
        settings.pageRect = m_pageRect;
        settings.font = m_font;
        settings.floatingContent = m_floatingContent;
        settings.color = m_color;
        settings.scalingEnabled = m_scalingEnabled;
        settings.pageHeightPixelsPerMM = m_pageHeightPixelsPerMM;
        settings.pageWidthPixelsPerMM = m_pageWidthPixelsPerMM;

        m_pages.push_back(CReportPage (settings));
    }


    return (m_pages.back());
}

CReportPage CReportGenerator::createPage()
{

    SReportPageSettings settings;
    settings.contentMarginBottom = m_contentMarginBottom;
    settings.contentMarginRight = m_contentMarginRight;
    settings.pageRect = m_pageRect;
    settings.font = m_font;
    settings.floatingContent = m_floatingContent;
    settings.color = m_color;
    settings.scalingEnabled = m_scalingEnabled;
    settings.pageHeightPixelsPerMM = m_pageHeightPixelsPerMM;
    settings.pageWidthPixelsPerMM = m_pageWidthPixelsPerMM;

    return CReportPage(settings);
}

void CReportGenerator::addPage(CReportPage& page)
{
    m_pages.push_back(page);
}

void CReportGenerator::removePage(int index)
{
    if (index >= m_pages.size() || index < 0)
        return;

    m_pages.erase(m_pages.begin() + index);
}

void CReportGenerator::setPageTemplate(CReportPage& page)
{
    m_templatePage =  page;
    m_usePageTemplate = true;
}

void CReportGenerator::setPage(int index, CReportPage& page)
{
    if (index >= m_pages.size() || index < 0)
        return;

    m_pages.insert(m_pages.begin() + index, page);
}

int CReportGenerator::getPageCount()
{
    return m_pages.size();
}

CReportPage& CReportGenerator::getPage(int index)
{
    assert(index >= m_pages.size() || index < 0);
    
    return (m_pages.at(index));
}

CReportPage& CReportGenerator::getLastPage()
{
    if (m_pages.empty())
        newPage();

    return m_pages.back();
}

void CReportGenerator::setPageNumberingOn(bool b, QTextOption options)
{
    setPageNumberingOn(b, m_font, options);
}

void CReportGenerator::setPageNumberingOn(bool b, QFont font, QTextOption options)
{
    m_pageNumberingOn = b;
    QRect rect = m_pageNumberingContent.getOverallRect();;

    //turn page numbering on
    if (b)
    {
        QString pageNumberString = QString::number(0);
        QSize pageSize = m_pageRect.size();
        QTextDocument td;
        td.setDefaultFont(font);
        td.setPageSize(pageSize);
        td.setPlainText(pageNumberString);

        //create page numbering rectangle
        rect = m_pageRect;
        rect.setTop(rect.bottom() - (td.documentLayout()->documentSize().height()));

        SReportPageContentSettings contentSettings;
        contentSettings.contentRect = rect;
        contentSettings.text = pageNumberString;
        contentSettings.contentType = ECT_TEXT;
        contentSettings.textOptions = options;
        contentSettings.font = font;
        m_pageNumberingContent.setContent(contentSettings);

        //adjust page rect
        m_pageRect.setBottom(rect.top());

    }
    //turn page numbering off
    else
    {
        //some page numbering rect was already defined
        if (!rect.isEmpty())
        {
            //adjust page rect
            m_pageRect.setBottom(rect.bottom());
        }
        //set page numbering rect to empty
        m_pageNumberingContent.setOverallRect(QRect());
        m_pageNumberingContent.setContentRect(QRect());

        return;
    }
}


bool CReportGenerator::getPageNumberingOn()
{
    return m_pageNumberingOn;
}

void CReportGenerator::setScalingEnabled(bool b)
{
    m_scalingEnabled = b;
}

bool CReportGenerator::getScalingEnabled()
{
    return m_scalingEnabled;
}

void CReportGenerator::setOutputFileName(QString fn)
{
    m_outputFileName = fn;
}

QString CReportGenerator::getOutpuFileName()
{
    return m_outputFileName;
}

void CReportGenerator::setFont(QFont font)
{
    m_font = font;

    if (!m_pages.empty())
    {
        CReportPage& lastPage = getLastPage();
        lastPage.setFont(m_font);
    }

}

QFont CReportGenerator::getFont()
{
    return m_font;
}

void CReportGenerator::setColor(QColor color)
{
    m_color = color;

    if (!m_pages.empty())
    {
        CReportPage& lastPage = getLastPage();
        lastPage.setColor(m_color);
    }
}

void CReportGenerator::setColor(Qt::GlobalColor color)
{
    m_color = QColor(color);

    if (!m_pages.empty())
    {
        CReportPage& lastPage = getLastPage();
        lastPage.setColor(m_color);
    }

}

QColor CReportGenerator::getColor()
{
    return m_color;
}

void CReportGenerator::setFloatingContent(bool b)
{
    m_floatingContent = b;
}

bool CReportGenerator::getFloatingContent()
{
    return m_floatingContent;
}

void CReportGenerator::setMargins(double left, double right, double top, double bottom)
{
    if (left < 0.0 || left > 1.0 || right < 0.0 || right > 1.0 ||
        top < 0.0 || top > 1.0 || bottom < 0.0 || bottom > 1.0)
        return;

    QSize pageSize = m_pageRect.size();
    //page rect margin
    double pageLeftMargin = pageSize.width()*left;
    double pageRightMargin = pageSize.width()*right;
    double pageTopMargin = pageSize.height()*top;
    double pageBottomMargin = pageSize.height()*bottom;

    m_pageRect.setLeft(m_pageRect.left() + pageLeftMargin);
    m_pageRect.setRight(m_pageRect.right() - pageRightMargin);
    m_pageRect.setTop(m_pageRect.top() + pageTopMargin);
    m_pageRect.setBottom(m_pageRect.bottom() - pageBottomMargin);
}

void CReportGenerator::setContentMargins(double right, double bottom)
{
    if (right < 0.0 || right > 1.0 || right < 0.0 || right > 1.0)
        return;

    QSize pageSize = m_pageRect.size();
    m_contentMarginBottom = bottom * pageSize.height();
    m_contentMarginRight = right * pageSize.width();
}

bool CReportGenerator::addText(QString text, SContentSettings settings, bool fromBottom)
{
    CReportPage& page = getLastPage();

    if (page.isEmpty())
    {
        return page.addText(text, settings, fromBottom);
    }

    if (!page.addText(text, settings, fromBottom))
    {
        CReportPage& newLastPage = newPage();
        if (!newLastPage.addText(text, settings, fromBottom))
        {
            removePage(m_pages.size() - 1);
            return false;
        }

    }

    return true;
}

bool CReportGenerator::addImage(QImage img, SContentSettings settings)
{
    CReportPage& page = getLastPage();

    if (page.isEmpty())
    {
        
        return page.addImage(img, settings);
    }

    if (!page.addImage(img, settings))
    {
        CReportPage& newLastPage = newPage();
        if (!newLastPage.addImage(img, settings))
        {
            removePage(m_pages.size() - 1);
            return false;
        }
    }

    return true;
}

bool CReportGenerator::addImagePhysical(QImage img, Qt::AspectRatioMode aspectRatioMode, SContentSettings settings)
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

bool CReportGenerator::addTable(CReportTable table, SContentSettings settings)
{
    CReportPage& page = getLastPage();

    if (page.isEmpty())
    {
        return page.addTable(table, settings);
    }

    if (!page.addTable(table, settings))
    {
        
        CReportPage& newLastPage = newPage();
        if(!newLastPage.addTable(table, settings))
        {
            removePage(m_pages.size() - 1);
            return false;
        }
    }

    return true;

}

bool CReportGenerator::addHTML(QString html, SContentSettings settings)
{
    CReportPage& page = getLastPage();

    if (page.isEmpty())
    {
        return page.addHTML(html, settings);
    }

    if (!page.addHTML(html, settings))
    {
        CReportPage& newLastPage = newPage();
        if(!newLastPage.addHTML(html, settings))
        {
            removePage(m_pages.size() - 1);
            return false;
        }
    }

    return true;

}

bool CReportGenerator::addImageGrid(CImageGrid& grid)
{
    CReportPage& page = getLastPage();

    grid.prepare();

    if (page.isEmpty())
    {
        return page.addImageGrid(grid);
    }

    if (!page.addImageGrid(grid))
    {
        CReportPage& newLastPage = newPage();
        if (!newLastPage.addImageGrid(grid))
        {
            removePage(m_pages.size() - 1);
            return false;
        }
    }

    return true;
}

QPrinter& CReportGenerator::getPrinter()
{
    return m_printer;
}

bool CReportGenerator::print()
{
    QPainter painter;
    if (!painter.begin(&m_printer))
        return false;

    for (int i = 0; i < m_pages.size(); ++i)
    {
        m_pages.at(i).print(painter);
        

        if (m_pageNumberingOn)
        {
            m_pageNumberingContent.setText(QString::number(i + 1));
            m_pageNumberingContent.print(painter);
        }

        //don't add last new page
        if(i < m_pages.size() -1)
            m_printer.newPage();
    }

    painter.end();
    //delete painter;

    return true;
}

QSize CReportGenerator::getPageSize()
{
    return m_pageRect.size();
}

QSize CReportGenerator::getPageSizeInMM()
{
    return QSize(m_pageRect.width() / m_pageWidthPixelsPerMM, m_pageRect.height() / m_pageHeightPixelsPerMM);
}

void CReportGenerator::getPixelsPerMM(double* width, double* height)
{
    *width = m_pageWidthPixelsPerMM;
    *height = m_pageHeightPixelsPerMM;
}

QImage CReportGenerator::getScaledImage(QImage image, QSize desiredSize, Qt::AspectRatioMode aspectRatioMode, SContentSettings settings)
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

    imageSize.setHeight(imageSize.height() - m_contentMarginBottom);
    imageSize.setWidth(imageSize.width() - m_contentMarginRight);


    QImage scaledImage = image.scaled(imageSize, aspectRatioMode);

    return scaledImage;
}

CImageGrid CReportGenerator::createImageGrid(int rows, int cols)
{
    CReportPage& page = getLastPage();

    return page.createImageGrid(rows, cols);
}

