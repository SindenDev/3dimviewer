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

#ifndef _3DVSignals_H
#define _3DVSignals_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <3dim/core/data/CColorVector.h>

#include <coremedi/app/Signals.h> 
#include <CPreviewDialogData.h>
#include <data/CModelManager.h>
//#include "cinfodialog.h"

///////////////////////////////////////////////////////////////////////////////
// definition of global signals

VPL_DECLARE_SIGNAL_2(1010, void, int, bool, SigSetModelCutVisibility);
VPL_DECLARE_SIGNAL_1(1011, bool, int, SigGetModelCutVisibility);

VPL_DECLARE_SIGNAL_0(1015, void, SigNumberOfRegionsChanged);

VPL_DECLARE_SIGNAL_4(1050, void, const QString&, const QString&, const QString&, const QColor&, SigShowInfoDialog);

// Preview dialog
VPL_DECLARE_SIGNAL_3(1130, void, const QString&, CPreviewDialogData&, int, SigShowPreviewDialog);
VPL_DECLARE_SIGNAL_3(1131, void, bool, CPreviewDialogData&, int, SigPreviewDialogClosed);

VPL_DECLARE_SIGNAL_0(1132, void, SigManSeg3DChanged);
VPL_DECLARE_SIGNAL_0(1133, void, SigVRChanged);
VPL_DECLARE_SIGNAL_0(1134, void, SigUpdateVR);
VPL_DECLARE_SIGNAL_0(1135, void, SigRemoveMeasurements);
VPL_DECLARE_SIGNAL_1(1136, void, const int, SigDrawingDone);

VPL_DECLARE_SIGNAL_0(1137, void, SigRegionDataLoaded);
VPL_DECLARE_SIGNAL_0(1138, void, SigRemoveAllModels);
VPL_DECLARE_SIGNAL_1(1139, void, bool, SigPluginAuxRegionsLoaded);

VPL_DECLARE_SIGNAL_0(1140, data::CModelManager*, SigGetModelManager);

VPL_DECLARE_SIGNAL_0(1141, void, SigActiveRegionChanged);

VPL_DECLARE_SIGNAL_1(1142, void, int, SigNewRegionSelected);
VPL_DECLARE_SIGNAL_1(1143, void, bool, SigSetActiveRegion3DPreviewVisibility);

VPL_DECLARE_SIGNAL_1(1144, void, bool, SigChangeVRVisibility);
VPL_DECLARE_SIGNAL_0(1145, bool, SigVRVisible);
VPL_DECLARE_SIGNAL_0(1146, bool, SigModelsVisible);
VPL_DECLARE_SIGNAL_1(1147, void, bool, SigChangeModelsVisibility);

VPL_DECLARE_SIGNAL_1(1148, void, int, SigSetRegion3DPreviewInterval);


VPL_DECLARE_SIGNAL_2(1149, bool, bool, int, SigStrokeOnOffWithCheck);
VPL_DECLARE_SIGNAL_2(1150, void, bool, int, SigStrokeOnOff);

VPL_DECLARE_SIGNAL_3(1149, void, int, bool, bool, SigMakeWindowFloating);
VPL_DECLARE_SIGNAL_2(1150, void, int, int, SigShowViewInWindow);

VPL_DECLARE_SIGNAL_1(1151, void, bool, SigSetModelDraggerVisibility);
VPL_DECLARE_SIGNAL_0(1152, bool, SigGetModelDraggerVisibility);
VPL_DECLARE_SIGNAL_2(1153, void, int, bool, SigSetModelDraggerModelId);
VPL_DECLARE_SIGNAL_0(1154, int, SigGetModelDraggerModelId);
VPL_DECLARE_SIGNAL_0(1155, void, SigPythonVolumeLoaded);

VPL_DECLARE_SIGNAL_3(1156, void, int,const std::string&, const std::string&, SigPluginLog);

VPL_DECLARE_SIGNAL_1(1157, void, int, SigSaveScreenshot);

VPL_DECLARE_SIGNAL_0(1158, bool, SigRegion3DPreviewVisible);

VPL_DECLARE_SIGNAL_1(1159, bool, const QString&, SigHasCnnModels);
VPL_DECLARE_SIGNAL_1(1160, bool, const QString&, SigHasLandmarksDefinition);

VPL_DECLARE_SIGNAL_1(1161, void, int, SigOpenPreferences);

#endif // _3DVSignals_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
