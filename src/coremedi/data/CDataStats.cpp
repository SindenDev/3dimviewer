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

#include <data/CDataStats.h>
#include <VPL/Image/VolumeFunctions.h>
#include <data/CActiveDataSet.h>
#include <data/CDensityData.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn data::CDataStats::CDataStats(void)
//
//\brief ! Default constructor. 
////////////////////////////////////////////////////////////////////////////////////////////////////

data::CDataStats::CDataStats(void)
   : m_minDensity( 0 )
   , m_maxDensity( 0 )
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void data::CDataStats::update(const CChangedEntries&Changes)
//
//\brief ! Regenerates the object state according to any changes in the data storage. 
//
//\param Changes  The changes. 
////////////////////////////////////////////////////////////////////////////////////////////////////

void data::CDataStats::update(const CChangedEntries & Changes)
{
    // Get active dataset
    data::CObjectPtr< data::CActiveDataSet > ptrDataset( APP_STORAGE.getEntry( data::Storage::ActiveDataSet::Id ) );
    // Get data volume
    data::CObjectPtr< data::CDensityData > sVolume( APP_STORAGE.getEntry( ptrDataset->getId() ) );

    // Compute min and max values
    vpl::img::getMinMax< vpl::img::CDVolume::tVoxel, vpl::img::CDVolume >( *sVolume.get(), m_minDensity, m_maxDensity );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void data:::::init(void)
//
//\brief ! Initializes the object to its default state. 
////////////////////////////////////////////////////////////////////////////////////////////////////

void data::CDataStats::init(void)
{
	m_minDensity = m_maxDensity = 0;
}

