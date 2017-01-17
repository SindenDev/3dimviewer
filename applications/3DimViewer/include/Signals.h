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
#include "cinfodialog.h"

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


#endif // _3DVSignals_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
