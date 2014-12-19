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

#ifndef CDataStats_H
#define CDataStats_H

#include <VPL/Image/DensityVolume.h>

#include <data/CSerializableData.h>
#include <data/CChangedEntries.h>
#include <data/CStorageEntry.h>
#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Collects basic statistics about loaded volume data such
//! as the minimum and the maximum value.

class CDataStats : public vpl::base::CObject
{
public:
   //! Smart pointer type.
   VPL_SHAREDPTR(CDataStats);
   
public:
   //! Default constructor.
   CDataStats();
   
   //! Regenerates the object state according to any changes in the data storage.
   void update(const CChangedEntries& Changes);
   
   //! Initializes the object to its default state.
   void init();
   
   //! Returns true if changes of a given parent entry may affect this object.
   bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

   //! Get density minimum
   vpl::img::CDensityVolume::tVoxel getMinimalDensity() { return m_minDensity; }

   //! Get density minimum
   vpl::img::CDensityVolume::tVoxel getMaximalDensity() { return m_maxDensity; }

   //! Object is computational so it has no data
   bool hasData() { return false; }

protected:
   //! Data minimal value
   vpl::img::CDensityVolume::tVoxel m_minDensity;

   //! Data minimal value
   vpl::img::CDensityVolume::tVoxel m_maxDensity;

}; // class CDataStats

namespace Storage
{
	//! Volume data statistics.
	DECLARE_OBJECT(DataStats, CDataStats, PATIENT_DATA + 3);
}

} // namespace data


namespace vpl
{
namespace img
{

//! Functional object which can be used to find minimal value.
template <typename T>
class CMinMax
{
public:
    //! Default constructor.
    CMinMax(const T& Value = T()) : m_Min(Value), m_Max(Value) {}

    //! Operator compares a given value to the actual minimum.
    void operator ()(const T& Value)
    {
        if( m_Min != Value || m_Max != Value )
        {
            m_Min = vpl::math::getMin(Value, m_Min);
            m_Max = vpl::math::getMax(Value, m_Max);
        }       
    }

    //! Returns the actual minimum.
    void getValue( T & min, T & max ) { min = m_Min; max = m_Max; }

protected:
    //! Actual minimal value.
    T m_Min;

    //! Actual maximal value
    T m_Max;
};


//! Returns minimum pixel value in the volume.
template <typename R, class V>
inline void getMinMax(const CVolumeBase<V>& Volume, R & min, R & max)
{
    typedef typename V::tVoxel tVoxel;
    const V& VolumeImpl = Volume.getImpl();

    tSize Count = VolumeImpl.getXSize() * VolumeImpl.getYSize() * VolumeImpl.getZSize();
    VPL_CHECK(Count > 0, return );

    // Compute values
    VolumeImpl.forEach( CMinMax<tVoxel>( VolumeImpl(0,0,0) ) ).getValue( min, max );
}


} // namespace img
} // namespace vpl

#endif // CDataStats_H

