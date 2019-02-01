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

#if defined( TRIDIM_USE_GDCM )

#include <data/CDicomSaverGDCM.h>
#include "data/CVolumeOfInterestData.h"
#include "data/CMultiClassRegionData.h"
#include <data/CMultiClassRegionColoring.h>
#include "data/CRegionData.h"
#include <data/CRegionColoring.h>

#include <data/CVolumeTransformation.h>
//#include <mainwindow.h>
#include <PluginInterface.h>
#include <data/CDensityData.h>

//VPL
#include <VPL/ImageIO/DefTagGDCM.h>

//GDCM
#include <gdcmImageChangeTransferSyntax.h>
#include <gdcmAnonymizer.h>
#include <gdcmUIDGenerator.h>
#include <gdcmAttribute.h>
#include <gdcmImageHelper.h>

//osg
#include <osg/Vec3>

//STL
#include <memory>

namespace data
{

CDicomSaverGDCM::CDicomSaverGDCM()
{
}

CDicomSaverGDCM::~CDicomSaverGDCM()
{
}

/*bool CDicomSaverGDCM::saveSerie(std::string dirName, bool bSaveSegmented, bool bSaveCompressed,
                                bool bSaveActive, bool bSaveVOI, 
                                bool bAnonymize, std::string anonymString, std::string anonymID, vpl::mod::CProgress::tProgressFunc & progress)
{

    //pointer do data storage
    CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(data::Storage::PatientData::Id));

    //helper variables
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

    //generate new UIDS
    gdcm::UIDGenerator gen;
    std::string studyUID = gen.Generate();
    std::string seriesUID = gen.Generate();
    
    //check if dataset was loaded from multiframe DICOM, if so, it's neccessary to save
    //data as multiframe, because GDCM will try to save it as multiframe (nested position....), when
    //using original Media Storage
    bool isMultiframe = false;
    gdcm::MediaStorage ms = gdcm::MediaStorage::MS_END;
    ms = gdcm::MediaStorage::GetMSType(spData->m_sMediaStorage.c_str());
    if (ms == gdcm::MediaStorage::EnhancedCTImageStorage
        || ms == gdcm::MediaStorage::EnhancedMRImageStorage
        || ms == gdcm::MediaStorage::EnhancedPETImageStorage
        || ms == gdcm::MediaStorage::MultiframeGrayscaleWordSecondaryCaptureImageStorage
        || ms == gdcm::MediaStorage::MultiframeGrayscaleByteSecondaryCaptureImageStorage
        || ms == gdcm::MediaStorage::SegmentationStorage)
    {
        isMultiframe = true;
    }

    //image writer, wich will create DICOM files 
    gdcm::ImageWriter writer;

    //image
    gdcm::Image &image = writer.GetImage();

    image.SetNumberOfDimensions(2);

    image.SetDimension(0, sizeX);
    image.SetDimension(1, sizeY);

    if (isMultiframe)
    {
        image.SetNumberOfDimensions(3);
        image.SetDimension(2, sizeZ);
    }

    //Image Position
    data::CObjectPtr<data::CVolumeTransformation> spVolumeTransformation(APP_STORAGE.getEntry(data::Storage::VolumeTransformation::Id));
    osg::Matrix volTransform = spVolumeTransformation->getTransformation();
    osg::Vec3 offLoad = -volTransform.getTrans();
    image.SetOrigin(0, spData->m_ImagePosition.getX() + offLoad[0]);
    image.SetOrigin(1, spData->m_ImagePosition.getY() + offLoad[1]);

    //Image Orientation
    image.SetDirectionCosines(0, 1.0);
    image.SetDirectionCosines(1, 0.0);
    image.SetDirectionCosines(2, 0.0);

    image.SetDirectionCosines(3, 0.0);
    image.SetDirectionCosines(4, 1.0);
    image.SetDirectionCosines(5, 0.0);

    //spacing
    image.SetSpacing(0, dX);
    image.SetSpacing(1, dY);
    image.SetSpacing(2, dZ);

    //Slope and intercept were aldready applied during loading, so it's save to use default values
    image.SetIntercept(0);
    image.SetSlope(1);

    gdcm::DataSet &ds = writer.GetFile().GetDataSet();

    //General information
    insertDataElement<VPL_GDCM_PatientName>(ds, spData->m_sPatientName.c_str());
    insertDataElement<VPL_GDCM_PatientID>(ds, spData->m_sPatientId.c_str());
    insertDataElement<VPL_GDCM_PatientSex>(ds, spData->m_sPatientSex.c_str());

    insertDataElement<VPL_GDCM_Manufacturer>(ds, "3Dim Laboratory");
    {
        app::CProductInfo info = app::getProductInfo();
        std::string product = QString("%1 %2.%3.%4").arg(QCoreApplication::applicationName()).arg(info.getVersion().getMajorNum()).arg(info.getVersion().getMinorNum()).arg(info.getVersion().getBuildNum()).toStdString();
        insertDataElement<VPL_GDCM_ManufacturerModelName>(ds, product.c_str());
    }

    insertDataElement<VPL_GDCM_PatientPosition>(ds, spData->m_sPatientPosition.c_str());
    insertDataElement<VPL_GDCM_PatientComments>(ds, spData->m_sPatientDescription.c_str());
    insertDataElement<VPL_GDCM_PatientBirthDate>(ds, spData->m_sPatientBirthday.c_str());

    insertDataElement<VPL_GDCM_SeriesDescription>(ds, spData->m_sSeriesDescription.c_str());
    insertDataElement<VPL_GDCM_StudyDescription>(ds, spData->m_sStudyDescription.c_str());

    insertDataElement<VPL_GDCM_Modality>(ds, spData->m_sModality.c_str());
    insertDataElement<VPL_GDCM_ScanOptions>(ds, spData->m_sScanOptions.c_str());

    insertDataElement<VPL_GDCM_SliceThickness>(ds, QString::number(dZ).toStdString().c_str());

    insertDataElement<VPL_GDCM_ImageType>(ds, "DERIVED");

    //anonymize
    if (bAnonymize)
    {
        anonymize(writer.GetFile());
    }

    //Insert values from user, when specified
    if (!anonymString.empty())
        insertDataElement<VPL_GDCM_PatientName>(ds, anonymString.c_str());
    if (!anonymID.empty())
        insertDataElement<VPL_GDCM_PatientID>(ds, anonymID.c_str());

    //uids  
    addUIDs(true, spData->m_sMediaStorage, studyUID, seriesUID, writer);
        
    //buffer size for raw image data is different for single and multiframe DICOMs
    unsigned long bufferSize;
    if(isMultiframe)
        bufferSize = 2 *sizeZ*sizeX*sizeY;
    else
        bufferSize = 2 * sizeX*sizeY;

    //buffer for image data
    std::unique_ptr<int16_t[]> ptr(new int16_t[bufferSize]);

    //previous part was identical for all new DICOM files, now iterate through all slices and load image data
    for (int z = minZ; z < maxZ; ++z)
    {
        //show progress
        if (!progress(z, maxZ))
        {
            return false;
        }

        //Pixel format - must be defined in each iteration, because compression overwrites it
        gdcm::PixelFormat pf = gdcm::PixelFormat::INT16;
        pf.SetSamplesPerPixel(1);
        pf.SetBitsAllocated(16);
        pf.SetBitsAllocated(16);
        pf.SetHighBit(15);
        image.SetPixelFormat(pf);
        gdcm::PhotometricInterpretation pi = gdcm::PhotometricInterpretation::MONOCHROME2;
        image.SetPhotometricInterpretation(pi);

        image.SetTransferSyntax(gdcm::TransferSyntax::ExplicitVRLittleEndian);

        //get pixel data
        if (isMultiframe)
            //in case of multiframe, load pixel data from all slices (z, maxZ)
            loadPixelData(minX, maxX, minY, maxY, z, maxZ, bSaveSegmented, bSaveActive, ptr.get());
        else
            //single frame, only one iteration (z, z+1)
            loadPixelData(minX, maxX, minY, maxY, z, z + 1, bSaveSegmented, bSaveActive,ptr.get());
            
        image.SetOrigin(2, spData->m_ImagePosition.getZ() + offLoad[2] + z *dZ );        
        insertDataElement<VPL_GDCM_InstanceNumber>(ds, QString::number(z).toStdString().c_str());

        if (!isMultiframe) // set image position tag
        {            
            gdcm::Attribute<VPL_GDCM_ImagePositionPatient> atpp;
            for (unsigned int i=0; i<3; ++i)
                atpp.SetValue(image.GetOrigin(i),i);

            gdcm::Tag tag(VPL_GDCM_ImagePositionPatient);
            //insert or replace
            if (ds.FindDataElement(tag))
                ds.Replace(atpp.GetAsDataElement());
            else
                ds.Insert(atpp.GetAsDataElement());
        }
        
        //copy pixel data to image
        gdcm::DataElement pixeldata(gdcm::Tag(0x7fe0, 0x0010));
        pixeldata.SetByteValue((char*)ptr.get(), (uint32_t)bufferSize);
        image.SetDataElement(pixeldata);
       
        //compression        
        if (bSaveCompressed)
        {
           if (!compress(image))
               throw CDicomSaverExceptionCompression();
        }   

        //absolute path to new file 
        std::stringstream stringStream;
        stringStream << dirName << "\\" << z << ".dcm";
        std::string absolutePath = stringStream.str();

        //times
        addStudyDateTime(ds, absolutePath.c_str());       

        //write output
        writer.SetFileName(absolutePath.c_str());
        if (!writer.Write())
        {            
            CDicomSaverExceptionFile();
            return false;
        } 

        //if it is multiframe, break from this cycle, because all image data were already loaded
        //we save only single file
        if (isMultiframe)
            break;
    }

    return true;
}*/

bool CDicomSaverGDCM::saveSerie(std::string dirName, bool bSaveSegmented, bool bSaveCompressed,
    bool bSaveActive, bool bSaveVOI,
    bool bAnonymize, std::string anonymString, std::string anonymID, vpl::mod::CProgress::tProgressFunc & progress)
{

    //pointer do data storage
    CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(data::Storage::PatientData::Id));

    //helper variables
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

    //generate new UIDS
    gdcm::UIDGenerator gen;
    std::string studyUID = gen.Generate();
    std::string seriesUID = gen.Generate();

    //check if dataset was loaded from multiframe DICOM, if so, it's neccessary to save
    //data as multiframe, because GDCM will try to save it as multiframe (nested position....), when
    //using original Media Storage
    bool isMultiframe = false;
    gdcm::MediaStorage ms = gdcm::MediaStorage::MS_END;
    ms = gdcm::MediaStorage::GetMSType(spData->m_sMediaStorage.c_str());
    if (ms == gdcm::MediaStorage::EnhancedCTImageStorage
        || ms == gdcm::MediaStorage::EnhancedMRImageStorage
        || ms == gdcm::MediaStorage::EnhancedPETImageStorage
        || ms == gdcm::MediaStorage::MultiframeGrayscaleWordSecondaryCaptureImageStorage
        || ms == gdcm::MediaStorage::MultiframeGrayscaleByteSecondaryCaptureImageStorage
        || ms == gdcm::MediaStorage::SegmentationStorage)
    {
        isMultiframe = true;
    }

    //image writer, wich will create DICOM files 
    gdcm::ImageWriter writer;

    //image
    gdcm::Image &image = writer.GetImage();

    image.SetNumberOfDimensions(2);

    image.SetDimension(0, sizeX);
    image.SetDimension(1, sizeY);

    if (isMultiframe)
    {
        image.SetNumberOfDimensions(3);
        image.SetDimension(2, sizeZ);
    }

    //Image Position
    data::CObjectPtr<data::CVolumeTransformation> spVolumeTransformation(APP_STORAGE.getEntry(data::Storage::VolumeTransformation::Id));
    osg::Matrix volTransform = spVolumeTransformation->getTransformation();
    osg::Vec3 offLoad = -volTransform.getTrans();
    image.SetOrigin(0, spData->m_ImagePosition.getX() + offLoad[0]);
    image.SetOrigin(1, spData->m_ImagePosition.getY() + offLoad[1]);

    //Image Orientation
    image.SetDirectionCosines(0, 1.0);
    image.SetDirectionCosines(1, 0.0);
    image.SetDirectionCosines(2, 0.0);

    image.SetDirectionCosines(3, 0.0);
    image.SetDirectionCosines(4, 1.0);
    image.SetDirectionCosines(5, 0.0);

    //spacing
    image.SetSpacing(0, dX);
    image.SetSpacing(1, dY);
    image.SetSpacing(2, dZ);

    //Slope and intercept were aldready applied during loading, so it's save to use default values
    image.SetIntercept(0);
    image.SetSlope(1);

    gdcm::DataSet &ds = writer.GetFile().GetDataSet();

    //General information
    insertDataElement<VPL_GDCM_PatientName>(ds, spData->m_sPatientName.c_str());
    insertDataElement<VPL_GDCM_PatientID>(ds, spData->m_sPatientId.c_str());
    insertDataElement<VPL_GDCM_PatientSex>(ds, spData->m_sPatientSex.c_str());

    insertDataElement<VPL_GDCM_Manufacturer>(ds, "3Dim Laboratory");
    {
        app::CProductInfo info = app::getProductInfo();
        std::string product = QString("%1 %2.%3.%4").arg(QCoreApplication::applicationName()).arg(info.getVersion().getMajorNum()).arg(info.getVersion().getMinorNum()).arg(info.getVersion().getBuildNum()).toStdString();
        insertDataElement<VPL_GDCM_ManufacturerModelName>(ds, product.c_str());
    }

    insertDataElement<VPL_GDCM_PatientPosition>(ds, spData->m_sPatientPosition.c_str());
    insertDataElement<VPL_GDCM_PatientComments>(ds, spData->m_sPatientDescription.c_str());
    insertDataElement<VPL_GDCM_PatientBirthDate>(ds, spData->m_sPatientBirthday.c_str());

    insertDataElement<VPL_GDCM_SeriesDescription>(ds, spData->m_sSeriesDescription.c_str());
    insertDataElement<VPL_GDCM_StudyDescription>(ds, spData->m_sStudyDescription.c_str());

    insertDataElement<VPL_GDCM_Modality>(ds, spData->m_sModality.c_str());
    insertDataElement<VPL_GDCM_ScanOptions>(ds, spData->m_sScanOptions.c_str());

    insertDataElement<VPL_GDCM_SliceThickness>(ds, QString::number(dZ).toStdString().c_str());

    std::stringstream spacing;
    spacing << std::fixed << std::setprecision(6) << dX << "\\" << dY;
    std::string sspacing = spacing.str();
    insertDataElement<VPL_GDCM_PixelSpacing>(ds, sspacing.c_str());

    // anonymization
    insertDataElement<VPL_GDCM_PatientName>(ds, bAnonymize ? anonymString.c_str() : spData->m_sPatientName.c_str());
    insertDataElement<VPL_GDCM_PatientID>(ds, bAnonymize ? anonymID.c_str() : spData->m_sPatientId.c_str());
    insertDataElement<VPL_GDCM_PatientSex>(ds, bAnonymize ? "" : spData->m_sPatientSex.c_str());
    insertDataElement<VPL_GDCM_PatientPosition>(ds, bAnonymize ? "" : spData->m_sPatientPosition.c_str());
    insertDataElement<VPL_GDCM_PatientComments>(ds, bAnonymize ? "" : spData->m_sPatientDescription.c_str());
    insertDataElement<VPL_GDCM_PatientBirthDate>(ds, bAnonymize ? "" : spData->m_sPatientBirthday.c_str());

    //uids
    insertDataElement<VPL_GDCM_SOPClassUID>(ds, spData->m_sMediaStorage.c_str());
    //insertDataElement<VPL_GDCM_SOPInstanceUID>(ds, uid);

    if (spData->m_sStudyUid.empty())
    {
        insertDataElement<VPL_GDCM_StudyInstanceUID>(ds, studyUID.c_str());
    }
    else
    {
        insertDataElement<VPL_GDCM_StudyInstanceUID>(ds, spData->m_sStudyUid.c_str());
    }

    if (spData->m_sSeriesUid.empty())
    {
        insertDataElement<VPL_GDCM_SeriesInstanceUID>(ds, seriesUID.c_str());
    }
    else
    {
        insertDataElement<VPL_GDCM_SeriesInstanceUID>(ds, spData->m_sSeriesUid.c_str());
    }

    //buffer size for raw image data is different for single and multiframe DICOMs
    unsigned long bufferSize;
    if (isMultiframe)
        bufferSize = 2 * sizeZ*sizeX*sizeY;
    else
        bufferSize = 2 * sizeX*sizeY;

    //buffer for image data
    std::unique_ptr<int16_t[]> ptr(new int16_t[bufferSize]);

    //previous part was identical for all new DICOM files, now iterate through all slices and load image data
    for (int z = minZ; z < maxZ; ++z)
    {
        //show progress
        if (!progress(z, maxZ))
        {
            return false;
        }

        //Pixel format - must be defined in each iteration, because compression overwrites it
        gdcm::PixelFormat pf = gdcm::PixelFormat::INT16;
        pf.SetSamplesPerPixel(1);
        pf.SetBitsAllocated(16);
        pf.SetBitsAllocated(16);
        pf.SetHighBit(15);
        image.SetPixelFormat(pf);
        gdcm::PhotometricInterpretation pi = gdcm::PhotometricInterpretation::MONOCHROME2;
        image.SetPhotometricInterpretation(pi);

        image.SetTransferSyntax(gdcm::TransferSyntax::ExplicitVRLittleEndian);

        //get pixel data
        if (isMultiframe)
            //in case of multiframe, load pixel data from all slices (z, maxZ)
            loadPixelData(minX, maxX, minY, maxY, z, maxZ, bSaveSegmented, bSaveActive, ptr.get());
        else
            //single frame, only one iteration (z, z+1)
            loadPixelData(minX, maxX, minY, maxY, z, z + 1, bSaveSegmented, bSaveActive, ptr.get());

        image.SetOrigin(2, spData->m_ImagePosition.getZ() + offLoad[2] + z * dZ);
        insertDataElement<VPL_GDCM_InstanceNumber>(ds, QString::number(z).toStdString().c_str());

        if (!isMultiframe) // set image position tag
        {
            gdcm::Attribute<VPL_GDCM_ImagePositionPatient> atpp;
            for (unsigned int i = 0; i<3; ++i)
                atpp.SetValue(image.GetOrigin(i), i);

            gdcm::Tag tag(VPL_GDCM_ImagePositionPatient);
            //insert or replace
            if (ds.FindDataElement(tag))
                ds.Replace(atpp.GetAsDataElement());
            else
                ds.Insert(atpp.GetAsDataElement());
        }

        //copy pixel data to image
        gdcm::DataElement pixeldata(gdcm::Tag(0x7fe0, 0x0010));
        pixeldata.SetByteValue((char*)ptr.get(), (uint32_t)bufferSize);
        image.SetDataElement(pixeldata);

        //compression        
        if (bSaveCompressed)
        {
            if (!compress(image))
                throw CDicomSaverExceptionCompression();
        }

        //absolute path to new file 
        std::stringstream stringStream;
        stringStream << dirName << "\\" << z << ".dcm";
        std::string absolutePath = stringStream.str();

        //times
        insertDataElement<VPL_GDCM_StudyDate>(ds, spData->m_sStudyDate.c_str());
        //insertDataElement<VPL_GDCM_StudyTime>(ds, date + datelen);

        //write output
        writer.SetFileName(absolutePath.c_str());
        if (!writer.Write())
        {
            CDicomSaverExceptionFile();
            return false;
        }

        //if it is multiframe, break from this cycle, because all image data were already loaded
        //we save only single file
        if (isMultiframe)
            break;
    }

    return true;
}

void CDicomSaverGDCM::anonymize(gdcm::File& file)
{
    gdcm::Anonymizer anonymizer;
    anonymizer.SetFile(file);
    // vector of all tags, that should be anonymized
    std::vector<gdcm::Tag> tagsToBeAnnonymized =
        anonymizer.GetBasicApplicationLevelConfidentialityProfileAttributes();

    //Empty values
    for (std::vector<gdcm::Tag>::iterator it = tagsToBeAnnonymized.begin(); it != tagsToBeAnnonymized.end(); ++it)
    {
        anonymizer.Empty(*it);
    }
}

bool CDicomSaverGDCM::compress(gdcm::Image& image)
{
    gdcm::ImageChangeTransferSyntax change;
    //change.SetTransferSyntax(gdcm::TransferSyntax::JPEG2000Lossless);
    change.SetForce(true);
    change.SetInput(image);

    //try to compress
    if (!change.Change())
    {
        return false;
    }
    image.SetTransferSyntax(gdcm::TransferSyntax::JPEG2000Lossless);

    return true;
}

bool CDicomSaverGDCM::addStudyDateTime(gdcm::DataSet &ds, const char* filename)
{
    // StudyDate
    char date[22];
    const size_t datelen = 8;
    int res = gdcm::System::GetCurrentDateTime(date);

    if (!res) return false;

    insertDataElement<VPL_GDCM_StudyDate>(ds, date);
    insertDataElement<VPL_GDCM_StudyTime>(ds, date + datelen);

    return addContentDateTime(ds, filename);
}

bool CDicomSaverGDCM::addContentDateTime(gdcm::DataSet &ds, const char* filename)
{
    time_t studydatetime = gdcm::System::FileTime(filename);
    char date[22];
    gdcm::System::FormatDateTime(date, studydatetime);
    const size_t datelen = 8;
  
    insertDataElement<VPL_GDCM_ContentDate>(ds, date);  
    insertDataElement<VPL_GDCM_ContentTime>(ds, date + datelen);      
    
    return true;
}

bool CDicomSaverGDCM::addUIDs(bool sopclassuid, std::string const& sopclass, std::string const& study_uid, std::string const& series_uid, gdcm::ImageWriter& writer)
{
    gdcm::DataSet &ds = writer.GetFile().GetDataSet();
    gdcm::MediaStorage ms = gdcm::MediaStorage::MS_END;
    gdcm::Image &image = writer.GetImage();
    if (sopclassuid)
    {
        // Is it by value or by name ?
        if (gdcm::UIDGenerator::IsValid(sopclass.c_str()))
        {
            ms = gdcm::MediaStorage::GetMSType(sopclass.c_str());
        }
        else
        {
            //set default media storage
            ms = gdcm::MediaStorage::SecondaryCaptureImageStorage;
        }
    }
    else
    { // guess a default
        ms = gdcm::ImageHelper::ComputeMediaStorageFromModality(
            "OT", image.GetNumberOfDimensions(),
            image.GetPixelFormat(), image.GetPhotometricInterpretation());
    }

    if (!gdcm::MediaStorage::IsImage(ms))
    {
        throw CDicomSaverExceptionMS();
    }
    if (ms.GetModalityDimension() < image.GetNumberOfDimensions())
    {
        throw CDicomSaverExceptionMS();
    }
    const char* msstr = gdcm::MediaStorage::GetMSString(ms);
    if (!msstr)
    {
        throw CDicomSaverExceptionMS();
    }   
        
    insertDataElement<VPL_GDCM_SOPClassUID>(ds, msstr);    
    insertDataElement<VPL_GDCM_StudyInstanceUID>(ds, study_uid.c_str());
    insertDataElement<VPL_GDCM_SeriesInstanceUID>(ds, series_uid.c_str());
    
    return true;
}

void CDicomSaverGDCM::loadPixelData(int minX, int maxX, int minY, int maxY, int minZ, int maxZ,
                                    bool bSaveSegmented, bool bSaveActive, int16_t* pData)
{
    CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(data::Storage::PatientData::Id));

    if (bSaveSegmented)
    {
        CObjectPtr<data::CMultiClassRegionColoring> spColoringMultiClass(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id));
        CObjectPtr<data::CMultiClassRegionData> rVolumeMultiClass(APP_STORAGE.getEntry(data::Storage::MultiClassRegionData::Id));
        CObjectPtr<data::CRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id));
        CObjectPtr<data::CRegionData> rVolume(APP_STORAGE.getEntry(data::Storage::RegionData::Id));

        int activeRegion;
        bool useMultiClass = false;

        if (rVolumeMultiClass->hasData())
        {
            activeRegion = spColoringMultiClass->getActiveRegion();
            useMultiClass = true;
        }
        else
        {
            activeRegion = spColoring->getActiveRegion();
        }

        if (bSaveActive)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                for (int y = minY; y < maxY; ++y)
                {
                    for (int x = minX; x < maxX; ++x)
                    {
                        //save segmented data, active region only
                        
                        if ((useMultiClass && rVolumeMultiClass->at(x, y, z, activeRegion)) || (!useMultiClass && rVolume->at(x, y, z) == activeRegion))
                        {
                            *pData = spData->at(x, y, z);
                        }
                        else
                        {
                            //air
                            *pData = -1000;
                        }
                        pData++;
                    }
                }
            }
        }
        else
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                for (int y = minY; y < maxY; ++y)
                {
                    for (int x = minX; x < maxX; ++x)
                    {
                        //save all segmented data
                        if ((useMultiClass && rVolumeMultiClass->at(x, y, z) != 0) || (!useMultiClass && rVolume->at(x, y, z) != 0))
                        {
                            *pData = spData->at(x, y, z);
                        }
                        else
                        {
                            //air
                            *pData = -1000;
                        }
                        pData++;
                    }
                }
            }
        }
    }
    else
    {
        for (int z = minZ; z < maxZ; ++z)
        {
            for (int y = minY; y < maxY; ++y)
            {
                for (int x = minX; x < maxX; ++x)
                {
                    //save all data
                    *pData = spData->at(x, y, z);
                    pData++;
                }
            }
        }
    }
    
}

template <uint16_t Group, uint16_t Element>
void CDicomSaverGDCM::insertDataElement(gdcm::DataSet &ds, const char* val)
{
   
    gdcm::Tag tag(Group, Element);
    gdcm::DataElement de(tag);
    de.SetByteValue(val, strlen(val));
    de.SetVR(gdcm::Attribute< Group, Element >::GetVR());

    //insert or replace
    if (ds.FindDataElement(tag))
        ds.Replace(de);
    else
        ds.Insert(de);
}

}// namespace data
#endif //TRIDIM_USE_GDCM
