///////////////////////////////////////////////////////////////////////////////
// $Id:$
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

#ifndef CStorageEntryDefs_H
#define CStorageEntryDefs_H

#include <VPL/Base/Exception.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Global definitions.

namespace Storage
{

//! Exception thrown on wrong data type.
VPL_DECLARE_EXCEPTION(CTypecastFailed, "Wrong data type")

//! Exception thrown on uninitialized object.
VPL_DECLARE_EXCEPTION(CNullEntry, "Dereferencing a NULL pointer")

//! ID of an unknown storage entry.
const int UNKNOWN = 0;

} // namespace Storage


///////////////////////////////////////////////////////////////////////////////
// Global definitions.

namespace StorageEntry
{

//! Internal flags used to mark storage entries.
enum EEntryState
{
    //! Any parent object has changed its value.
    DIRTY            = 1 << 0,

    //! Entry invalidated after deserialization.
    DESERIALIZED     = 1 << 14,

    //! Entry invalidated on its creation.
    NEWLY_CREATED    = 1 << 15,

    //! Entry invalidated on undo/redo.
    UNDOREDO         = 1 << 16,
};

} // namespace StorageEntry


} // namespace data

#endif // CStorageEntryDefs_H
