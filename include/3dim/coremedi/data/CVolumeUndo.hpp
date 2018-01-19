///////////////////////////////////////////////////////////////////////////////
// $Id: CVolumeUndo.hpp 1289 2011-05-15 00:08:39Z spanel $
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
////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constructor

template < class V >
CVolumeSnapshot< V >::CVolumeSnapshot( int type, tVolume * ptrVolume, CUndoProvider * provider )
    : CSnapshot( type, provider )
{
    assert( ptrVolume != NULL );

    // Compress volume
    //bool rv = m_compress.compress( ptrVolume->getBegin(), ptrVolume->getEnd(), ptrVolume->getXSize() * ptrVolume->getYSize() * ptrVolume->getZSize() / 8 );
    //typename tVolume::tIterator It(*ptrVolume);
    m_compress.compress( *ptrVolume );
}


///////////////////////////////////////////////////////////////////////////////
// Undo function

template < class V >
bool CVolumeSnapshot< V >::restore( tVolume * ptrVolume )
{
    //return m_compress.decompress( ptrVolume->getBegin(), ptrVolume->getEnd() );
    //typename tVolume::tIterator It(*ptrVolume);
    return m_compress.decompress( *ptrVolume );
}

/******************************************************************************
    CLASS CPlaneXYSnapshot - do undo on plane
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Constructor

template < class V >
CPlaneXYSnapshot< V >::CPlaneXYSnapshot(  int type, tVolume * ptrVolume, vpl::tSize position, CUndoProvider * provider )
    : CSnapshot( type, provider )
    , m_position( position )
{
    assert( ptrVolume != NULL );

    // Get plane
    m_plane = new vpl::img::CImage< tVoxel >( ptrVolume->getXSize(), ptrVolume->getYSize() );

    ptrVolume->getPlaneXY( position, *m_plane );
    //bool rv = ptrVolume->getPlaneXY( position, *m_plane );
    //assert( rv );

    // Compress plane
    //rv = m_compress.compress( m_plane->getBegin(), m_plane->getEnd() );
    typename vpl::img::CImage<tVoxel>::tIterator It(*m_plane);
    m_compress.compress( It );

    delete m_plane;
}


///////////////////////////////////////////////////////////////////////////////
// Undo function

template < class V >
bool CPlaneXYSnapshot< V >::restore( tVolume * ptrVolume )
{
    m_plane = new vpl::img::CImage< tVoxel >( ptrVolume->getXSize(), ptrVolume->getYSize() );

    //bool rv = m_compress.decompress( m_plane->getBegin(), m_plane->getEnd() );
    typename vpl::img::CImage<tVoxel>::tIterator It(*m_plane);
    m_compress.decompress( It );
    //assert( rv );

    bool rv = ptrVolume->setPlaneXY( m_position, *m_plane );
    delete m_plane;

    return rv;
}

/******************************************************************************
    CLASS CPlaneXZSnapshot - do undo on plane
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Constructor

template < class V >
CPlaneXZSnapshot< V >::CPlaneXZSnapshot(  int type, tVolume * ptrVolume, vpl::tSize position, CUndoProvider * provider )
    : CSnapshot( type, provider )
    , m_position( position )
{
    assert( ptrVolume != NULL );

    // Get plane
    m_plane = new vpl::img::CImage< tVoxel >( ptrVolume->getXSize(), ptrVolume->getZSize() );

    ptrVolume->getPlaneXZ( position, *m_plane );
    //bool rv = ptrVolume->getPlaneXZ( position, *m_plane );
    //assert( rv );

    // Compress plane
    //rv = m_compress.compress( m_plane->getBegin(), m_plane->getEnd() );
    typename vpl::img::CImage<tVoxel>::tIterator It(*m_plane);
    m_compress.compress( It );

    delete m_plane;
}


///////////////////////////////////////////////////////////////////////////////
// Undo function

template < class V >
bool CPlaneXZSnapshot< V >::restore( tVolume * ptrVolume )
{
    m_plane = new vpl::img::CImage< tVoxel >( ptrVolume->getXSize(), ptrVolume->getZSize() );

    //bool rv = m_compress.decompress( m_plane->getBegin(), m_plane->getEnd() );
    typename vpl::img::CImage<tVoxel>::tIterator It(*m_plane);
    m_compress.decompress( It );

    bool rv = ptrVolume->setPlaneXZ( m_position, *m_plane );
    delete m_plane;
        
    return rv;
}

/******************************************************************************
    CLASS CPlaneYZSnapshot - do undo on plane
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Constructor

template < class V >
CPlaneYZSnapshot< V >::CPlaneYZSnapshot(  int type, tVolume * ptrVolume, vpl::tSize position, CUndoProvider * provider)
    : CSnapshot( type, provider )
    , m_position( position )
{
    assert( ptrVolume != NULL );

    // Get plane
    m_plane = new vpl::img::CImage< tVoxel >( ptrVolume->getYSize(), ptrVolume->getZSize() );

    ptrVolume->getPlaneYZ( position, *m_plane );
    //bool rv = ptrVolume->getPlaneYZ( position, *m_plane );
    //assert( rv );

    // Compress plane
    //rv = m_compress.compress( m_plane->getBegin(), m_plane->getEnd() );
    typename vpl::img::CImage<tVoxel>::tIterator It(*m_plane);
    m_compress.compress( It );

    delete m_plane;
}


///////////////////////////////////////////////////////////////////////////////
// Undo function

template < class V >
bool CPlaneYZSnapshot< V >::restore( tVolume * ptrVolume )
{
    m_plane = new vpl::img::CImage< tVoxel >( ptrVolume->getYSize(), ptrVolume->getZSize() );

    typename vpl::img::CImage<tVoxel>::tIterator It(*m_plane);
    m_compress.decompress( It );

    bool rv = ptrVolume->setPlaneYZ( m_position, *m_plane );

    delete m_plane;
    return rv;
}

/******************************************************************************
    CLASS CVolumeUndo
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Restore state from the snapshot

template < class V >
void CVolumeUndo< V >::restore( CSnapshot * snapshot )
{ 
    tSnapshot * s = dynamic_cast< tSnapshot * >( snapshot );

    if(s)
    {
        s->restore( m_ptrVolume.get() );
        // commented because invalidation of storage entry is also done in CUndoManager::processSnapshot()
        //invalidate();
        return;
    }

    tSnapshotXY * sxy = dynamic_cast< tSnapshotXY * >( snapshot );

    if(sxy)
    {
        sxy->restore( m_ptrVolume.get() );
        // commented because invalidation of storage entry is also done in CUndoManager::processSnapshot()
        //invalidate();
        return;
    }

    tSnapshotXZ * sxz = dynamic_cast< tSnapshotXZ * >( snapshot );  

    if(sxz) 
    { 
        sxz->restore( m_ptrVolume.get() ); 
        // commented because invalidation of storage entry is also done in CUndoManager::processSnapshot()
        //invalidate(); 
        return;
    }

    tSnapshotYZ * syz = dynamic_cast< tSnapshotYZ * >( snapshot );  

    if(syz) 
    { 
        syz->restore( m_ptrVolume.get() ); 
        // commented because invalidation of storage entry is also done in CUndoManager::processSnapshot()
        //invalidate(); 
        return;
    }

}

///////////////////////////////////////////////////////////////////////////////
// Get snapshot - undo/redo phase

template < class V >
CSnapshot * CVolumeUndo< V >::getSnapshot( CSnapshot *  snapshot )
{
    tSnapshot * s = dynamic_cast< tSnapshot * >( snapshot );  
    if(s) 
        return getSnapshotVolume();

    tSnapshotXY * sxy = dynamic_cast< tSnapshotXY * >( snapshot );  
    if(sxy) 
        return getSnapshotXY( sxy->getPosition() );

    tSnapshotXZ * sxz = dynamic_cast< tSnapshotXZ * >( snapshot );  
    if(sxz) 
        return getSnapshotXZ( sxz->getPosition() );

    tSnapshotYZ * syz = dynamic_cast< tSnapshotYZ * >( snapshot );  
    if(syz) 
        return getSnapshotYZ( syz->getPosition() );

    // unknown type
    return NULL;
}

