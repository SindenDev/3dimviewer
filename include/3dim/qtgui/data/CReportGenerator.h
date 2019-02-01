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

#ifndef _REPORTGENERATOR_H
#define _REPORTGENERATOR_H

#include <QPainter>

#if defined(REPORTDEBUG)
#include "CReportPageContent.h"
#include "CReportPage.h"

#else
#include "data/CReportPageContent.h"
#include "data/CReportPage.h"

#endif

class CReportTable;

class QString;
class QImage;
class QPrinter;
class QPainter;

/**
* \brief  Class encapsulating creating of PDF documents.
*/
class CReportGenerator
{
public:

    /**
    * \brief        Constructs object using already created QPrinter.
                    ATTENTION!: If you wish to print to PDF file, class QPrinter:setOutputFileName() before creating CReportGenerator.
    *
    */
    CReportGenerator(QPrinter& printer);

    CReportGenerator(const CReportGenerator& other)
        : m_printer(other.m_printer)
    {
        
        m_outputFileName        =other.m_outputFileName;
        m_templatePage          =other.m_templatePage;
        m_font                  =other.m_font;
        m_pageRect              =other.m_pageRect;
        m_contentMarginBottom   =other.m_contentMarginBottom;
        m_contentMarginRight    =other.m_contentMarginRight;
        m_pages                 =other.m_pages;
        m_color                 =other.m_color;
        m_floatingContent       =other.m_floatingContent;
        m_scalingEnabled        =other.m_scalingEnabled;
        m_usePageTemplate       =other.m_usePageTemplate;
        m_pageHeightPixelsPerMM =other.m_pageHeightPixelsPerMM;
        m_pageWidthPixelsPerMM  =other.m_pageWidthPixelsPerMM;
        //page numbering        
        m_pageNumberingOn       =other.m_pageNumberingOn;
        m_pageNumberingContent  =other.m_pageNumberingContent;
    }

    /** Assignment operator */
    CReportGenerator& operator= (const CReportGenerator& other)
    {
        m_outputFileName = other.m_outputFileName;
        m_templatePage = other.m_templatePage;
        m_font = other.m_font;
        m_pageRect = other.m_pageRect;
        m_contentMarginBottom = other.m_contentMarginBottom;
        m_contentMarginRight = other.m_contentMarginRight;
        m_pages = other.m_pages;
        m_color = other.m_color;
        m_floatingContent = other.m_floatingContent;
        m_scalingEnabled = other.m_scalingEnabled;
        m_usePageTemplate = other.m_usePageTemplate;
        m_pageHeightPixelsPerMM = other.m_pageHeightPixelsPerMM;
        m_pageWidthPixelsPerMM = other.m_pageWidthPixelsPerMM;
        //page numbering        
        m_pageNumberingOn = other.m_pageNumberingOn;
        m_pageNumberingContent = other.m_pageNumberingContent;

        return *this;
    }

    ~CReportGenerator();

    /**
    * \brief        Creates new page WITH CURRENT SETTINGS!
    *
    * \return        Reference to new page.
    */
    CReportPage& newPage();

    /**
    * \brief        Creates new page WITH CURRENT SETTINGS, but does not add it to resulting document. This method is useful when creating template page.
    *
    * \return        New page.
    */
    CReportPage createPage();

    /**
    * \brief        Adds new page to the end of the document.
    *
    * \param        Page to add.
    */
    void addPage(CReportPage& page);

    /**
    * \brief            Removes page specified by index from the document.
    *
    * \param    index   Index of page to remove.
    */
    void removePage(int index);

    /**
    * \brief        Sets template page. All new pages will be created from this page.
    *
    */
    void setPageTemplate(CReportPage& page);

    /**
    * \brief            Inserts page before page specified by index.
    *
    * \param    index   Index of page to remove.
    * \param    page    Index of page to remove.
    */
    void setPage(int index, CReportPage& page);

    /**
    *
    * \return        Page count.
    */
    int getPageCount();

    /**
    * \brief            Returns reference to page specified by index
    *
    * \param    index   Index of desired page.
    * \return           Reference to page on success, mullptr otherwise.
    */
    CReportPage& getPage(int index);

    /**
    * \brief        Returns reference to last page
    *
    * \return       Reference to last page, mullptr if there is not page.
    */
    CReportPage& getLastPage();

    /**
    * \brief                Sets document font.
    *
    * \param    font        Desired font.
    */
    void setFont(QFont font);

    /**
    * \brief        Returns curretly used font for this document.
    *
    * \return       Document font.
    */
    QFont getFont();

    /**
    * \brief        Enabled or disables page numbering. This method allocated space need for page numbering and CHANGES PAGE SIZE.
    *               Uses document font.
    *
    * \param    b   Enables/disables page numbering.
    */
    void setPageNumberingOn(bool b, QTextOption options = QTextOption());

    /**
    * \brief            Enabled or disables page numbering. This method allocated space need for page numbering and CHANGES PAGE SIZE.
    *                   Uses specified font.
    *
    * \param    b       Enables/disables page numbering.
    * \param    font    Font used for page numbering.
    */
    void setPageNumberingOn(bool b, QFont  font, QTextOption options = QTextOption());

    /**
    * \brief        
    *
    * \return        True if page numbering is enabled, false otherwise.
    */
    bool getPageNumberingOn();

    /**
    * \brief            Enabled or disables automatic scaling of content.
    *
    * \param    b       Enables/disables automatic scaling.
    */
    void setScalingEnabled(bool b);

    /**
    * \brief
    *
    * \return        True if automatic scaling is enabled, false otherwise.
    */
    bool getScalingEnabled();

    /**
    * \brief                Sets document color.
    *
    * \param    color       Desired color.
    */
    void setColor(QColor color);
    
    /**
    * \brief                This is an overloaded method.
    *
    * \param    color        Desired color.
    */
    void setColor(Qt::GlobalColor color);

    /**
    * \brief    Returns curretly used color for this document.
    *
    * \return    Document color.
    */
    QColor getColor();

    /**
    * \brief            Sets output file name.
    *
    * \param    fn      File name.
    */
    void setOutputFileName(QString fn);

    /**
    * \brief
    *
    * \return    Output file name.
    */
    QString getOutpuFileName();

    /**
    * \brief        Enables or disables floating content. Floating content means, that the content rectangles will not
    *               stretch to whole page, but will take only so much place as needed. This allows to print multiple contents
    *               new to each other. Disabled by default.
    *
    * \param    b   Flag indicating if floating content should be enabled or disabled.
    */
    void setFloatingContent(bool b);

    /**
    * \brief
    *
    * \return    True if floating content is enabled, false otherwise.
    */
    bool getFloatingContent();

    /**
    * \brief                Sets page margins in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    *
    * \param    left        Margin from left side of the page.
    * \param    right       Margin from left right of the page
    * \param    top         Margin from left top of the page
    * \param    bottom      Margin from left bottom of the page
    */
    void setMargins(double left, double right, double top, double bottom);

    /**
    * \brief            Sets content margins in PERCENTAGE relative to page size. Allowed values are <0.0,1.0>.
    *
    * \param    right    Margin from right side of the content.
    * \param    bottom    Margin from bottom side of the content.
    */
    void setContentMargins(double right, double bottom);

    /**
    * \brief                Adds text UNDER last rectangle (that is after uppermost rectangle) on current page , if floating content is not enabled.
    *                       If there is not enough space, new page will be automatically created.
    *
    * \param    text        Text to be printed.
    * \param    settings    Additional settings
    * \param    fromBottom  If set to true, text will be added in the bottom of the page
    *
    * \return               True if on success, false otherwise.
    */
    bool addText(QString text, SContentSettings settings = SContentSettings(), bool fromBottom = false);

    /**
    * \brief                Adds image UNDER last rectangle (that is after uppermost rectangle) on current page , if floating content is not enabled.
    *                        If there is not enough space, new page will be automatically created.
    *
    * \param    img            Image to be printed.
    * \param    settings    Additional settings
    *
    * \return    True if on success, false otherwise.
    */
    bool addImage(QImage img, SContentSettings settings = SContentSettings());

    /**
    * \brief                Adds image UNDER last rectangle (that is after uppermost rectangle) on current page , if floating content is not enabled.
    *                       If there is not enough space, new page will be automatically created.
    *                       This method will try to scale image according to its physical resolution (dotsPerMeterX() and dotsPerMeterY())
    *                       and physical resolution of page rectangle.                                
    *
    * \param    img                     Qt aspect ration flag.
    * \param    aspectRatioMode         Aspect ratio mode.
    * \param    settings                Additional settings
    *
    * \return    True if on success, false otherwise.
    */
    bool addImagePhysical(QImage img, Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio, SContentSettings settings = SContentSettings());

    /**
    * \brief                Adds table UNDER last rectangle (that is after uppermost rectangle) on current page , if floating content is not enabled.
    *                       If there is not enough space, new page will be automatically created.
    *
    * \param    table       Table to be printed.
    * \param    settings    Additional settings
    *
    * \return    True if on success, false otherwise.
    */
    bool addTable(CReportTable table, SContentSettings settings = SContentSettings());

    /**
    * \brief                Adds HTML UNDER last rectangle (that is after uppermost rectangle) on current page , if floating content is not enabled.
    *                       If there is not enough space, new page will be automatically created.
    *
    * \param    html        HTML to be printed.
    * \param    settings    Additional settings
    *
    */
    bool addHTML(QString html, SContentSettings settings = SContentSettings());

    /**
    * \brief                Adds Image Grid to last page. If there is not enough space, new page will be automatically created.
    *
    * \param    grid        Image Grid.
    *
    */
    bool addImageGrid(CImageGrid& grid);

    /**
    * \brief    Returns currently used printer.
    *            
    *
    * \return    Reference to currently used printer. Can return nullptr.
    */
    QPrinter& getPrinter();

    /**
    * \brief    Prints all document pages. This method can fail if printing could not start. This could happen if document
    *           in wich printing should be done, is opened in another program.
    *
    * \return    True if on success, false otherwise.
    */
    bool print();

    //! Returns current page size
    QSize getPageSize();

    //! Returns current page size in milimeters
    QSize getPageSizeInMM();

    /**
    * \brief                    Returns amount of pixels representing 1mm on physical page.
    *
    * \param    width   [out]   Amount of pixels per 1mm in x-direction.
    * \param    height  [out]   Amount of pixels per 1mm in y-direction.
    */
    void getPixelsPerMM(double* width, double* height);

    QImage getScaledImage(QImage image, QSize desiredSize, Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio, SContentSettings settings = SContentSettings());

    /**
    * \brief    Creates Image Grid on last page  with current page settings., occupying all available space on this page.
    *
    * \param    rows    Desired number of Image Grid rows.
    * \param    cols    Desired number of Image Grid columns.
    *
    * \return    Returns Image Grid that takes up all available space on currently last page.
    */
    CImageGrid createImageGrid(int rows, int cols);

private:
    QPrinter& m_printer;
    QString m_outputFileName;

    CReportPage m_templatePage;
    QFont m_font;
    QRect m_pageRect;
    double m_contentMarginBottom;
    double m_contentMarginRight;
    std::vector<CReportPage> m_pages;

    QColor m_color;
    bool m_floatingContent;
    bool m_scalingEnabled;
    bool m_usePageTemplate;
    double m_pageHeightPixelsPerMM;
    double m_pageWidthPixelsPerMM;
    //page numbering
    bool m_pageNumberingOn;
    CReportPageContent m_pageNumberingContent;

};

#endif
