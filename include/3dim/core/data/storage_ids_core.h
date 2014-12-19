//////////////////////////////////////////////////////////////////////////////
// $Id$
//
// This file comes from BSB software and was modified for 
// 
// BlueSkyPlan version 3.x
// Diagnostic and implant planning software for dentistry.
//
// The original DentalViewer legal notice can be found below.
//
// Copyright 2012 Blue Sky Bio, LLC
// All rights reserved 
//
// Changelog:
//    [2012/mm/dd] - ...
//
///////////////////////////////////////////////////////////////////////////////

#ifndef storage_ids_core_H_included
#define storage_ids_core_H_included

namespace data
{
	///////////////////////////////////////////////////////////////////////////////
	// Global definitions.

	//! Available density data sets.
	enum EDataSet
	{
		PATIENT_DATA    = 101,
		AUX_DATA        = 201
	};
}

#define CORE_STORAGE_DENSITY_WINDOW_ID 10
#define CORE_STORAGE_ACTIVE_DATASET_ID 11
#define CORE_STORAGE_SLICE_XY_ID 21
#define CORE_STORAGE_SLICE_XZ_ID 22
#define CORE_STORAGE_SLICE_YZ_ID 23
#define CORE_STORAGE_REGION_DATA_ID 501
#define CORE_STORAGE_REGION_COLORING_ID 502
#define CORE_STORAGE_SCENE_MANIPULATOR_DUMMY_ID 550
#define CORE_STORAGE_DRAWING_OPTIONS_ID 555
#define CORE_STORAGE_MEASUREMENT_OPTIONS_ID 556
#define CORE_STORAGE_ALL_DRAWINGS_ID 557
#define CORE_STORAGE_UNDO_MANAGER_ID 558
#define CORE_STORAGE_SCENE_WIDGETS_PARAMETERS_ID 559
#define CORE_STORAGE_IMAGE_LOADER_INFO_ID 600
#define CORE_STORAGE_APP_SETTINGS_ID 900
#define CORE_STORAGE_SAVED_ENTRIES_ID 2000

// storage_ids_core_H_included
#endif


