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

#ifndef CoreMediSignals_H
#define CoreMediSignals_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Module/GlobalSignal.h>

#include "data/CDensityWindow.h"
#include "data/CCoordinatesConv.h"
#include <data/CColorVector.h>

#include <osg/Vec3f>

namespace data
{
    class CSnapshot;
    template <typename T> class CColoringFunc;
    typedef CColoringFunc<unsigned char> CColoringFunc4b;
}

namespace geometry
{
    class CMesh;
}

class CMainThreadCallback;

///////////////////////////////////////////////////////////////////////////////
// prototypes
class CVolumeRenderer;

///////////////////////////////////////////////////////////////////////////////
// definition of global signals

// Rendering
VPL_DECLARE_SIGNAL_0(11, void, SigNewFrame);
VPL_DECLARE_SIGNAL_0(12, CVolumeRenderer *, SigGetRenderer);

// Generate unique id 
VPL_DECLARE_SIGNAL_0(15, std::string, SigGenerateUniqueId);

// "Do on main thread" signal
VPL_DECLARE_SIGNAL_1(20, void, CMainThreadCallback *, SigDoOnMainThread);
//VPL_DECLARE_SIGNAL_4(21, void, const QString&, const QString&, const QString&, const QString&, SigLogCustomEvent);

// Orthogonal slices
VPL_DECLARE_SIGNAL_1(102, void, int, SigSetSliceXY);
VPL_DECLARE_SIGNAL_1(103, void, int, SigSetSliceXZ);
VPL_DECLARE_SIGNAL_1(104, void, int, SigSetSliceYZ);
VPL_DECLARE_SIGNAL_1(105, void, int, SigSetSliceModeXY);
VPL_DECLARE_SIGNAL_1(106, void, int, SigSetSliceModeXZ);
VPL_DECLARE_SIGNAL_1(107, void, int, SigSetSliceModeYZ);

// Limiter dialog
VPL_DECLARE_SIGNAL_1(108, void, int, SigSetMinX);
VPL_DECLARE_SIGNAL_1(109, void, int, SigSetMaxX);
VPL_DECLARE_SIGNAL_1(110, void, int, SigSetMinY);
VPL_DECLARE_SIGNAL_1(111, void, int, SigSetMaxY);
VPL_DECLARE_SIGNAL_1(112, void, int, SigSetMinZ);
VPL_DECLARE_SIGNAL_1(113, void, int, SigSetMaxZ);

VPL_DECLARE_SIGNAL_1(114, void, int, SigSetSliceARB);
VPL_DECLARE_SIGNAL_0(115, void, SigOrthoSliceMoved);
VPL_DECLARE_SIGNAL_0(116, double, SigGetSliceARBDoublePos);

// Volume of interest
VPL_DECLARE_SIGNAL_0(120, void, SigShowVolumeOfInterestDialog);
VPL_DECLARE_SIGNAL_1(121, void, bool, SigVolumeOfInterestChanged);
VPL_DECLARE_SIGNAL_1(122, void, bool, SigSetVolumeOfInterestVisibility);
VPL_DECLARE_SIGNAL_0(123, bool, SigGetVolumeOfInterestVisibility);

// Density data
VPL_DECLARE_SIGNAL_1(201, void, int, SigSetActiveDataSet);
VPL_DECLARE_SIGNAL_0(202, int, SigGetActiveDataSet);
VPL_DECLARE_SIGNAL_0(203, int, SigGetActiveConv);
VPL_DECLARE_SIGNAL_0(204, data::CCoordinatesConv, SigGetActiveConvObject);
VPL_DECLARE_SIGNAL_0(205, data::CCoordinatesConv, SigGetPatientConvObject);
VPL_DECLARE_SIGNAL_0(206, data::CCoordinatesConv, SigGetAuxConvObject);

// Density window
VPL_DECLARE_SIGNAL_3(301, void, int, int, int, SigSetDensityWindow);
VPL_DECLARE_SIGNAL_1(302, data::SDensityWindow, int, SigGetDensityWindow);
VPL_DECLARE_SIGNAL_0(303, data::SDensityWindow, SigEstimateDensityWindow);
VPL_DECLARE_SIGNAL_1(304, void, data::CColoringFunc4b*, SigSetColoring);
VPL_DECLARE_SIGNAL_0(305, int, SigGetColoringType);
VPL_DECLARE_SIGNAL_1(306, void, bool, SigSetContoursVisibility);
VPL_DECLARE_SIGNAL_0(307, bool, SigGetContoursVisibility);

// Surface models
VPL_DECLARE_SIGNAL_2(401, void, int, geometry::CMesh *, SigSetModel);
VPL_DECLARE_SIGNAL_2(402, void, int, const data::CColor4f&, SigSetModelColor);
VPL_DECLARE_SIGNAL_2(403, void, int, bool, SigSetModelVisibility);
VPL_DECLARE_SIGNAL_1(404, data::CColor4f, int, SigGetModelColor);
VPL_DECLARE_SIGNAL_1(405, bool, int, SigGetModelVisibility);
VPL_DECLARE_SIGNAL_2(406, void, int, int, SigTransparencyNeededChange);
VPL_DECLARE_SIGNAL_1(407, void, int, SigSelectModel);
VPL_DECLARE_SIGNAL_0(408, int, SigGetSelectedModelId);
VPL_DECLARE_SIGNAL_1(409, bool, int, SigSaveModel);
VPL_DECLARE_SIGNAL_0(410, int, GetSelectedModel);
VPL_DECLARE_SIGNAL_1(411, void, int, SigRemoveModel);
VPL_DECLARE_SIGNAL_1(412, void, int, SigModelRemoved);
VPL_DECLARE_SIGNAL_2(413, bool, int, bool, SigSaveModelExt);

// transparency flags for SigTransparencyNeededChange
#define TRANSPARENCY_NEEDED_MODELS         1
#define TRANSPARENCY_NEEDED_GUIDE_WINDOWS  2

// World position
VPL_DECLARE_SIGNAL_0(501, osg::Vec3f, SigGetXYWorld);
VPL_DECLARE_SIGNAL_0(502, osg::Vec3f, SigGetXZWorld);
VPL_DECLARE_SIGNAL_0(503, osg::Vec3f, SigGetYZWorld);

// Set planes on/off
VPL_DECLARE_SIGNAL_1(600, void, bool, SigSetPlaneARBVisibility);
VPL_DECLARE_SIGNAL_1(601, void, bool, SigSetPlaneXYVisibility);
VPL_DECLARE_SIGNAL_1(602, void, bool, SigSetPlaneXZVisibility);
VPL_DECLARE_SIGNAL_1(603, void, bool, SigSetPlaneYZVisibility);
VPL_DECLARE_SIGNAL_0(604, bool, SigGetPlaneXYVisibility);
VPL_DECLARE_SIGNAL_0(605, bool, SigGetPlaneXZVisibility);
VPL_DECLARE_SIGNAL_0(606, bool, SigGetPlaneYZVisibility);
VPL_DECLARE_SIGNAL_1(607, void, bool, SigSetNormalPlaneVisibility );
VPL_DECLARE_SIGNAL_0(608, bool, SigGetNormalPlaneVisibility);
VPL_DECLARE_SIGNAL_0(609, bool, SigGetPlaneARBVisibility);

// Set planes on/off in 3D view
VPL_DECLARE_SIGNAL_1(611, void, bool, SigSetPlaneXYVisibility3D);
VPL_DECLARE_SIGNAL_1(612, void, bool, SigSetPlaneXZVisibility3D);
VPL_DECLARE_SIGNAL_1(613, void, bool, SigSetPlaneYZVisibility3D);
VPL_DECLARE_SIGNAL_0(614, bool, SigGetPlaneXYVisibility3D);
VPL_DECLARE_SIGNAL_0(615, bool, SigGetPlaneXZVisibility3D);
VPL_DECLARE_SIGNAL_0(616, bool, SigGetPlaneYZVisibility3D);
VPL_DECLARE_SIGNAL_1(617, void, bool, SigSetNormalPlaneVisibility3D );
VPL_DECLARE_SIGNAL_0(618, bool, SigGetNormalPlaneVisibility3D );
VPL_DECLARE_SIGNAL_1(619, void, bool, SigSetGridVisibility3D);
VPL_DECLARE_SIGNAL_0(620, bool, SigGetGridVisibility3D);

// Clear all gizmos signal
VPL_DECLARE_SIGNAL_0(701, void, SigClearAllGizmos);

// Landmark annotation signals (used in Deep Learning Plugin)
// Clear drawn landmark annotation specifying its id
VPL_DECLARE_SIGNAL_1(711, void, std::string, SigClearLandmarkAnnotationDrawable);
// Clear all drawn landmark annotations 
VPL_DECLARE_SIGNAL_0(712, void, SigClearAllLandmarkAnnotationDrawables);
// Create landmark annotation drawable with the specific id and given volume position
VPL_DECLARE_SIGNAL_2(713, void, std::string, osg::Vec3, SigCreateLandmarkAnnotationDrawable);
// Set landmark annotation drawables visibility state on/off
VPL_DECLARE_SIGNAL_1(714, void, bool, SigSetLandmarkAnnotationDrawablesVisibility);

// Undo signals
VPL_DECLARE_SIGNAL_1(750, void, data::CSnapshot *, SigUndoSnapshot);

//These do not cause undo/redo. They are invoked just before undo manager performs that action and restore is called.
VPL_DECLARE_SIGNAL_1(751, void, int, SigUndo);
VPL_DECLARE_SIGNAL_0(752, void, SigRedo);
VPL_DECLARE_SIGNAL_0(753, void, SigClearUndoRedoQueues);

// Segmented data coloring
VPL_DECLARE_SIGNAL_1(802, void, bool, SigEnableRegionColoring);
VPL_DECLARE_SIGNAL_2(803, void, int, const data::CColor4b&, SigSetRegionColor);
VPL_DECLARE_SIGNAL_1(804, data::CColor4b, int, SigGetRegionColor);
VPL_DECLARE_SIGNAL_0(805, bool, SigIsRegionColoringEnabled);

// Segmented multi-class data coloring
VPL_DECLARE_SIGNAL_1(806, void, bool, SigEnableMultiClassRegionColoring);
VPL_DECLARE_SIGNAL_2(807, void, int, const data::CColor4b&, SigSetMultiClassRegionColor);
VPL_DECLARE_SIGNAL_1(808, data::CColor4b, int, SigGetMultiClassRegionColor);
VPL_DECLARE_SIGNAL_0(809, bool, SigIsMultiClassRegionColoringEnabled);

// Volume rendering
VPL_DECLARE_SIGNAL_1(850, void, bool, SigVREnabledChange);
VPL_DECLARE_SIGNAL_1(851, void, int, SigVRModeChange);
VPL_DECLARE_SIGNAL_1(852, void, int, SigVRLutChange);
VPL_DECLARE_SIGNAL_1(853, void, int, SigVRQualityChange);
VPL_DECLARE_SIGNAL_2(854, void, float, float, SigVRDataRemapChange);
VPL_DECLARE_SIGNAL_1(855, void, bool, SigSurfaceMaskUsedChange);
VPL_DECLARE_SIGNAL_1(860, void, bool, SigLowResRender);

VPL_DECLARE_SIGNAL_1(880, void, osg::Vec3Array *, SigSendLineSceneCoordinates);

// Notes plugin signal for PSVRenderer to set transform matrix to given value
VPL_DECLARE_SIGNAL_2(890, void, osg::Matrix&, double, SigNewTransformMatrixFromNote);
VPL_DECLARE_SIGNAL_0(891, void, SigRefreshNotesVisibility);

// Surface definitions editing
VPL_DECLARE_SIGNAL_1(900, void, int, SigSurfaceSelected);

// Orpedigs signal for enabling multiselection in the step for selecting examined bones
VPL_DECLARE_SIGNAL_1(901, void, bool, SigModelsMultiSelectionEnabled);

//Signal for xlab denture panel to open add tooth dialog..
VPL_DECLARE_SIGNAL_0(902, void, SigShowXlabAddToothDialog);

VPL_DECLARE_SIGNAL_0(903, void, SigXlabAddingTeethFinished);

VPL_DECLARE_SIGNAL_1(904, void, int, SigScrollPerformed);

VPL_DECLARE_SIGNAL_0(905, void, SigUndoOnColoringPerformed);

VPL_DECLARE_SIGNAL_1(906, void, bool, SigDrawingInProgress);

VPL_DECLARE_SIGNAL_1(907, void, int, SigXlabWizardFinished);

VPL_DECLARE_SIGNAL_2(908, void, int, int, SigXlabWizardStep);

VPL_DECLARE_SIGNAL_2(1310, void, int, bool, SigAlignmentDraggerMove);

//! Updates color visualization of collisions between models
VPL_DECLARE_SIGNAL_2(1524, void, int, int, SigDragerMoveForCollisions);

#endif // CoreMediSignals_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
