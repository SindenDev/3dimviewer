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

#ifndef ESnapshotType_H
#define ESnapshotType_H

namespace data
{

    //! Types of the undo objects
    enum EType
    {
        UNDO_ALL = 0,
        UNDO_SEGMENTATION = 1,
        //UNDO_IMPLANTS = 2,
        UNDO_MODELS = 4,
        //UNDO_NERVES = 8,
        //UNDO_TEETH = 16
    };

} // namespace data

#endif // ESnapshotType_H
