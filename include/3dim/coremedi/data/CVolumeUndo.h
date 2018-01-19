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

#ifndef CVolumeUndo_H
#define CVolumeUndo_H

#include <VPL/Image/Volume.h>

#include <data/CUndoBase.h>
#include <data/CRLECompress.h>
#include <data/CDataStorage.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Creates snapshot of volume data (e.g. segmentation/region data).

template < class V >
class CVolumeSnapshot : public CSnapshot
{
public:
    //! Volume type
    typedef typename V::tVolume tVolume;

    //! Voxel type
    typedef typename V::tVoxel tVoxel;

public:
    //! Constructor
    CVolumeSnapshot(  int type,  tVolume * ptrVolume, CUndoProvider * provider = NULL );

    //! Each undo object must define restore function
    virtual bool restore( tVolume * ptrVolume );

    //! Each undo object must return its data size in bytes
    virtual long getDataSize() { return m_compress.getDataSize(); }

protected:
    //! Compress
    CRLECompressVolume< V > m_compress;

}; // class CVolumeUndo


///////////////////////////////////////////////////////////////////////////////
//! CLASS CPlaneXYSnapshot - do undo on XY plane.

template < class V >
class CPlaneXYSnapshot : public CSnapshot
{
public:
    //! Volume type
    typedef typename V::tVolume tVolume;

    //! Voxel type
    typedef typename V::tVoxel tVoxel;

public:
    //! Constructor
    CPlaneXYSnapshot(  int type, tVolume * ptrVolume, vpl::tSize position, CUndoProvider * provider = NULL );

    //! Restore plane
    bool restore( tVolume * ptrVolume );

    //! Each undo object must return its data size in bytes
    virtual long getDataSize() { return m_compress.getDataSize(); }

    //! Return position of the plane
    vpl::tSize getPosition() { return m_position; }

protected:
    //! Compress
    CRLECompressedData< tVoxel > m_compress;

    //! Plane position
    vpl::tSize m_position;

    vpl::img::CImage< tVoxel > * m_plane;

}; // class CPlaneUndo


///////////////////////////////////////////////////////////////////////////////
//! CLASS CPlaneXZSnapshot - do undo on XZ plane.

template < class V >
class CPlaneXZSnapshot : public CSnapshot
{
public:
    //! Volume type
    typedef typename V::tVolume tVolume;

    //! Voxel type
    typedef typename V::tVoxel tVoxel;

public:
    //! Constructor
    CPlaneXZSnapshot(  int type, tVolume * ptrVolume, vpl::tSize position, CUndoProvider * provider = NULL );

    //! Restore plane
    bool restore( tVolume * ptrVolume );

    //! Each undo object must return its data size in bytes
    virtual long getDataSize() { return m_compress.getDataSize(); }

    //! Return position of the plane
    vpl::tSize getPosition() { return m_position; }

protected:
    //! Compress
    CRLECompressedData< tVoxel > m_compress;

    //! Plane position
    vpl::tSize m_position;

    vpl::img::CImage< tVoxel > * m_plane;

}; // class CPlaneUndo


///////////////////////////////////////////////////////////////////////////////
//! CLASS CPlaneYZSnapshot - do undo on YZ plane

template < class V >
class CPlaneYZSnapshot : public CSnapshot
{
public:
    //! Volume type
    typedef typename V::tVolume tVolume;

    //! Voxel type
    typedef typename V::tVoxel tVoxel;

public:
    //! Constructor
    CPlaneYZSnapshot(  int type, tVolume * ptrVolume, vpl::tSize position, CUndoProvider * provider = NULL );

    //! Restore plane
    bool restore( tVolume * ptrVolume );

    //! Each undo object must return its data size in bytes
    virtual long getDataSize() { return m_compress.getDataSize(); }

    //! Return position of the plane
    vpl::tSize getPosition() { return m_position; }

protected:
    //! Compress
    CRLECompressedData< tVoxel> m_compress;

    //! Plane position
    vpl::tSize m_position;

    vpl::img::CImage< tVoxel > * m_plane;

}; // class CPlaneUndo


///////////////////////////////////////////////////////////////////////////////
//! CLASS CVolumeUndo

template < class V >
class CVolumeUndo : public  CUndoProvider
{
public:
    //! Volume type
    typedef typename V::tVolume tVolume;

    //! Voxel type
    typedef typename V::tVoxel tVoxel;

    //! Snapshot type
    typedef data::CVolumeSnapshot< V > tSnapshot;

    //! Plane XY snapshot type
    typedef data::CPlaneXYSnapshot< V > tSnapshotXY;

    //! Plane XZ snapshot type
    typedef data::CPlaneXZSnapshot< V > tSnapshotXZ;

    //! Plane YZ snapshot type
    typedef data::CPlaneYZSnapshot< V > tSnapshotYZ;

public:
    //! Constructor
    CVolumeUndo( tVolume * ptrVolume, int InvalidationID = 0, bool invalidate = false ) 
	    : CUndoProvider( InvalidationID, invalidate ), m_ptrVolume( ptrVolume ){}

    //! Default constructor. Volume pointer must be set before the first use
    CVolumeUndo( ) {}

    //! Virtual destructor.
    virtual ~CVolumeUndo() {}

    //! Set volume pointer
    void setVolumePtr( tVolume * ptrVolume ) { m_ptrVolume = ptrVolume; } 

    //! Get snapshot - undo/redo phase
    virtual CSnapshot * getSnapshot( CSnapshot *  snapshot );

    //! Create snapshot of the current state
    virtual CSnapshot * getSnapshotVolume( ) 
	    { return NULL==m_ptrVolume.get() ? NULL : new tSnapshot( data::UNDO_SEGMENTATION, m_ptrVolume.get(), this ); }

    //! Create snapshot of plane XY
    virtual CSnapshot * getSnapshotXY( int position ) 
	    { return NULL==m_ptrVolume.get() ? NULL : new tSnapshotXY( data::UNDO_SEGMENTATION,  m_ptrVolume.get(), position, this ); }

    //! Create snapshot of plane XZ
    virtual CSnapshot * getSnapshotXZ( int position ) 
	    { return NULL==m_ptrVolume.get() ? NULL : new tSnapshotXZ( data::UNDO_SEGMENTATION, m_ptrVolume.get(), position, this ); }

    //! Create snapshot of plane YZ
    virtual CSnapshot * getSnapshotYZ( int position ) 
	    { return NULL==m_ptrVolume.get() ? NULL : new tSnapshotYZ( data::UNDO_SEGMENTATION, m_ptrVolume.get(), position, this ); }

    //! Restore state from the snapshot
    virtual void restore( CSnapshot * snapshot );

protected:
    /* // commented because invalidation of storage entry is also done in CUndoManager::processSnapshot()
    //! Invalidate volume
    void invalidate();
    {
        if( m_invalidate )
        {
            APP_STORAGE.invalidate( APP_STORAGE.getEntry(m_storageId, Storage::NO_UPDATE).get() );
        }
    }*/

protected:
    //! Volume pointer
    vpl::base::CSharedPtr< tVolume > m_ptrVolume;
};


#include <data/CVolumeUndo.hpp>

} // namespace data

#endif // CVolumeUndo_H
