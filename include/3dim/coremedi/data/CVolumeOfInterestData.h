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

#ifndef CVolumeOfInterestData_H
#define CVolumeOfInterestData_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/Lock.h>
#include <VPL/Module/Serializable.h>
#include <data/storage_ids_core.h>
#include <data/CStorageInterface.h>

#include "data/CObjectHolder.h"


namespace data
{

class CVolumeOfInterestData : public vpl::base::CObject, public vpl::base::CLockableObject<CVolumeOfInterestData>
{
public:
	//! Standard method getEntityName().
	VPL_ENTITY_NAME("CVolumeOfInterestData");

	//! Standard method getEntityCompression().
	VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

	//! Smart pointer type.
	//! - Declares type tSmartPtr.
	VPL_SHAREDPTR(CVolumeOfInterestData);

	//! Default constructor.
	CVolumeOfInterestData(){}

	//! Destructor.
	~CVolumeOfInterestData(){}

	//! Does object contain any relevant data?
	bool hasData();

	//! Regenerates the object state according to any changes in the data storage.
	void update(const CChangedEntries& Changes);

	//! Initializes the object to its default state.
	void init();

	//! Returns true if changes of a given parent entry may affect this object.
	bool checkDependency(CStorageEntry *pParent);

	//! Returns one of the limits.
	vpl::tSize getMinX() { return m_minX; }
	vpl::tSize getMinY() { return m_minY; }
	vpl::tSize getMinZ() { return m_minZ; }
	vpl::tSize getMaxX() { return m_maxX; }
	vpl::tSize getMaxY() { return m_maxY; }
	vpl::tSize getMaxZ() { return m_maxZ; }

	//! Sets one of the limits.
	void setMinX(vpl::tSize minX) { m_minX = minX; m_isVOISet = true; }
	void setMaxX(vpl::tSize maxX) { m_maxX = maxX; m_isVOISet = true; }
	void setMinY(vpl::tSize minY) { m_minY = minY; m_isVOISet = true; }
	void setMaxY(vpl::tSize maxY) { m_maxY = maxY; m_isVOISet = true; }
	void setMinZ(vpl::tSize minZ) { m_minZ = minZ; m_isVOISet = true; }
	void setMaxZ(vpl::tSize maxZ) { m_maxZ = maxZ; m_isVOISet = true; }

	//! Sets all limits.
	void setLimits(vpl::tSize minX, vpl::tSize minY, vpl::tSize minZ, vpl::tSize maxX, vpl::tSize maxY, vpl::tSize maxZ);

	//! True if voxel on given coordinates is inside the volume of interest.
	bool isInside(vpl::tSize x, vpl::tSize y, vpl::tSize z);

	//! True if volume of interest has been set.
	bool isSet() { return m_isVOISet; }

	//! Sets flag, that tells if the volume of interest is set.
	void setSetFlag(bool set) { m_isVOISet = set; }

	//! Writes the data to a given output channel.
	template <class S>
	void serialize(vpl::mod::CChannelSerializer<S>& Writer)
	{
	}

	//! Reads the data from a given input channel.
	template <class S>
	void deserialize(vpl::mod::CChannelSerializer<S>& Reader)
	{
	}

protected:
	vpl::tSize m_minX;
	vpl::tSize m_maxX;
	vpl::tSize m_minY;
	vpl::tSize m_maxY;
	vpl::tSize m_minZ;
	vpl::tSize m_maxZ;

	bool m_isVOISet;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Serialization wrapper. 
////////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_SERIALIZATION_WRAPPER(CVolumeOfInterestData)

namespace Storage
{
	//! Volume of interest data.
	DECLARE_OBJECT(VolumeOfInterestData, CVolumeOfInterestData, CORE_STORAGE_VOLUME_OF_INTEREST_DATA_ID);
}
} // namespace data

#endif // CVolumeOfInterestData_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
