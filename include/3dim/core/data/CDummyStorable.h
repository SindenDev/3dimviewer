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

#ifndef CDummyStorable_H
#define CDummyStorable_H

#include "CStorableData.h"


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Dummy data stored in the data storage.

class CDummyStorable : public CStorableData
{
public:
	//! Object creation function.
    static CStorableData *create() { return new CDummyStorable(); }

    //! Constructor
    CDummyStorable() {}

	//! Destructor.
    virtual ~CDummyStorable() {}

    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries& VPL_UNUSED(Changes)) {}

	//! Initializes the object to its default state.
    virtual void init() {}

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }
};


} // namespace data

#endif // CDummyStorable_H

