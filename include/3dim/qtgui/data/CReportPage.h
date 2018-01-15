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

#ifndef _REPORTPAGE_H
#define _REPORTPAGE_H

#if defined(REPORTDEBUG)
#include "CReportPageContent.h"
#else
#include "data/CReportPageContent.h"
#endif

#include <QString>
#include <QVector>
#include <QFont>
#include <QColor>

class CReportTable;
class QImage;
class QPainter;
class QPrinter;
class CImageGrid;
/**
* \brief  Enum used to specify wich point (corner) should be used to align rectangle, when creating new rectangle with addRectanlge methods.
*/
enum eRectangleAlignmentPosition
{
    ERAP_TOPLEFT = 0,
    ERAP_TOPRIGHT,
    ERAP_BOTTOMLEFT,
    ERAP_BOTTOMRIGHT,

};

/**
* \brief  Structure used to pass settings from CReportGenerator to CReportPage. Used when new page is created.
*/
struct SReportPageSettings
{
    QRect	pageRect;
    QFont	font;
    QColor	color;
    double	contentMarginBottom;
    double	contentMarginRight;
    bool	floatingContent = false;
    bool	scalingEnabled = true;
    double  pageHeightPixelsPerMM = 0.0;
    double  pageWidthPixelsPerMM = 0.0;
};

/**
* \brief  Structure used to pass additional settings when iserting new content.
*/
struct SContentSettings
{
    bool useColor = false;
    QColor color;
    bool useFont = false;
    QFont font;
    QTextOption textOptions = QTextOption(Qt::AlignVCenter | Qt::AlignHCenter);
    bool overlay = false;

    //image
    double scaleFactor = 1.0;
    bool centered = true;
    bool stretch = false;
    QString caption;
    eImageCaptionPosition captionPos = EICP_BOTTOM;
    bool captionInsideImage = false;
    Qt::AspectRatioMode     aspectRatioMode = Qt::KeepAspectRatio;
};

/**
* \brief  Class representing one page of final document.
*/
class CReportPage
{
    friend class CReportGenerator;

public:
    /*Default constructor*/
    CReportPage();

    CReportPage(SReportPageSettings settings);


    /** Copy constructor */
    CReportPage(const CReportPage& other)
    {
        m_content.resize(other.m_content.size());
        for (int i = 0; i < other.m_content.size(); ++i)
            m_content[i] = new CReportPageContent(*other.m_content[i]);
        
        m_pageSize = other.m_pageSize;
        m_pageRect = other.m_pageRect;
        m_font = other.m_font;
        m_color = other.m_color;
        m_contentMarginBottom = other.m_contentMarginBottom;
        m_contentMarginRight = other.m_contentMarginRight;
        m_floatingContent = other.m_floatingContent;
        m_scalingEnabled = other.m_scalingEnabled;
        m_pageHeightPixelsPerMM = other.m_pageHeightPixelsPerMM;
        m_pageWidthPixelsPerMM = other.m_pageWidthPixelsPerMM;
    }

    /** Assignment operator */
    CReportPage& operator= (const CReportPage& other)
    {
        m_content.resize(other.m_content.size());
        for (int i = 0; i < other.m_content.size(); ++i)
            m_content[i] = new CReportPageContent(*other.m_content[i]);

        m_pageSize = other.m_pageSize;
        m_pageRect = other.m_pageRect;
        m_font = other.m_font;
        m_color = other.m_color;
        m_contentMarginBottom = other.m_contentMarginBottom;
        m_contentMarginRight = other.m_contentMarginRight;
        m_floatingContent = other.m_floatingContent;
        m_scalingEnabled = other.m_scalingEnabled;
        m_pageHeightPixelsPerMM = other.m_pageHeightPixelsPerMM;
        m_pageWidthPixelsPerMM = other.m_pageWidthPixelsPerMM;

        return *this;
    }

    /** Move constructor */
    //CReportPage(const CReportPage&& other) = delete;
    /**Move assignment operator */
    //CReportPage& operator= (const CReportPage&& other) = delete;

    ~CReportPage();

    /**
    * \brief				Adds new rectanle UNDER last rectangle (that is after uppermost rectangle), if floating content is not enabled.
    *
    * \param   width		Desired width in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    * \param   height		Desired height in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    *
    * \return				Index to created rectangle, -1 on failure.
    */
    int addRectangle(double width, double height);

    /**
    * \brief				Adds new rectanle UNDER last rectangle (that is after uppermost rectangle), if floating content is not enabled.
    *
    * \param	size		Desired rectangle size.
    *
    * \return				Index to created rectangle, -1 on failure.
    */
    int addRectangle(QSize size);

    /**
    * \brief				    Adds new rectanle in position defined by parameters x and y.
    *
    * \param	width		    Desired width in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    * \param	height		    Desired height in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    * \param	x			    Position in x-axis in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    * \param	y			    Position in y-axis in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    * \param	alignPosition	Specifies wich corner of rectangle should be moved to desired position (specified by x and y values).
    * \return				    Index to created rectangle, -1 on failure.
    */
    int addRectangle(double width, double height, double x, double y, eRectangleAlignmentPosition alignPosition = ERAP_TOPLEFT);

    /**
    * \brief				     Adds new rectanle in position defined by parameters x and y.
    *
    * \param	size		    Desired rectangle size.
    * \param	x			    Position in x-axis in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    * \param	y			    Position in y-axis in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    * \param	alignPosition	Specifies wich corner of rectangle should be moved to desired position (specified by x and y values).
    * \return				    Index to created rectangle, -1 on failure.
    */
    int addRectangle(QSize size, double x, double y, eRectangleAlignmentPosition alignPosition = ERAP_TOPLEFT);

    /**
    * \brief				     Adds new rectanle.
    *
    * \param	rect		    Rectangle to add.
    * \return				    Index to created rectangle, -1 on failure.
    */
    int addRectangle(QRect rect);

    /**
    * \brief				Inserts text into rectangle specified by parameter index.
    *
    * \param	index		Index of rectangle in wich the text should be inserted.
    * \param	text		Text to be printed.
    * \param	settings	Additional settings.
    *
    * \return				True on success, false on failure.
    */
    bool insertText(int index, QString text, SContentSettings settings = SContentSettings());

    /**
    * \brief				Inserts table into rectangle specified by parameter index.
    *
    * \param	index		Index of rectangle in wich the table should be inserted.
    * \param	table		Table to be printed.
    * \param	settings	Additional settings.
    *
    * \return				True on success, false on failure.
    */
    bool insertTable(int index, CReportTable table, SContentSettings settings = SContentSettings());

    /**
    * \brief				Inserts image into rectangle specified by parameter index.
    *
    * \param	index		Index of rectangle in wich the image should be inserted.
    * \param	img			Image to be printed.
    * \param	settings	Additional settings
    *
    * \return				True on success, false on failure.
    */
    bool insertImage(int index, QImage img, SContentSettings settings = SContentSettings());

    /**
    * \brief				Adds text UNDER last rectangle (that is after uppermost rectangle), if floating content is not enabled.
    *
    * \param	text		Text to be printed.
    * \param	settings	Additional settings
    * \param    fromBottom  If set to true, text will be added in the bottom of the page
    *
    * \return				True on success, false on failure.
    */
    bool addText(QString text, SContentSettings settings = SContentSettings(), bool fromBottom = false);

    /**
    * \brief				Adds image UNDER last rectangle (that is after uppermost rectangle), if floating content is not enabled.
    *
    * \param	img			Image to be printed.
    * \param	settings	Additional settings
    * \param    fromBottom  If set to true, image will be added in the bottom of the page
    *
    * \return				True on success, false on failure.
    */
    bool addImage(QImage img, SContentSettings settings = SContentSettings(), bool fromBottom = false);

    /**
    * \brief				Adds image UNDER last rectangle (that is after uppermost rectangle) on current page , if floating content is not enabled.
    *						If there is not enough space, new page will be automatically created.
    *                       This method will try to scale image according to its physical resolution (dotsPerMeterX() and dotsPerMeterY())
    *                       and physical resolution of page rectangle.
    *
    * \param	img                     Qt aspect ration flag.
    * \param	aspectRatioMode			Image to be printed.
    * \param	settings                Additional settings
    *
    * \return				True on success, false on failure.
    */
    bool addImagePhysical(QImage img, Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio, SContentSettings settings = SContentSettings());

    /**
    * \brief				Adds table UNDER last rectangle (that is after uppermost rectangle), if floating content is not enabled.
    *
    * \param	table		Table to be printed.
    * \param	settings	Additional settings
    * \param    fromBottom  If set to true, table will be added in the bottom of the page
    *
    * \return				True on success, false on failure.
    */
    bool addTable(CReportTable table, SContentSettings settings = SContentSettings(), bool fromBottom = false);

    /**
    * \brief				Adds HTML UNDER last rectangle (that is after uppermost rectangle), if floating content is not enabled.
    *
    * \param	html		HTML to be printed.
    * \param	settings	Additional settings
    * \param    fromBottom  If set to true, HTML will be added in the bottom of the page
    *
    * \return				True on success, false on failure.
    */
    bool addHTML(QString html, SContentSettings settings = SContentSettings(), bool fromBottom = false);

    /**
    * \brief				Sets page font.
    *
    * \param	font		Desired font.
    */
    void setFont(QFont font);

    /**
    * \brief				Returns curretly used font for this page.
    *
    * \return				Page font.
    */
    QFont getFont();

    /**
    * \brief			Sets page color.
    *
    * \param	color		Desired color.
    */
    void setColor(QColor color);

    /**
    * \brief				This is an overloaded method.
    *
    * \param	color		Desired color.
    */
    void setColor(Qt::GlobalColor color);

    /**
    * \brief	Returns curretly used color for this page.
    *
    * \return	Page color.
    */
    QColor getColor();

    /**
    * \brief		Enables or disables floating content. Floating content means, that the content rectangles will not
    *				stretch to whole page, but will take only so much place as needed. This allows to print multiple contents
    *				new to each other. Disabled by default.
    *
    * \param	b	Flag indicating if floating content should be enabled or disabled.
    */
    void setFloatingContent(bool b);

    /**
    * \brief
    *
    * \return	True if floating content is enabled, false otherwise.
    */
    bool getFloatingContent();

    /**
    * \brief		Enables or disables automatic scaling of content.
    *
    * \param	b	Flag indicating if scaling should be enabled or disabled.
    */
    void setScalingEnabled(bool b);

    /**
    * \brief
    *
    * \return	True if automatic scaling of content is enabled, false otherwise.
    */
    bool getScalingEnabled();

    /**
    * \brief
    *
    * \return	Page size.
    */
    QSize getPageSize();

    /**
    * \brief			Sets page margins in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    *
    * \param	left	Margin from left side of the page.
    * \param	right	Margin from left right of the page
    * \param	top		Margin from left top of the page
    * \param	bottom	Margin from left bottom of the page
    */
    void setMargins(double left, double right, double top, double bottom);

    /**
    * \brief			Sets content margins in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    *
    * \param	right	Margin from right side of the content.
    * \param	bottom	Margin from bottom side of the content.
    */
    void setContentMargins(double right, double bottom);

    /**
    * \brief			Test if content fits into page without scaling.
    *
    * \param	text	Text to test.
    * \param	font	Used font
    * \return			True if content fits without scaling, false otherwise.
    */
    bool canFitContentWithoutScaling(QString text, QFont font);

    /**
    * \brief			Test if content fits into page without scaling.
    *
    * \param	text	Text to test.
    * \param	font	Used font.
    * \return			True if content fits without scaling, false otherwise.
    */
    bool canFitContentWithoutScaling(QString text);

    /**
    * \brief			Test if content fits into page without scaling.
    *
    * \param	table	Table to test.
    * \return			True if content fits without scaling, false otherwise.
    */
    bool canFitContentWithoutScaling(CReportTable table);

    /**
    * \brief			Test if content fits into page without scaling.
    *
    * \param	table	table to test.
    * \param	font	Used font.
    * \return			True if content fits without scaling, false otherwise.
    */
    bool canFitContentWithoutScaling(CReportTable table, QFont font);

    /**
    * \brief			Test if content fits into page without scaling.
    *
    * \param	img         Image to test.
    * \param	settings    Additional settings.
    * \return               True if content fits without scaling, false otherwise.
    */
    bool canFitContentWithoutScaling(QImage img, SContentSettings settings = SContentSettings());

    /**
    * \brief			    Test if content fits into give rectangle without scaling. This method also considers image caption and content margins of this page.
    *
    * \param	img         Image to test.
    * \param	rect        Rectangle in wich image should fit.
    * \param	settings    Additional settings.
    * \return               True if content fits without scaling, false otherwise.
    */
    bool canFitContentWithoutScaling(QImage img, QRect rect, SContentSettings settings = SContentSettings());

    /**
    * \brief	Test if the page is emty (without content). Page numbering DOES NOT count as content!
    *
    * \return	True if page is empty, false otherwise.
    */
    bool isEmpty();

    /**
    * \brief
    *
    * \return	Content count.
    */
    int getContentCount();

    /**
    * \brief			Returns pointer to content specified by index.
    *
    * \param	index	Content index.
    * \return			Pointer to content if index is valid, nullptr otherwise.
    */
    CReportPageContent* getContent(int index);

    /**
    * \brief					Returns free space on this page. Free space can be devided in multiple rectangles.
    *
    * \return					Vector of rectangles representing unused space on this page.
    */
    QVector<QRect> getFreeSpace();

    /**
    * \brief					Finds largest free rectangle on this page and returns it.
    *
    * \return					Largest free rectangle on success, empty rectangle on failure.
    */
    QRect getBiggestFreeRectangle();

    /**
    * \brief			            Scales input image to desired size. This method also considers image caption and content margins of this page.
    * \param	image	            Input image.
    * \param	desiredSize	        Size in wich the input image should fit.
    * \param	aspectRatioMode	    Aspect ratio mode.
    * \param	settings	        Additional settings.
    * \return			            Scaled image.
    */
    QImage getScaledImage(QImage image, QSize desiredSize, Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio, SContentSettings settings = SContentSettings());

    QSize getOverallSizeNeeded(QImage image, SContentSettings settings = SContentSettings());

    CImageGrid* createImageGrid(int rows, int cols);
protected:

    /**
    * \brief				Prints all content of this page using given painter.
    *
    * \param	painter		Painter used to print.
    */
    void print(QPainter* painter);

private:
    std::vector<CReportPageContent*> m_content;
    QColor m_color;
    QSize m_pageSize;
    QRect m_pageRect;
    QFont m_font;
    double m_contentMarginBottom;
    double m_contentMarginRight;
    bool m_floatingContent;
    bool m_scalingEnabled;
    double m_pageHeightPixelsPerMM;
    double m_pageWidthPixelsPerMM;

private:

    /**
    * \brief					Finds free space of desired size on this page. If scaling is enabled it can return rectangle with smaller size than desired.
    *
    * \param	desiredSize		Size needed.
    * \return					Rectangle of uppermost free space on success, empty rectangle otherwise.
    */
    QRect findFreeSpace(QSizeF desiredSize);
};

#endif
