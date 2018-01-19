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
#include "CReportTable.h"
#else
#include "data/CReportTable.h"
#endif

#include <QStringList>

//TEST
#include <QDebug>

//TODO: comments & clean up

CTableData::CTableData()
    :m_row(-1)
    , m_col(-1)
{
}

CTableData::CTableData(QString text, tAttributes attributes)
    : m_text(text)
    , m_attributes(attributes)
    , m_row(-1)
    , m_col(-1)
{
}

CTableData::CTableData(int row, int col, QString text, tAttributes attributes)
    : m_text(text)
    , m_attributes(attributes)
    , m_row(row)
    , m_col(col)
{
}

tAttributes CTableData::attributes()
{
    return  m_attributes;
}

QString CTableData::text()
{
    return  m_text;
}

bool CTableData::isValid()
{
    return m_row >= 0 && m_col >= 0;
}

CTableDataMatrix::CTableDataMatrix()
    :m_rowCount(0)
    , m_colCount(0)
{
}

void CTableDataMatrix::clear()
{
    m_dataMatrix.clear();
    m_unorderedData.clear();
    m_rowCount = 0;
    m_colCount = 0;
}

CTableData CTableDataMatrix::at(int row, int col)
{
    if (row < m_rowCount && col < m_colCount)
    {
        return m_dataMatrix.at(row).at(col);
    }

    return CTableData();
}

void CTableDataMatrix::insert(int row, int col, CTableData data)
{
    data.m_row = row;
    data.m_col = col;

    m_unorderedData.push_back(data);
}

void CTableDataMatrix::insert(CTableData data)
{
    m_unorderedData.push_back(data);
}

bool CTableDataMatrix::isEmpty()
{
    return (m_colCount < 0) && (m_rowCount < 0);
}

int CTableDataMatrix::rowsCount()
{
    return  m_rowCount;
}

int CTableDataMatrix::colsCount()
{
    return  m_colCount;
}

void CTableDataMatrix::prepare()
{
    if (isEmpty())
        return;

    //initialize data matrix to desired size with empty values
    m_dataMatrix.resize(m_rowCount);
    for (int i = 0; i < m_rowCount; ++i)
    {
        QVector<CTableData> row;
        row.resize(m_colCount);

        m_dataMatrix.insert(i, row);
    }

    //insert data
    for (int i = 0; i < m_unorderedData.size(); ++i)
    {
        CTableData item = m_unorderedData.at(i);

        if (item.m_row < m_rowCount && item.m_row >= 0
            && item.m_col < m_colCount && item.m_col >= 0)
        {
            int r = item.m_row;
            int c = item.m_col;
            m_dataMatrix[r][c] = item;
        }
    }

    //TODO delete unordered data and insert them again from ordered data
}

void CTableDataMatrix::setRowCount(int rowCount)
{
    m_rowCount = rowCount;
}

void CTableDataMatrix::setColCount(int colCount)
{
    m_colCount = colCount;
}

CReportTable::CReportTable()
    : m_rowCount(0)
    , m_colCount(0)
    , m_underlineCaption(false)
{
    m_tableAttributes.insert("width", "100%");
    m_tableAttributes.insert("border", "1");
    m_tableAttributes.insert("cellspacing", "0");
    m_tableAttributes.insert("class", QString());
}


CReportTable::~CReportTable()
{
}

void CReportTable::setRowCount(int count)
{
    //m_rowCount = count;
    m_dataMatrix.setRowCount(count);
}

void CReportTable::setColCount(int count)
{
    //m_colCount = count;
    m_dataMatrix.setColCount(count);

}

void CReportTable::setHeader(QStringList header)
{
    m_HTML_header = header;
}

void CReportTable::setTableTitle(QString title, tAttributes attributes)
{
    m_title = title;
    m_titleAttribudes = attributes;
}

void CReportTable::underlineTitle(bool underline)
{
    m_underlineCaption = underline;
}

void CReportTable::setTableAttribute(QString attribute, QString value)
{
    if (m_tableAttributes.contains(attribute))
        m_tableAttributes[attribute] = value;
}

void CReportTable::addRow(QStringList rows, tAttributes attributes)
{
    //sTableRow row;
    //row.rowCells = rows;
    //row.rowAttributes = attributes;

    //m_rows.push_back(row);

    if (rows.empty())
        return;

    for (int i = 0; i < rows.size(); ++i)
    {
        QString text = rows.at(i);

        m_dataMatrix.insert(m_rowCount, i, CTableData(text, attributes));
    }

    ++m_rowCount;

}

void CReportTable::insertItem(int row, int col, QString text, tAttributes attributes)
{
    m_dataMatrix.insert(CTableData(row, col, text, attributes));
}

void CReportTable::setStyle(QString style)
{
    m_style = style;
}

void CReportTable::clear()
{
    m_dataMatrix.clear();
    m_style.clear();
    m_title.clear();
    m_HTML_header.clear();
    m_rows.clear();
    m_tableAttributes.clear();
    m_rowCount = 0;
    m_colCount = 0;
    m_underlineCaption = false;

    m_titleAttribudes.clear();
    m_tableAttributes.insert("width", "100%");
    m_tableAttributes.insert("border", "1");
    m_tableAttributes.insert("cellspacing", "0");
    m_tableAttributes.insert("class", QString());
}

QString CReportTable::createHTMLTable()
{
    m_dataMatrix.prepare();

    QString HTML =
        "<html>";
    HTML += m_style;
    HTML +=
        "<body>";

    if (!m_title.isEmpty())
    {
        HTML +=
            "<caption " + createAttributesString(m_titleAttribudes) + ">" + m_title + "</caption>";

        if (m_underlineCaption)
            HTML += "<div style='font-size:4px;height:4px;line-height:4px; background-color:black'> </div>";

    }

    HTML +=
        "<table " + createTableAttributes() + ">";

    HTML += "   <tbody>";

    if (!m_HTML_header.isEmpty())
    {
        HTML += "<tr>";
        for (int headerIndex = 0; headerIndex < m_HTML_header.size(); ++headerIndex)
        {
            HTML += "    <th >" + m_HTML_header.at(headerIndex) + "</td>";
        }
        HTML += "</tr>";
    }



    if (!m_dataMatrix.isEmpty())
    {

        for (int rowIndex = 0; rowIndex < m_dataMatrix.rowsCount() /*&& rowIndex < m_rowCount*/; ++rowIndex)
        {
            HTML += "<tr>";


            for (int colIndex = 0; colIndex < m_dataMatrix.colsCount() /*&& colIndex < m_colCount*/; ++colIndex)
            {
                CTableData item = m_dataMatrix.at(rowIndex, colIndex);
                if (!item.isValid() || item.text().isEmpty())
                    continue;

                HTML += "<td " + createAttributesString(item.attributes()) + "> <nobr>" + item.text() + " </nobr></td>";

            }
            HTML += "</tr>";
        }
    }

    HTML +=
        "   </tbody>"
        "</table>"
        "</body>"
        "</html>";

    return HTML;
}

QString CReportTable::createTableAttributes()
{
    QString str;

    QMapIterator<QString, QString> i(m_tableAttributes);
    while (i.hasNext()) {
        i.next();

        if (i.value().isEmpty())
            continue;

        str += i.key() + "=" + i.value() + " ";
    }

    return str;
}

QString CReportTable::createAttributesString(tAttributes attribudes)
{
    QString str;

    for (int i = 0; i < attribudes.size(); ++i)
    {
        if (attribudes.at(i).second.isEmpty())
            continue;

        str += attribudes.at(i).first + "=" + attribudes.at(i).second + " ";
    }

    return str;
}
