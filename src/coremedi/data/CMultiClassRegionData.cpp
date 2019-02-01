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

///////////////////////////////////////////////////////////////////////////////
// include files

#include <data/CMultiClassRegionData.h>
#include <data/CDensityData.h>

#ifdef _OPENMP
#    include <omp.h>
#endif


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//

CMultiClassRegionData::CMultiClassRegionData()
    : CBitVolume<tRegionVoxel>(1, 1, 1, DEFAULT_MARGIN )
    , m_bColoringEnabled(false)
    , m_sizeMutex(false)
    , m_volumeUndo(NULL, data::Storage::MultiClassRegionData::Id, true)
{
	m_volumeUndo.setVolumePtr( this );
}


///////////////////////////////////////////////////////////////////////////////
//
CMultiClassRegionData::CMultiClassRegionData(const CMultiClassRegionData& Data)
    : CBitVolume<tRegionVoxel>(Data)
    , m_sizeMutex(false)
    , m_bColoringEnabled(Data.m_bColoringEnabled)
{
	m_volumeUndo.setVolumePtr( this );
}

///////////////////////////////////////////////////////////////////////////////
//

CMultiClassRegionData& CMultiClassRegionData::enableColoring(bool bEnabled)
{
    m_bColoringEnabled = bEnabled;
    return *this;
}


///////////////////////////////////////////////////////////////////////////////
//

CMultiClassRegionData::~CMultiClassRegionData()
{
}

vpl::sys::CMutex& CMultiClassRegionData::getSizeMutex()
{
    return m_sizeMutex;
}


////////////////////////////////////////////////////////////
//

bool CMultiClassRegionData::checkDependency(CStorageEntry *pParent)
{
    return true;
}


////////////////////////////////////////////////////////////
//

void CMultiClassRegionData::update(const CChangedEntries& Changes)
{
    if (Changes.checkFlagAny(data::Storage::STORAGE_RESET))
    {
        init();
    }
    else
    {
        // Get the patient data
        CObjectPtr<CDensityData> spData(APP_STORAGE.getEntry(Storage::PatientData::Id));

        // Re-initializes the data if density volume has changed its proportions.
        vpl::tSize XSize = spData->getXSize();
        vpl::tSize YSize = spData->getYSize();
        vpl::tSize ZSize = spData->getZSize();
        if (getXSize() != XSize || getYSize() != YSize || getZSize() != ZSize)
        {
            unsigned long rx = XSize;
            unsigned long ry = YSize;
            unsigned long rz = ZSize;
            unsigned long dx = spData->getXSize();
            unsigned long dy = spData->getYSize();
            unsigned long dz = spData->getZSize();

            if (rx * ry * rz < dx * dy * dz)
            {
                // Enforce data deallocation
                resizeSafe(0, 0, 0, 0);
            }

            resizeSafe(XSize, YSize, ZSize, vpl::math::getMax< vpl::tSize >(spData->getMargin(), DEFAULT_MARGIN));
            fillEntire(0);
        }
    }
}


////////////////////////////////////////////////////////////
//

void CMultiClassRegionData::init()
{
    // Enforce data deallocation
    resizeSafe(0, 0, 0, 0);

    enableDummyMode(true);

    resizeSafe(INIT_SIZE, INIT_SIZE, INIT_SIZE, DEFAULT_MARGIN );

    fillEntire(0);

    m_bColoringEnabled = false;
}

void CMultiClassRegionData::disableDummyMode()
{
    enableDummyMode(false);
    resizeSafe(0, 0, 0, 0);
}

CMultiClassRegionData& CMultiClassRegionData::resizeSafe(vpl::tSize xSize, vpl::tSize ySize, vpl::tSize zSize, vpl::tSize margin)
{
    m_sizeMutex.lock();
    resize(xSize, ySize, zSize, margin);
    m_sizeMutex.unlock();

    return *this;
}


///////////////////////////////////////////////////////////////////////////////
//

bool CMultiClassRegionData::hasData()
{
    return (!isDummy() && getXSize() > 1);
}


///////////////////////////////////////////////////////////////////////////////
//

//void CMultiClassRegionData::()
//{
//
//}


} // namespace data

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
