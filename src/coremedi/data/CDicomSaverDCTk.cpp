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

//3DV
#include <data/CDicomSaverDCTk.h>

#include <PluginInterface.h>
#include <data/CDensityData.h>
#include <data/CVolumeTransformation.h>
#include <data/CRegionData.h>
#include <data/CVolumeOfInterestData.h>
#include <VPL/Module/Progress.h>
#include <data/CRegionColoring.h>

//STL
#include <memory>

// DCMTK
#ifdef __APPLE__
  #undef UNICODE
#endif
#include "dcmtk/config/osconfig.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmjpeg/djrplol.h"
#include "dcmtk/dcmdata/dcmetinf.h"

namespace data
{

CDicomSaverDCTk::CDicomSaverDCTk()
{
}

CDicomSaverDCTk::~CDicomSaverDCTk()
{
}

bool CDicomSaverDCTk::saveSerie(std::string dirName, bool bSaveSegmented, bool bSaveCompressed,
                                bool bSaveActive, bool bSaveVOI, bool bAnonymize,
                                std::string anonymString, std::string anonymID, vpl::mod::CProgress::tProgressFunc & progress)
{
    

    data::CObjectPtr<data::CDensityData> spData( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );    
    int sizeX = spData->getXSize();
    int sizeY = spData->getYSize();
    int sizeZ = spData->getZSize();
    int minX = 0;
    int minY = 0;
    int minZ = 0;
    int maxX = spData->getXSize();
    int maxY = spData->getYSize();
    int maxZ = spData->getZSize();
    const double dX = spData->getDX();
    const double dY = spData->getDY();
    const double dZ = spData->getDZ();

    bool bOk = true;

    QString SeriesID(spData->m_sSeriesUid.c_str());	
    if (SeriesID.isEmpty())
    	SeriesID = "cs" + QString::number(spData->getDataCheckSum());

    if (bSaveVOI)
    {
    data::CObjectPtr<data::CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(data::Storage::VolumeOfInterestData::Id));

    minX = spVOI->getMinX();
    minY = spVOI->getMinY();
    minZ = spVOI->getMinZ();
    maxX = spVOI->getMaxX() + 1;
    maxY = spVOI->getMaxY() + 1;
    maxZ = spVOI->getMaxZ() + 1;
    sizeX = maxX - minX;
    sizeY = maxY - minY;
    sizeZ = maxZ - minZ;
    }

//
//#ifdef WIN32
//    // path in ACP
//    std::wstring uniName = (const wchar_t*)dirName.utf16();
//    std::string ansiName = wcs2ACP(uniName);
//#else // UTF8
//    std::string ansiName = fileName.toStdString();
//#endif

#ifdef __APPLE__
    std::auto_ptr<qint16> ptr(new qint16[2 * sizeX*sizeY]);
#else
    std::unique_ptr<qint16> ptr(new qint16[2 * sizeX*sizeY]);
#endif

    // TODO: check whether the top 3 slices are not empty ones (used for volume size alignment)

    char uid[128] = {};
    char studyUIDStr[128] = {};
    char seriesUIDStr[128] = {};
    dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT);
    dcmGenerateUniqueIdentifier(studyUIDStr, SITE_STUDY_UID_ROOT);
    dcmGenerateUniqueIdentifier(seriesUIDStr, SITE_SERIES_UID_ROOT);

    DJEncoderRegistration::registerCodecs(); // register JPEG codecs
    for (int z = minZ; z < maxZ && bOk; z++)
    {
        if (!progress(z, maxZ))
        {
            bOk = false;
            break;
        }

        // prepare pixel data
        if (bSaveSegmented)
        {
            data::CObjectPtr< data::CRegionData > rVolume(APP_STORAGE.getEntry(data::Storage::RegionData::Id));
            if (rVolume->getSize() == spData->getSize())
            {
                if (bSaveActive)
                {
                    data::CObjectPtr<data::CRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id));
                    const int activeRegion(spColoring->getActiveRegion());
                    qint16* pData = ptr.get();
                    for (int y = minY; y < maxY; y++)
                    {
                        for (int x = minX; x < maxX; x++)
                        {
                            if (rVolume->at(x, y, z) == activeRegion)
                                *pData = std::max(spData->at(x, y, z) + 1024, 0);
                            else
                                *pData = 0;
                            pData++;
                        }
                    }
                }
                else
                {
                    qint16* pData = ptr.get();
                    for (int y = minY; y < maxY; y++)
                    {
                        for (int x = minX; x < maxX; x++)
                        {
                            if (rVolume->at(x, y, z) != 0)
                                *pData = std::max(spData->at(x, y, z) + 1024, 0);
                            else
                                *pData = 0;
                            pData++;
                        }
                    }
                }
            }
        }
        else // all data
        {
            qint16* pData = ptr.get();
            for (int y = minY; y < maxY; y++)
            {
                for (int x = minX; x < maxX; x++)
                {
                    *pData = std::max(spData->at(x, y, z) + 1024, 0);
                    pData++;
                }
            }
        }


        //DCMTk
        // create dicom structures
        DcmFileFormat fileFormat;
        DcmDataset *pDataset = fileFormat.getDataset();

        {	
            // set dicom tags	
            //if (bSaveSegmented)
            //    pDataset->putAndInsertString(DCM_SOPClassUID, UID_SecondaryCaptureImageStorage);
            //else
                pDataset->putAndInsertString(DCM_SOPClassUID, spData->m_sMediaStorage.c_str());

            pDataset->putAndInsertString(DCM_SOPInstanceUID, uid);
            if (spData->m_sStudyUid.empty())
                pDataset->putAndInsertString(DCM_StudyInstanceUID, studyUIDStr);
            else
                pDataset->putAndInsertString(DCM_StudyInstanceUID, spData->m_sStudyUid.c_str());

            if (spData->m_sSeriesUid.empty())
                pDataset->putAndInsertString(DCM_SeriesInstanceUID, seriesUIDStr);
            else
                pDataset->putAndInsertString(DCM_SeriesInstanceUID, spData->m_sSeriesUid.c_str());

            pDataset->putAndInsertString(DCM_Manufacturer, "3Dim Laboratory");
            {
                app::CProductInfo info = app::getProductInfo();
                std::string product = QString("%1 %2.%3.%4").arg(QCoreApplication::applicationName()).arg(info.getVersion().getMajorNum()).arg(info.getVersion().getMinorNum()).arg(info.getVersion().getBuildNum()).toStdString();
                pDataset->putAndInsertString(DCM_ManufacturerModelName, product.c_str());
            }

            pDataset->putAndInsertString(DCM_PatientName, bAnonymize ? anonymString.c_str() : spData->m_sPatientName.c_str());
            pDataset->putAndInsertString(DCM_PatientID, bAnonymize ? anonymID.c_str() : spData->m_sPatientId.c_str());
            pDataset->putAndInsertString(DCM_PatientSex, bAnonymize ? "" : spData->m_sPatientSex.c_str());
            pDataset->putAndInsertString(DCM_PatientPosition, bAnonymize ? "" : spData->m_sPatientPosition.c_str());
            pDataset->putAndInsertString(DCM_PatientComments, bAnonymize ? "" : spData->m_sPatientDescription.c_str());
            pDataset->putAndInsertString(DCM_PatientBirthDate, bAnonymize ? "" : spData->m_sPatientBirthday.c_str());

            pDataset->putAndInsertString(DCM_SeriesDescription, spData->m_sSeriesDescription.c_str());
            pDataset->putAndInsertString(DCM_StudyDescription, spData->m_sStudyDescription.c_str());

            pDataset->putAndInsertString(DCM_SeriesDate, spData->m_sSeriesDate.c_str());
            pDataset->putAndInsertString(DCM_SeriesTime, spData->m_sSeriesTime.c_str());

            pDataset->putAndInsertString(DCM_StudyDate, spData->m_sStudyDate.c_str());

            pDataset->putAndInsertString(DCM_Modality, spData->m_sModality.c_str());
            pDataset->putAndInsertString(DCM_ScanOptions, spData->m_sScanOptions.c_str());

            pDataset->putAndInsertString(DCM_ImageOrientationPatient, "1.000000\0.000000\0.000000\0.000000\1.000000\0.000000");

            std::stringstream spacing;
            spacing << std::fixed << std::setprecision(6) << dX << "\\" << dY;
            std::string sspacing = spacing.str();
            pDataset->putAndInsertString(DCM_PixelSpacing, sspacing.c_str());
            pDataset->putAndInsertUint16(DCM_Rows, sizeY);
            pDataset->putAndInsertUint16(DCM_Columns, sizeX);
            pDataset->putAndInsertUint16(DCM_PixelRepresentation, 0); // 1 signed, 0 unsigned
            //pDataset->putAndInsertUint16(DCM_NumberOfFrames,1);
            pDataset->putAndInsertUint16(DCM_SamplesPerPixel, 1); // count of channels			
            pDataset->putAndInsertString(DCM_PhotometricInterpretation, "MONOCHROME2");
            //pDataset->putAndInsertUint16(DCM_PlanarConfiguration,0);
            pDataset->putAndInsertUint16(DCM_BitsAllocated, 16);
            pDataset->putAndInsertUint16(DCM_BitsStored, 16);
            pDataset->putAndInsertUint16(DCM_HighBit, 15);
            pDataset->putAndInsertString(DCM_RescaleIntercept, "-1024");
            pDataset->putAndInsertString(DCM_RescaleSlope, "1");
            //pDataset->putAndInsertString(DCM_WindowCenter, "0");
            //pDataset->putAndInsertString(DCM_WindowWidth, "1000");
            {
                data::CObjectPtr<data::CVolumeTransformation> spVolumeTransformation(APP_STORAGE.getEntry(data::Storage::VolumeTransformation::Id));
                osg::Matrix volTransform = spVolumeTransformation->getTransformation();
                osg::Vec3 offLoad = -volTransform.getTrans();

                std::stringstream pos;
                pos << std::fixed << std::setprecision(6) << spData->m_ImagePosition.x() + offLoad[0] << "\\" << spData->m_ImagePosition.y() + offLoad[1] << "\\" << spData->m_ImagePosition.z() + offLoad[2] + z * dZ;
                std::string spos = pos.str();
                pDataset->putAndInsertString(DCM_ImagePositionPatient, spos.c_str());
            }
                {
                    std::stringstream thickness;
                    thickness << dZ;
                    pDataset->putAndInsertString(DCM_SliceThickness, thickness.str().c_str());
                }
                pDataset->putAndInsertUint16(DCM_InstanceNumber, z);

                pDataset->putAndInsertUint16Array(DCM_PixelData, (Uint16*)ptr.get(), sizeX*sizeY);
        }

        // data compression
        E_TransferSyntax xfer = EXS_Unknown;
        if (bSaveCompressed)
        {
            DJ_RPLossless params; // codec parameters, we use the defaults        			
            // this causes the lossless JPEG version of the dataset to be created
#if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361 || PACKAGE_VERSION_NUMBER == 362)
            pDataset->chooseRepresentation(EXS_JPEGProcess14SV1, &params);
#else
            pDataset->chooseRepresentation(EXS_JPEGProcess14SV1TransferSyntax, &params);
#endif

            // check if everything went well
#if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361 || PACKAGE_VERSION_NUMBER == 362)
            if (pDataset->canWriteXfer(EXS_JPEGProcess14SV1))
            {
                xfer = EXS_JPEGProcess14SV1;
#else
            if (pDataset->canWriteXfer(EXS_JPEGProcess14SV1TransferSyntax))
            {
                xfer = EXS_JPEGProcess14SV1TransferSyntax;
#endif
            }
            }

            {	// save file
                std::stringstream ss;
                ss << dirName << "/" << z << ".dcm";
                std::string curFile = ss.str();

                if (xfer == EXS_Unknown)
                    xfer = EXS_LittleEndianExplicit;
                OFCondition status = fileFormat.saveFile(curFile.c_str(), xfer);
                if (status.bad())
                    bOk = false;
            }
        }

    DJEncoderRegistration::cleanup(); // deregister JPEG codecs


    return bOk;
}
}// namespace data
