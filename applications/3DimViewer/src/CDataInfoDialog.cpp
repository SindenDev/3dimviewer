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

#include "CDataInfoDialog.h"
#include "ui_CDataInfoDialog.h"
#include <C3DimApplication.h>

#include <QTableWidgetItem>
#include <QSettings>
#include <QDateTime>
#include <QDebug>
#include <QClipboard>
#include <data/CDensityData.h>
#include <data/CModelManager.h>
#include <data/CImageLoaderInfo.h>
#include <data/CVolumeTransformation.h>
#include <data/CRegionData.h>
#include <data/CRegionColoring.h>
#include <coremedi/app/Signals.h>



///////////////////////////////////////////////////////////////////////////////
// DICOM tags dump

#if defined (TRIDIM_USE_GDCM)
//GDCM
#include <gdcmReader.h>
#include <gdcmStringFilter.h>

bool dumpFileGDCM(CDataInfoDialog *pDialog, const char *ifname)
{
    // Read file
    gdcm::Reader reader;
    reader.SetFileName(ifname);
    if (!reader.Read())
    {
        QString message = "Failed to read file ";
        message += ifname;
        QMessageBox::critical(pDialog, QApplication::applicationName(), message);
        return false;
    }

    //String filter used to get tag values as strings
    gdcm::StringFilter stringFilter;
    stringFilter.SetFile(reader.GetFile());

    //Get all data elements from dataset
    const std::set<gdcm::DataElement> &dataElementSet = reader.GetFile().GetDataSet().GetDES();

    foreach(gdcm::DataElement el, dataElementSet)
    {
        //Convert to string pair: first = parameter, second = value
        std::pair<std::string, std::string> pair = stringFilter.ToStringPair(el);

        //if empty or undifined do not add
        if (!pair.first.empty() && !pair.second.empty() && pair.first != "?" && pair.second != "?")
        {
            pDialog->AddRow(QString::fromStdString(pair.first), QString::fromStdString(pair.second));

        }
    }

    //Return true if we managed to get any data elements
    return dataElementSet.size() > 0;
}


#else

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#define HAVE_STD_STRING
#include "dcmtk/ofstd/ofstring.h"
#include <dcmtk/dcmdata/dcfilefo.h>

//ORIGIINAL
bool dumpFileDCTk(
    CDataInfoDialog *pDialog,
    const char *ifname,
    const E_FileReadMode readMode,
    const E_TransferSyntax xfer,
    const size_t printFlags,
    const OFBool loadIntoMemory,
    const OFBool stopOnErrors,
    const OFBool convertToUTF8)
{
#define COUT qDebug()
    if (NULL == ifname || 0 == ifname[0])
        return false;

    DcmFileFormat dfile;
    DcmObject *dset = &dfile;
    if (readMode == ERM_dataset)
        dset = dfile.getDataset();
    const OFCmdUnsignedInt maxReadLength = 4096; // default is 4 KB
    OFCondition cond = dfile.loadFile(ifname, xfer, EGL_noChange, maxReadLength, readMode);
    if (cond.bad())
    {
        qDebug() << "error invalid filename";
        return false;
    }

    if (loadIntoMemory)
        dfile.loadAllDataIntoMemory();

#ifdef WITH_LIBICONV
    if (convertToUTF8)
    {
        qDebug() << "converting all element values that are affected by Specific Character Set (0008,0005) to UTF-8";
        cond = dfile.convertToUTF8();
        if (cond.bad())
        {
            qDebug() << "error converting file to UTF-8";
            if (stopOnErrors) 
                return false;
        }
}
#endif

    size_t pixelCounter = 0;
    const char *pixelFileName = NULL;
    OFString pixelFilenameStr;

    // dump file content 
    std::stringstream out;
    dset->print(out, printFlags, 0 /*level*/, pixelFileName, &pixelCounter);
    QStringList sl = (QString::fromStdString(out.str())).split('\n');
    foreach(QString str, sl)
    {
        if (!str.isEmpty())
        {
            QStringList entrySplit = str.split('#');
            if (entrySplit.size()>1)
            {
                if (2 == entrySplit.size())
                    pDialog->AddRow(entrySplit[1], entrySplit[0], QColor(QColor::Invalid), Qt::AlignLeft);
                else
                    pDialog->AddRow(entrySplit[entrySplit.size() - 1], str.left(str.length() - entrySplit[entrySplit.size() - 1].length() - 1), QColor(QColor::Invalid), Qt::AlignLeft);
            }
            else
                pDialog->AddRow("", str, QColor(QColor::Invalid), Qt::AlignLeft);
        }
    }
    return sl.size()>0;
}

#endif

///////////////////////////////////////////////////////////////////////////////


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

CDataInfoDialog::CDataInfoDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
    ui(new Ui::CDataInfoDialog)
{
    ui->setupUi(this);

    int activeDataset = VPL_SIGNAL(SigGetActiveDataSet).invoke2();

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(tableContextMenu(const QPoint &)));
    {
        data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(activeDataset) );

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
	
    {        
        data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(activeDataset) );
        vpl::img::CVector3D pos = spVolume->m_ImagePosition;
        data::CObjectPtr<data::CVolumeTransformation> spVolumeTransformation(APP_STORAGE.getEntry(data::Storage::VolumeTransformation::Id));
        osg::Matrix volMx = spVolumeTransformation->getTransformation();
        if (!volMx.isIdentity() || pos.getLength()>0.001)
        {
            AddRow(tr("Volume Transformation"),"",QColor(0,128,255));
            if (!volMx.isIdentity())
            {
                for(int i=0;i<4;i++)
                    AddRow(i==0?(tr("Matrix")):"",QString("%1 %2 %3 %4").arg(volMx(i,0),0,'f',3).arg(volMx(i,1),0,'f',3).arg(volMx(i,2),0,'f',3).arg(volMx(i,3),0,'f',3));
            }
            AddRow(tr("Position"),QString("%1 %2 %3").arg(pos.getX(),0,'f',3).arg(pos.getY(),0,'f',3).arg(pos.getZ(),0,'f',3));
        }
        APP_STORAGE.invalidate(spVolumeTransformation.getEntryPtr());
    }

	for(int id=0;id<MAX_IMPORTED_MODELS;id++)
	{
		data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(data::Storage::ImportedModel::Id+id, data::Storage::NO_UPDATE) );
		if(spModel->hasData())
		{
			geometry::CMesh *pMesh = spModel->getMesh();
			//const std::string &std_name(spModel->getLabel());
			//QString qs_name(QString::fromStdString(std_name));
			AddRow(tr("Model %1").arg(id+1),"",QColor(0,128,255));
			if (!spModel->getLabel().empty())
				AddRow(tr("Name"),QString::fromStdString(spModel->getLabel()));
			AddRow(tr("Triangles"),QString::number(pMesh->n_faces()));
			AddRow(tr("Nodes"),QString::number(pMesh->n_vertices()));
			AddRow(tr("Components"),QString::number(pMesh->componentCount()));
			if (!pMesh->isClosed())
				AddRow(tr("Closed"),tr("No"));
            if (!spModel->getTransformationMatrix().isIdentity())
            {
                osg::Matrix mx = spModel->getTransformationMatrix();
                for (int i = 0; i<4; i++)
                    AddRow(i == 0 ? (tr("Matrix")) : "", QString("%1 %2 %3 %4").arg(mx(i, 0), 0, 'f', 3).arg(mx(i, 1), 0, 'f', 3).arg(mx(i, 2), 0, 'f', 3).arg(mx(i, 3), 0, 'f', 3));
            }
		}
	}

	// info on region data
	{
		data::CObjectPtr<data::CRegionColoring> spColoring( APP_STORAGE.getEntry(data::Storage::RegionColoring::Id) );
		data::CObjectPtr< data::CRegionData > rVolume( APP_STORAGE.getEntry( data::Storage::RegionData::Id ) );
		data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );
		const int xSize = rVolume->getXSize();
		const int ySize = rVolume->getYSize();
		const int zSize = rVolume->getZSize();
		const double voxelSize = spVolume->getDX() * spVolume->getDY() * spVolume->getDZ();
		const int nRegions = spColoring->getNumOfRegions();
		std::map<int,int> rInfo;
		for(int i = 0; i < nRegions; i++)
			rInfo[i]=0;
		for(int z=0; z<zSize; z++)
		{
			for(int y=0; y<ySize; y++)
			{
				for(int x=0; x<xSize; x++)
				{
					auto val = rVolume->at(x,y,z);
					if (val!=0)
						rInfo[val]++;
				}
			}
		}
		for(auto it = rInfo.begin(); it!=rInfo.end(); it++)
		{
			if (it->second>0)
			{
				auto color = spColoring->getColor( it->first );
				AddRow(tr("Region %1").arg(it->first),"",QColor(color.getR(),color.getG(),color.getB()));
				AddRow(tr("Voxels"),QString::number(it->second));
				AddRow(tr("Volume"), QString::number(it->second * voxelSize, 'f', 2) + QString(" mm%1").arg(QChar(179)));
			}
		}
	}

	{	// dicom file info
		data::CObjectPtr<data::CImageLoaderInfo> spInfo( APP_STORAGE.getEntry(data::Storage::ImageLoaderInfo::Id) );
		if (spInfo->getNumOfFiles()>0)
		{			
			const data::CImageLoaderInfo::tFiles & list =  spInfo->getList();
			QString str;
#ifdef _MSC_VER
			str = QString::fromUtf16((const ushort *)list[0].c_str());
#else
			str =  QString::fromStdString(list[0]);
#endif
			if (QFile::exists(str))
			{
				AddRow(tr("DICOM Info"),str,QColor(0,128,255));
			
#ifdef _WIN32
				std::string file = C3DimApplication::wcs2ACP(list[0]);
#else
                std::string file = list[0];
#endif


#if defined(TRIDIM_USE_GDCM)

                dumpFileGDCM(this, file.c_str());

#else
                E_FileReadMode readMode = ERM_autoDetect;
                E_TransferSyntax xfer = EXS_Unknown;
                size_t printFlags = DCMTypes::PF_shortenLongTagValues;
                OFBool loadIntoMemory = OFTrue;
                OFBool stopOnErrors = OFTrue;
                OFBool convertToUTF8 = OFFalse;
                dumpFileDCTk(this, file.c_str(), readMode, xfer, printFlags, loadIntoMemory, stopOnErrors, convertToUTF8);
#endif
				

                
			}
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

void CDataInfoDialog::AddRow(const QString& param, const QString& value, const QColor& color, int alignment)
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
    i1->setTextAlignment(alignment);
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

void CDataInfoDialog::tableContextMenu(const QPoint &pos)
{
    QTableWidget* pTable = qobject_cast<QTableWidget*>(sender());
    if (NULL != pTable)
    {
        QTableWidgetItem *item = pTable->itemAt(pos);
        if (NULL != item)
        {
            QMenu *menu = new QMenu;
            QAction* copyToClipboard = menu->addAction(tr("Copy"));
            QAction* res = menu->exec(pTable->viewport()->mapToGlobal(pos));
            if (NULL != res)
            {
                if (res == copyToClipboard)
                {
                    QClipboard *clipboard = QApplication::clipboard();
                    clipboard->setText(item->text());
                }
            }
        }
    }
}