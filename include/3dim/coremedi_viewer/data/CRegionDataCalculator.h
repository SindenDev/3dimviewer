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

#ifndef CRegionDataCalculator_H
#define CRegionDataCalculator_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/Lock.h>
#include <VPL/Image/Volume.h>
#include <VPL/Module/Serializable.h>
#include <data/storage_ids_core.h>
#include <data/CStorageInterface.h>

#include "data/CObjectHolder.h"


namespace data
{

class CRegionDataCalculator : public vpl::base::CObject, public vpl::base::CLockableObject<CRegionDataCalculator>
{
public:
	//! Standard method getEntityName().
	VPL_ENTITY_NAME("CRegionDataCalculator");

	//! Standard method getEntityCompression().
	VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

	//! Smart pointer type.
	//! - Declares type tSmartPtr.
	VPL_SHAREDPTR(CRegionDataCalculator);

public:
	//! Default constructor.
	CRegionDataCalculator();

	//! Destructor.
	~CRegionDataCalculator();

	//! Does object contain any relevant data?
	bool hasData();

	//! Regenerates the object state according to any changes in the data storage.
	void update(const CChangedEntries& Changes);

	//! Initializes the object to its default state.
	void init();

	//! Returns true if changes of a given parent entry may affect this object.
	bool checkDependency(CStorageEntry *pParent);

	int getVoxelsCnt(int regionIndex)
	{
		return (regionIndex < 0 || regionIndex >= m_voxelsCnt.size() ? 0 : m_voxelsCnt[regionIndex]);
	}

	int getMinDensity(int regionIndex)
	{
		return (regionIndex < 0 || regionIndex >= m_minDensity.size() ? 0 : m_minDensity[regionIndex]);
	}

	int getMaxDensity(int regionIndex)
	{
		return (regionIndex < 0 || regionIndex >= m_maxDensity.size() ? 0 : m_maxDensity[regionIndex]);
	}

	double getAvgDensity(int regionIndex)
	{
		return (regionIndex < 0 || regionIndex >= m_voxelsCnt.size() || regionIndex >= m_sumDensity.size() || m_voxelsCnt[regionIndex] == 0 ? 0 : m_sumDensity[regionIndex] / m_voxelsCnt[regionIndex]);
	}

	double getDensityStandardDeviation(int regionIndex)
	{
		return (regionIndex < 0 || regionIndex >= m_densityVariance.size() ? 0 : sqrt(m_densityVariance[regionIndex]));
	}

	double getDensityVariance(int regionIndex)
	{
		return (regionIndex < 0 || regionIndex >= m_densityVariance.size() ? 0 : m_densityVariance[regionIndex]);
	}

	int getXSize(int regionIndex)
	{
		return (regionIndex < 0 || regionIndex >= m_maxXCoord.size() || regionIndex >= m_minXCoord.size() ? 0 : m_maxXCoord[regionIndex] - m_minXCoord[regionIndex] + 1);
	}

	int getYSize(int regionIndex)
	{
		return (regionIndex < 0 || regionIndex >= m_maxYCoord.size() || regionIndex >= m_minYCoord.size() ? 0 : m_maxYCoord[regionIndex] - m_minYCoord[regionIndex] + 1);
	}

	int getZSize(int regionIndex)
	{
		return (regionIndex < 0 || regionIndex >= m_maxZCoord.size() || regionIndex >= m_minZCoord.size() ? 0 : m_maxZCoord[regionIndex] - m_minZCoord[regionIndex] + 1);
	}

	//! Writes the density volume data to a given output channel.
	template <class S>
	void serialize(vpl::mod::CChannelSerializer<S>& Writer)
	{
	}

	//! Reads the density volume data from a given input channel.
	template <class S>
	void deserialize(vpl::mod::CChannelSerializer<S>& Reader)
	{
	}

protected:

	std::vector<int> m_voxelsCnt;
	std::vector<int> m_minDensity;
	std::vector<int> m_maxDensity;
	std::vector<double> m_sumDensity;
	std::vector<double> m_densityVariance;
	std::vector<int> m_minXCoord;
	std::vector<int> m_maxXCoord;
	std::vector<int> m_minYCoord;
	std::vector<int> m_maxYCoord;
	std::vector<int> m_minZCoord;
	std::vector<int> m_maxZCoord;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Serialization wrapper. 
////////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_SERIALIZATION_WRAPPER(CRegionDataCalculator)

namespace Storage
{
	//! Segmented data.
	DECLARE_OBJECT(RegionDataCalculator, CRegionDataCalculator, CORE_STORAGE_REGION_DATA_CALCULATOR_ID);
}
} // namespace data

#endif // CRegionDataCalculator_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
