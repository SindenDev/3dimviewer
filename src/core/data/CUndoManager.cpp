///////////////////////////////////////////////////////////////////////////////
// $Id: CUndoManager.cpp 3908 2013-08-13 05:34:45Z wilczak $
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

#include <data/CUndoManager.h>
#include <data/CDataStorage.h>

/******************************************************************************
	CLASS CUndoManager
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Constructor
data::CUndoManager::CUndoManager(  )
    : m_currentSize( 0 )
    , m_maxSize( 50000000 )
    , m_timeCounter( 0 )
{
    // Connect to the signals
	VPL_SIGNAL( SigUndoSnapshot ).connect( this, &CUndoManager::insert );
	VPL_SIGNAL( SigUndo ).connect( this, &CUndoManager::undo );
	VPL_SIGNAL( SigRedo ).connect( this, &CUndoManager::redo );
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
data::CUndoManager::~CUndoManager()
{
    // same as clear but without notification
    clearAll();
}

///////////////////////////////////////////////////////////////////////////////
// Init 
void data::CUndoManager::init() 
{
	
}

///////////////////////////////////////////////////////////////////////////////
// Insert new item
void data::CUndoManager::insert(data::CSnapshot *item)
{
	if( item == NULL)
		return;

	// Clear redo queue
	clearRedo();

	// compute ocuppied space
	long newSize = item->getCompleteSize() + m_currentSize;

	// Make place
	while(newSize > m_maxSize && (! m_undoQueue.empty() ) )
	{
		// remove and delete object from the front of the queue
		data::CSnapshot * obj( m_undoQueue.front() );
		newSize -= obj->getCompleteSize();

		delete obj;
		m_undoQueue.pop_front();
	}

   if( newSize > m_maxSize )
   {
      // Queue is empty but still not enough size
      m_sigUndoChanged.invoke();
      return;
   }

	// Set timestamp
	item->setTime( getTime() );

	// insert item
	m_undoQueue.push_back( item );
	m_currentSize = newSize;
    m_sigUndoChanged.invoke();
}

///////////////////////////////////////////////////////////////////////////////
// Clear undo queue
void data::CUndoManager::clearUndo()
{
	tUndoItemsQueue::iterator i;

	for( i = m_undoQueue.begin(); i != m_undoQueue.end(); ++i )
	{
		m_currentSize -= (*i)->getDataSize();
		delete *i;
	}

	m_undoQueue.clear();
}

///////////////////////////////////////////////////////////////////////////////
//  Clear redo queue
void data::CUndoManager::clearRedo()
{
	tUndoItemsQueue::iterator i;

	for( i = m_redoQueue.begin(); i != m_redoQueue.end(); ++i )
	{
		m_currentSize -= (*i)->getDataSize();
		delete *i;
	}

	m_redoQueue.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Clear all

void data::CUndoManager::clearAll()
{
    clearUndo();
    clearRedo();
    m_currentSize = 0;
}

void data::CUndoManager::clear()
{
    clearAll();
    m_sigUndoChanged.invoke();
}

///////////////////////////////////////////////////////////////////////////////
//  Undo operation
void data::CUndoManager::undo( int type )
{
	// find undo object
	tUndoItemsQueue::reverse_iterator i;

	for( i = m_undoQueue.rbegin(); i != m_undoQueue.rend(); ++i )
		if( (*i)->isType( type ) )
			break;

	// if found
	if( i != m_undoQueue.rend() )
	{
//		long time = (*i)->getTime();

		// insert current state to the redo queue
		m_redoQueue.push_front( processSnapshot( *i ) );

		// delete snapshot
		delete *i;

		// reverse iterator points on previous item
		m_undoQueue.erase( (++i).base() );
	}
    m_sigUndoChanged.invoke();
}

///////////////////////////////////////////////////////////////////////////////
//  Redo operation
void data::CUndoManager::redo()
{
	// Is in redo queue any item?
	if( m_redoQueue.empty() ) 
		return;

	// get item
	CSnapshot * item = m_redoQueue.front();

	// insert current state to the redo queue
	m_undoQueue.push_back( processSnapshot( item ) );

   // Delete old snapshot
   delete item;

	// move to the undo queue
	m_redoQueue.pop_front();

    m_sigUndoChanged.invoke();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void data:::::update(const CChangedEntries&Changes)
//
//\brief ! Update. 
//
//\param Changes  The changes. 
////////////////////////////////////////////////////////////////////////////////////////////////////

void data::CUndoManager::update(const data::CChangedEntries&Changes)
{
   if( Changes.checkFlagAny(data::Storage::STORAGE_RESET) )
   {
       clearAll();
   }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void data:::::processSnapshot(CSnapshot *snapshot)
//
//\brief ! Process snapshot. 
//
//\param [in,out] snapshot If non-null, the snapshot. 
////////////////////////////////////////////////////////////////////////////////////////////////////

data::CSnapshot * data::CUndoManager::processSnapshot(CSnapshot *snapshot)
{
	assert( snapshot != 0 );

   CSnapshot * s = snapshot->getProvider()->getSnapshot( snapshot );

   snapshot->getProvider()->restore( snapshot );

   // If snapshot should invalidate some storage entry
   if( snapshot->getProvider()->shouldInvalidate() )
   {
      // Get entry
      data::CPtrWrapper< data::CStorageEntry > entry( APP_STORAGE.getEntry( snapshot->getProvider()->getStorageId() ) );

      if( entry.get() != 0 )
      {
          APP_STORAGE.invalidate( entry.get(), data::StorageEntry::UNDOREDO );
      }
   }


   // Process all linked snapshots
   if( snapshot->m_chained != 0 )
   {
      s->addSnapshot( processSnapshot( snapshot->m_chained ) );
   }

   return s;
}

