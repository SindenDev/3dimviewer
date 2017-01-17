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

#include <data/CCustomData.h>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QStringList>
#include <QVector3D>
#include <QBuffer>
#include <QDataStream>

namespace data 
{

CCustomData::CCustomData()
{

}

void CCustomData::init()
{
	m_data.clear();
}

void CCustomData::readXML(const std::string& str)
{
	// clear any existing data
	m_data.clear();
	// read data from xml
	QXmlStreamReader xml(QString::fromStdString(str));
	while(!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::StartDocument)
            continue;
        if(token == QXmlStreamReader::StartElement)
        {
            if(xml.name() == "groups")
                continue;
            if(xml.name() == "group") 
            {
                QString name;
                QXmlStreamAttributes attributes = xml.attributes();
                if(attributes.hasAttribute("name")) 
                    name = attributes.value("name").toString();
                xml.readNext();     
				// create group
				Q_ASSERT(!name.isEmpty());
				auto it = m_data.insertMulti(name,QList<sEntry>());
				// read group entries
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "group")) 
                {
					if(xml.tokenType() == QXmlStreamReader::Invalid) 
						break;
                    if(xml.tokenType() == QXmlStreamReader::StartElement) 
                    {
						QStringRef e_name = xml.name();

                        QString data;
                        QXmlStreamAttributes e_attr = xml.attributes();
                        //if(e_attr.hasAttribute("data")) 
                            //data = e_attr.value("data").toString();

                        if(data.isEmpty() && e_attr.hasAttribute("dataX")) 
                            data = e_attr.value("dataX").toString();

                        QString e_val = xml.readElementText();
						Q_ASSERT(!e_name.isEmpty());
						
						sEntry entry;
						entry.name = (e_name.toString());
						entry.value = (e_val);
                        if (!data.isEmpty())
                        {                      
                            QByteArray arr(QByteArray::fromBase64(data.toLatin1()));
                            QBuffer readBuffer(&arr);
                            readBuffer.open(QIODevice::ReadOnly);
                            QDataStream strm(&readBuffer);
                            strm >> entry.data;
                        }
						
						it->push_back(entry);
                    }
                    xml.readNext();
                }
            }
        }
    }
}

std::string CCustomData::writeXML()
{
	QString xmlString;
	QXmlStreamWriter xmlWriter(&xmlString);
	xmlWriter.setCodec("UTF-8");
	xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("groups");
	QMapIterator< QString, QList<sEntry> > it(m_data);
	while(it.hasNext())
	{
		it.next();
		xmlWriter.writeStartElement("group");
		xmlWriter.writeAttribute("name",it.key());
		foreach(auto entry, it.value())
		{
			Q_ASSERT(!entry.name.isEmpty());
			xmlWriter.writeStartElement(entry.name);
			if (!entry.data.isNull() && entry.data.isValid())
			{
				//QString xstr;
				//&xstr << entry.data;
				//xmlWriter.writeAttribute(QString("data"),xstr);

                QByteArray byteArray;
                QBuffer writeBuffer(&byteArray);
                writeBuffer.open(QIODevice::WriteOnly);
                QDataStream out(&writeBuffer); 
                out << entry.data; 
                writeBuffer.close();

                xmlWriter.writeAttribute(QString("dataX"),byteArray.toBase64());
			}
			xmlWriter.writeCharacters(entry.value);
			xmlWriter.writeEndElement();
		}
		xmlWriter.writeEndElement();
	}
	xmlWriter.writeEndElement();
	xmlWriter.writeEndDocument();

	return xmlString.toStdString();
}


void CCustomData::setDataValue(const QString& group, const QString& name, const QString& value, const QVariant& data)
{
	// get/create group
    auto it = m_data.find(group);
	if (it==m_data.end())
	{
		it = m_data.insertMulti(group,QList<sEntry>());
	}

	// find existing entry with the same name
	for(QList<data::CCustomData::sEntry>::iterator eit = it.value().begin(), eend = it.value().end(); eit!=eend; eit++)
	{
		if (0==(eit->name.compare(name,Qt::CaseInsensitive)))
		{
			eit->value = value;
			eit->data = data;
			return;
		}
	}
	// add a new one
	sEntry entry;
	entry.name = name;
	entry.value = value;
	entry.data = data;
	it->push_back(entry);
}

QString CCustomData::getDataValue(const QString& group, const QString& name)
{
    auto it = m_data.find(group);
	if (it!=m_data.end())
	{
		foreach(auto entry, it.value())
		{
			if (0==(entry.name.compare(name,Qt::CaseInsensitive)))
			{
				return entry.value;
			}
		}
	}
	return QString();
}

bool CCustomData::getDataValue(const QString& group, const QString& name, QString& value, QVariant& data)
{
    value.clear();
    data.clear();
	auto it = m_data.find(group);
	if (it!=m_data.end())
	{
		foreach(auto entry, it.value())
		{
			if (0==(entry.name.compare(name,Qt::CaseInsensitive)))
			{
				value = entry.value;
				data = entry.data;
				return true;
			}
		}
	}
	return false;
}

const QList<CCustomData::sEntry>& CCustomData::getGroupData(const QString& group)
{	
	auto it = m_data.find(group);
	if (it!=m_data.end())
	{
		return *it;
	}
	static QList<CCustomData::sEntry> empty;
	return empty;
}

//! Clear group data
void CCustomData::clearGroup(const QString& group)
{
	auto it = m_data.find(group);
	if (it!=m_data.end())
	{
		m_data.erase(it);
	}
}

//! Clear all data
void CCustomData::clearAll()
{
	m_data.clear();
}

}