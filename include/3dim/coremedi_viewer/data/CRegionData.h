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

#ifndef CRegionData_H
#define CRegionData_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/Lock.h>
#include <VPL/Image/Volume.h>
#include <VPL/Module/Serializable.h>
#include <data/CVolumeUndo.h>
#include <data/storage_ids_core.h>
#include <data/CStorageInterface.h>

#include "data/CObjectHolder.h"


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Class manages segmented volumetric data.

class CRegionData : public vpl::img::CVolume16//, public vpl::base::CLockableObject<CRegionData>
{
public:
    //! Standard method getEntityName().
    VPL_ENTITY_NAME("CRegionData");

    //! Standard method getEntityCompression().
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CRegionData);

    //! Default size of the volume margin.
    enum { DEFAULT_MARGIN = 4 };

    //! Lock type.
//    typedef CLockableObject<CRegionData>::CLock tLock;

    //! Initial size.
    enum { INIT_SIZE = 1 };

	//! Volume snapshot provider type
	typedef data::CVolumeUndo< vpl::img::CVolume16 > tVolumeUndo;

public:
    //! Default constructor.
    CRegionData();

    //! Copy constructor.
    CRegionData(const CRegionData& Data);

    //! Destructor.
    ~CRegionData();

    //! Enables region coloring.
    CRegionData& enableColoring(bool bEnabled = true);

    //! Returns true if the coloring is enabled.
    bool isColoringEnabled() const { return m_bColoringEnabled; }

    CRegionData& makeDummy(bool dummy);

    //! Writes the density volume data to a given output channel.
    template <class S>
    void serialize(vpl::mod::CChannelSerializer<S>& Writer)
    {
	    vpl::img::CVolume16::serialize(Writer);
        Writer.beginWrite( *this );

        // need to write some magic value, because the version was missing and i needed to write another stuff (JS)
        Writer.write((unsigned char)126);
        WRITEINT32(2);

        Writer.write( (unsigned char)m_bColoringEnabled );

        Writer.write(m_bMakeDummy);

        Writer.endWrite(*this);

    }

    //! Reads the density volume data from a given input channel.
    template <class S>
    void deserialize(vpl::mod::CChannelSerializer<S>& Reader)
    {
	    vpl::img::CVolume16::deserialize(Reader);
        Reader.beginRead(*this);
        unsigned char b = 0;
        Reader.read( b );

        if (b != 126)
        {
            m_bColoringEnabled = (b != 0);
        }
        else
        {
            int version = 0;
            READINT32(version);

            if (version > 1)
            {
                Reader.read(m_bMakeDummy);
            }
        }

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

protected:
    //! Is the coloring enabled?
    bool m_bColoringEnabled;

    bool m_bMakeDummy;

    //! Volume undo object
    tVolumeUndo m_volumeUndo;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Serialization wrapper. 
////////////////////////////////////////////////////////////////////////////////////////////////////
DECLARE_SERIALIZATION_WRAPPER( CRegionData )

namespace Storage
{
	//! Segmented data.
	DECLARE_OBJECT(RegionData, CRegionData, CORE_STORAGE_REGION_DATA_ID);
}
} // namespace data

#endif // CRegionData_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
