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

#include <CAppExamination.h>
#include <base/Macros.h>
#include <data/CDensityData.h>
#include <data/CRegionData.h>
#include <data/CMultiClassRegionData.h>
#include <data/CCustomData.h>
#include <data/CSavedEntries.h>
#include <data/CRegionDataCalculator.h>
#include <data/Notes.h>
#ifdef ENABLE_PYTHON
#include <qtpython/interpret.h>
#endif
#ifdef ENABLE_DEEPLEARNING
#include <coremedi_viewer/data/CAnnotatedAnatomicalLandmarkGroup.h>
#endif
//=============================================================================
data::CAppExamination::CAppExamination() : CExamination()
{
    // Init the storage
    CAppExamination::init();
}

//=============================================================================
data::CAppExamination::~CAppExamination()
{
}

//=============================================================================
void data::CAppExamination::init()
{
	using namespace data::Storage;

	//STORABLE_FACTORY.registerObject( SavedEntries::Id, SavedEntries::Type::create);
	STORABLE_FACTORY.registerObject( CustomData::Id, CustomData::Type::create );
	STORABLE_FACTORY.registerObject(RegionDataCalculator::Id, RegionDataCalculator::Type::create, CEntryDeps().insert(MultiClassRegionData::Id));
    STORABLE_FACTORY.registerObject(NoteData::Id, NoteData::Type::create);
#ifdef ENABLE_PYTHON
    STORABLE_FACTORY.registerObject(InterpretData::Id, InterpretData::Type::create);
#endif
#ifdef ENABLE_DEEPLEARNING
    STORABLE_FACTORY.registerObject(AnnotatedAnatomicalLandmarkGroup::Id,
        AnnotatedAnatomicalLandmarkGroup::Type::create,
        CEntryDeps().insert(PatientData::Id));
#endif
    // Enforce object creation
	//APP_STORAGE.getEntry(SavedEntries::Id);
	APP_STORAGE.getEntry(CustomData::Id);
	APP_STORAGE.getEntry(RegionDataCalculator::Id);
#ifdef ENABLE_DEEPLEARNING
    APP_STORAGE.getEntry(AnnotatedAnatomicalLandmarkGroup::Id);
#endif

	// Set entries to save
	setSaved();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	! Constructor - set all stored entries. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void data::CAppExamination::setSaved(void)
{
	// Get entries to save
	data::CObjectPtr< CSavedEntries > entries( APP_STORAGE.getEntry( data::Storage::SavedEntries::Id ) );

	if( entries.get() == NULL )
		return;

	// Density window
	entries->addId( data::Storage::DensityWindow::Id );

	// Volume
	entries->addId( data::Storage::PatientData::Id );
	
	// Models
//	entries->addId( data::Storage::BonesModel::Id );
//    entries->addId( data::Storage::ImprintModel::Id );
//	entries->addId( data::Storage::TemplateModel::Id );

    entries->addId( data::Storage::RegionData::Id );

	// Custom data
	entries->addId( data::Storage::CustomData::Id);   

    entries->addId(data::Storage::MultiClassRegionData::Id);
}

