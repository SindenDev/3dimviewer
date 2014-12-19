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

#include <base/Defs.h>
#include <osg/CLimiterSceneOSG.h>
//#include <osgManipulator/Translate2DDragger>


//====================================================================================================================
scene::CLimiterSceneOSG::CLimiterSceneOSG(OSGCanvas *canvas)
    : p_Canvas(canvas)
    , m_dSizeX(data::CSlice::INIT_SIZE)
    , m_dSizeY(data::CSlice::INIT_SIZE)
    , m_dSizeZ(data::CSlice::INIT_SIZE)
{
	// turn off the lights
	this->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	// add event handlers
	p_DraggerEventHandler = new CDraggerEventHandler(canvas);
	p_Canvas->addEventHandler(p_DraggerEventHandler.get());

    // create volume of interest geometry
    p_Dummy = new scene::CDummyCubeGeode();
    p_Dummy->setUpCube();
    p_Dummy->setColor(1.0f, 1.0f, 0.0f);

    // add geometry
    p_DummyMatrix = new osg::MatrixTransform();
    p_DummyMatrix->addChild(p_Dummy.get());
    this->addChild(p_DummyMatrix.get());

    // add reference image geometry
    p_SliceMatrix = new osg::MatrixTransform();
    this->addChild(p_SliceMatrix.get());
}

//====================================================================================================================
scene::CLimiterSceneOSG::~CLimiterSceneOSG()
{
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMinX(int iValue)
{
    p_Limit[4]->moveInDepth(iValue);
    m_Limits.m_MinX = iValue;

    updateScaleMatrices();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMaxX(int iValue)
{
    p_Limit[5]->moveInDepth(iValue);
    m_Limits.m_MaxX = iValue;

    updateScaleMatrices();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMinY(int iValue)
{
    p_Limit[2]->moveInDepth(iValue);
    m_Limits.m_MinY = iValue;

    updateScaleMatrices();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMaxY(int iValue)
{
    p_Limit[3]->moveInDepth(iValue);
    m_Limits.m_MaxY = iValue;

    updateScaleMatrices();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMinZ(int iValue)
{
    p_Limit[0]->moveInDepth(iValue);
    m_Limits.m_MinZ = iValue;

    updateScaleMatrices();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMaxZ(int iValue)
{
    p_Limit[1]->moveInDepth(iValue);
    m_Limits.m_MaxZ = iValue;

    updateScaleMatrices();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::defaultPositioning()
{
    if( p_Limit[0].get() )
    {
        p_Limit[0]->moveInDepth(m_Limits.m_MinZ);
        p_Limit[1]->moveInDepth(m_Limits.m_MaxZ);
    }
    if( p_Limit[2].get() )
    {
        p_Limit[2]->moveInDepth(m_Limits.m_MinY);
        p_Limit[3]->moveInDepth(m_Limits.m_MaxY);
    }
    if( p_Limit[4].get() )
    {
        p_Limit[4]->moveInDepth(m_Limits.m_MinX);
        p_Limit[5]->moveInDepth(m_Limits.m_MaxX);
    }

    updateScaleMatrices();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::setupScene(data::CDensityData& data, bool bInitLimits)
{
    m_dSizeX = data.getDX() * data.getXSize();
    m_dSizeY = data.getDY() * data.getYSize();
    m_dSizeZ = data.getDZ() * data.getZSize();

    double dTransZ = 0.0;
    if( p_Limit[0].get() )
    {
    	p_Limit[0]->scaleScene(data.getDX(), data.getDY(), data.getDZ(), data.getXSize(), data.getYSize(), data.getZSize());
	    p_Limit[1]->scaleScene(data.getDX(), data.getDY(), data.getDZ(), data.getXSize(), data.getYSize(), data.getZSize());
        dTransZ = -0.5 * m_dSizeZ;
    }

    double dTransY = 0.0;
    if( p_Limit[2].get() )
    {
	    p_Limit[2]->scaleScene(data.getDX(), data.getDY(), data.getDZ(), data.getXSize(), data.getYSize(), data.getZSize());
	    p_Limit[3]->scaleScene(data.getDX(), data.getDY(), data.getDZ(), data.getXSize(), data.getYSize(), data.getZSize());
        dTransY = -0.5 * m_dSizeY;
    }

    double dTransX = 0.0;
    if( p_Limit[4].get() )
    {
	    p_Limit[4]->scaleScene(data.getDX(), data.getDY(), data.getDZ(), data.getXSize(), data.getYSize(), data.getZSize());
	    p_Limit[5]->scaleScene(data.getDX(), data.getDY(), data.getDZ(), data.getXSize(), data.getYSize(), data.getZSize());
        dTransX = -0.5 * m_dSizeX;
    }
    
    if( bInitLimits )
    {
        m_Limits.m_MinX = 0;
        m_Limits.m_MaxX = data.getXSize() - 1;
        m_Limits.m_MinY = 0;
        m_Limits.m_MaxY = data.getYSize() - 1;
        m_Limits.m_MinZ = 0;
        m_Limits.m_MaxZ = data.getZSize() - 1;
        
        defaultPositioning();
    }
    else
    {
        updateScaleMatrices();
    }
    
    osg::Matrix m;
    m.makeScale(osg::Vec3d(m_dSizeX, m_dSizeY, m_dSizeZ));
    
    m = m * osg::Matrix::translate(osg::Vec3d(dTransX, dTransY, dTransZ));
    p_SliceMatrix->setMatrix(m);
    
    p_Canvas->getView()->getCameraManipulator()->setNode(p_SliceMatrix.get());
    p_Canvas->centerAndScale();
    p_Canvas->Refresh(false);
}

//====================================================================================================================
void scene::CLimiterSceneOSG::setLimits(const data::SVolumeOfInterest& Limits)
{
    m_Limits = Limits;

    if( p_Limit[0].get() )
    {
        p_Limit[0]->moveInDepth(m_Limits.m_MinZ);
        p_Limit[1]->moveInDepth(m_Limits.m_MaxZ);
    }
    if( p_Limit[2].get() )
    {
        p_Limit[2]->moveInDepth(m_Limits.m_MinY);
        p_Limit[3]->moveInDepth(m_Limits.m_MaxY);
    }
    if( p_Limit[4].get() )
    {
        p_Limit[4]->moveInDepth(m_Limits.m_MinX);
        p_Limit[5]->moveInDepth(m_Limits.m_MaxX);
    }
    
    updateScaleMatrices();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::setTextureAndCoordinates(osg::Texture2D * texture,
                                                       float x_min, float x_max,
                                                       float y_min, float y_max
                                                       )
{
    p_Slice->setTextureAndCoordinates(texture, x_min, x_max, y_min, y_max);

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateScaleMatrices()
{
    double dScaleZ = 0.9 * m_dSizeZ, dTransZ = -0.45 * m_dSizeZ;
    if( p_Limit[0].get() )
    {
        dScaleZ = (m_Limits.m_MaxZ - m_Limits.m_MinZ) * p_Limit[0]->getVoxelSize();
        dTransZ = m_Limits.m_MinZ * p_Limit[0]->getVoxelSize() - 0.5 * m_dSizeZ;
    }

    double dScaleY = 0.9 * m_dSizeY, dTransY = -0.45 * m_dSizeY;
    if( p_Limit[2].get() )
    {
        dScaleY = (m_Limits.m_MaxY - m_Limits.m_MinY) * p_Limit[2]->getVoxelSize();
        dTransY = m_Limits.m_MinY * p_Limit[2]->getVoxelSize() - 0.5 * m_dSizeY;
    }

    double dScaleX = 0.9 * m_dSizeX, dTransX = -0.45 * m_dSizeX;
    if( p_Limit[4].get() )
    {
        dScaleX = (m_Limits.m_MaxX - m_Limits.m_MinX) * p_Limit[4]->getVoxelSize();
        dTransX = m_Limits.m_MinX * p_Limit[4]->getVoxelSize() - 0.5 * m_dSizeX;
    }

    osg::Matrix m;
    m.makeScale(osg::Vec3d(dScaleX, dScaleY, dScaleZ));
    m = m * osg::Matrix::translate(osg::Vec3d(dTransX, dTransY, dTransZ));
    p_DummyMatrix->setMatrix(m);
}


//====================================================================================================================
//====================================================================================================================
scene::CLimiterXY::CLimiterXY(OSGCanvas *canvas) : CLimiterSceneOSG(canvas)
{	
	// connect signals updating slice positions
    m_SetLimitConnection[0] = VPL_SIGNAL(SigSetMinX).connect( this, &CLimiterSceneOSG::updateMinX );
    m_SetLimitConnection[1] = VPL_SIGNAL(SigSetMaxX).connect( this, &CLimiterSceneOSG::updateMaxX );
    m_SetLimitConnection[2] = VPL_SIGNAL(SigSetMinY).connect( this, &CLimiterSceneOSG::updateMinY );
    m_SetLimitConnection[3] = VPL_SIGNAL(SigSetMaxY).connect( this, &CLimiterSceneOSG::updateMaxY );

	// create draggable limiters
    p_Limit[2] = new CDraggableSliceXZ(canvas,false,data::Storage::UNKNOWN,true);
    p_Limit[3] = new CDraggableSliceXZ(canvas,false,data::Storage::UNKNOWN,true);
    p_Limit[4] = new CDraggableSliceYZ(canvas,false,data::Storage::UNKNOWN,true);
    p_Limit[5] = new CDraggableSliceYZ(canvas,false,data::Storage::UNKNOWN,true);

    // change the frame color
    p_Limit[2]->setFrameColor(1.0f, 1.0f, 0.0f);
    p_Limit[3]->setFrameColor(1.0f, 1.0f, 0.0f);
    p_Limit[4]->setFrameColor(1.0f, 1.0f, 0.0f);
    p_Limit[5]->setFrameColor(1.0f, 1.0f, 0.0f);

	// pass signals to each slice
    p_Limit[2]->setSignal(&(VPL_SIGNAL(SigSetMinY)));
    p_Limit[3]->setSignal(&(VPL_SIGNAL(SigSetMaxY)));
    p_Limit[4]->setSignal(&(VPL_SIGNAL(SigSetMinX)));
    p_Limit[5]->setSignal(&(VPL_SIGNAL(SigSetMaxX)));

	// set voxel depths for scene
    p_Limit[2]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_Limit[3]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_Limit[4]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_Limit[5]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);

	// add slices to scene graph
	this->addChild(p_Limit[2].get());
	this->addChild(p_Limit[3].get());
	this->addChild(p_Limit[4].get());
	this->addChild(p_Limit[5].get());

    // create a reference image
    p_Slice = new scene::CSliceXYGeode();
    p_SliceMatrix->addChild(p_Slice.get());

    // position scene
	defaultPositioning();

    // rotate scene to face orthowindow
	osg::Matrix	m = osg::Matrix::rotate( osg::DegreesToRadians( 180.0 ), osg::Vec3( 0.0, 0.0, 1.0 ) );

	// translate scene further away, so that it is always rendered
	m = m * osg::Matrix::translate( osg::Vec3f( 0.0, 0.0, 1000.0 ) );
	this->setMatrix( m );
}

//====================================================================================================================
scene::CLimiterXY::~CLimiterXY()
{
    VPL_SIGNAL(SigSetMinX).disconnect(m_SetLimitConnection[0]);
    VPL_SIGNAL(SigSetMaxX).disconnect(m_SetLimitConnection[1]);
    VPL_SIGNAL(SigSetMinY).disconnect(m_SetLimitConnection[2]);
    VPL_SIGNAL(SigSetMaxY).disconnect(m_SetLimitConnection[3]);
}

//====================================================================================================================
//====================================================================================================================
scene::CLimiterXZ::CLimiterXZ(OSGCanvas *canvas) : CLimiterSceneOSG(canvas)
{
	// connect signals updating slice positions
    m_SetLimitConnection[0] = VPL_SIGNAL(SigSetMinX).connect( this, &CLimiterSceneOSG::updateMinX );
    m_SetLimitConnection[1] = VPL_SIGNAL(SigSetMaxX).connect( this, &CLimiterSceneOSG::updateMaxX );
    m_SetLimitConnection[2] = VPL_SIGNAL(SigSetMinZ).connect( this, &CLimiterSceneOSG::updateMinZ );
    m_SetLimitConnection[3] = VPL_SIGNAL(SigSetMaxZ).connect( this, &CLimiterSceneOSG::updateMaxZ );

	// create draggable limiters
    p_Limit[0] = new CDraggableSliceXY(canvas,false,data::Storage::UNKNOWN,true);
    p_Limit[1] = new CDraggableSliceXY(canvas,false,data::Storage::UNKNOWN,true);
    p_Limit[4] = new CDraggableSliceYZ(canvas,false,data::Storage::UNKNOWN,true);
    p_Limit[5] = new CDraggableSliceYZ(canvas,false,data::Storage::UNKNOWN,true);

    // change the frame color
    p_Limit[0]->setFrameColor(1.0f, 1.0f, 0.0f);
    p_Limit[1]->setFrameColor(1.0f, 1.0f, 0.0f);
    p_Limit[4]->setFrameColor(1.0f, 1.0f, 0.0f);
    p_Limit[5]->setFrameColor(1.0f, 1.0f, 0.0f);

	// pass signal pointer to each slice
    p_Limit[0]->setSignal(&(VPL_SIGNAL(SigSetMinZ)));
    p_Limit[1]->setSignal(&(VPL_SIGNAL(SigSetMaxZ)));
    p_Limit[4]->setSignal(&(VPL_SIGNAL(SigSetMinX)));
    p_Limit[5]->setSignal(&(VPL_SIGNAL(SigSetMaxX)));

	// set voxel depths for scene
    p_Limit[0]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_Limit[1]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_Limit[4]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_Limit[5]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);

	// add slices to scene graph
	this->addChild(p_Limit[0].get());
	this->addChild(p_Limit[1].get());
	this->addChild(p_Limit[4].get());
	this->addChild(p_Limit[5].get());

    // create a reference image
    p_Slice = new scene::CSliceXZGeode();
    p_SliceMatrix->addChild(p_Slice.get());

    // position scene
	defaultPositioning();

    // rotate scene to face orthowindow
    osg::Matrix	m = osg::Matrix::rotate( osg::DegreesToRadians( 180.0 ), osg::Vec3f( 0.0, 0.0, 1.0 ) );
    m = m * osg::Matrix::rotate( osg::DegreesToRadians( -90.0 ), osg::Vec3f( 1.0, 0.0, 0.0 ) );

	// translate scene further away, so that it is always rendered
	m = m * osg::Matrix::translate( osg::Vec3f( 0.0, 0.0, 1000.0 ) );
	this->setMatrix( m );
}

//====================================================================================================================
scene::CLimiterXZ::~CLimiterXZ()
{
    VPL_SIGNAL(SigSetMinX).disconnect(m_SetLimitConnection[0]);
    VPL_SIGNAL(SigSetMaxX).disconnect(m_SetLimitConnection[1]);
    VPL_SIGNAL(SigSetMinZ).disconnect(m_SetLimitConnection[2]);
    VPL_SIGNAL(SigSetMaxZ).disconnect(m_SetLimitConnection[3]);
}

//====================================================================================================================
//====================================================================================================================
scene::CLimiterYZ::CLimiterYZ(OSGCanvas *canvas) : CLimiterSceneOSG(canvas)
{
	// connect signals updating slice positions
    m_SetLimitConnection[0] = VPL_SIGNAL(SigSetMinY).connect( this, &CLimiterSceneOSG::updateMinY );
    m_SetLimitConnection[1] = VPL_SIGNAL(SigSetMaxY).connect( this, &CLimiterSceneOSG::updateMaxY );
    m_SetLimitConnection[2] = VPL_SIGNAL(SigSetMinZ).connect( this, &CLimiterSceneOSG::updateMinZ );
    m_SetLimitConnection[3] = VPL_SIGNAL(SigSetMaxZ).connect( this, &CLimiterSceneOSG::updateMaxZ );

	// create draggable limiters
    p_Limit[0] = new CDraggableSliceXY(canvas,false,data::Storage::UNKNOWN,true);
    p_Limit[1] = new CDraggableSliceXY(canvas,false,data::Storage::UNKNOWN,true);
    p_Limit[2] = new CDraggableSliceXZ(canvas,false,data::Storage::UNKNOWN,true);
    p_Limit[3] = new CDraggableSliceXZ(canvas,false,data::Storage::UNKNOWN,true);

    // change the frame color
    p_Limit[0]->setFrameColor(1.0f, 1.0f, 0.0f);
    p_Limit[1]->setFrameColor(1.0f, 1.0f, 0.0f);
    p_Limit[2]->setFrameColor(1.0f, 1.0f, 0.0f);
    p_Limit[3]->setFrameColor(1.0f, 1.0f, 0.0f);

	// pass signal pointer to each slice
    p_Limit[0]->setSignal(&(VPL_SIGNAL(SigSetMinZ)));
    p_Limit[1]->setSignal(&(VPL_SIGNAL(SigSetMaxZ)));
    p_Limit[2]->setSignal(&(VPL_SIGNAL(SigSetMinY)));
    p_Limit[3]->setSignal(&(VPL_SIGNAL(SigSetMaxY)));

	// set voxel depths for scene
    p_Limit[0]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_Limit[1]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_Limit[2]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);
    p_Limit[3]->scaleScene(1.0, 1.0, 1.0, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE, data::CSlice::INIT_SIZE);

	// add slices to scene graph
	this->addChild(p_Limit[0].get());
	this->addChild(p_Limit[1].get());
	this->addChild(p_Limit[2].get());
	this->addChild(p_Limit[3].get());

    // create a reference image
    p_Slice = new scene::CSliceYZGeode();
    p_SliceMatrix->addChild(p_Slice.get());

    // position scene
	defaultPositioning();

    // rotate scene to face orthowindow
    osg::Matrix	m = osg::Matrix::rotate( osg::DegreesToRadians( -90.0 ), osg::Vec3f( 0.0, 0.0, 1.0 ) );
    m = m * osg::Matrix::rotate( osg::DegreesToRadians( -90.0 ), osg::Vec3f( 1.0, 0.0, 0.0 ) );

	// translate scene further away, so that it is always rendered
	m = m * osg::Matrix::translate( osg::Vec3f( 0.0, 0.0, 1000.0 ) );
	this->setMatrix( m );
}

//====================================================================================================================
scene::CLimiterYZ::~CLimiterYZ()
{
    VPL_SIGNAL(SigSetMinY).disconnect(m_SetLimitConnection[0]);
    VPL_SIGNAL(SigSetMaxY).disconnect(m_SetLimitConnection[1]);
    VPL_SIGNAL(SigSetMinZ).disconnect(m_SetLimitConnection[2]);
    VPL_SIGNAL(SigSetMaxZ).disconnect(m_SetLimitConnection[3]);
}
