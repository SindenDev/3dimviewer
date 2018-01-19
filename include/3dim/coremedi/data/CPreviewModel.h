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

#ifndef CPreviewModel_H
#define CPreviewModel_H

#include "CModel.h"
#include <data/storage_ids_core.h>
#include <data/CStorageInterface.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Polygonal surface model shown as a preview in bottom-left corner
//! of 3D and ortho scenes.

class CPreviewModel : public CModel
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CPreviewModel);

public:
	//! Default constructor.
    CPreviewModel();

	//! Destructor.
    ~CPreviewModel();

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& Changes);

	//! Initializes the object to its default state.
    void init();

    //! Does object contain relevant data?
    bool hasData() { return CModel::hasData(); }

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry *) { return true; }
};

namespace Storage
{
	//! Patient preview.
	DECLARE_OBJECT(PreviewModel, CPreviewModel, PATIENT_DATA + 4);
}

} // namespace data

#endif // CPreviewModel_H
