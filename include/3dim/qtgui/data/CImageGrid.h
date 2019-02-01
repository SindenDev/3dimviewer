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

#ifndef _CIMAGEGRID_H
#define _CIMAGEGRID_H

#include <QSize>
#include <QImage>

#if defined(REPORTDEBUG)
#include "CReportPage.h"
#else
#include "data/CReportPage.h"
#endif

struct SImageInfo
{
    QImage img;
        
    double maxScale;
    double minScale;
    SContentSettings settings;

    QSize overallSize;
    QSize overallSizeMin;
    QSize imageSizeMin;

    CReportPageContent content;

};

typedef QVector<QVector<SImageInfo>> tRow;

class CImageGrid : public CReportPageContent
{
    friend class CReportPage;
    friend class CReportGenerator;

public:

    CImageGrid(int rows, int cols, QRect rect, SReportPageSettings pageSettings);

    bool addImage(QImage img,double maxScale, double minScale, SContentSettings settings = SContentSettings());

    void newRow();

    /** Copy constructor */
    CImageGrid(const CImageGrid &other)
        :CReportPageContent(other)
    {
        m_colsCount = other.m_colsCount;
        m_rowsCount = other.m_rowsCount;
        m_pageRect = other.m_pageRect;
        m_font = other.m_font;
        m_size = other.m_size;
        m_freeSpace = other.m_freeSpace;
        m_images = other.m_images;
        m_currentRow = other.m_currentRow;
        m_rect = other.m_rect;
        m_useMinimumHeight = other.m_useMinimumHeight;
        m_rows = other.m_rows;

    }

    /** Assignment operator */
    CImageGrid& operator=(const CImageGrid& other)        
    {
        CReportPageContent::operator=(other);
        m_colsCount      = other.m_colsCount;
        m_rowsCount      = other.m_rowsCount;
        m_pageRect       = other.m_pageRect;
        m_font           = other.m_font;
        m_size           = other.m_size;
        m_freeSpace      = other.m_freeSpace;
        m_images         = other.m_images;
        m_currentRow     = other.m_currentRow;
        m_rect           = other.m_rect;
        m_useMinimumHeight = other.m_useMinimumHeight;
        m_rows = other.m_rows;

        return *this;
    }

    
    void setUseMinimumHeight(bool value);
    bool getUseMinimumHeight();

    void setContentMargins(double right, double bottom);
    void setContentMargins(int right, int bottom);

    void getContentMargins(double* right, double* bottom);

protected:
    void prepare();
    void print(QPainter& painter) override;

private:

    int m_colsCount;
    int m_rowsCount;

    QRect m_pageRect;
    QFont m_font;

    QSize m_size;
    QSize m_freeSpace;
    QVector<SImageInfo> m_images;   

    int m_currentRow;
    tRow m_rows;
    QRect m_rect;

    bool m_useMinimumHeight;


};

#endif