///////////////////////////////////////////////////////////////////////////////
// $Id: CISEventHandler.cpp
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

#include <drawing/CISEventHandlerEx.h>
#include <coremedi/app/Signals.h>

using namespace osgGA;

/******************************************************************************
    CLASS CISSceneARBEH - event handler for ARB scene
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Constructor
CISSceneARBEH::CISSceneARBEH(OSGCanvas * canvas, scene::CArbitrarySliceScene * scene)
    : CISEventHandler(canvas, scene)
{
    m_handlerType = data::CDrawingOptions::HANDLER_ARB;

    // Add drawables
    AddNode(scene->getSlice());

    // Enable drawing
    SetDraw();
}

osg::Vec3 CISSceneARBEH::RecomputeToVolume(const osg::Vec3 & point)
{
    data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id, data::Storage::NO_UPDATE));
    osg::Vec3 normal(slice->getPlaneNormal());
    osg::Vec3 right(slice->getPlaneRight());
    osg::Vec3 center(slice->getPlaneCenter());

    osg::Vec3 base_X, base_Y, base_Z;

    base_X = normal ^ right;
    base_Y = right;
    base_Z = base_Y ^ base_X;

    base_Z.normalize();
    base_Y.normalize();
    base_X.normalize();

    osg::Matrix transformation(osg::Matrix(
        base_X[0], base_X[1], base_X[2], 0,
        base_Y[0], base_Y[1], base_Y[2], 0,
        base_Z[0], base_Z[1], base_Z[2], 0,
        center[0], center[1], center[2], 1
    ));

    osg::Vec3 p;
    osg::Vec3 pt(-point[0], point[1], 0.0);
    p = pt * transformation;

    data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();
    p.x() = osg::Vec3::value_type(CoordConv.fromRealXd(p.x()));
    p.y() = osg::Vec3::value_type(CoordConv.fromRealYd(p.y()));
    p.z() = osg::Vec3::value_type(CoordConv.fromRealZd(p.z()));

    return p;
}
