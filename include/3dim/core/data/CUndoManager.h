///////////////////////////////////////////////////////////////////////////////
// $Id: CUndoManager.h 3908 2013-08-13 05:34:45Z wilczak $
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

#ifndef CUndoManager_H
#define CUndoManager_H

#include <VPL/Base/SharedPtr.h>
#include <data/CStorageEntry.h>
//#include <coremedi/app/Signals.h>
#include <data/CSnapshot.h>

#include <deque>

#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! CLASS CUndoManager - manager for the all undo objects.

class CUndoManager : public vpl::base::CObject
{
protected:
    //! Undo items queue type
    typedef std::deque< CSnapshot * > tUndoItemsQueue;

    //! Signal type for undo change
    typedef vpl::mod::CSignal<void> tSigUndoChanged;

    //! Signal for undo/redo change
    tSigUndoChanged m_sigUndoChanged;

public:
    //! Shared pointer type
    VPL_SHAREDPTR( CUndoManager );

    //! Constructor
    CUndoManager( );

    //! init
    void init();

    //! Update
    void update( const CChangedEntries& Changes );

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency( CStorageEntry * VPL_UNUSED(pParent) ) { return true; }

    //! Does object contain relevant data?
    virtual bool hasData(){ return false; }

    //! Destructor
    virtual ~CUndoManager();

    //! Undo operation
    void undo( int type = data::UNDO_ALL );

    //! Redo operation
    void redo();

    //! Insert new undo object
    void insert( CSnapshot * item );

    //! Undo steps available now
    int getUndoSteps() const { return m_undoQueue.size(); }

    //! Redo steps available
    int getRedoSteps() const { return m_redoQueue.size(); }

    //! Can be undo done?
    bool canUndo() const { return getUndoSteps() > 0; }

    //! Can be redo done?
    bool canRedo() const { return getRedoSteps() > 0; }

    //! Clear all
    void clear();

    //! Get mode changed signal.
    tSigUndoChanged& getUndoChangedSignal()
    {
        return m_sigUndoChanged;
    }

    //! Set max memory taken by undo
    void setMaxSize(long long maxSize) { m_maxSize = maxSize; }

protected:
    //! Get new timestamp
    long getTime() { return ++m_timeCounter; }

    //! Clear undo queue
    void clearUndo();

    //! Clear redo queue
    void clearRedo();

    //! clears all but doesn't send notification as clear
    void clearAll();

    //! Process snapshot
    CSnapshot * processSnapshot( CSnapshot * snapshot );

	//! Compute current ammount of allocated memory in the undo and redo queues
	long long calcMemorySize() const;

protected:
    //! Maximal possible occuppied space
    long long m_maxSize;

    //! Timestamp counter
    long m_timeCounter;

    //! Undo objects queue
    tUndoItemsQueue m_undoQueue;

    //! Redo queue
    tUndoItemsQueue m_redoQueue;

}; // class CUndoManager

namespace Storage
{
	//! Undo manager
	DECLARE_OBJECT(UndoManager, CUndoManager, CORE_STORAGE_UNDO_MANAGER_ID);
}

} // namespace data

#endif // CUndoManager_H
