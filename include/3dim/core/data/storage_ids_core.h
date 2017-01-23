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

#ifndef storage_ids_core_H_included
#define storage_ids_core_H_included

namespace data
{
    //! Available density data sets.
    enum EDataSet
    {
        PATIENT_DATA = 101,
        CUSTOM_DATA = 151,
        AUX_DATA = 201
    };
}

#define CORE_STORAGE_RTG_DENSITY_WINDOW_ID 9
#define CORE_STORAGE_DENSITY_WINDOW_ID 10
#define CORE_STORAGE_ACTIVE_DATASET_ID 11
#define CORE_STORAGE_SLICE_XY_ID 21
#define CORE_STORAGE_SLICE_XZ_ID 22
#define CORE_STORAGE_SLICE_YZ_ID 23
#define CORE_STORAGE_REGION_DATA_ID 501
#define CORE_STORAGE_REGION_COLORING_ID 502
#define CORE_STORAGE_REGION_DATA_CALCULATOR_ID 503
#define CORE_STORAGE_VOLUME_OF_INTEREST_DATA_ID 504
#define CORE_STORAGE_SCENE_MANIPULATOR_DUMMY_ID 550
#define CORE_STORAGE_DRAWING_OPTIONS_ID 555
#define CORE_STORAGE_MEASUREMENT_OPTIONS_ID 556
#define CORE_STORAGE_ALL_DRAWINGS_ID 557
#define CORE_STORAGE_UNDO_MANAGER_ID 558
#define CORE_STORAGE_SCENE_WIDGETS_PARAMETERS_ID 559
#define CORE_STORAGE_IMAGE_LOADER_INFO_ID 600
#define CORE_STORAGE_APP_SETTINGS_ID 900
#define CORE_STORAGE_SAVED_ENTRIES_ID 2000

#define CORE_STORAGE_IMPORTED_MODEL_CUTSLICE_XY_ID 1520
#define CORE_STORAGE_IMPORTED_MODEL_CUTSLICE_XZ_ID 1540
#define CORE_STORAGE_IMPORTED_MODEL_CUTSLICE_YZ_ID 1560

#define CORE_STORAGE_VOLUME_TRANSFORMATION_ID 1910

#define CORE_STORAGE_NOTE_PLUGIN_ID 2100

//max ID is 2500

// storage_ids_core_H_included
#endif


