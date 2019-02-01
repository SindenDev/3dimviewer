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

#ifndef _REPORTPAGECONTENT_H
#define _REPORTPAGECONTENT_H

#include <QRect>
#include <QImage>
#include <QString>
#include <QFont>
#include <QTextOption>
#include <QColor>
#include <QDebug>

class QPainter;
class CReportTable;

/**
* \brief  Enum used to identify content type.
*/
enum EContentType
{
    ECT_UNUSED = 0,
    ECT_TEXT,
    ECT_IMG,
    ECT_HTML

};

/**
* \brief  Enum used to specify image caption position.
*/
enum eImageCaptionPosition
{
    EICP_TOP = 0,
    EICP_BOTTOM,

};

/**
* \brief  Structure defyning all necessary content settings.
*/
struct SReportPageContentSettings
{
    //content geometry
    QRect                   overallRect;
    QRect                   contentRect;
    QRect                   imageRect;
    QRect                   captionRect;
    double                  marginBottom;
    double                  marginRight;

    QString                    text;
    QString                    HTML;
    EContentType            contentType = ECT_UNUSED;
    QColor                    color;
    QFont                    font;
    QTextOption                textOptions;
    bool                    scalingEnabled = true;
    bool                    overlay = false;

    //image
    QImage                    img;
    QString                    caption;
    double                    imageScale = 1.0;
    bool                    centered = true;
    bool                    stretchImage = false;
    bool                    captionInsideImage = false;
    eImageCaptionPosition    captionPos = EICP_BOTTOM;
    Qt::AspectRatioMode     aspectRatioMode = Qt::KeepAspectRatio;

};

/**
* \brief  Class encapsulating one specific content unit on page. Content can be HTML, plain text, or image.
*/
class CReportPageContent 
{
    friend class CReportPage;
    friend class CReportGenerator;
    friend class CImageGrid;

public:
    /** Default constructor */
    CReportPageContent();

    CReportPageContent(SReportPageContentSettings settings);

    /** Destructor */
    ~CReportPageContent();

    /** Copy constructor */
    CReportPageContent(const CReportPageContent &other)
    {
        m_overallRect = other.m_overallRect;
        m_contentRect = other.m_contentRect;
        m_imageRect = other.m_imageRect;
        m_captionRect = other.m_captionRect;
        m_marginBottom = other.m_marginBottom;
        m_marginRight = other.m_marginRight;

        m_contentType = other.m_contentType;
        m_text = other.m_text;
        m_img = other.m_img;
        m_HTML = other.m_HTML;
        m_color = other.m_color;
        m_font = other.m_font;
        m_textOptions = other.m_textOptions;
        m_imageScale = other.m_imageScale;
        m_centered = other.m_centered;
        m_stretchImage = other.m_stretchImage;
        m_scalingEnabled = other.m_scalingEnabled;
        m_caption = other.m_caption;
        m_captionInsideImage = other.m_captionInsideImage;
        m_captionPos = other.m_captionPos;
        m_overlay = other.m_overlay;
        m_aspectRatioMode = other.m_aspectRatioMode;
    }

    /** Assignment operator */
    CReportPageContent& operator=(const CReportPageContent& other)
    {
        m_overallRect = other.m_overallRect;
        m_contentRect = other.m_contentRect;
        m_imageRect = other.m_imageRect;
        m_captionRect = other.m_captionRect;
        m_marginBottom = other.m_marginBottom;
        m_marginRight = other.m_marginRight;

        m_img = other.m_img;
        m_text = other.m_text;
        m_HTML = other.m_HTML;
        m_contentType = other.m_contentType;
        m_color = other.m_color;
        m_font = other.m_font;
        m_textOptions = other.m_textOptions;
        m_imageScale = other.m_imageScale;
        m_centered = other.m_centered;
        m_stretchImage = other.m_stretchImage;
        m_scalingEnabled = other.m_scalingEnabled;
        m_caption = other.m_caption;
        m_captionInsideImage = other.m_captionInsideImage;
        m_captionPos = other.m_captionPos;
        m_overlay = other.m_overlay;
        m_aspectRatioMode = other.m_aspectRatioMode;

        return *this;
    }

public:

    /**
    * \brief            Sets content color.
    *
    * \param    col        Desired color.
    */
    void setColor(QColor col);

    /**
    * \brief    Returns curretly used color for this content.
    *
    * \return    Content color.
    */
    QColor getColor();

    /**
    * \brief                Returns curretly used font for this content.
    *
    * \return                Content font.
    */
    QFont getFont();

    /**
    * \brief                Returns overall rectangle in page allocated for this content and margins.
    *
    * \return                Overall rectangle.
    */
    QRect getOverallRect();

    /**
    * \brief                Returns content rectangle in page allocated for this content.
    *
    * \return                Content rectangle.
    */
    QRect getContentRect();

    /**
    * \brief       Getter for type of content.
    * \return      Content type.
    */
    EContentType getContentType();

    /**
    * \brief                Returns text options used for this content.
    *
    * \return                Content text options.
    */
    QTextOption getTextOptions();

    double getImageScaleFactor();
    double getImageWidthScaleFactor();
    double getImageHeightScaleFactor();

protected:

    /**
    * \brief            Sets contetn text, does not recalculate content rectange
    *
    * \param    text    Text to be printed.
    */
    void setText(QString text);

    /**
    * \brief            Sets contetn table, does not recalculate content rectange
    *
    * \param    table    Table to be printed.
    */
    void setTable(CReportTable table);

    /**
    * \brief            Sets contetn image, does not recalculate content rectange
    *
    * \param    img        Image to be printed.
    */
    void setImage(QImage img);

    /**
    * \brief                Sets contetn text options.
    *
    * \param    options        Text options.
    */
    void setTextOptions(QTextOption options);

    /**
    * \brief                Sets whole new content.
    *
    * \param    settings    Content settings.
    */
    void setContent(SReportPageContentSettings settings);

    /**
    * \brief                Sets overall content rectangle in page.
    *
    * \param    rect        Overall rectangle.
    */
    void setOverallRect(QRect rect);

    /**
    * \brief                Sets content rectangle.
    *
    * \param    rect        Content rectangle.
    */
    void setContentRect(QRect rect);

    /**
    * \brief                Sets content font.
    *
    * \param    font        Desired font.
    */
    void setFont(QFont font);

    /**
    * \brief                Prints content.
    *
    * \param    painter        Painter used to print.
    */
    virtual void print(QPainter& painter);

    void moveTop(int value);
    void moveLeft(int value);

protected:

    //content geometry
    QRect m_overallRect;
    QRect m_contentRect;
    QRect m_imageRect;
    QRect m_captionRect;
    double m_marginBottom;
    double m_marginRight;

    EContentType m_contentType;
    QString m_text;
    QString m_HTML;
    QColor m_color;
    QFont m_font;
    QTextOption m_textOptions;
    bool m_scalingEnabled;
    bool m_overlay;

    //image
    QImage m_img;
    double m_imageScale;
    bool m_centered;
    bool m_stretchImage;
    QString m_caption;
    bool m_captionInsideImage;
    eImageCaptionPosition m_captionPos;
    Qt::AspectRatioMode m_aspectRatioMode;

private:

    /**
    * \brief                Prints HTML text.
    *
    * \param    painter        Painter used to print.
    */
    void printHTMLText(QPainter& painter);

    /**
    * \brief                Prints plaint text.
    *
    * \param    painter        Painter used to print.
    */
    void printPlainText(QPainter& painter);

    /**
    * \brief                Prints image.
    *
    * \param    painter        Painter used to print.
    */
    void printImage(QPainter& painter);
};

#endif
