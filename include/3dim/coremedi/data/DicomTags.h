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

#ifndef DicomTags_H
#define DicomTags_H

// VPL
#include <VPL/Base/Setup.h>

// DCMTk
//#include <dcmtk/dcmdata/dctag.h>


///////////////////////////////////////////////////////////////////////////////
// Useful dicom tags

#define	TAG_STUDY_DATE				DcmTag( DcmTagKey( 0x0008, 0x0020 ), DcmVR( EVR_DA ))
#define	TAG_STUDY_DESCRIPTION		DcmTag( DcmTagKey( 0x0008, 0x1030 ), DcmVR( EVR_LO ))
#define	TAG_MODALITY				DcmTag( DcmTagKey( 0x0008, 0x0060 ), DcmVR( EVR_ST ))
#define	TAG_SERIES_DATE				DcmTag( DcmTagKey( 0x0008, 0x0012 ), DcmVR( EVR_DA ))
#define	TAG_SERIES_TIME				DcmTag( DcmTagKey( 0x0008, 0x0013 ), DcmVR( EVR_TM ))
#define	TAG_SERIES_DESCRIPTION		DcmTag( DcmTagKey( 0x0008, 0x103E ), DcmVR( EVR_LO ))
#define	TAG_IMAGE_TYPE				DcmTag( DcmTagKey( 0x0008, 0x0008 ), DcmVR( EVR_ST ))
#define	TAG_MANUFACTURER			DcmTag( DcmTagKey( 0x0008, 0x0070 ), DcmVR( EVR_LO ))
#define	TAG_MODEL_NAME			    DcmTag( DcmTagKey( 0x0008, 0x1090 ), DcmVR( EVR_LO ))

#define	TAG_PATIENTS_NAME			DcmTag( DcmTagKey( 0x0010, 0x0010 ), DcmVR( EVR_PN ))
#define	TAG_PATIENTS_ID				DcmTag( DcmTagKey( 0x0010, 0x0020 ), DcmVR( EVR_LO ))
#define	TAG_PATIENTS_BIRTHDAY		DcmTag( DcmTagKey( 0x0010, 0x0030 ), DcmVR( EVR_DA ))
#define	TAG_PATIENTS_SEX			DcmTag( DcmTagKey( 0x0010, 0x0040 ), DcmVR( EVR_SH ))
#define	TAG_PATIENTS_DESCRIPTION	DcmTag( DcmTagKey( 0x0010, 0x4000 ), DcmVR( EVR_LO ))

#define	TAG_THICKNESS				DcmTag( DcmTagKey( 0x0018, 0x0050 ), DcmVR( EVR_FD ))
#define	TAG_SCAN_OPTIONS			DcmTag( DcmTagKey( 0x0018, 0x0022 ), DcmVR( EVR_ST ))
#define	TAG_SPACING_BETWEEN_SLICES	DcmTag( DcmTagKey( 0x0018, 0x0088 ), DcmVR( EVR_DS ))
#define	TAG_PATIENT_POSITION		DcmTag( DcmTagKey( 0x0018, 0x5100 ), DcmVR( EVR_CS ))

#define	TAG_STUDY_UID				DcmTag( DcmTagKey( 0x0020, 0x000D ), DcmVR( EVR_UI ))
#define	TAG_STUDY_ID				DcmTag( DcmTagKey( 0x0020, 0x0010 ), DcmVR( EVR_LO ))
#define	TAG_SERIES_UID				DcmTag( DcmTagKey( 0x0020, 0x000E ), DcmVR( EVR_UI ))
#define	TAG_SERIES_NUMBER			DcmTag( DcmTagKey( 0x0020, 0x0011 ), DcmVR( EVR_IS ))
#define	TAG_SLICE_NUMBER			DcmTag( DcmTagKey( 0x0020, 0x0013 ), DcmVR( EVR_SH ))
#define	TAG_IMAGE_ORIENTATION		DcmTag( DcmTagKey( 0x0020, 0x0037 ), DcmVR( EVR_FD ))
#define	TAG_IMAGE_POSITION			DcmTag( DcmTagKey( 0x0020, 0x0032 ), DcmVR( EVR_FD ))
#define	TAG_PATIENT_ORIENTATION		DcmTag( DcmTagKey( 0x0020, 0x0020 ), DcmVR( EVR_CS ))

#define	TAG_NUMBER_OF_FRAMES		DcmTag( DcmTagKey( 0x0028, 0x0008 ), DcmVR( EVR_IS ))
#define	TAG_WINDOW_CENTER			DcmTag( DcmTagKey( 0x0028, 0x1050 ), DcmVR( EVR_FD ))
#define	TAG_WINDOW_WIDTH			DcmTag( DcmTagKey( 0x0028, 0x1051 ), DcmVR( EVR_FD ))
#define	TAG_PIXEL_REPRESENTATION	DcmTag( DcmTagKey( 0x0028, 0x0103 ), DcmVR( EVR_IS ))
#define	TAG_SLOPE					DcmTag( DcmTagKey( 0x0028, 0x1053 ), DcmVR( EVR_FD ))
#define	TAG_INTERCEPT				DcmTag( DcmTagKey( 0x0028, 0x1052 ), DcmVR( EVR_FD ))
#define	TAG_PIXEL_SIZE				DcmTag( DcmTagKey( 0x0028, 0x0030 ), DcmVR( EVR_FD ))
#define	TAG_BITS_ALLOCATED	        DcmTag( DcmTagKey( 0x0028, 0x0100 ), DcmVR( EVR_US ))


#endif // DicomTags_H
