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

#ifndef CSnapshot_H
#define CSnapshot_H

#include <VPL/Base/Lock.h>
#include <data/ESnapshotType.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
// CLASS CUndoProvider - predeclaration

class CUndoProvider;

///////////////////////////////////////////////////////////////////////////////
//! CLASS CSnapshot - interface for the undo objects.

class CSnapshot : public vpl::base::CObject
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR( CSnapshot );

    //! Constructor
    CSnapshot( int type, CUndoProvider * provider = NULL ) 
	    :  m_time( 0 ), m_type( type ), m_provider( provider ), m_chained( 0 ) {}

    //! Destructor
    virtual ~CSnapshot(){ if( m_chained != 0 ) delete m_chained; }

    //! Each snapshot object must return its data size in bytes
    virtual long getDataSize() = 0;

    //! Get complete data size including chained objects
    long getCompleteSize() 
    { 
       long size = getDataSize();
       
       if( m_chained != 0 ) 
          size += m_chained->getDataSize(); 

       return size;
    }

    //! Get timestamp
    long getTime() { return m_time; }

    //! Set timestamp
    void setTime( long time ) { m_time = time; }

    //! Get provider
    CUndoProvider * getProvider() { return m_provider; }

    //! Set provider
    void setProvider( CUndoProvider * provider ) { m_provider = provider; }

    //! Is this snapshot of the given type?
    bool isType( int type ) { return type == m_type || type == data::UNDO_ALL; }

    //! Add snapshot to create multiple snapshots in one
    CSnapshot * addSnapshot( CSnapshot * snapshot ) 
    { 
       if( m_chained != 0 ) 
       { 
          m_chained->addSnapshot( snapshot );
       }
       else
       {
          m_chained = snapshot;
       }

       return this;
    }

protected:
    //! Timestamp
    long m_time;

    //! Type
    int m_type;

    //! Who has created this snapshot?
    CUndoProvider * m_provider;

    //! Chainded snapshot for multiple snapshots in one
    CSnapshot * m_chained;

    friend class CUndoManager;
}; // class CSnapshot


///////////////////////////////////////////////////////////////////////////////
//! CLASS CUndoProvider - all objects capable of undo operation must
//! contain this interface.

class CUndoProvider
{
public:
    //! Constructor
    CUndoProvider( int InvalidationID = 0, bool invalidate = false) : m_storageId( InvalidationID ), m_invalidate( invalidate ) {}

    //! Virtual destructor
    virtual ~CUndoProvider() {}

    //! Create snapshot of the current state. Parameter snapshot 
    //! is NULL, or old snapshot.
    virtual CSnapshot * getSnapshot( CSnapshot * snapshot ) = 0;

    //! Restore state from the snapshot
    virtual void restore( CSnapshot * snapshot ) = 0;

    //! Set whether should be invalidated
    void setShouldInvalidate(bool bShouldInvalidate) { m_invalidate = bShouldInvalidate; }

    //! Should be invalidated
    bool shouldInvalidate() { return m_invalidate; }

    //! Set storage id
    void setStorageId(int storageID) { m_storageId = storageID; }

    //! Get storage id
    int getStorageId() { return m_storageId; }

protected:
    //! Possible id for invalidation
    int m_storageId;

    //! Should invalidation be used?
    bool m_invalidate;
};


} // namespace data

#endif // CSnapshot_H
