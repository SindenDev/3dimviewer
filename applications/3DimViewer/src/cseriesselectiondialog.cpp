///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#include "cseriesselectiondialog.h"
#include "ui_cseriesselectiondialog.h"
#include <CDataInfoDialog.h>
#include <data/CDensityWindow.h>
#include <VPL/Image/ImageFunctions/General.h>
#include <VPL/Image/Filters/Gaussian.h>
#include <QSettings>
#include <QSpinBox>
#include <QMessageBox>

CSeriesSelectionDialog::CSeriesSelectionDialog(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    ui(new Ui::CSeriesSelectionDialog)
{
    ui->setupUi(this);
    m_Selection = -1;
    ui->tableWidget->setColumnWidth(0,128);

    QSettings settings;
    settings.beginGroup("SeriesSelectionWindow");
    resize(settings.value("size").toSize());
    settings.endGroup();

    QObject::connect(ui->spinSubsamplingX, SIGNAL(valueChanged(double)), this, SLOT(on_spinSubsampling_valueChanged(double)));
    QObject::connect(ui->spinSubsamplingY, SIGNAL(valueChanged(double)), this, SLOT(on_spinSubsampling_valueChanged(double)));
    QObject::connect(ui->spinSubsamplingZ, SIGNAL(valueChanged(double)), this, SLOT(on_spinSubsampling_valueChanged(double)));

	ui->tableWidget->setColumnWidth(1,std::max(128,(this->width()-180)/2));
	ui->tableWidget->setFocus();
}

CSeriesSelectionDialog::~CSeriesSelectionDialog()
{
    QSettings settings;
    settings.beginGroup("SeriesSelectionWindow");
    settings.setValue("size",size());
    settings.endGroup();
    delete ui;
}

void CSeriesSelectionDialog::addInfoRow(const QString &info, bool bStudyID)
{
	QColor color;
	if (bStudyID)
		color = QColor(220,236,255); 
	else
		color = QColor(96,160,255);
	{
		long Id = ui->tableWidget->rowCount();
		ui->tableWidget->insertRow(Id);
        QTableWidgetItem *item = NULL;
		if (bStudyID)
			item = new QTableWidgetItem("    " + info);
		else
			item = new QTableWidgetItem(info);
		item->setData(Qt::UserRole,-1);
		item->setBackgroundColor(color);
        ui->tableWidget->setItem(Id,0,item);
		ui->tableWidget->setSpan(Id,0,1,3);
	}
}

struct sEntry
{
	int id;
	QString studyID; // sort the list by study id, then by series id
	QString seriesID;
	QString info;
	QString infoEx;
	QString infoSize;
	QPixmap preview;	
};

bool CSeriesSelectionDialog::setSeries(data::CSeries *pSeries)
{
    if (NULL==pSeries)
        return false;

    m_spSeries=pSeries;		

	// key is patient name
	QMap<QString, QList<sEntry> > seriesData;

    for( int i = 0; i < m_spSeries->getNumSeries(); ++i )
    {
        // Get pointer to the series
        data::CSerieInfo::tSmartPtr spInfo = m_spSeries->getSerie(i);
        if( !spInfo.get() )
        {
            continue;
        }

        // Load dicom file
        vpl::img::CDicomSlice Slice;
        {
            data::sExtendedTags tags;
            if (!spInfo->loadDicomFile(spInfo->getNumOfDicomFiles() / 2, Slice, tags))
            {
                continue;
            }
        }

		sEntry seriesEntry;
		seriesEntry.seriesID = QString::fromStdString(Slice.m_sSeriesUid);
		seriesEntry.studyID = QString::fromStdString(Slice.m_sStudyUid);
		if (!Slice.m_sStudyDate.empty())
		{
			QString studyDate = CDataInfoDialog::formatDicomDateAndTime(QString::fromUtf8(Slice.m_sStudyDate.c_str()),"");
			if (seriesEntry.studyID.isEmpty())
				seriesEntry.studyID = studyDate;
			else
				seriesEntry.studyID = studyDate + " - " + seriesEntry.studyID;
		}

		QString patient = QString::fromStdString(Slice.m_sPatientName);
		// format patient name
        patient.replace("^"," ");
        int idxGroup = patient.indexOf('=');
        if (idxGroup>0)
            patient = patient.left(idxGroup);
		QString patientID = QString::fromStdString(Slice.m_sPatientId);
		if (!patientID.isEmpty())
			patient += " - " + patientID;

        {   // column 0

            // Gaussian filter
            vpl::img::CGaussFilter<vpl::img::CDImage> Filter(7);
            vpl::img::CDImage Filtered(Slice);
            Slice.mirrorMargin();
            Filter(Slice, Filtered);

            // Size scaling
            static const int Margin = 0;
            double dSX = (Filtered.getXSize() - 1) / (128.0 - 2 * Margin);
            double dSY = (Filtered.getYSize() - 1) / (128.0 - 2 * Margin);

            // Use default density window
            data::CDensityWindow DensityWindow(data::DEFAULT_DENSITY_WINDOW);

            QImage bitmap(128,128,QImage::Format_RGB32);
            for(int y=0;y<(128.0 - 2 * Margin);y++)
            {
                for(int x=0;x<(128.0 - 2 * Margin);x++)
                {
                    data::CColor4b Color = DensityWindow.getColorSafe(Filtered.at(vpl::math::round2Int(x * dSX), vpl::math::round2Int(y * dSY)));
                    if( Color.getR() == 255 )
                    {
                        Color.getR() -= 1;
                    }
                    bitmap.setPixel(x + Margin, y + Margin, qRgb(Color.getR(), Color.getG(), Color.getB()));
                 }
            }
			seriesEntry.preview = QPixmap::fromImage(bitmap);
			seriesEntry.id  = i;
        }
        {   // column 1
            QString patientName=QString::fromUtf8(Slice.m_sPatientName.c_str());
            if( !Slice.m_sPatientPosition.empty() )
            {
                patientName+=", "+QString::fromUtf8(Slice.m_sPatientPosition.c_str());
            }
			seriesEntry.info  = patientName;
        }

        {   // column 2
            QString seriesDate=QString::fromUtf8(Slice.m_sSeriesDate.c_str());
			QString seriesTime=QString::fromUtf8(Slice.m_sSeriesTime.c_str());
			seriesDate = CDataInfoDialog::formatDicomDateAndTime(seriesDate,seriesTime);
			if (seriesDate.isEmpty())
				seriesDate = CDataInfoDialog::formatDicomDateAndTime(QString::fromUtf8(Slice.m_sStudyDate.c_str()),"");
			seriesDate += "\n";
            if( !Slice.m_sModality.empty() )
            {
                seriesDate += QString::fromUtf8(Slice.m_sModality.c_str()) + ", ";
            }
            if( !Slice.m_sScanOptions.empty() )
            {
                seriesDate += QString::fromUtf8(Slice.m_sScanOptions.c_str());
            }
			if( !Slice.m_sSeriesUid.empty() )
            {
                seriesDate += "\n" + QString::fromUtf8(Slice.m_sSeriesUid.c_str());
            }
			seriesEntry.infoEx  = seriesDate;
        }

        {   // column 3

            vpl::img::CDicomSlice SliceFirst;
            data::sExtendedTags tags;
            if (!spInfo->loadDicomFile(0, SliceFirst, tags))
            {
                Q_ASSERT(false); // could not load first slice
            }
            vpl::img::CDicomSlice SliceLast;
            if (!spInfo->loadDicomFile(spInfo->getNumOfDicomFiles() - 1, SliceLast, tags))
            {
                Q_ASSERT(false); // could not load first slice
            }
            QString imageInfo = QString(tr("%1 x %2 x %3 voxels\n"
                                        "%4 x %5 x %6 mm"
                               #ifdef _DEBUG
                                         "\n%7 - %8"
                               #endif
                                           ))
                                    .arg(Slice.getXSize()).arg(Slice.getYSize()).arg(spInfo->getNumOfSlices())
                                    .arg(Slice.getDX(),0,'f',2).arg(Slice.getDY(),0,'f',2).arg(Slice.getThickness(),0,'f',2)
                                    .arg(SliceFirst.getPosition(),0,'f',2).arg(SliceLast.getPosition(),0,'f',2);
            if( !Slice.m_sImageType.empty() )
            {
                imageInfo+="\n"+QString::fromUtf8(Slice.m_sImageType.c_str());
            }
			seriesEntry.infoSize = imageInfo;
        }

		// add to list		
		auto it = seriesData.find(patient);
		if (it==seriesData.end())
			it = seriesData.insertMulti(patient,QList<sEntry>());
		it->push_back(seriesEntry);
    }

	// map is always sorted, sort also contained lists
	{
		QMap< QString, QList<sEntry> >::iterator it(seriesData.begin());
		while(it!=seriesData.end())
		{			
			qSort(it.value().begin(),it.value().end(),[](const sEntry& e1,const sEntry& e2)->bool
			{ 
				int val = e1.studyID.compare(e2.studyID,Qt::CaseInsensitive);
				if (0==val)
					val = e1.seriesID.compare(e2.seriesID,Qt::CaseInsensitive);
				return val;
			});
			it++;
		}
	}

	// add data to table
	QMapIterator< QString, QList<sEntry> > it(seriesData);
	while(it.hasNext())
	{
		it.next();
		addInfoRow(it.key());
		const QList<sEntry>& list = it.value();
		const int nListEntries = list.size();
		for(int i = 0; i < nListEntries; i++ )
		{
			const sEntry& entry = list[i];
			if (i==0 || (i>0 && 0!=list[i].studyID.compare(list[i-1].studyID,Qt::CaseInsensitive)))
				addInfoRow(entry.studyID,true);
			long Id = ui->tableWidget->rowCount();
			ui->tableWidget->insertRow(Id);
			ui->tableWidget->setRowHeight(Id,128);

			{   // column 0				
				QTableWidgetItem *previewItem = new QTableWidgetItem();
				previewItem->setData(Qt::DecorationRole,entry.preview);
				previewItem->setData(Qt::UserRole,entry.id);
				previewItem->setTextAlignment(Qt::AlignCenter);
				ui->tableWidget->setItem(Id,0,previewItem);
			}
			{   // column 1
				QTableWidgetItem *seriesItem = new QTableWidgetItem(entry.infoEx);
				seriesItem->setTextAlignment(Qt::AlignCenter);
				ui->tableWidget->setItem(Id,1,seriesItem);
			}

			{   // column 2
				QTableWidgetItem *infoItem = new QTableWidgetItem(entry.infoSize);
				infoItem->setTextAlignment(Qt::AlignCenter);
				ui->tableWidget->setItem(Id,2,infoItem);
			}
		}
	}

    // Select the first item
	int i = 0;
    while (i < ui->tableWidget->rowCount() && ui->tableWidget->item(i,0)->data(Qt::UserRole).toInt()<0)
	{
		i++;
	}
	if (i < ui->tableWidget->rowCount())
    {
        ui->tableWidget->selectRow(i);
        m_Selection = ui->tableWidget->item(i,0)->data(Qt::UserRole).toInt();
    }
    return true;
}

//! Returns index of the choosen series.
int CSeriesSelectionDialog::getSelection() const
{
    return m_Selection;
}

void CSeriesSelectionDialog::getSubsampling(double &subsamplingX, double &subsamplingY, double &subsamplingZ) const
{
    subsamplingX = ui->spinSubsamplingX->value();
    subsamplingY = ui->spinSubsamplingY->value();
    subsamplingZ = ui->spinSubsamplingZ->value();
}

void CSeriesSelectionDialog::accept()
{
    QItemSelectionModel * selection = ui->tableWidget->selectionModel();
    QModelIndexList list = selection->selectedIndexes();
    if (list.count()>0)
        m_Selection=list.at(0).data(Qt::UserRole).toInt();
	if (m_Selection<0)
	{
		QMessageBox::warning(this,QCoreApplication::applicationName(),tr("Please select a series."));
		return;
	}
    QDialog::accept();
}

void CSeriesSelectionDialog::on_spinSubsampling_valueChanged(double value)
{
    QDoubleSpinBox *spin = dynamic_cast<QDoubleSpinBox *>(QObject::sender());
    if (spin == NULL)
    {
        return;
    }

    if ((value > 1.0) && (spin->singleStep() == 0.01))
    {
        spin->setSingleStep(1.0);
        spin->setValue(1.0 + 1.0);
    }
    else if ((value < 1.0) && (spin->singleStep() == 1.0))
    {
        spin->setSingleStep(0.01);
        spin->setValue(1.0 - 0.01);
    }
}

void CSeriesSelectionDialog::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QTableWidgetItem* item=ui->tableWidget->item(row,0);
    if (NULL!=item)
    {
        m_Selection=item->data(Qt::UserRole).toInt();
		if (m_Selection>=0)
			QDialog::accept();
    }
}

