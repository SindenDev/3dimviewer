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

#ifndef CSceneManipulatorDummy_H
#define CSceneManipulatorDummy_H

#include <VPL/Base/SharedPtr.h>

#include <data/storage_ids_core.h>
#include <data/CStorageInterface.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Object updated on any view point change in any scene...

class CSceneManipulatorDummy : public vpl::base::CObject
{
public:
    //! Shared pointer
    VPL_SHAREDPTR( CSceneManipulatorDummy );

public:
    //! Default constructor
    CSceneManipulatorDummy(){}

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& VPL_UNUSED(Changes)) {}

    //! Initializes the object to its default state.
    void init() {}

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    //! Does object contain relevant data?
    virtual bool hasData(){ return false; }
};

namespace Storage
{
	//! Scene trackball manipulator dummy object
	DECLARE_OBJECT(SceneManipulatorDummy, CSceneManipulatorDummy, CORE_STORAGE_SCENE_MANIPULATOR_DUMMY_ID);
}

} // namespace data

#endif // CSceneManipulatorDummy_H
