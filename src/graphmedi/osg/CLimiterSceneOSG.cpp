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

#include <base/Defs.h>
#include <osg/CLimiterSceneOSG.h>
//#include <osgManipulator/Translate2DDragger>
#include <app/Signals.h>
#include <osg/OSGCanvas.h>


//====================================================================================================================
scene::CLimiterSceneOSG::CLimiterSceneOSG(OSGCanvas *canvas)
    : p_Canvas(canvas)
    , m_dSizeX(data::CSlice::INIT_SIZE)
    , m_dSizeY(data::CSlice::INIT_SIZE)
    , m_dSizeZ(data::CSlice::INIT_SIZE)
{
	m_transformedSlice = new osg::MatrixTransform();
	osg::StateSet *transformedSliceStateSet = m_transformedSlice->getOrCreateStateSet();
	this->addChild(m_transformedSlice);

	// create lines
	m_lines = new osg::MatrixTransform;

	// add event handlers
    p_DraggerEventHandler = new CDraggerEventHandler(p_Canvas);
	p_Canvas->addEventHandler(p_DraggerEventHandler.get());

	// create dummy geometry (semi-transparent geometry covering area outside of volume of interest)
	for (int i = 0; i < 4; ++i)
	{
		p_Dummy[i] = new CDummyCubeGeode();
		p_Dummy[i]->setUpCube();
		p_Dummy[i]->setColor(0.5f, 0.5f, 0.0f);
		p_Dummy[i]->setAlpha(0.5);

		p_DummyMatrix[i] = new osg::MatrixTransform();
		p_DummyMatrix[i]->addChild(p_Dummy[i]);
		this->addChild(p_DummyMatrix[i]);
	}
}

//====================================================================================================================
scene::CLimiterSceneOSG::~CLimiterSceneOSG()
{
    this->removeChild(m_lines);
    this->removeChild(m_transformedSlice);
}

void scene::CLimiterSceneOSG::scaleScene(vpl::img::CPoint3i volumeSize, vpl::img::CPoint3d voxelSize)
{
	m_dSizeX = voxelSize.getX() * volumeSize.getX();
	m_dSizeY = voxelSize.getY() * volumeSize.getY();
	m_dSizeZ = voxelSize.getZ() * volumeSize.getZ();

	double dTransX = -0.5 * m_dSizeX;
	double dTransY = -0.5 * m_dSizeY;
	double dTransZ = -0.5 * m_dSizeZ;

	if (p_Limit[0].get())
	{
		p_Limit[0]->scaleScene(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ(), volumeSize.getX(), volumeSize.getY(), volumeSize.getZ());
		p_Limit[1]->scaleScene(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ(), volumeSize.getX(), volumeSize.getY(), volumeSize.getZ());
	}
	if (p_Limit[2].get())
	{
		p_Limit[2]->scaleScene(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ(), volumeSize.getX(), volumeSize.getY(), volumeSize.getZ());
		p_Limit[3]->scaleScene(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ(), volumeSize.getX(), volumeSize.getY(), volumeSize.getZ());
	}
	if (p_Limit[4].get())
	{
		p_Limit[4]->scaleScene(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ(), volumeSize.getX(), volumeSize.getY(), volumeSize.getZ());
		p_Limit[5]->scaleScene(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ(), volumeSize.getX(), volumeSize.getY(), volumeSize.getZ());
	}

	m_lines->setMatrix(osg::Matrix::scale(m_dSizeX, m_dSizeY, m_dSizeZ));

	m_transformedSliceScale->setMatrix(osg::Matrix::scale(m_dSizeX, m_dSizeY, m_dSizeZ));

	osg::Matrix m = osg::Matrix::translate(osg::Vec3d(dTransX, dTransY, dTransZ));
	m_transformedSlice->setMatrix(m);

	updateDummyGeometry();

	osg::Vec3 vertex(volumeSize.x() * voxelSize.x(), volumeSize.y() * voxelSize.y(), volumeSize.z() * voxelSize.z());
	osg::BoundingBox bbox(vertex * (-0.6), vertex * 0.6);
	p_Canvas->centerAndScale(bbox);
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMinX(int iValue)
{
    p_Limit[4]->moveInDepth(iValue);
    m_Limits.m_MinX = iValue;

	updateDummyGeometry();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMaxX(int iValue)
{
    p_Limit[5]->moveInDepth(iValue);
    m_Limits.m_MaxX = iValue;

	updateDummyGeometry();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMinY(int iValue)
{
    p_Limit[2]->moveInDepth(iValue);
    m_Limits.m_MinY = iValue;

	updateDummyGeometry();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMaxY(int iValue)
{
    p_Limit[3]->moveInDepth(iValue);
    m_Limits.m_MaxY = iValue;

	updateDummyGeometry();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMinZ(int iValue)
{
    p_Limit[0]->moveInDepth(iValue);
    m_Limits.m_MinZ = iValue;

	updateDummyGeometry();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::updateMaxZ(int iValue)
{
    p_Limit[1]->moveInDepth(iValue);
    m_Limits.m_MaxZ = iValue;

	updateDummyGeometry();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::setupScene(data::CDensityData& data, bool bInitLimits)
{
	p_Canvas->getView()->getCameraManipulator()->setNode(m_transformedSlice.get());
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
    
	updateDummyGeometry();

    p_Canvas->Refresh( false );
}

//====================================================================================================================
void scene::CLimiterSceneOSG::setTextureAndCoordinates(osg::Texture2D * texture, float x_min, float x_max, float y_min, float y_max)
{
	m_transformedSliceGeode->setTextureAndCoordinates(texture, x_min, x_max, y_min, y_max);

    p_Canvas->Refresh( false );
}

void scene::CLimiterSceneOSG::updateDummyGeometry()
{ }


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

	p_Limit[2]->moveInDepth(m_Limits.m_MinY);
	p_Limit[3]->moveInDepth(m_Limits.m_MaxY);
	p_Limit[4]->moveInDepth(m_Limits.m_MinX);
	p_Limit[5]->moveInDepth(m_Limits.m_MaxX);

	p_Limit[2]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	p_Limit[3]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	p_Limit[4]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	p_Limit[5]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	// create slices
	m_transformedSliceGeode = new scene::CSliceXYGeode();
	m_transformedSliceScale = new osg::MatrixTransform();
	m_transformedSliceScale->addChild(m_transformedSliceGeode);
	m_transformedSlice->addChild(m_transformedSliceScale);

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

void scene::CLimiterXY::scaleScene(vpl::img::CPoint3i volumeSize, vpl::img::CPoint3d voxelSize)
{
	CLimiterSceneOSG::scaleScene(volumeSize, voxelSize);
}

void scene::CLimiterXY::updateDummyGeometry()
{
	double dTransX = -0.5 * m_dSizeX;
	double dTransY = -0.5 * m_dSizeY;
	double dTransZ = -0.5 * m_dSizeZ;

	osg::Matrix matrix[4];
	for (int i = 0; i < 4; ++i)
	{
		matrix[i] = osg::Matrix::identity();
	}

	vpl::img::CPoint3d voxelSize(p_Limit[4]->getVoxelSize(), p_Limit[2]->getVoxelSize(), 1.0);
	vpl::img::CPoint3i volumeSize(p_Limit[4]->getVoxelDepth(), p_Limit[2]->getVoxelDepth(), 1);

	matrix[0] *= osg::Matrix::scale(m_Limits.m_MinX * voxelSize.x(), voxelSize.y() * volumeSize.y(), 1.0);
	matrix[0] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	matrix[1] *= osg::Matrix::scale((volumeSize.x() - m_Limits.m_MaxX) * voxelSize.x(), voxelSize.y() * volumeSize.y(), 1.0);
	matrix[1] *= osg::Matrix::translate(m_Limits.m_MaxX * voxelSize.x(), 0.0, 0.0);
	matrix[1] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	matrix[2] *= osg::Matrix::scale((m_Limits.m_MaxX - m_Limits.m_MinX) * voxelSize.x(), m_Limits.m_MinY * voxelSize.y(), 1.0);
	matrix[2] *= osg::Matrix::translate(m_Limits.m_MinX * voxelSize.x(), 0.0, 0.0);
	matrix[2] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	matrix[3] *= osg::Matrix::scale((m_Limits.m_MaxX - m_Limits.m_MinX) * voxelSize.x(), (volumeSize.y() - m_Limits.m_MaxY) * voxelSize.y(), 1.0);
	matrix[3] *= osg::Matrix::translate(m_Limits.m_MinX * voxelSize.x(), m_Limits.m_MaxY * voxelSize.y(), 0.0);
	matrix[3] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	for (int i = 0; i < 4; ++i)
	{
		p_DummyMatrix[i]->setMatrix(matrix[i]);
	}
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

	p_Limit[0]->moveInDepth(m_Limits.m_MinZ);
	p_Limit[1]->moveInDepth(m_Limits.m_MaxZ);
	p_Limit[4]->moveInDepth(m_Limits.m_MinX);
	p_Limit[5]->moveInDepth(m_Limits.m_MaxX);

	p_Limit[0]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	p_Limit[1]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	p_Limit[4]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	p_Limit[5]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	// create slices
	m_transformedSliceGeode = new scene::CSliceXZGeode();
	m_transformedSliceScale = new osg::MatrixTransform();
	m_transformedSliceScale->addChild(m_transformedSliceGeode);
	m_transformedSlice->addChild(m_transformedSliceScale);

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

void scene::CLimiterXZ::scaleScene(vpl::img::CPoint3i volumeSize, vpl::img::CPoint3d voxelSize)
{
	CLimiterSceneOSG::scaleScene(volumeSize, voxelSize);
}

void scene::CLimiterXZ::updateDummyGeometry()
{
	double dTransX = -0.5 * m_dSizeX;
	double dTransY = -0.5 * m_dSizeY;
	double dTransZ = -0.5 * m_dSizeZ;

	osg::Matrix matrix[4];
	for (int i = 0; i < 4; ++i)
	{
		matrix[i] = osg::Matrix::identity();
	}

	vpl::img::CPoint3d voxelSize(p_Limit[4]->getVoxelSize(), 1.0, p_Limit[0]->getVoxelSize());
	vpl::img::CPoint3i volumeSize(p_Limit[4]->getVoxelDepth(), 1, p_Limit[0]->getVoxelDepth());

	matrix[0] *= osg::Matrix::scale(m_Limits.m_MinX * voxelSize.x(), 1.0, voxelSize.z() * volumeSize.z());
	matrix[0] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	matrix[1] *= osg::Matrix::scale((volumeSize.x() - m_Limits.m_MaxX) * voxelSize.x(), 1.0, voxelSize.z() * volumeSize.z());
	matrix[1] *= osg::Matrix::translate(m_Limits.m_MaxX * voxelSize.x(), 0.0, 0.0);
	matrix[1] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	matrix[2] *= osg::Matrix::scale((m_Limits.m_MaxX - m_Limits.m_MinX) * voxelSize.x(), 1.0, m_Limits.m_MinZ * voxelSize.z());
	matrix[2] *= osg::Matrix::translate(m_Limits.m_MinX * voxelSize.x(), 0.0, 0.0);
	matrix[2] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	matrix[3] *= osg::Matrix::scale((m_Limits.m_MaxX - m_Limits.m_MinX) * voxelSize.x(), 1.0, (volumeSize.z() - m_Limits.m_MaxZ) * voxelSize.z());
	matrix[3] *= osg::Matrix::translate(m_Limits.m_MinX * voxelSize.x(), 0.0, m_Limits.m_MaxZ * voxelSize.z());
	matrix[3] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	for (int i = 0; i < 4; ++i)
	{
		p_DummyMatrix[i]->setMatrix(matrix[i]);
	}
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

	p_Limit[0]->moveInDepth(m_Limits.m_MinZ);
	p_Limit[1]->moveInDepth(m_Limits.m_MaxZ);
	p_Limit[2]->moveInDepth(m_Limits.m_MinY);
	p_Limit[3]->moveInDepth(m_Limits.m_MaxY);

	p_Limit[0]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	p_Limit[1]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	p_Limit[2]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	p_Limit[3]->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	// create slices
	m_transformedSliceGeode = new scene::CSliceYZGeode();
	m_transformedSliceScale = new osg::MatrixTransform();
	m_transformedSliceScale->addChild(m_transformedSliceGeode);
	m_transformedSlice->addChild(m_transformedSliceScale);

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

void scene::CLimiterYZ::scaleScene(vpl::img::CPoint3i volumeSize, vpl::img::CPoint3d voxelSize)
{
	CLimiterSceneOSG::scaleScene(volumeSize, voxelSize);
}

void scene::CLimiterYZ::updateDummyGeometry()
{
	double dTransX = -0.5 * m_dSizeX;
	double dTransY = -0.5 * m_dSizeY;
	double dTransZ = -0.5 * m_dSizeZ;

	osg::Matrix matrix[4];
	for (int i = 0; i < 4; ++i)
	{
		matrix[i] = osg::Matrix::identity();
	}

	vpl::img::CPoint3d voxelSize(1.0, p_Limit[2]->getVoxelSize(), p_Limit[0]->getVoxelSize());
	vpl::img::CPoint3i volumeSize(1, p_Limit[2]->getVoxelDepth(), p_Limit[0]->getVoxelDepth());

	matrix[0] *= osg::Matrix::scale(1.0, m_Limits.m_MinY * voxelSize.y(), voxelSize.z() * volumeSize.z());
	matrix[0] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	matrix[1] *= osg::Matrix::scale(1.0, (volumeSize.y() - m_Limits.m_MaxY) * voxelSize.y(), voxelSize.z() * volumeSize.z());
	matrix[1] *= osg::Matrix::translate(0.0, m_Limits.m_MaxY * voxelSize.y(), 0.0);
	matrix[1] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	matrix[2] *= osg::Matrix::scale(1.0, (m_Limits.m_MaxY - m_Limits.m_MinY) * voxelSize.y(), m_Limits.m_MinZ * voxelSize.z());
	matrix[2] *= osg::Matrix::translate(0.0, m_Limits.m_MinY * voxelSize.y(), 0.0);
	matrix[2] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	matrix[3] *= osg::Matrix::scale(1.0, (m_Limits.m_MaxY - m_Limits.m_MinY) * voxelSize.y(), (volumeSize.z() - m_Limits.m_MaxZ) * voxelSize.z());
	matrix[3] *= osg::Matrix::translate(0.0, m_Limits.m_MinY * voxelSize.y(), m_Limits.m_MaxZ * voxelSize.z());
	matrix[3] *= osg::Matrix::translate(dTransX, dTransY, dTransZ);

	for (int i = 0; i < 4; ++i)
	{
		p_DummyMatrix[i]->setMatrix(matrix[i]);
	}
}
