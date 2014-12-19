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

///////////////////////////////////////////////////////////////////////////////
// include files

#include <data/CExamination.h>
#include <app/Signals.h>

#include "AppConfigure.h"


#include "data/CActiveDataSet.h"
#include "data/CDensityWindow.h"
#include "data/CDensityData.h"
#include "data/COrthoSlice.h"
#include "data/CCoordinatesConv.h"
#include "data/CVolumeOfInterest.h"
#include "data/CRegionData.h"
#include "data/CRegionColoring.h"
#include "data/CSceneManipulatorDummy.h"
#include "data/CDrawingOptions.h"
#include "data/CMeasurementOptions.h"
#include "data/CUndoManager.h"
#include "data/CImageLoaderInfo.h"
#include "data/CAllDrawings.h"
#include "data/CSeries.h"
#include "data/CSavedEntries.h"
#include "data/CDataStats.h"
#include <data/CSceneWidgetParameters.h>
#include "data/CPreviewModel.h"
#include "data/CAppSettings.h"
#include <data/CRegionData.h>
#include <data/CRegionColoring.h>


#include <VPL/Base/Logging.h>
#include <VPL/Image/VolumeFunctions.h>
#include <VPL/ImageIO/DicomSlice.h>
#include <VPL/Module/Serialization.h>
#include <VPL/Math/StaticVector.h>
#include <VPL/Math/MatrixFunctions.h>

#include <cmath>
#include <algorithm>
#include <deque>

#undef min
#undef max
#include <limits>

namespace data
{

#define DEFAULT_GPU_SIZE (256 * 1024 * 1024) // constant reserve
#define OTHER_GFX_STUFF_SIZE (128 * 1024 * 1024) // constant reserve
// Majkl 2013/04/03: removed 2 bytes reserve as the data are subsampled now
// and there is no need to reserve so much memory for the aux data
#define VOLUME_BYTE_DEPTH (2 + 1 + 0) // 2 bytes density
                                      // 1 byte culling mask,
                                      // 2 bytes reserve for aux (lower resolution, but 4 bytes per voxel) and other stuff
#define VOLUME_MULTIPLE 4

///////////////////////////////////////////////////////////////////////////////
//

CExamination::CExamination() : CStorageInterface(APP_STORAGE)
{
    m_adapterRAM = DEFAULT_GPU_SIZE;
    m_textureSize2D = 0;
    m_textureSize3D = 0;

    // Register all callback functions
    VPL_SIGNAL(SigGetDensityWindow).connect(this, &CExamination::getDensityWindow);
    VPL_SIGNAL(SigEstimateDensityWindow).connect(this, &CExamination::estimateDensityWindow);
    VPL_SIGNAL(SigSetDensityWindow).connect(this, &CExamination::setDensityWindow);
    VPL_SIGNAL(SigSetColoring).connect(this, &CExamination::setColoring);
    VPL_SIGNAL(SigGetColoringType).connect(this, &CExamination::getColoringType);

    VPL_SIGNAL(SigGetActiveDataSet).connect(this, &CExamination::getActiveDataSet);
    VPL_SIGNAL(SigSetActiveDataSet).connect(this, &CExamination::setActiveDataSet);
    VPL_SIGNAL(SigGetActiveConv).connect(this, &CExamination::getActiveConv);
    VPL_SIGNAL(SigGetActiveConvObject).connect(this, &CExamination::getActiveConvObject);
    VPL_SIGNAL(SigGetPatientConvObject).connect(this, &CExamination::getPatientConvObject);
    VPL_SIGNAL(SigGetAuxConvObject).connect(this, &CExamination::getAuxConvObject);

    VPL_SIGNAL(SigSetSliceXY).connect(this, &CExamination::setSlicePositionXY);
    VPL_SIGNAL(SigSetSliceXZ).connect(this, &CExamination::setSlicePositionXZ);
    VPL_SIGNAL(SigSetSliceYZ).connect(this, &CExamination::setSlicePositionYZ);
    VPL_SIGNAL(SigSetSliceModeXY).connect(this, &CExamination::setSliceModeXY);
    VPL_SIGNAL(SigSetSliceModeXZ).connect(this, &CExamination::setSliceModeXZ);
    VPL_SIGNAL(SigSetSliceModeYZ).connect(this, &CExamination::setSliceModeYZ);

    VPL_SIGNAL(SigEnableRegionColoring).connect(this, &CExamination::enableRegionColoring);
    VPL_SIGNAL(SigIsRegionColoringEnabled).connect(this, &CExamination::isRegionColoringEnabled);
    VPL_SIGNAL(SigSetRegionColor).connect(this, &CExamination::setRegionColor);
    VPL_SIGNAL(SigGetRegionColor).connect(this, &CExamination::getRegionColor);

    CExamination::init();
}

///////////////////////////////////////////////////////////////////////////////
//

CExamination::~CExamination()
{
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::init()
{
    using namespace Storage;

    STORABLE_FACTORY.registerObject(DensityWindow::Id, DensityWindow::Type::create);

    STORABLE_FACTORY.registerObject(ActiveDataSet::Id,
                                    ActiveDataSet::Type::create,
                                    CEntryDeps().insert(PatientData::Id).insert(AuxData::Id)
                                    );

    CEntryDeps SliceDeps;
    SliceDeps.insert(ActiveDataSet::Id).insert(DensityWindow::Id);
    SliceDeps.insert(RegionData::Id).insert(RegionColoring::Id);

    STORABLE_FACTORY.registerObject(SliceXY::Id, SliceXY::Type::create, SliceDeps);
    STORABLE_FACTORY.registerObject(SliceXZ::Id, SliceXZ::Type::create, SliceDeps);
    STORABLE_FACTORY.registerObject(SliceYZ::Id, SliceYZ::Type::create, SliceDeps);

    STORABLE_FACTORY.registerObject(PatientData::Id, PatientData::Type::create);
    STORABLE_FACTORY.registerObject(PatientConv::Id, PatientConv::Type::create, CEntryDeps().insert(PatientData::Id));
    STORABLE_FACTORY.registerObject(PatientVOI::Id, PatientVOI::Type::create, CEntryDeps().insert(PatientData::Id));
    STORABLE_FACTORY.registerObject(DataStats::Id, DataStats::Type::create, CEntryDeps().insert(PatientData::Id));

    STORABLE_FACTORY.registerObject(PreviewModel::Id, PreviewModel::Type::create, CEntryDeps().insert(PatientData::Id));

    STORABLE_FACTORY.registerObject(AuxData::Id, AuxData::Type::create, CEntryDeps().insert(PatientData::Id));
    STORABLE_FACTORY.registerObject(AuxConv::Id, AuxConv::Type::create, CEntryDeps().insert(AuxData::Id));
    STORABLE_FACTORY.registerObject(AuxVOI::Id, AuxVOI::Type::create, CEntryDeps().insert(AuxData::Id));

    STORABLE_FACTORY.registerObject(RegionData::Id, RegionData::Type::create, CEntryDeps().insert(PatientData::Id));
    STORABLE_FACTORY.registerObject(RegionColoring::Id, RegionColoring::Type::create);

    STORABLE_FACTORY.registerObject(SceneManipulatorDummy::Id, SceneManipulatorDummy::Type::create);

    STORABLE_FACTORY.registerObject(DrawingOptions::Id, DrawingOptions::Type::create);

    STORABLE_FACTORY.registerObject(MeasurementOptions::Id, MeasurementOptions::Type::create);

    // Scene widgets
    STORABLE_FACTORY.registerObject(SceneWidgetsParameters::Id, SceneWidgetsParameters::Type::create);

    // Undo manager
    STORABLE_FACTORY.registerObject(UndoManager::Id, UndoManager::Type::create, CEntryDeps().insert(PatientData::Id));

    // Image loader info
    STORABLE_FACTORY.registerObject(ImageLoaderInfo::Id, ImageLoaderInfo::Type::create);

    // All drawings dummy
    STORABLE_FACTORY.registerObject(AllDrawings::Id, AllDrawings::Type::create);

    // Application settings
    STORABLE_FACTORY.registerObject(AppSettings::Id, AppSettings::Type::create );

    // Saved entries
    STORABLE_FACTORY.registerObject( SavedEntries::Id, SavedEntries::Type::create );

    // Enforce object creation to initialize reverse dependencies
    APP_STORAGE.getEntry(AppSettings::Id);

    APP_STORAGE.getEntry(DensityWindow::Id);
    APP_STORAGE.getEntry(ActiveDataSet::Id);

    APP_STORAGE.getEntry(SliceXY::Id);
    APP_STORAGE.getEntry(SliceXZ::Id);
    APP_STORAGE.getEntry(SliceYZ::Id);

    APP_STORAGE.getEntry(PatientData::Id);
    APP_STORAGE.getEntry(PatientConv::Id);
    APP_STORAGE.getEntry(PatientVOI::Id);
    APP_STORAGE.getEntry(DataStats::Id);

    APP_STORAGE.getEntry(PreviewModel::Id);

    APP_STORAGE.getEntry(AuxData::Id);
    APP_STORAGE.getEntry(AuxConv::Id);
    APP_STORAGE.getEntry(AuxVOI::Id);

    APP_STORAGE.getEntry(RegionData::Id);
    APP_STORAGE.getEntry(RegionColoring::Id);

    APP_STORAGE.getEntry(SceneManipulatorDummy::Id);
    APP_STORAGE.getEntry(DrawingOptions::Id);
    APP_STORAGE.getEntry(MeasurementOptions::Id);
    APP_STORAGE.getEntry(SceneWidgetsParameters::Id);

    APP_STORAGE.getEntry(ImageLoaderInfo::Id);
    APP_STORAGE.getEntry(AllDrawings::Id);
    APP_STORAGE.getEntry(UndoManager::Id);

    APP_STORAGE.getEntry(SavedEntries::Id);

    // init volume undo
    {
        data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(Storage::PatientData::Id) );
        spVolume->initVolumeUndo(Storage::PatientData::Id);
    }
    {
        data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(Storage::AuxData::Id) );
        spVolume->initVolumeUndo(Storage::AuxData::Id);
    }
}

///////////////////////////////////////////////////////////////////////////////
//

const SDensityWindow &CExamination::getDensityWindow()
{
    // Get pointer to the density window
    CObjectPtr<CDensityWindow> spWindow( APP_STORAGE.getEntry(Storage::DensityWindow::Id) );
    return spWindow->getParams();
}

///////////////////////////////////////////////////////////////////////////////
//

SDensityWindow CExamination::estimateDensityWindow()
{
    static const vpl::img::tDensityPixel AlmostAir = -800;

    // Get pointer to the active data set
    CObjectPtr<CDensityData> spVolume( APP_STORAGE.getEntry(getActiveDataSet()) );

    // Estimate optimal density window
    int iCount = 0;
    double dSum = 0.0, dSumSqr = 0.0;
    vpl::tSize k = spVolume->getZSize() / 2;
    for( vpl::tSize j = 0; j < spVolume->getYSize(); ++j )
    {
        for( vpl::tSize i = 0; i < spVolume->getXSize(); ++i )
        {
            vpl::img::tDensityPixel Value = spVolume->at(i, j, k);
//            if( Value != spVolume->at(0, 0, 0) )
            if( Value > AlmostAir )
            {
                dSum += Value;
                dSumSqr += double(Value) * Value;
                ++iCount;
            }
        }
    }
    
    double dMean = 0.0, dVar = 0.0;
    if( iCount > 0 )
    {
        double dInvCount = 1.0 / iCount;
        dMean = dSum * dInvCount;
        dVar =  dSumSqr * dInvCount - (dMean * dMean);
    }

//    return SDensityWindow(int(dMean), int(6 * std::sqrt(dVar)));
    double dStdDev = std::sqrt(dVar);
    return SDensityWindow(int(dMean + dStdDev), int(6 * dStdDev));
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setDensityWindow(int Center, int Width)
{
    // Get pointer to the density window
    CObjectPtr<CDensityWindow> spWindow( APP_STORAGE.getEntry(Storage::DensityWindow::Id, Storage::NO_UPDATE) );

    if( spWindow->getCenter() == Center && spWindow->getWidth() == Width )
    {
        return;
    }

    // Update the density window
    spWindow->setParams(SDensityWindow(Center, Width));

    // Entry changed
    APP_STORAGE.invalidate(spWindow.getEntryPtr());
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setColoring(data::CColoringFunc4b *pFunc)
{
    // Get pointer to the density window
    CObjectPtr<CDensityWindow> spWindow( APP_STORAGE.getEntry(Storage::DensityWindow::Id, Storage::NO_UPDATE) );

    // Update the density window
    spWindow->setColoring(pFunc);

    // Entry changed
    APP_STORAGE.invalidate(spWindow.getEntryPtr());
}

///////////////////////////////////////////////////////////////////////////////
//

int CExamination::getColoringType()
{
    // Get pointer to the density window
    CObjectPtr<CDensityWindow> spWindow( APP_STORAGE.getEntry(Storage::DensityWindow::Id, Storage::NO_UPDATE) );

    // Update the density window
    return spWindow->getColoringType();
}

///////////////////////////////////////////////////////////////////////////////
//

int CExamination::getActiveDataSet()
{
    CObjectPtr<CActiveDataSet> spDataSet( APP_STORAGE.getEntry(Storage::ActiveDataSet::Id) );

    return spDataSet->getId();
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setActiveDataSet(int Id)
{
    if( Id < PATIENT_DATA || Id > AUX_DATA )
    {
        return;
    }

    // Get active data set
    CObjectPtr<CActiveDataSet> spDataSet( APP_STORAGE.getEntry(Storage::ActiveDataSet::Id, Storage::NO_UPDATE) );

    // Any change?
    if( spDataSet->getId() == Id )
    {
        return;
    }

    spDataSet->setId(Id);

    // Notify all interested windows
    APP_STORAGE.invalidate(spDataSet.getEntryPtr());
}

///////////////////////////////////////////////////////////////////////////////
//

int CExamination::getActiveConv()
{
    CObjectPtr<CActiveDataSet> spDataSet( APP_STORAGE.getEntry(Storage::ActiveDataSet::Id) );

    return spDataSet->getId() + 1;
}

///////////////////////////////////////////////////////////////////////////////
//

CCoordinatesConv CExamination::getActiveConvObject()
{
    CObjectPtr<CCoordinatesConv> spConv( APP_STORAGE.getEntry(getActiveConv()) );

    return CCoordinatesConv(*spConv);
}

///////////////////////////////////////////////////////////////////////////////
//

CCoordinatesConv CExamination::getPatientConvObject()
{
    CObjectPtr<CCoordinatesConv> spConv( APP_STORAGE.getEntry(Storage::PatientConv::Id) );
    return CCoordinatesConv(*spConv);
}

///////////////////////////////////////////////////////////////////////////////
//

CCoordinatesConv CExamination::getAuxConvObject()
{
    CObjectPtr<CCoordinatesConv> spConv( APP_STORAGE.getEntry(Storage::AuxConv::Id) );
    return CCoordinatesConv(*spConv);
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::enableRegionColoring(bool bEnable)
{
    CObjectPtr<CRegionData> spRegionData( APP_STORAGE.getEntry(Storage::RegionData::Id, Storage::NO_UPDATE) );

    if( spRegionData->isColoringEnabled() == bEnable )
    {
        return;
    }

    spRegionData->enableColoring(bEnable);

    APP_STORAGE.invalidate(spRegionData.getEntryPtr());
}

///////////////////////////////////////////////////////////////////////////////
//

bool CExamination::isRegionColoringEnabled()
{
    CObjectPtr<CRegionData> spRegionData( APP_STORAGE.getEntry(Storage::RegionData::Id, Storage::NO_UPDATE) );

    return spRegionData->isColoringEnabled();
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setRegionColor(int i, const CColor4b& Color)
{
    CObjectPtr<CRegionColoring> spColoring( APP_STORAGE.getEntry(Storage::RegionColoring::Id, Storage::NO_UPDATE) );

    spColoring->setColor(i, Color);

    APP_STORAGE.invalidate(spColoring.getEntryPtr());
}

///////////////////////////////////////////////////////////////////////////////
//

CColor4b CExamination::getRegionColor(int i)
{
    CObjectPtr<CRegionColoring> spColoring( APP_STORAGE.getEntry(Storage::RegionColoring::Id) );
    return spColoring->getColor(i);
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setSlicePositionXY(int Position)
{
//    CObjectPtr<COrthoSliceXY> spSlice( APP_STORAGE.getEntryPtr(Storage::SliceXY::Id) );
    CObjectPtr<COrthoSliceXY> spSlice( APP_STORAGE.getEntry(Storage::SliceXY::Id, Storage::NO_UPDATE) );

    // Check the current position
    if( spSlice->getPosition() == Position )
    {
        return;
    }

    // Set the current position
    spSlice->setPosition(Position);

    // Update the slice data
    APP_STORAGE.invalidate(spSlice.getEntryPtr());
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setSlicePositionXZ(int Position)
{
//    CObjectPtr<COrthoSliceXZ> spSlice( APP_STORAGE.getEntryPtr(Storage::SliceXZ::Id) );
    CObjectPtr<COrthoSliceXZ> spSlice( APP_STORAGE.getEntry(Storage::SliceXZ::Id, Storage::NO_UPDATE) );

    // Check the current position
    if( spSlice->getPosition() == Position )
    {
        return;
    }

    // Set the current position
    spSlice->setPosition(Position);

    // Update the slice data
    APP_STORAGE.invalidate(spSlice.getEntryPtr());
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setSlicePositionYZ(int Position)
{
//    CObjectPtr<COrthoSliceYZ> spSlice( APP_STORAGE.getEntryPtr(Storage::SliceYZ::Id) );
    CObjectPtr<COrthoSliceYZ> spSlice( APP_STORAGE.getEntry(Storage::SliceYZ::Id, Storage::NO_UPDATE) );

    // Check the current position
    if( spSlice->getPosition() == Position )
    {
        return;
    }

    // Set the current position
    spSlice->setPosition(Position);

    // Update the slice data
    APP_STORAGE.invalidate(spSlice.getEntryPtr());
}

///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setSliceModeXY(int Mode)
{
    // Is the mode valid?
    if( Mode < COrthoSlice::MODE_SLICE || Mode > COrthoSlice::MODE_RTG )
    {
        return;
    }

//    CObjectPtr<COrthoSliceXY> spSlice( APP_STORAGE.getEntryPtr(Storage::SliceXY::Id) );
    CObjectPtr<COrthoSliceXY> spSlice( APP_STORAGE.getEntry(Storage::SliceXY::Id, Storage::NO_UPDATE) );

    // Check the current mode
    if( spSlice->getMode() == Mode )
    {
        return;
    }

    // Set the slice mode
    spSlice->setMode(COrthoSlice::EMode(Mode));

    APP_STORAGE.invalidate(spSlice.getEntryPtr(), COrthoSlice::MODE_CHANGED);
}


///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setSliceModeXZ(int Mode)
{
    // Is the mode valid?
    if( Mode < COrthoSlice::MODE_SLICE || Mode > COrthoSlice::MODE_RTG )
    {
        return;
    }

//    CObjectPtr<COrthoSliceXZ> spSlice( APP_STORAGE.getEntryPtr(Storage::SliceXZ::Id) );
    CObjectPtr<COrthoSliceXZ> spSlice( APP_STORAGE.getEntry(Storage::SliceXZ::Id, Storage::NO_UPDATE) );

    // Check the current mode
    if( spSlice->getMode() == Mode )
    {
        return;
    }

    // Set the slice mode
    spSlice->setMode(COrthoSlice::EMode(Mode));

    APP_STORAGE.invalidate(spSlice.getEntryPtr(), COrthoSlice::MODE_CHANGED);
}


///////////////////////////////////////////////////////////////////////////////
//

void CExamination::setSliceModeYZ(int Mode)
{
    // Is the mode valid?
    if( Mode < COrthoSlice::MODE_SLICE || Mode > COrthoSlice::MODE_RTG )
    {
        return;
    }

//    CObjectPtr<COrthoSliceYZ> spSlice( APP_STORAGE.getEntryPtr(Storage::SliceYZ::Id) );
    CObjectPtr<COrthoSliceYZ> spSlice( APP_STORAGE.getEntry(Storage::SliceYZ::Id, Storage::NO_UPDATE) );

    // Check the current mode
    if( spSlice->getMode() == Mode )
    {
        return;
    }

    // Set the slice mode
    spSlice->setMode(COrthoSlice::EMode(Mode));

    APP_STORAGE.invalidate(spSlice.getEntryPtr(), COrthoSlice::MODE_CHANGED);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//

bool CExamination::loadDensityData(const std::string& ssFilename,
                                   vpl::mod::CProgress::tProgressFunc& Progress,
                                   EDataSet Id
                                   )
{
    // Open input file channel
    vpl::mod::CFileChannel Channel(vpl::mod::CH_IN, ssFilename);
    if( !Channel.connect() )
    {
        return false;
    }

    // Try to load the data
    // CDensityData Data;
    vpl::img::CDensityVolume Data;

    if( !vpl::mod::read(Data, Channel, Progress) )
    {
        return false;
    }

    // RESIZE VOLUME SO THAT (DIMENSION_k mod VOLUME_MULTIPLE == 0)
    //correctVolumeSize(Data, VOLUME_MULTIPLE);

    CObjectPtr<CDensityData> spVolume( APP_STORAGE.getEntry(Id) );

    // Use the data
    spVolume->makeRef(Data);
    spVolume->clearDicomData();

    // Fill the margin
    spVolume->mirrorMargin();

    // Notify all interested windows
//    APP_STORAGE.invalidate(spVolume.getEntryPtr(), data::Storage::FORCE_UPDATE);
    APP_STORAGE.invalidate(spVolume.getEntryPtr() );

    onDataLoad( Id );

    // O.K.
    return true;
}

bool CExamination::loadDensityDataU(const vpl::sys::tString& ssFilename,
                                    vpl::mod::CProgress::tProgressFunc& Progress,
                                    EDataSet Id
                                    )
{
    // Open input file channel
    vpl::mod::CFileChannelU Channel(vpl::mod::CH_IN, ssFilename);
    if( !Channel.connect() )
    {
        return false;
    }

    // Try to load the data
    // CDensityData Data;
    vpl::img::CDensityVolume Data;

    if( !vpl::mod::read(Data, Channel, Progress) )
    {
        return false;
    }

    // RESIZE VOLUME SO THAT (DIMENSION_k mod VOLUME_MULTIPLE == 0)
    //correctVolumeSize(Data, VOLUME_MULTIPLE);

    CObjectPtr<CDensityData> spVolume( APP_STORAGE.getEntry(Id) );

    // Use the data
    spVolume->makeRef(Data);
    spVolume->clearDicomData();

    // Fill the margin
    spVolume->mirrorMargin();

    // Notify all interested windows
    //APP_STORAGE.invalidate(spVolume.getEntryPtr(), data::Storage::FORCE_UPDATE);
    //APP_STORAGE.lockInvalidation();
    APP_STORAGE.invalidate(spVolume.getEntryPtr() );
    //APP_STORAGE.unlockInvalidation();

    onDataLoad( Id );

    // O.K.
    return true;
}


///////////////////////////////////////////////////////////////////////////////
//

/*bool CExamination::loadDicomData(std::vector<std::string> & DicomFiles,
                                 vpl::mod::CProgress::tProgressFunc & Progress,
                                 EDataSet Id
                                 )
{
    // Create temporary loading Density volume
    CDensityData Data;
    if( !Data.loadDicomData(DicomFiles, Progress) )
    {
        return false;
    }

    CObjectPtr<CDensityData> spVolume( APP_STORAGE.getEntry(Id) );

    // Use the data
    spVolume->makeRef(Data);
//    spVolume->copy(Data);

    // Fill the margin
    spVolume->mirrorMargin();

    // Notify all interested windows
    APP_STORAGE.invalidate(spVolume.getEntryPtr(), Storage::FORCE_UPDATE);

    this->onDataLoad( Id );

    // O.K.
    return true;
}*/


///////////////////////////////////////////////////////////////////////////////
//! Functor that compares position of two slices.

struct SCompareImagePosition
{
    //! Compares the position of two images.
    bool operator() (const vpl::img::CDicomSlicePtr& p1, const vpl::img::CDicomSlicePtr& p2)
    {
        return p1->getPosition() < p2->getPosition();
    }
};


///////////////////////////////////////////////////////////////////////////////
// Version 4
// - inverse projection to the orthogonal volume
// - added support for multi-frame dicom files

CExamination::ELoadState CExamination::loadDicomData( data::CSerieInfo * serie,
                                  vpl::mod::CProgress::tProgressFunc & Progress,
                                  EDataSet Id,
                                  ESubsamplingType subsamplingType,
                                  vpl::img::CVector3d subsampling,
                                  bool bCompatibilityMode
                                  )
{
    // Get number of dicom files to load
    int total_dicom_files = serie->getNumOfDicomFiles();

    // Get number of slices to load
    int total_dicom_slices = serie->getNumOfSlices();

    // No dicom files
    if( total_dicom_slices <= 0 )
    {
        return ELS_FAILED;
    }


    // PRELOAD A REPRESENTATIVE SLICE

    // Create slice that will be used for loading
    vpl::img::CDicomSlice RepSlice;

    // Preload one of the slices
    if( !serie->loadDicomFile( total_dicom_files / 2, RepSlice ) )
    {
        return ELS_FAILED;
    }

    // Remember both the X and the Y axis
    vpl::img::CVector3D XAxis(RepSlice.m_ImageOrientationX);
    vpl::img::CVector3D YAxis(RepSlice.m_ImageOrientationY);

    // Calculate the Z axis (normal to the image plane)
    vpl::img::CVector3D ZAxis;
    ZAxis.vectorProduct(XAxis, YAxis);

    // Normalize all vectors
    XAxis.normalize();
    YAxis.normalize();
    ZAxis.normalize();


    // PRELOAD ALL SLICES

    // Loaded dicom images
    typedef std::deque<vpl::img::CDicomSlicePtr> tSliceQueue;
    tSliceQueue slices;

    // Preload all slices
    int counterMax = 2 * total_dicom_slices + 1;
    int counter = 0;
    int failures = 0;
    for( int i = 0; i < total_dicom_files; ++i )
    {
        // Load the dicom file including the image data
        tDicomSlices aux;
        if( !serie->loadDicomFile( i, aux, true, bCompatibilityMode ) )
        {
            return ELS_FAILED;
        }

        // Process all frames
        tDicomSlices::iterator it = aux.begin();
        tDicomSlices::iterator itEnd = aux.end();
        for( ; it != itEnd; ++it, ++counter )
        {
            vpl::img::CDicomSlicePtr pSlice( *it );

            // Normalize the image orientation
            pSlice->m_ImageOrientationX.normalize();
            pSlice->m_ImageOrientationY.normalize();

            // Verify the image orientation
            if( !(XAxis == pSlice->m_ImageOrientationX) || !(YAxis == pSlice->m_ImageOrientationY) )
            {
                ++failures;
                VPL_LOG_INFO("Warning: Differently oriented slice was found");
                continue;
            }

            // Vector from the origin to the slice position
            vpl::img::CVector3D PositionVector(vpl::img::CPoint3D(0, 0, 0), pSlice->m_ImagePosition);

            // Calculate the projection
            double dPosition = vpl::img::CVector3D::dotProduct(ZAxis, PositionVector);

            // Set the slice position
            pSlice->setPosition(dPosition);

            // Store the slice
            slices.push_back(pSlice);

            // Progress incrementation
            if( !Progress(counter, counterMax) )
            {
                return ELS_FAILED;
            }
        }
    }

    // Check the number of wrongly oriented slices
    int failuresMax = total_dicom_slices / 10;
    if( failures > failuresMax )
    {
        VPL_LOG_INFO("Warning: Cannot load DICOM datasets with varying image orientation");
        return ELS_FAILED;
    }


    // RANK SLICES ACCORDING TO THEIR POSITION

    // Create ranking of projected slice positions
    std::sort(slices.begin(), slices.end(), SCompareImagePosition());

    // Correct the minimal and maximal position
    // - This is due to the fact that we have observed that dicom datasets may contain
    //   a wrongly positioned slice sometimes!
    if( slices.size() > 2 )
    {
        double d1 = vpl::math::getAbs( slices[0]->getPosition() - slices[1]->getPosition() );
        double d2 = 2.0 * vpl::math::getAbs( slices[1]->getPosition() - slices[2]->getPosition() );
        if( d1 > d2 )
        {
            ++counter;
            slices.pop_front();
            VPL_LOG_INFO("Warning: Wrongly positioned slice was found");
        }
    }
    if( slices.size() > 2 )
    {
        double d1 = vpl::math::getAbs( slices[slices.size() - 1]->getPosition() - slices[slices.size() - 2]->getPosition() );
        double d2 = 2.0 * vpl::math::getAbs( slices[slices.size() - 2]->getPosition() - slices[slices.size() - 3]->getPosition() );
        if( d1 > d2 )
        {
            ++counter;
            slices.pop_back();
            VPL_LOG_INFO("Warning: Wrongly positioned slice was found");
        }
    }

    // Modify the number of slices
    total_dicom_slices = int(slices.size());


    // RECALCULATE THE SLICE THICKNESS

    // Minimal and maximal position (absolute value)
    double min_position = slices[0]->getPosition();
    double max_position = slices[slices.size() - 1]->getPosition();

    // Find optimal slice thickness
    typedef std::vector<double> tRank;
    double dThickness = RepSlice.getThickness();
    if( total_dicom_slices > 1 )
    {
        // Position differences of neighbouring slices
        tRank DiffRank;
        for( int j = 0; j < total_dicom_slices - 1; ++j )
        {
            DiffRank.push_back( vpl::math::getAbs( slices[j]->getPosition() - slices[j+1]->getPosition() ) );
        }
        
        // Create ranking
        std::sort(DiffRank.begin(), DiffRank.end());
        
        // Choose the optimal thickness as a median value
//        dThickness = DiffRank[total_dicom_slices / 2 - 1];
        dThickness = DiffRank[(total_dicom_slices - 1) / 2];
    }

    // Check the thickness
    if( dThickness <= 0.0 )
    {
        return ELS_FAILED;
    }
    double dInvThickness = 1.0 / dThickness;

    // Original slice dimensions
    vpl::tSize orig_size_x = RepSlice.getXSize();
    vpl::tSize orig_size_y = RepSlice.getYSize();

    // Calculate the number of slices
    vpl::tSize orig_size_z = vpl::tSize(vpl::math::round2Int((max_position - min_position) * dInvThickness) + 1);


    // LOAD THE IMAGE DATA

    // Create temporary loading volume
    VPL_LOG_INFO("Loading volume " << orig_size_x << "x" << orig_size_y << "x" << orig_size_z);

    vpl::tSize alignedSizeX = orig_size_x + (VOLUME_MULTIPLE - orig_size_x % VOLUME_MULTIPLE) % VOLUME_MULTIPLE;
    vpl::tSize alignedSizeY = orig_size_y + (VOLUME_MULTIPLE - orig_size_y % VOLUME_MULTIPLE) % VOLUME_MULTIPLE;
    vpl::tSize alignedSizeZ = orig_size_z + (VOLUME_MULTIPLE - orig_size_z % VOLUME_MULTIPLE) % VOLUME_MULTIPLE;    
    if (subsampling != vpl::img::CVector3d(1.0, 1.0, 1.0))
    {
        alignedSizeX = orig_size_x;
        alignedSizeY = orig_size_y;
        alignedSizeZ = orig_size_z;
    }
    vpl::img::CDensityVolume AuxData( alignedSizeX, alignedSizeY, alignedSizeZ );

    // Create a volume of corresponding size
    AuxData.fillEntire( vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin() );

    // Set the voxel size
    AuxData.setDX( RepSlice.getDX() );
    AuxData.setDY( RepSlice.getDY() );
    AuxData.setDZ( dThickness );

    // Create a set of all possible slice positions
    typedef std::vector<int> tInserted;
    int size_p = orig_size_z;
    tInserted Inserted( size_p, 0 );

    // Insert all slices into the temporary volume
    tSliceQueue::iterator it = slices.begin();
    tSliceQueue::iterator itEnd = slices.end();
    for( ; it != itEnd; ++it, ++counter )
    {
        vpl::img::CDicomSlicePtr pSlice( *it );
        
        // Estimate the slice position
//        int slice_position = static_cast<int>( (pSlice->getPosition() - min_position) * dInvThickness );
        int slice_position = static_cast<int>( (pSlice->getPosition() - min_position) * dInvThickness + 0.5 );
        if( slice_position < 0 || slice_position >= orig_size_z )
        {
            continue;
        }

        // Check if any similar slice was already inserted?
        if( Inserted[slice_position] == 0 )
        {
            // Add current slice to the volume
            AuxData.setPlaneXY( slice_position, *pSlice );

            // Update the set of already loaded slices
            Inserted[slice_position] = 1;

            // Enforce deallocation of the data
            pSlice->vpl::img::CDImage::resize( 0, 0, 0 );
        }
        else
        {
            VPL_LOG_INFO("Warning: Overlapping slices was detected");
        }

        // Update the progress bar
        if( !Progress( counter, counterMax ) )
        {
            return ELS_FAILED;
        }
    }


    // INTERPOLATE MISSING DATA
    // - This may happen sometimes...
    
    // Find leftmost and rightmost correctly imported slice
    int LeftmostSlice = 0;
    for( ; LeftmostSlice < size_p && Inserted[LeftmostSlice] == 0; ++LeftmostSlice );
    
    int RightmostSlice = size_p - 1;
    for( ; RightmostSlice >= 0 && Inserted[RightmostSlice] == 0; --RightmostSlice );

    // Is an interpolation possible?
    if( LeftmostSlice >= RightmostSlice )
    {
        VPL_LOG_INFO("Warning: Cannot interpolate missing slices");
    }

    // Interpolate the missing data
    int LeftNeighbour = LeftmostSlice;
    for( int z = LeftmostSlice + 1; z < RightmostSlice; ++z )
    {
        if( Inserted[z] != 0 )
        {
            LeftNeighbour = z;
            continue;
        }

        // Find the closest right neighbour
        int RightNeighbour = z + 1;
        for( ; RightNeighbour < RightmostSlice && Inserted[RightNeighbour] == 0; ++RightNeighbour );

        // Estimate interpolation coeeficients
        int Distance = RightNeighbour - LeftNeighbour;
        int DistLeft = z - LeftNeighbour;
        if( Distance <= 0 || DistLeft <= 0 )
        {
            continue;
        }
        double dInvDistance = 1.0 / Distance;
        double dRightWeight = DistLeft * dInvDistance;
        double dLeftWeight = 1.0 - dRightWeight;

        // Interpolate from neighbours
        for( int y = 0; y < orig_size_y; ++y )
        {
            for( int x = 0; x < orig_size_x; ++x )
            {
                double dValue = dLeftWeight * AuxData(x, y, LeftNeighbour) + dRightWeight * AuxData(x, y, RightNeighbour);
                AuxData(x, y, z) = vpl::img::tDensityPixel(dValue);
            }
        }

        VPL_LOG_INFO("Warning: Missing slice was interpolated");
    }


    // CHECK IF ANY TRANSFORMATION IS NECESSARY

    // Final density data
    CDensityData Data;

    // Is the transformation necessary?
    if( vpl::math::getAbs(1.0 - XAxis.x()) < 0.001
        && vpl::math::getAbs(1.0 - YAxis.y()) < 0.001
        && vpl::math::getAbs(1.0 - ZAxis.z()) < 0.001 )
    {
        // Just a reference to the existing data
        Data.makeRef( AuxData );
        
        // Set dicom meta data
        Data.takeDicomData( RepSlice );

        Data.setImagePosition( RepSlice.m_ImagePosition.getX(), RepSlice.m_ImagePosition.getY(), min_position);
        
        // Set the voxel size
        Data.setDX( RepSlice.getDX() );
        Data.setDY( RepSlice.getDY() );
        Data.setDZ( dThickness );
        
        goto lFinishLoading;
    }


    // BEGIN - THE TRANSFORMATION
    {
        // Calculate the correct Z axis as the difference of position of two neighbouring slices
        if( total_dicom_slices > 1 )
        {
            vpl::img::CVector3D NewZAxis( slices[slices.size() / 2 - 1]->m_ImagePosition, slices[slices.size() / 2]->m_ImagePosition );
            NewZAxis.normalize();
            if( vpl::img::CVector3D::dotProduct(ZAxis, NewZAxis) < 0 )
            {
                NewZAxis *= -1;
            }
            ZAxis = NewZAxis;
        }

        // Recalculate position of all slices after projection on the Z axis
        for( int i = 0; i < total_dicom_slices; ++i )
        {
            // Vector from the origin to the slice position
            vpl::img::CVector3D PositionVector( vpl::img::CPoint3D(0, 0, 0), slices[i]->m_ImagePosition );

            // Calculate the projection
            double dPosition = vpl::img::CVector3D::dotProduct(ZAxis, PositionVector);

            // Store the position
            slices[i]->setPosition(dPosition);
        }

        // Create ranking of projected slice positions
        std::sort(slices.begin(), slices.end(), SCompareImagePosition());

        // Minimal and maximal position (absolute value)
        min_position = slices[0]->getPosition();
        max_position = slices[slices.size() - 1]->getPosition();

        // Find the optimal slice thickness again
        if( total_dicom_slices > 1 )
        {
            // Position differences of neighbouring slices
            tRank DiffRank;
            for( int j = 0; j < total_dicom_slices - 1; ++j )
            {
                DiffRank.push_back( vpl::math::getAbs( slices[j]->getPosition() - slices[j+1]->getPosition() ) );
            }
            
            // Create ranking
            std::sort(DiffRank.begin(), DiffRank.end());
            
            // Choose the optimal thickness as a median value
//            dThickness = DiffRank[total_dicom_slices / 2 - 1];
            dThickness = DiffRank[(total_dicom_slices - 1) / 2];
        }

        // Check the thickness
        if( dThickness <= 0.0 )
        {
            return ELS_FAILED;
        }

        // Delete the loaded slices
        slices.clear();

        // Create the transformation matrix
        CDensityData::tMatrix Transform;
        Transform(0, 0) = XAxis.getX();
        Transform(0, 1) = XAxis.getY();
        Transform(0, 2) = XAxis.getZ();
        Transform(1, 0) = YAxis.getX();
        Transform(1, 1) = YAxis.getY();
        Transform(1, 2) = YAxis.getZ();
        Transform(2, 0) = ZAxis.getX();
        Transform(2, 1) = ZAxis.getY();
        Transform(2, 2) = ZAxis.getZ();

        // Helper array of box corners
        double Corners[8][3] = {
            { 0, 0, 0 },
            { 1, 0, 0 },
            { 1, 1, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 },
            { 1, 0, 1 },
            { 1, 1, 1 },
            { 0, 1, 1 }
        };

        // Transform all corners of the volume and find min/max
        vpl::math::CDVector3 Temp, OrigSize;
        OrigSize(0) = (orig_size_x - 1) * Corners[0][0] * RepSlice.getDX();
        OrigSize(1) = (orig_size_y - 1) * Corners[0][1] * RepSlice.getDY();
        OrigSize(2) = (orig_size_z - 1) * Corners[0][2] * dThickness;
        Temp.mult(OrigSize, Transform);
        double Rminx = Temp(0), Rmaxx = Temp(0);
        double Rminy = Temp(1), Rmaxy = Temp(1);
        double Rminz = Temp(2), Rmaxz = Temp(2);

        // All remaining corners
        for( int i = 1; i < 8; ++i )
        {
            OrigSize(0) = (orig_size_x - 1) * Corners[i][0] * RepSlice.getDX();
            OrigSize(1) = (orig_size_y - 1) * Corners[i][1] * RepSlice.getDY();
            OrigSize(2) = (orig_size_z - 1) * Corners[i][2] * dThickness;
            Temp.mult(OrigSize, Transform);

            Rminx = vpl::math::getMin(Rminx, Temp(0));
            Rminy = vpl::math::getMin(Rminy, Temp(1));
            Rminz = vpl::math::getMin(Rminz, Temp(2));

            Rmaxx = vpl::math::getMax(Rmaxx, Temp(0));
            Rmaxy = vpl::math::getMax(Rmaxy, Temp(1));
            Rmaxz = vpl::math::getMax(Rmaxz, Temp(2));
        }

        // Calculate the voxel size
        vpl::math::CDVector3 Voxel;
        Voxel(0) = Voxel(1) = Voxel(2) = 0;

        // PRVNI VARIANTA - ponechava stejny rozmer voxelu (resi pouze prevraceni, apod.)

        OrigSize(0) = 1;
        OrigSize(1) = 0;
        OrigSize(2) = 0;
        Temp.mult(OrigSize, Transform);
        if( OrigSize(0) > OrigSize(1) )
        {
            if( OrigSize(0) > OrigSize(2) )
            {
                Voxel(0) = RepSlice.getDX();
            }
            else
            {
                Voxel(2) = RepSlice.getDX();
            }
        }
        else
        {
            if( OrigSize(1) > OrigSize(2) )
            {
                Voxel(1) = RepSlice.getDX();
            }
            else
            {
                Voxel(2) = RepSlice.getDX();
            }
        }

        OrigSize(0) = 0;
        OrigSize(1) = 1;
        OrigSize(2) = 0;
        Temp.mult(OrigSize, Transform);
        if( OrigSize(0) >= OrigSize(1) )
        {
            if( OrigSize(0) >= OrigSize(2) )
            {
                Voxel(0) = RepSlice.getDY();
            }
            else
            {
                Voxel(2) = RepSlice.getDY();
            }
        }
        else
        {
            if( OrigSize(1) >= OrigSize(2) )
            {
                Voxel(1) = RepSlice.getDY();
            }
            else
            {
                Voxel(2) = RepSlice.getDY();
            }
        }

        OrigSize(0) = 0;
        OrigSize(1) = 0;
        OrigSize(2) = 1;
        Temp.mult(OrigSize, Transform);
        if( OrigSize(0) >= OrigSize(1) )
        {
            if( OrigSize(0) > OrigSize(2) )
            {
                Voxel(0) = dThickness;
            }
            else
            {
                Voxel(2) = dThickness;
            }
        }
        else
        {
            if( OrigSize(1) > OrigSize(2) )
            {
                Voxel(1) = dThickness;
            }
            else
            {
                Voxel(2) = dThickness;
            }
        }

        // Final volume size
        vpl::tSize new_size_x = vpl::math::round2Int((Rmaxx - Rminx) / Voxel(0) + 1);
        vpl::tSize new_size_y = vpl::math::round2Int((Rmaxy - Rminy) / Voxel(1) + 1);
        vpl::tSize new_size_z = vpl::math::round2Int((Rmaxz - Rminz) / Voxel(2) + 1);

        // TRANSFORM THE VOLUME DATA

        CDensityData::tMatrix InvTransform;
        InvTransform = Transform;
        try {
            inverse(InvTransform);
        }
        catch( ... )
        {
            return ELS_FAILED;
        }

        // Translation vector
        CDensityData::tVector Shift;
        Shift(0) = Rminx;
        Shift(1) = Rminy;
        Shift(2) = Rminz;

        // Create the final volume
        Data.resize(new_size_x, new_size_y, new_size_z);
        Data.fillEntire(vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin());

        // Set dicom meta data
        Data.takeDicomData( RepSlice );

        // Set the voxel size
        Data.setDX(Voxel(0));
        Data.setDY(Voxel(1));
        Data.setDZ(Voxel(2));

        // Normalization against the voxel size
        vpl::math::CDVector3 Norm;
        Norm(0) = 1.0 / RepSlice.getDX();
        Norm(1) = 1.0 / RepSlice.getDY();
        Norm(2) = 1.0 / dThickness;

        // Inverted number of steps in X and Y axis
        double dInvSizeX = 1.0 / (new_size_x - 1);
        double dInvSizeY = 1.0 / (new_size_y - 1);

        // Interpolate the volume data
        counterMax += Data.getZSize();
        for( int z = 0; z < Data.getZSize(); ++z, ++counter )
        {
            // Four slice corners
            vpl::math::CDVector3 A, B, C, D;
            A(0) = 0;
            A(1) = 0;
            B(0) = A(0) + (Data.getXSize() - 1) * Data.getDX();
            B(1) = A(1);
            C(0) = A(0) + (Data.getXSize() - 1) * Data.getDX();
            C(1) = A(1) + (Data.getYSize() - 1) * Data.getDY();
            D(0) = A(0);
            D(1) = A(1) + (Data.getYSize() - 1) * Data.getDY();
            A(2) = B(2) = C(2) = D(2) = z * Data.getDZ();

            // Shift all corners
            A += Shift;
            B += Shift;
            C += Shift;
            D += Shift;

            // Use the inverse transformation matrix to calculate original positions of all corners
            vpl::math::CDVector3 a, b, c, d;
            a.mult(A, InvTransform);
            b.mult(B, InvTransform);
            c.mult(C, InvTransform);
            d.mult(D, InvTransform);

            // Normalization against the original voxel size
            a *= Norm;
            b *= Norm;
            c *= Norm;
            d *= Norm;

            // Calculate steps in Y axis
            vpl::img::CVector3D step_ad, step_bc;
            step_ad.x() = (d(0) - a(0)) * dInvSizeY;
            step_ad.y() = (d(1) - a(1)) * dInvSizeY;
            step_ad.z() = (d(2) - a(2)) * dInvSizeY;
            step_bc.x() = (c(0) - b(0)) * dInvSizeY;
            step_bc.y() = (c(1) - b(1)) * dInvSizeY;
            step_bc.z() = (c(2) - b(2)) * dInvSizeY;

            // Starting points
            vpl::img::CPoint3D ad(a(0), a(1), a(2));
            vpl::img::CPoint3D bc(b(0), b(1), b(2));

            // Add slice to the volume
            for( int j = 0; j < Data.getYSize(); ++j )
            {
                // Calculate step in the X axis
                vpl::img::CVector3D step_adbc;
                step_adbc.x() = (bc.x() - ad.x()) * dInvSizeX;
                step_adbc.y() = (bc.y() - ad.y()) * dInvSizeX;
                step_adbc.z() = (bc.z() - ad.z()) * dInvSizeX;

                // Initial point
                vpl::img::CPoint3D adbc(ad);

                // Get pointer to first voxel of actual row
                vpl::tSize dst_index=Data.getIdx(0,j,z);

                // Actual row voxels cycle
                for( int i = 0; i < Data.getXSize(); ++i, dst_index += Data.getXOffset() )
                {
                    if( AuxData.checkPosition(int(adbc.x()), int(adbc.y()), int(adbc.z())) )
                    {
                        // Source voxel value
                        Data.set(dst_index,AuxData.interpolate(adbc));
                    }

                    // Increment the position
                    adbc += step_adbc;
                }

                // Increment the position
                ad += step_ad;
                bc += step_bc;
            }

            // Update the progress bar
            if( !Progress( counter, counterMax ) )
            {
                return ELS_FAILED;
            }
        }

        // Destroy the temporary volume
        AuxData.resize(0, 0, 0, 0);
    }
    // END - THE TRANSFORMATION

lFinishLoading:

    // SUBSAMPLE LOADED DATA
    bool shouldFit = subsample(Data, subsamplingType, subsampling);

    // RESIZE VOLUME SO THAT (DIMENSION_k mod VOLUME_MULTIPLE == 0)
    correctVolumeSize(Data, VOLUME_MULTIPLE);

    // SET APPLICATION DATA

    // get the volume object from storage
    CObjectPtr<CDensityData> spVolume( APP_STORAGE.getEntry(Id) );

    // Use the data
    spVolume->makeRef( Data );

    // Fill the margin
    spVolume->mirrorMargin();

    // Copy dicom meta data
    spVolume->takeDicomData( RepSlice );

    spVolume->setImagePosition( Data.m_ImagePosition.getX(), Data.m_ImagePosition.getY(), Data.m_ImagePosition.getZ());
    spVolume->setImageSubSampling( subsampling );

    // Notify all interested windows
//    APP_STORAGE.invalidate( spVolume.getEntryPtr(), Storage::FORCE_UPDATE );
    APP_STORAGE.invalidate( spVolume.getEntryPtr() );

    this->onDataLoad( Id );

    // OK
    return shouldFit ? ELS_OK : ELS_WARNING_GPU_MEMORY;
}


///////////////////////////////////////////////////////////////////////////////
//

bool CExamination::setLimits(SVolumeOfInterest Limits, EDataSet Id, bool bForce)
{
    CObjectPtr<CVolumeOfInterest> spLimits( APP_STORAGE.getEntry(Id + 2) );
    CObjectPtr<CDensityData> spVolume( APP_STORAGE.getEntry(Id) );

    correctLimits(Limits, spVolume->getSize(), VOLUME_MULTIPLE);

    // Normalize limits
//    Limits.normalize();

    // Any changes?
    if( spLimits->getMinX() == Limits.m_MinX
        && spLimits->getMaxX() == Limits.m_MaxX
        && spLimits->getMinY() == Limits.m_MinY
        && spLimits->getMaxY() == Limits.m_MaxY
        && spLimits->getMinZ() == Limits.m_MinZ
        && spLimits->getMaxZ() == Limits.m_MaxZ )
    {
        if (!bForce)
            return false;
    }

    // Save the volume of interest
    spLimits->set(Limits).normalize();

    // Create temporary volume
    vpl::img::CDVolume VolumeData;
    VolumeData.copy(*spVolume,
                    Limits.m_MinX,
                    Limits.m_MinY,
                    Limits.m_MinZ,
                    Limits.m_MaxX - Limits.m_MinX + 1,
                    Limits.m_MaxY - Limits.m_MinY + 1,
                    Limits.m_MaxZ - Limits.m_MinZ + 1
                    );

    // Force data deallocation
//    spVolume->resize(0, 0 ,0);

    // Use the data
    spVolume->makeRef(VolumeData);

    // Fill the margin
    spVolume->mirrorMargin();

    // Entry has been changed
//    APP_STORAGE.invalidate(spVolume.getEntryPtr(), data::Storage::FORCE_UPDATE);
    APP_STORAGE.invalidate(spVolume.getEntryPtr());

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//

bool CExamination::saveDensityData(const std::string & ssFilename,
                                   vpl::mod::CProgress::tProgressFunc & Progress,
                                   EDataSet Id
                                   )
{
    // Open output file channel
    vpl::mod::CFileChannel Channel(vpl::mod::CH_OUT, ssFilename);
    if( !Channel.connect() )
    {
        return false;
    }

    CObjectPtr<CDensityData> spVolume( APP_STORAGE.getEntry(Id) );

    vpl::img::CDensityVolume * ptrVolume = dynamic_cast< vpl::img::CDensityVolume *  >(spVolume.get() );

    // Save the data as a density volume (without dicom obtained info).
    return vpl::mod::write(*ptrVolume, Channel, Progress);
}


///////////////////////////////////////////////////////////////////////////////
//

bool CExamination::saveDensityDataU(const vpl::sys::tString & ssFilename,
                                    vpl::mod::CProgress::tProgressFunc & Progress,
                                    EDataSet Id
                                    )
{
    // Open output file channel
    vpl::mod::CFileChannelU Channel(vpl::mod::CH_OUT, ssFilename);
    if( !Channel.connect() )
    {
        return false;
    }

    CObjectPtr<CDensityData> spVolume( APP_STORAGE.getEntry(Id) );

    vpl::img::CDensityVolume * ptrVolume = dynamic_cast< vpl::img::CDensityVolume *  >(spVolume.get() );

    // Save the data as a density volume (without dicom obtained info).
    return vpl::mod::write(*ptrVolume, Channel, Progress);
}


///////////////////////////////////////////////////////////////////////////////
//

void CExamination::onDataLoad( EDataSet Id )
{
}


bool CExamination::subsample(data::CDensityData &densityData, ESubsamplingType subsamplingType, vpl::img::CVector3d& subsampling)
{
    // calculate subsampling factors according to available video RAM
    unsigned long long int adapterRAM = m_adapterRAM;
    adapterRAM = vpl::math::getMax<unsigned long long int>(adapterRAM, DEFAULT_GPU_SIZE);
    unsigned long long int textureSize3D = m_textureSize3D;    
    unsigned long long int maximumVolumeSize = (adapterRAM - OTHER_GFX_STUFF_SIZE) / (VOLUME_BYTE_DEPTH);
    if (textureSize3D>0) // check agains maximum 3d texture size
    {
        unsigned long long int  maxTextureSize = textureSize3D*textureSize3D*textureSize3D;
        maximumVolumeSize = std::min(maximumVolumeSize,maxTextureSize);
    }
    unsigned long long int currentSize = densityData.getXSize() * densityData.getYSize() * densityData.getZSize();
    double subsamplingFactor = 1.0;

    if (currentSize > maximumVolumeSize)
    {
        double averageDimension = std::exp(std::log(static_cast<double>(currentSize)) / 3.0);
        double maximumVolumeDimension = std::exp(std::log(static_cast<double>(maximumVolumeSize)) / 3.0);

        if (textureSize3D>0) // can not exceed max 3d texture size in any dimension
        {
            unsigned long maxDim = std::max(densityData.getXSize(), std::max( densityData.getYSize(), densityData.getZSize()));
            if (maxDim>textureSize3D)
                averageDimension = maxDim;
        }

        subsamplingFactor = static_cast<double>(maximumVolumeDimension) / averageDimension;
    }

    // adjust subsampling factors
    switch (subsamplingType)
    {
    case EST_MAXIMUM_QUALITY:
        subsampling = vpl::img::CVector3d(1.0, 1.0, 1.0);
        break;

    case EST_3D_OPTIMIZED:
        subsampling = vpl::img::CVector3d(subsamplingFactor, subsamplingFactor, subsamplingFactor);
        break;

    case EST_MANUAL:
    default:
        // keep given factors
        break;
    }

    // do not subsample
    if (subsampling == vpl::img::CVector3d(1.0, 1.0, 1.0))
    {
        return ((unsigned long long)densityData.getXSize() * (unsigned long long)densityData.getYSize() * (unsigned long long)densityData.getZSize()) <= maximumVolumeSize;
    }

    vpl::img::CVector3i subsampledSize;
    subsampledSize.x() = densityData.getXSize() * subsampling.x();
    subsampledSize.y() = densityData.getYSize() * subsampling.y();
    subsampledSize.z() = densityData.getZSize() * subsampling.z();

    vpl::img::CVector3d realVolumeSize;
    realVolumeSize.x() = densityData.getDX() * densityData.getXSize();
    realVolumeSize.y() = densityData.getDY() * densityData.getYSize();
    realVolumeSize.z() = densityData.getDZ() * densityData.getZSize();

    vpl::img::CVector3d subsampledVoxelSize;
    subsampledVoxelSize.x() = realVolumeSize.x() / subsampledSize.x();
    subsampledVoxelSize.y() = realVolumeSize.y() / subsampledSize.y();
    subsampledVoxelSize.z() = realVolumeSize.z() / subsampledSize.z();

    subsampling.x() = densityData.getDX() / subsampledVoxelSize.x();
    subsampling.y() = densityData.getDY() / subsampledVoxelSize.y();
    subsampling.z() = densityData.getDZ() / subsampledVoxelSize.z();

    vpl::img::CVector3d subsamplingStep;
    subsamplingStep.x() = 1.0 / subsampling.x();
    subsamplingStep.y() = 1.0 / subsampling.y();
    subsamplingStep.z() = 1.0 / subsampling.z();

    data::CDensityData auxData;
    auxData.resize(subsampledSize.x(), subsampledSize.y(), subsampledSize.z());
    auxData.setDX(subsampledVoxelSize.x());
    auxData.setDY(subsampledVoxelSize.y());
    auxData.setDZ(subsampledVoxelSize.z());

    #pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < subsampledSize.z(); ++z)
    {
        double zz = z * subsamplingStep.z();

        for (vpl::tSize y = 0; y < subsampledSize.y(); ++y)
        {
            double yy = y * subsamplingStep.y();

            for (vpl::tSize x = 0; x < subsampledSize.x(); ++x)
            {
                double xx = x * subsamplingStep.x();

                auxData.at(x, y, z) = densityData.interpolate(vpl::img::CPoint3d(xx, yy, zz));
            }
        }
    }

    densityData.setDX(subsampledVoxelSize.x());
    densityData.setDY(subsampledVoxelSize.y());
    densityData.setDZ(subsampledVoxelSize.z());
    densityData.makeRef(auxData);

    return ((unsigned long long)densityData.getXSize() * (unsigned long long)densityData.getYSize() * (unsigned long long)densityData.getZSize()) <= maximumVolumeSize;
}

void CExamination::correctVolumeSize(vpl::img::CDVolume &densityData, vpl::tSize multiple)
{
    vpl::tSize sizeX = densityData.getXSize();
    vpl::tSize sizeY = densityData.getYSize();
    vpl::tSize sizeZ = densityData.getZSize();
    vpl::tSize correctX = densityData.getXSize() + (multiple - densityData.getXSize() % multiple) % multiple;
    vpl::tSize correctY = densityData.getYSize() + (multiple - densityData.getYSize() % multiple) % multiple;
    vpl::tSize correctZ = densityData.getZSize() + (multiple - densityData.getZSize() % multiple) % multiple;    
    if (correctX == sizeX && correctY==sizeY && correctZ == sizeZ)
        return;
    
    data::CDensityData auxData;
    auxData.resize(correctX, correctY, correctZ);
    auxData.fillEntire(vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin());

    #pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < densityData.getZSize(); ++z)
    {
        for (vpl::tSize y = 0; y < densityData.getYSize(); ++y)
        {
            for (vpl::tSize x = 0; x < densityData.getXSize(); ++x)
            {
                auxData.at(x, y, z) = densityData.at(x, y, z);
            }
        }
    }

    densityData.makeRef(auxData);
}

void CExamination::correctLimits(SVolumeOfInterest &limits, vpl::img::CSize3i volumeSize, vpl::tSize multiple)
{
    vpl::tSize limitedSizeX = limits.m_MaxX + 1 - limits.m_MinX;
    vpl::tSize limitedSizeY = limits.m_MaxY + 1 - limits.m_MinY;
    vpl::tSize limitedSizeZ = limits.m_MaxZ + 1 - limits.m_MinZ;

    vpl::tSize correctX = limitedSizeX + (multiple - limitedSizeX % multiple) % multiple;
    vpl::tSize correctY = limitedSizeY + (multiple - limitedSizeY % multiple) % multiple;
    vpl::tSize correctZ = limitedSizeZ + (multiple - limitedSizeZ % multiple) % multiple;

    if (limitedSizeX != correctX)
    {
        vpl::tSize newMin = limits.m_MinX;
        vpl::tSize newMax = limits.m_MinX + correctX - 1;

        if (newMax >= volumeSize.x())
        {
            vpl::tSize shift = newMax - volumeSize.x() + 1;
            newMin -= shift;
            newMax -= shift;
        }

        limits.m_MinX = newMin;
        limits.m_MaxX = newMax;
    }

    if (limitedSizeY != correctY)
    {
        vpl::tSize newMin = limits.m_MinY;
        vpl::tSize newMax = limits.m_MinY + correctY - 1;

        if (newMax >= volumeSize.y())
        {
            vpl::tSize shift = newMax - volumeSize.y() + 1;
            newMin -= shift;
            newMax -= shift;
        }

        limits.m_MinY = newMin;
        limits.m_MaxY = newMax;
    }

    if (limitedSizeZ != correctZ)
    {
        vpl::tSize newMin = limits.m_MinZ;
        vpl::tSize newMax = limits.m_MinZ + correctZ - 1;

        if (newMax >= volumeSize.z())
        {
            vpl::tSize shift = newMax - volumeSize.z() + 1;
            newMin -= shift;
            newMax -= shift;
        }

        limits.m_MinZ = newMin;
        limits.m_MaxZ = newMax;
    }
}

} // namespace data

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
