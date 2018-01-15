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

#ifndef _REPORTTABLE_H
#define _REPORTTABLE_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QStringList>

class QStringList;

typedef QVector<QPair<QString, QString>> tAttributes;

class CTableData;

typedef QVector<QVector<CTableData>> tTableDataMatrix;


class CTableDataMatrix
{
public:
    CTableDataMatrix();

    /** Copy constructor */
    CTableDataMatrix(const CTableDataMatrix &other)
    {
        m_rowCount = other.m_rowCount;
        m_colCount = other.m_colCount;

        m_dataMatrix = other.m_dataMatrix;
        m_unorderedData = other.m_unorderedData;

    }

    /** Assignment operator */
    CTableDataMatrix& operator= (const CTableDataMatrix& other)
    {
        m_rowCount = other.m_rowCount;
        m_colCount = other.m_colCount;

        m_dataMatrix = other.m_dataMatrix;
        m_unorderedData = other.m_unorderedData;

        return *this;
    }

    void clear();

    CTableData at(int row, int col);
    void insert(int row, int col, CTableData data);
    void insert(CTableData data);

    bool isEmpty();
    int rowsCount();
    int colsCount();
    void prepare();

    void setRowCount(int rowCount);
    void setColCount(int colCount);
private:
    int m_rowCount;
    int m_colCount;

    tTableDataMatrix m_dataMatrix;
    QVector<CTableData> m_unorderedData;
};

class CTableData
{
    friend class CTableDataMatrix;

public:
    CTableData();
    CTableData(QString text, tAttributes attributes);
    CTableData(int row, int col, QString text, tAttributes attributes);

    /** Copy constructor */
    CTableData(const CTableData &other)
    {
        m_text = other.m_text;
        m_attributes = other.m_attributes;
        m_row = other.m_row;
        m_col = other.m_col;

    }

    /** Assignment operator */
    CTableData& operator= (const CTableData& other)
    {
        m_text = other.m_text;
        m_attributes = other.m_attributes;
        m_row = other.m_row;
        m_col = other.m_col;

        return *this;
    }


    tAttributes attributes();
    QString text();
    bool isValid();
private:
    QString m_text;
    tAttributes m_attributes;
    int m_row;
    int m_col;
};



/**
* \brief  Class encapsulating HTML table.
*/
class CReportTable
{
    friend class CReportPageContent;
    friend class CReportPage;

    /**
    * \brief  Structure representing one row with attributes.
    */
    struct sTableRow
    {
        QStringList rowCells;
        tAttributes rowAttributes;
    };

public:
    CReportTable();
    ~CReportTable();

    /** Copy constructor */
    CReportTable(const CReportTable &other)
    {
        m_style = other.m_style;
        m_title = other.m_title;
        m_titleAttribudes = other.m_titleAttribudes;
        m_HTML_header = other.m_HTML_header;
        m_rows = other.m_rows;
        m_tableAttributes = other.m_tableAttributes;
        m_rowCount = other.m_rowCount;
        m_colCount = other.m_colCount;
        m_dataMatrix = other.m_dataMatrix;
        m_underlineCaption = other.m_underlineCaption;

    }

    /** Assignment operator */
    CReportTable& operator= (CReportTable& other)
    {
        m_style = other.m_style;
        m_title = other.m_title;
        m_titleAttribudes = other.m_titleAttribudes;
        m_HTML_header = other.m_HTML_header;
        m_rows = other.m_rows;
        m_tableAttributes = other.m_tableAttributes;
        m_rowCount = other.m_rowCount;
        m_colCount = other.m_colCount;
        m_dataMatrix = other.m_dataMatrix;
        m_underlineCaption = other.m_underlineCaption;
        return *this;
    }

    /** Move constructor */
    //CReportTable::CReportTable(const CReportTable &&other)
    /** Move operator */
    //CReportTable& operator= (CReportTable&& other);

    /**
    * \brief			Sets row count. Final table will have not more than "count" rows.
    *
    * \param	count	Row count.
    */
    void setRowCount(int count);

    /**
    * \brief			Sets column count. Final table will have not more than "count" columns.
    *
    * \param	count	Column count.
    */
    void setColCount(int count);

    /**
    * \brief			Sets table headers (for each column).
    *
    * \param	header	Table headers.
    */
    void setHeader(QStringList header);

    /**
    * \brief				Sets table title (above table).
    *
    * \param	title		Table title.
    * \param	attributes	Additional HTML attribudes.
    */
    void setTableTitle(QString title, tAttributes attributes = tAttributes());

    void underlineTitle(bool underline);

    /**
    * \brief				Adds one row.
    *
    * \param	rows		Values for each cell in this row.
    * \param	attributes	Additional HTML attribudes.
    */
    void addRow(QStringList rows, tAttributes attributes = tAttributes());

    void insertItem(int row, int col, QString text, tAttributes attributes = tAttributes());
    /**
    * \brief				Sets attribudes for <table> tag. Allowed attribude names are:
    *						width (default 100%), border (default 1), cellspacing (default 0), class (default empty)
    *
    * \param	attribute	Attribute name.
    * \param	value		Attribute value.
    */
    void setTableAttribute(QString attribute, QString value);

    /**
    * \brief				Sets HTML style.
    *
    * \param	style		HTML style.
    */
    void setStyle(QString style);

    /**
    * \brief	Clears whole table.
    *
    */
    void clear();

private:
    QString m_style;
    QString m_title;
    tAttributes m_titleAttribudes;
    QStringList m_HTML_header;
    QVector<sTableRow> m_rows;
    QMap<QString, QString> m_tableAttributes;
    int m_rowCount;
    int m_colCount;
    bool m_underlineCaption;
private:

    /**
    * \brief		Creates final HTML table and returns it as QString.
    *
    * \return		HTML table.
    */
    QString createHTMLTable();

    QString createTableAttributes();
    QString createAttributesString(tAttributes attribudes);

    CTableDataMatrix m_dataMatrix;
};

#endif
