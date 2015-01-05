///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2013 3Dim Laboratory s.r.o.
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

#include "CDataInfoDialog.h"
#include "ui_CDataInfoDialog.h"

#include <QTableWidgetItem>
#include <QSettings>
#include <QDateTime>

#include <data/CDensityData.h>
#include <data/CModelManager.h>

// static method
QString CDataInfoDialog::formatDicomDateAndTime(const QString& wsDate, const QString& wsTime)
{
    // convert date and time from dicom format
    QDate date = QDate::fromString(wsDate,"yyyy.MM.dd");
    if (!date.isValid())
         date = QDate::fromString(wsDate,"yyyyMMdd");
	if (!date.isValid() && !wsDate.isEmpty())
		return wsDate + " " + wsTime;
    QTime time = QTime::fromString(wsTime,"hhmmss.zzz");
    if (!time.isValid())
        time = QTime::fromString(wsTime,"hhmmss");
    if (!time.isValid())
        time = QTime::fromString(wsTime,"hh:mm:ss.zzz");
    if (!time.isValid())
        time = QTime::fromString(wsTime,"hh:mm:ss");
    if (!wsTime.isEmpty() && time.isValid())
	{		
		if (date.isValid())
		{
			QDateTime dt;
			dt.setDate(date);
			dt.setTime(time);
			return dt.toString(Qt::DefaultLocaleLongDate);
		}
		else
			return time.toString(Qt::DefaultLocaleLongDate);
	}
	else
	{
		if (date.isValid())
			return date.toString(Qt::DefaultLocaleLongDate);
	}
	return "";
}

CDataInfoDialog::CDataInfoDialog(int model_storage_id, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
    ui(new Ui::CDataInfoDialog)
{
    ui->setupUi(this);

    {
        data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );

        QString PatientName(spVolume->m_sPatientName.c_str());
        QString PatientID(spVolume->m_sPatientId.c_str());
        QString PatientBirthday(spVolume->m_sPatientBirthday.c_str());
        QString PatientSex(spVolume->m_sPatientSex.c_str());
		QString PatientDescription(spVolume->m_sPatientDescription.c_str());
        QString Date(spVolume-> m_sSeriesDate.c_str());
        QString Time(spVolume-> m_sSeriesTime.c_str());
        QString SeriesID(spVolume-> m_sSeriesUid.c_str());
		QString SeriesDescription(spVolume-> m_sSeriesDescription.c_str());
		QString StudyID(spVolume-> m_sStudyUid.c_str());
		QString StudyDate(spVolume-> m_sStudyDate.c_str());
		QString StudyDescription(spVolume-> m_sStudyDescription.c_str());
        QString PatientPosition(spVolume-> m_sPatientPosition.c_str());
        QString Manufacturer(spVolume-> m_sManufacturer.c_str());
        QString ModelName(spVolume-> m_sModelName.c_str());
		QString ScanOptions(spVolume-> m_sScanOptions.c_str());

        AddRow(tr("Patient Data"),"",QColor(0,128,255));
        AddRow(tr("Dimensions"),QString(tr("%1 x %2 x %3 voxels")).arg(spVolume->getXSize()).arg(spVolume->getYSize()).arg(spVolume->getZSize()));
        AddRow(tr("Resolution"),QString(tr("%1 x %2 x %3 mm")).arg(spVolume->getDX(),0,'f',4).arg(spVolume->getDY(),0,'f',4).arg(spVolume->getDZ(),0,'f',4));
        AddRow(tr("Size"),QString(tr("%1 x %2 x %3 mm")).arg(spVolume->getXSize()*spVolume->getDX(),0,'f',2).arg(spVolume->getYSize()*spVolume->getDY(),0,'f',2).arg(spVolume->getZSize()*spVolume->getDZ(),0,'f',2));
        AddRow(tr("Patient Name"),PatientName);
        AddRow(tr("Patient ID"),PatientID);
        AddRow(tr("Patient Position"),PatientPosition);
        AddRowIfNotEmpty(tr("Patient Birthday"),PatientBirthday);        
        AddRowIfNotEmpty(tr("Patient Sex"),PatientSex);        
		AddRowIfNotEmpty(tr("Patient Description"),PatientDescription);        
		AddRowIfNotEmpty(tr("Study ID"),StudyID);
		AddRowIfNotEmpty(tr("Study Date"),formatDicomDateAndTime(StudyDate,""));
		AddRowIfNotEmpty(tr("Study Description"),StudyDescription);
        AddRow(tr("Series ID"),SeriesID);
        AddRow(tr("Series Date"),formatDicomDateAndTime(Date,""));
        AddRow(tr("Series Time"),formatDicomDateAndTime("",Time));
		AddRowIfNotEmpty(tr("Series Description"),SeriesDescription);
		AddRowIfNotEmpty(tr("Scan Options"),ScanOptions);
        AddRow(tr("Manufacturer"),Manufacturer);
        AddRow(tr("Model Name"),ModelName);

        spVolume.release();
    }
    if(model_storage_id > 0)
    {
        data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(model_storage_id) );
        data::CMesh *pMesh = spModel->getMesh();
        if (pMesh && pMesh->n_vertices()>0)
        {
            AddRow(tr("Bone Model"),"",QColor(0,128,255));
            if (!spModel->getLabel().empty())
                AddRow(tr("Name"),QString::fromStdString(spModel->getLabel()));
            AddRow(tr("Triangles"),QString::number(pMesh->n_faces()));
            AddRow(tr("Nodes"),QString::number(pMesh->n_vertices()));
			AddRow(tr("Components"),QString::number(pMesh->componentCount()));
            if (!pMesh->isClosed())
                AddRow(tr("Closed"),tr("No"));
        }
    }
    QSettings settings;
    settings.beginGroup("DataInfoWindow");
    resize(settings.value("size",minimumSize()).toSize());
    settings.endGroup();
}

void CDataInfoDialog::AddRowIfNotEmpty(const QString& param, const QString& value, const QColor& color)
{
    if (value.isEmpty()) return;
    AddRow(param,value,color);
}

void CDataInfoDialog::AddRow(const QString& param, const QString& value, const QColor& color)
{
    int Id=ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(Id);
    QTableWidgetItem *i0 = new QTableWidgetItem(param);
    i0->setFlags(i0->flags() & ~Qt::ItemIsEditable);
    if (color.isValid())
        i0->setBackgroundColor(color);
    ui->tableWidget->setItem(Id,0,i0);
    QTableWidgetItem *i1 = new QTableWidgetItem(value);
    i1->setFlags(i1->flags() & ~Qt::ItemIsEditable);
    i1->setTextAlignment(Qt::AlignCenter);
    if (color.isValid())
        i1->setBackgroundColor(color);
    ui->tableWidget->setItem(Id,1,i1);
}

CDataInfoDialog::~CDataInfoDialog()
{
    QSettings settings;
    settings.beginGroup("DataInfoWindow");
    settings.setValue("size",size());
    settings.endGroup();
    delete ui;
}
