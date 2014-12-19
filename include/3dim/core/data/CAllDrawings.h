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

#ifndef CAllDrawings_H
#define CAllDrawings_H

#include <VPL/Module/Signal.h>
#include <VPL/Base/SharedPtr.h>

#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Helper object used to clear all drawings on scene manipulation.

class CAllDrawings : public vpl::base::CObject
{
public:
    //! Smart pointer.
	VPL_SHAREDPTR( CAllDrawings );

public:
    //! Default constructor.
    CAllDrawings() {}

    //! Vritual destructor.
    virtual ~CAllDrawings() {}

    //! Regenerates the object state according to any changes in the data storage.
    void update( const CChangedEntries& VPL_UNUSED(Changes) ) {}

	//! Initializes the object to its default state.
    void init() {}

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency( CStorageEntry * VPL_UNUSED(pParent) ) { return true; }

	//! Clear all gizmos...
	void clearGizmos() { VPL_SIGNAL( SigClearAllGizmos ).invoke(); }

    //! Does object contain relevant data?
    virtual bool hasData(){ return false; }
};

namespace Storage
{
	//! All drawings clear dummy object.
	DECLARE_OBJECT(AllDrawings, CAllDrawings, CORE_STORAGE_ALL_DRAWINGS_ID);
}

} // namespace data

#endif // CAllDrawings_H
