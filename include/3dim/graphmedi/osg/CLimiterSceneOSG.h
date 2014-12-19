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

#ifndef CLimiterSceneOSG_H
#define CLimiterSceneOSG_H

#include <osg/CDraggableSlice.h>
#include <osg/CDraggerEventHandler.h>
#include "CCommandEventHandler.h"

#include <data/CVolumeOfInterest.h>

#include <osgViewer/Viewer>
#include <osg/CoordinateSystemNode>


namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! OSG scene containing draggable limiters used to cut volume of interest.
//! - The scene doesn't change any item in the storage.

class CLimiterSceneOSG : public osg::MatrixTransform
{
public:
    //! Destructor
    virtual ~CLimiterSceneOSG();

    //! Sets the scene up
    void setupScene(data::CDensityData& data, bool bInitLimits = true);

    //! Sets texture and texture coordinates of the reference image
    void setTextureAndCoordinates(osg::Texture2D * texture, float x_min, float x_max, float y_min, float y_max);

    //! Initializes the volume of interest.
    void setLimits(const data::SVolumeOfInterest& Limits);

    //! Returns current volume of interest.
    const data::SVolumeOfInterest& getLimits() const { return m_Limits; }

    //! Default positioning of the scene
    void defaultPositioning();

    //! Methods called on limit change.
    void updateMinX(int iValue);
    void updateMaxX(int iValue);
    void updateMinY(int iValue);
    void updateMaxY(int iValue);
    void updateMinZ(int iValue);
    void updateMaxZ(int iValue);

    //! Updates the geometry.
    void updateScaleMatrices();

protected:
    //! OpenGL canvas...
    OSGCanvas *p_Canvas;

    //! Current volume limits
    data::SVolumeOfInterest m_Limits;

    //! Scene event handlers
    osg::ref_ptr<CDraggerEventHandler> p_DraggerEventHandler;

    //! Dummy geometry ().
    osg::ref_ptr<osg::MatrixTransform> p_DummyMatrix;

    //! Dummy geometry (semi-transparent cube)
    osg::ref_ptr<CDummyCubeGeode> p_Dummy;

    //! Reference image scaling.
    osg::ref_ptr<osg::MatrixTransform> p_SliceMatrix;

    //! Reference image.
    osg::ref_ptr<CSliceGeode> p_Slice;

    //! Pointers to each draggable limiter.
    osg::ref_ptr<CDraggableSlice> p_Limit[6];

    //! Connections to signals received on limit change.
    vpl::mod::tSignalConnection m_SetLimitConnection[4];

    //! Real volume size.
    double m_dSizeX, m_dSizeY, m_dSizeZ;

protected:
    //! Constructor
    CLimiterSceneOSG(OSGCanvas *canvas);
};


////////////////////////////////////////////////////////////
//! Provides limiters in X and Y axes.

class CLimiterXY : public CLimiterSceneOSG
{
public:
    //! Constructor.
	CLimiterXY(OSGCanvas *canvas);

    //! Virtual destructor.
	virtual ~CLimiterXY();
};


////////////////////////////////////////////////////////////
//! Provides limiters in X and Z axes.

class CLimiterXZ : public CLimiterSceneOSG
{
public:
    //! Constructor.
	CLimiterXZ(OSGCanvas *canvas);

    //! Virtual destructor.
	virtual ~CLimiterXZ();
};


////////////////////////////////////////////////////////////
//! Provides limiters in Y and Z axes.

class CLimiterYZ : public CLimiterSceneOSG
{
public:
    //! Constructor.
	CLimiterYZ(OSGCanvas *canvas);

    //! Virtual destructor.
	virtual ~CLimiterYZ();
};


} // namespace scene

#endif // CLimiterSceneOSG_H
