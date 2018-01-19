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

#include <data/CCoordinatesConv.h>
#include <data/CDensityData.h>
#include <coremedi/app/Signals.h>
#include <VPL/Base/Logging.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////

CCoordinatesConv::CCoordinatesConv()
    : m_dDX(1.0)
    , m_dDY(1.0)
    , m_dDZ(1.0)
    , m_dInvDX(1.0)
    , m_dInvDY(1.0)
    , m_dInvDZ(1.0)
{
    setupScene(CDensityData::INIT_SIZE, CDensityData::INIT_SIZE, CDensityData::INIT_SIZE);
}

///////////////////////////////////////////////////////////////////////////////

CCoordinatesConv::CCoordinatesConv(double dDX, double dDY, double dDZ, int SizeX, int SizeY, int SizeZ)
    : m_dDX(dDX)
    , m_dDY(dDY)
    , m_dDZ(dDZ)
{
    VPL_ASSERT(dDX > 0.0 && dDY > 0.0 && dDZ > 0.0 && SizeX > 0 && SizeY > 0 && SizeZ > 0);

    m_dInvDX = 1.0 / dDX;
    m_dInvDY = 1.0 / dDY;
    m_dInvDZ = 1.0 / dDZ;

    setupScene(SizeX, SizeY, SizeZ);
}

///////////////////////////////////////////////////////////////////////////////

CCoordinatesConv::CCoordinatesConv(const CCoordinatesConv& Conv)
    : vpl::base::CObject()
    , m_dDX(Conv.m_dDX)
    , m_dDY(Conv.m_dDY)
    , m_dDZ(Conv.m_dDZ)
    , m_dInvDX(Conv.m_dInvDX)
    , m_dInvDY(Conv.m_dInvDY)
    , m_dInvDZ(Conv.m_dInvDZ)
    , m_dMaxX(Conv.m_dMaxX)
    , m_dMaxY(Conv.m_dMaxY)
    , m_dMaxZ(Conv.m_dMaxZ)
{
}

///////////////////////////////////////////////////////////////////////////////

CCoordinatesConv& CCoordinatesConv::operator =(const CCoordinatesConv& Conv)
{
    m_dDX = Conv.m_dDX;
    m_dDY = Conv.m_dDY;
    m_dDZ = Conv.m_dDZ;
    m_dInvDX = Conv.m_dInvDX;
    m_dInvDY = Conv.m_dInvDY;
    m_dInvDZ = Conv.m_dInvDZ;
    m_dMaxX = Conv.m_dMaxX;
    m_dMaxY = Conv.m_dMaxY;
    m_dMaxZ = Conv.m_dMaxZ;
    return *this;
}
    
///////////////////////////////////////////////////////////////////////////////

void CCoordinatesConv::update(const CChangedEntries& Changes) // TODO: I think it should be better to save density object id because it can be missing in the changes list
{
    // Find parent volume data that has been changed
	typedef CObjectHolder<CDensityData> tObjectHolder;
	int id = APP_STORAGE.findEntryId<tObjectHolder>(Changes);
	if (id != Storage::UNKNOWN)
	{
		CObjectPtr<CDensityData> spData( APP_STORAGE.getEntry(id) /* APP_STORAGE.findObject<CDensityData>(Changes) */ );

		// Initialize the conversion
		setVoxel(spData->getDX(), spData->getDY(), spData->getDZ());
		setupScene(spData->getXSize(), spData->getYSize(), spData->getZSize());
	}
}


///////////////////////////////////////////////////////////////////////////////

void CCoordinatesConv::init()
{
    setVoxel(1.0, 1.0, 1.0);

    setupScene(CDensityData::INIT_SIZE, CDensityData::INIT_SIZE, CDensityData::INIT_SIZE);
}


///////////////////////////////////////////////////////////////////////////////

void CAuxCoordinatesConv::update(const CChangedEntries& Changes)
{
    // Find parent volume data that has been changed
    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId == CUSTOM_DATA)
    {
        return;
    }

    CObjectPtr<CDensityData> spData(APP_STORAGE.getEntry(datasetId));

    // Initialize the conversion
    setVoxel(spData->getDX(), spData->getDY(), spData->getDZ());
    setupScene(spData->getXSize(), spData->getYSize(), spData->getZSize());
}


///////////////////////////////////////////////////////////////////////////////

void CAuxCoordinatesConv::init()
{
    setVoxel(1.0, 1.0, 1.0);

    setupScene(CDensityData::INIT_SIZE, CDensityData::INIT_SIZE, CDensityData::INIT_SIZE);
}


} // namespace data
