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

#ifndef CMultiClassRegionData_H
#define CMultiClassRegionData_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/Lock.h>
#include <VPL/Image/Volume.h>
#include <VPL/Module/Serializable.h>
#include <data/CVolumeUndo.h>
#include <data/storage_ids_core.h>
#include <data/CStorageInterface.h>

#include "data/CObjectHolder.h"
#include <data/CBitVolume.h>

namespace data
{

typedef vpl::img::tPixel32 tRegionVoxel;

///////////////////////////////////////////////////////////////////////////////
//! Class manages segmented volumetric data.
//! Every region is represented by one bit in voxel, so there can be maximum of sizeof(tRegionVoxel) overlapping regions.
//! When resizing manually, m_sizeMutex must be locked!!! (because of region 3D preview)
//! Rather, use method resizeSafe, which hadles it itself.
class CMultiClassRegionData : public CBitVolume<tRegionVoxel>
{
public:
    //! Standard method getEntityName().
    VPL_ENTITY_NAME("CMultiClassRegionData");

    //! Standard method getEntityCompression().
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CMultiClassRegionData);

    //! Default size of the volume margin.
    enum { DEFAULT_MARGIN = 4 };

    //! Initial size.
    enum { INIT_SIZE = 1 };

    //! Initial number of regions.
    enum { MAX_REGIONS = 32 };

	//! Volume snapshot provider type
	typedef data::CVolumeUndo< vpl::img::CVolume<tRegionVoxel> > tVolumeUndo;

public:
    //! Default constructor.
    CMultiClassRegionData();

    //! Copy constructor.
    CMultiClassRegionData(const CMultiClassRegionData& Data);

    //! Destructor.
    ~CMultiClassRegionData();

    static int getMaxNumberOfRegions()
    {
        return MAX_REGIONS;
    }

    //! Returns mutex, which must be locked before resizing the volume.
    vpl::sys::CMutex& getSizeMutex();

    void disableDummyMode();

    //! Resizes the volume safely. Mutex is locked before resizing and unlocked afterwards.
    CMultiClassRegionData& resizeSafe(vpl::tSize xSize, vpl::tSize ySize, vpl::tSize zSize, vpl::tSize margin = 0);

    //! Enables region coloring.
    CMultiClassRegionData& enableColoring(bool bEnabled = true);

    //! Returns true if the coloring is enabled.
    bool isColoringEnabled() const { return m_bColoringEnabled; }

    //! Writes the density volume data to a given output channel.
    template <class S>
    void serialize(vpl::mod::CChannelSerializer<S>& Writer)
    {
	    vpl::img::CVolume<tRegionVoxel>::serialize(Writer);
        Writer.beginWrite( *this );
        WRITEINT32(1);
        Writer.write( (unsigned char)m_bColoringEnabled );
        Writer.endWrite(*this);

    }

    //! Reads the density volume data from a given input channel.
    template <class S>
    void deserialize(vpl::mod::CChannelSerializer<S>& Reader)
    {
	    vpl::img::CVolume<tRegionVoxel>::deserialize(Reader);
        Reader.beginRead(*this);
        int version = 0;
        READINT32(version);
        unsigned char b = 0;
        Reader.read( b );
        m_bColoringEnabled = (b != 0);
        Reader.endRead( *this );
    }

    //! Does object contain any relevant data?
    bool hasData();

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& Changes);

	//! Initializes the object to its default state.
    void init();

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry *pParent);

    //! Get snapshot of the whole volume
    data::CSnapshot * getVolumeSnapshot() { return m_volumeUndo.getSnapshotVolume( ); }

    //! Get snapshot of the plane XY 
    data::CSnapshot * getPlaneXYSnapshot( int position ) { return m_volumeUndo.getSnapshotXY( position ); }

    //! Get snapshot of the plane XZ 
    data::CSnapshot * getPlaneXZSnapshot( int position ) { return m_volumeUndo.getSnapshotXZ( position ); }

    //! Get snapshot of the plane YZ 
    data::CSnapshot * getPlaneYZSnapshot( int position ) { return m_volumeUndo.getSnapshotYZ( position ); }

    //! Fills region volume from another volume.
    //! Volumes must be of the same size.
    //! Gets labels from source volumes and converts them to bits.
    //! Only first MAX_REGIONS values are processed.
    template <class T>
    void fillFromVolume(const vpl::img::CVolume<T> &volume)
    {
        vpl::img::CVolSize vSize = volume.getSize();

        const vpl::tSize sx = m_Size.x();
        const vpl::tSize sy = m_Size.y();
        const vpl::tSize sz = m_Size.z();

        if (vSize.x() != sx || vSize.y() != sy || vSize.z() != sz)
        {
            return;
        }

        std::set<int> labels;

        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                for (vpl::tSize i = 0; i < sx; ++i)
                {
                    T val = volume.at(i, j, k);

                    if (val > 0)
                    {
                        labels.insert(val);
                    }
                }
            }
        }

        std::vector<int> labelToBitIndexMapping;
        labelToBitIndexMapping.resize(*(--labels.end()) + 1);

        for (int i = 0; i < labelToBitIndexMapping.size(); ++i)
        {
            labelToBitIndexMapping[i] = -1;
        }

        int index = 0;
        for (auto it = labels.begin(); it != labels.end(); ++it)
        {
            labelToBitIndexMapping[*it] = index;
            ++index;

            if (index == MAX_REGIONS)
            {
                break;
            }
        }

        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                for (vpl::tSize i = 0; i < sx; ++i)
                {
                    T val = volume.at(i, j, k);
                    int bitIndex = labelToBitIndexMapping[val];

                    if (val > 0 && bitIndex >= 0)
                    {
                        setBit(i, j, k, bitIndex);
                    }
                }
            }
        }
    }

    template <class T>
    vpl::img::CVolume<T> getSelectedRegion(vpl::tSize bitIndex)
    {
        vpl::img::CVolume<T> volume;
        volume.resize(m_Size, m_Margin);
        volume.fillEntire((T)0);

        const vpl::tSize sx = m_Size.x();
        const vpl::tSize sy = m_Size.y();
        const vpl::tSize sz = m_Size.z();
        const vpl::tSize offset = getXOffset();

#pragma omp parallel for
        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                vpl::tSize idx = getIdx(0, j, k);
                for (vpl::tSize i = 0; i < sx; ++i, idx += offset)
                {
                    if (at(idx, bitIndex))
                    {
                        volume.at(idx) = (T)1;
                    }
                }
            }
        }

        return volume;
    }

protected:
    //! Is the coloring enabled?
    bool m_bColoringEnabled;

    //! Must be locked before resizing volume!
    vpl::sys::CMutex m_sizeMutex;

    //! Volume undo object
    tVolumeUndo m_volumeUndo;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Serialization wrapper. 
////////////////////////////////////////////////////////////////////////////////////////////////////
DECLARE_SERIALIZATION_WRAPPER( CMultiClassRegionData )

namespace Storage
{
	//! Segmented data.
	DECLARE_OBJECT(MultiClassRegionData, CMultiClassRegionData, CORE_STORAGE_MULTI_CLASS_REGION_DATA_ID);
}
} // namespace data

#endif // CMultiClassRegionData_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
