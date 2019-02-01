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
#include "CImageGrid.h"
#else
#include "data/CImageGrid.h"
#endif

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QDebug>

//TODO: pass page config?
CImageGrid::CImageGrid(int rows, int cols, QRect rect, SReportPageSettings pageSettings)
    : m_rowsCount(rows)
    , m_colsCount(cols)
    , m_size(rect.size())
    , m_pageRect(pageSettings.pageRect)
    , m_font(pageSettings.font)
    , m_rect(rect)
    , m_useMinimumHeight(true)

{
    m_marginBottom = pageSettings.contentMarginBottom;
    m_marginRight = pageSettings.contentMarginRight;
    m_overallRect = rect;
    m_freeSpace = m_overallRect.size();
}


bool CImageGrid::addImage(QImage img, double maxScale, double minScale, SContentSettings settings)
{
    //grid is full
    if (m_rows.size() >= m_rowsCount + 1 && m_rowsCount != -1)
        return false;

    QSize imageSize = img.size();
    imageSize.scale(img.size() * settings.scaleFactor, settings.aspectRatioMode);

    QSize imageSizeMin = img.size();
    imageSizeMin.scale(img.size() *minScale, settings.aspectRatioMode);

    //compute content size
    QSize contentSizeMin = imageSizeMin;
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
            contentSizeMin.setHeight(contentSizeMin.height() + textHeight);
            contentSizeMin.setWidth(qMax(textWidth, contentSizeMin.width()));

            contentSize.setHeight(contentSize.height() + textHeight);
            contentSize.setWidth(qMax(textWidth, contentSize.width()));

        }
    }

    QSize overallSizeMin = contentSizeMin;
    QSize overallSize = contentSize;

    //margins
    overallSizeMin.setHeight(overallSizeMin.height() + m_marginBottom);
    overallSizeMin.setWidth(overallSizeMin.width() + m_marginRight);
    overallSize.setHeight(overallSize.height() + m_marginBottom);
    overallSize.setWidth(overallSize.width() + m_marginRight);

    if (m_rows.empty())
        m_rows.resize(1);
    
    //can fit in current row
    auto& row = m_rows.last();

    QSize rowSizeMin; 
    rowSizeMin.setHeight(0);
    rowSizeMin.setWidth(0);
    
    for (int i = 0; i < row.size(); ++i)
    {
        rowSizeMin.setWidth(rowSizeMin.width() + row.at(i).overallSizeMin.width());
        if (row.at(i).overallSizeMin.height() > rowSizeMin.height())
            rowSizeMin.setHeight(row.at(i).overallSizeMin.height());
    }
    
    QSize newSize = rowSizeMin;
    newSize.setWidth(newSize.width() + overallSizeMin.width());
    if (newSize.height() < contentSizeMin.height())
        newSize.setHeight(contentSizeMin.height());
    
    //does not fit on this row, try to add new row and add this image on new row
    if (newSize.width() > m_overallRect.width() )
    {
        //cannot add new row
        if (m_rows.size() >= m_rowsCount && m_rowsCount != -1)
            return false;

        m_freeSpace.setHeight(m_freeSpace.height() - rowSizeMin.height());
        m_rows.push_back(QVector<SImageInfo>());
        return this->addImage(img, maxScale, minScale, settings);
    }
    // it does not fit in this grid anymore
    else if (newSize.height() > m_freeSpace.height())
    {
        return false;
    }
    else
    {
        SImageInfo imageInfo;
        imageInfo.img = img;    
        imageInfo.overallSizeMin =  overallSizeMin;    
        imageInfo.imageSizeMin = imageSizeMin;
        imageInfo.settings = settings;
        imageInfo.maxScale = maxScale;
        imageInfo.minScale = minScale;
        imageInfo.overallSize = overallSize;

        row.push_back(imageInfo);    
    }

    //add new row if needed
    if (row.size() >= m_colsCount && m_colsCount != -1)
    {
        m_freeSpace.setHeight(m_freeSpace.height() - rowSizeMin.height());
        m_rows.push_back(QVector<SImageInfo>());
    }

    return true;
}

void CImageGrid::newRow()
{
    if (!m_rows.empty())
    {
        auto& row = m_rows.last();
        QSize rowSizeMin;
        rowSizeMin.setHeight(0);
        rowSizeMin.setWidth(0);

        for (int i = 0; i < row.size(); ++i)
        {
            rowSizeMin.setWidth(rowSizeMin.width() + row.at(i).overallSizeMin.width());
            if (row.at(i).overallSizeMin.height() > rowSizeMin.height())
                rowSizeMin.setHeight(row.at(i).overallSizeMin.height());
        }

        m_freeSpace.setHeight(m_freeSpace.height() - rowSizeMin.height());
    }

    m_rows.push_back(QVector<SImageInfo>());
}


void CImageGrid::print(QPainter& painter)
{
    prepare();
    //painter->drawRect(m_overallRect);
    for (int i = 0; i < m_rows.size(); ++i)
    {
        for (int j = 0; j < m_rows[i].size(); ++j)
        {
            m_rows[i][j].content.print(painter);
        }
    }
}

void CImageGrid::prepare()
{
    //find height scale factor
    int availableHeight = m_overallRect.height();
    int contentHeight = 0;
    for (int i = 0; i < m_rows.size(); ++i)
    {    
        int biggestRowHeight = 0;
        int biggestRowImageHeight = 0;
    
        for (int j = 0; j < m_rows[i].size(); ++j)
        {
            if (m_rows[i][j].overallSize.height() > biggestRowHeight)
            {
                biggestRowHeight = m_rows[i][j].overallSize.height();
                biggestRowImageHeight = m_rows[i][j].img.size().height();
    
            }
        }
    
        int fixedRowHeight = biggestRowHeight - biggestRowImageHeight;
        availableHeight -= fixedRowHeight;
        contentHeight += biggestRowImageHeight;
    }
    
    double heightScaleFactor = ((double)availableHeight / (double)contentHeight) ;
    
    //for each row find width scale factor
    int nextTop = m_overallRect.top();
    for (int i = 0; i < m_rows.size(); ++i)
    {
        int rowOverallWidth = 0;
        int rowImageWidth = 0;
    
        for (int j = 0; j < m_rows[i].size(); ++j)
        {
            rowOverallWidth += m_rows[i][j].overallSize.width();
            rowImageWidth += m_rows[i][j].img.size().width();
        }
    
        int fixedRowWidth = rowOverallWidth - rowImageWidth;
        int availableWidth = m_overallRect.width();
        availableWidth -= fixedRowWidth;
    
        double widthScaleFactor = ((double)availableWidth / (double)rowImageWidth) ;
    
        double commonScaleFactor = qMin(widthScaleFactor, heightScaleFactor);
    
        //create content
        int nextLeft = m_overallRect.left();
        int biggestHeight = 0;
        for (int j = 0; j < m_rows[i].size(); ++j)
        {
            double scaleFactor;

            if (commonScaleFactor > m_rows[i][j].maxScale)
                scaleFactor = m_rows[i][j].maxScale;
            else if (commonScaleFactor < m_rows[i][j].minScale)
                scaleFactor = m_rows[i][j].minScale;
            else
                scaleFactor = commonScaleFactor;

            SContentSettings settings = m_rows[i][j].settings;
            QSize imageSize = m_rows[i][j].img.size();
            imageSize.scale(imageSize * scaleFactor, settings.aspectRatioMode);

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
            overallSize.setHeight(overallSize.height() + m_marginBottom);
            overallSize.setWidth(overallSize.width() + m_marginRight);

            QRect overallRect(QPoint(nextLeft, nextTop), overallSize);
            QRect contentRect(overallRect);

            //set content size
            contentRect.setSize(contentSize);

            QRect imageRect(contentRect);
            imageRect.setSize(imageSize);
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
                        if (captionRect.top() < contentRect.top())
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
            contentSettings.overallRect = overallRect;
            contentSettings.contentRect = contentRect;
            contentSettings.imageRect = imageRect;
            contentSettings.captionRect = captionRect;
            contentSettings.marginBottom = m_marginBottom;
            contentSettings.marginRight = m_marginRight;

            contentSettings.contentType = ECT_IMG;
            contentSettings.img = m_rows[i][j].img;
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

            m_rows[i][j].content = CReportPageContent(contentSettings);

            nextLeft += overallSize.width();
            
            if (biggestHeight < overallSize.height())
                biggestHeight = overallSize.height();
                
        }
         nextTop += biggestHeight;
    }

    //centering
    int overallHeight = 0;
    QVector<int> widthOffsets;
    widthOffsets.resize(m_rows.size());

    for (int i = 0; i < m_rows.size(); ++i)
    {
        int biggestHeight = 0;
        int rowWidth = 0;

        for (int j = 0; j < m_rows[i].size(); ++j)
        {
            //heightRemaining += row.at(j).content.m_overallRect.height();
            int contentHeight = m_rows[i][j].content.m_overallRect.height();;

            if (contentHeight > biggestHeight)
                biggestHeight = contentHeight;

            rowWidth += m_rows[i][j].content.m_overallRect.width();

        }

        int rowWidthRemaining = qMax(m_overallRect.width() - rowWidth, 0);        
        //why +1? so that the offset remains after last content and it looks centered
        widthOffsets[i] = rowWidthRemaining / (m_rows[i].size() + 1);

        overallHeight += biggestHeight;
    }

    int heightRemaining = m_overallRect.height() - overallHeight;  
    //why +1? so that the offset remains after last content and it looks centered
    int heightOffset = heightRemaining / (m_rows.count() + 1);

    if (m_useMinimumHeight)
    {
        heightOffset = 0;
        m_overallRect.setHeight(overallHeight);
    }

    for (int i = 0; i < m_rows.size(); ++i)
    {
        for (int j = 0; j < m_rows[i].size(); ++j)
        {
            //why (i+1)*heightOffset ? because 1.row (i = 0) is moved by heightOffset, so the 2.row needs to move 
            //as much as the 1.row and the again heightOffset, so that between this rows there is heightOffset space => (i+1)*heightOffset 
            m_rows[i][j].content.moveTop((i+1)*heightOffset);
            m_rows[i][j].content.moveLeft((j + 1)*widthOffsets.at(i));

        }
    }

}

void CImageGrid::setUseMinimumHeight(bool value)
{
    m_useMinimumHeight = value;
}

bool CImageGrid::getUseMinimumHeight()
{
    return m_useMinimumHeight;
}

void CImageGrid::setContentMargins(double right, double bottom)
{
    if (right < 0.0 || right > 1.0 || right < 0.0 || right > 1.0)
        return;

    m_marginBottom = bottom * m_pageRect.height();
    m_marginRight = right * m_pageRect.width();
}

void CImageGrid::setContentMargins(int right, int bottom)
{
    m_marginBottom = qMax(bottom, 0);
    m_marginRight = qMax(right, 0);
}

void CImageGrid::getContentMargins(double * right, double * bottom)
{
    *right = m_marginRight;
    *bottom = m_marginBottom;
}
