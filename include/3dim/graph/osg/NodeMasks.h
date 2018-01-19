///////////////////////////////////////////////////////////////////////////////
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

#ifndef NodeMasks_H_included
#define NodeMasks_H_included

// Used masks for nodes
#define MASK_VISIBLE_OBJECT 1 << 0
#define MASK_GIZMO 1 << 1

#define MASK_DRAGGABLE_SLICE_DRAGGER 1 << 2
#define MASK_OVERLAY_ITEM_DRAGGER 1 << 3
#define MASK_DRAGGER_GEOMETRY 1 << 4
#define IMPLANT_DRAGGER_EH_MASK 1 << 5
#define TOOTH_DRAGGER_EH_MASK 1 << 6
#define OVERLAY_ITEM_EH_MASK 1 << 7
#define MASK_NERVE_DRAGGER 1 << 8
#define MASK_CURVE_DRAGGER 1 << 9
#define MASK_MODEL_DRAGGER 1 << 10
#define MASK_CEPHALO_SHAPE_DRAGGER 1 << 11
#define MASK_CEPHALO_LANDMARK_DRAGGER 1 << 12

// NodeMasks_H_included
#endif

