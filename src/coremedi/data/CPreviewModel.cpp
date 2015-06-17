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

#include <data/CPreviewModel.h>
#include <data/CDensityData.h>

#include <alg/CMarchingCubes.h>
#include <alg/CDecimator.h>
#include <alg/CSmoothing.h>
#include <alg/CReduceSmallSubmeshes.h>

#include <VPL/Image/VolumeFiltering.h>
#include <VPL/Image/VolumeFunctions.h>


///////////////////////////////////////////////////////////////////////////////
//

data::CPreviewModel::CPreviewModel()
{
}

///////////////////////////////////////////////////////////////////////////////
//

data::CPreviewModel::~CPreviewModel()
{
}

///////////////////////////////////////////////////////////////////////////////
//

void data::CPreviewModel::init()
{
    CModel::init();

//    this->setColor( 0.95f, 0.95f, 0.0f, 1.0f );
    this->setColor( 1.0f, 1.0f, 1.0f, 1.0f );
}

///////////////////////////////////////////////////////////////////////////////
//

void data::CPreviewModel::update(const data::CChangedEntries& Changes)
{
    // New density data loaded?
    if( !Changes.hasChanged(data::Storage::PatientData::Id)
        || Changes.checkFlagAll(data::CDensityData::DENSITY_MODIFIED) )
    {
        // Nothing to do...
        return;
    }

    // Clear the model
    hide();
    clear();

    //// Generate a new one ////

    // Get patient data from the storage
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );
    if (!spVolume->hasData()) return;

    // Subsampling the volume
    const vpl::tSize XSize = 48;
    const vpl::tSize YSize = 48;
    const vpl::tSize ZSize = 48;

    double XStep = double(spVolume->getXSize() - 1) / (XSize - 1);
    double YStep = double(spVolume->getYSize() - 1) / (YSize - 1);
    double ZStep = double(spVolume->getZSize() - 1) / (ZSize - 1);

    // Create a helper volume
    vpl::img::CDensityVolume SmallVolume(XSize, YSize, ZSize);

    // Normalizing the volume size to a unit cube
    double RealXSize = spVolume->getDX() * XStep * XSize;
    double RealYSize = spVolume->getDY() * YStep * YSize;
    double RealZSize = spVolume->getDZ() * ZStep * ZSize;
    double Norm = 1.0 / vpl::math::getMax(RealXSize, RealYSize, RealZSize);

    SmallVolume.setDX( spVolume->getDX() * XStep * Norm );
    SmallVolume.setDY( spVolume->getDY() * YStep * Norm );
    SmallVolume.setDZ( spVolume->getDZ() * ZStep * Norm );

    // Interpolating the volume data
    vpl::img::CPoint3D Point(0.0, 0.0, 0.0);
    for( vpl::tSize z = 0; z < ZSize; z++, Point.z() += ZStep )
    {
        Point.y() = 0.0;
        for( vpl::tSize y = 0; y < YSize; y++, Point.y() += YStep )
        {
            Point.x() = 0.0;
            for( vpl::tSize x = 0; x < XSize; x++, Point.x() += XStep )
            {
                SmallVolume(x, y, z) = spVolume->interpolate(Point);
            }
        }
    }

    // Release pointer to the patient data
    spVolume.release();

    // Simple thresholding to separate foreground from the background
    if( !vpl::img::singleOtsuThresholding(SmallVolume) )
    {
        return;
    }

    // Create a new mesh
    vpl::base::CScopedPtr<geometry::CMesh> pMesh(new geometry::CMesh);

    // Marching Cubes algorithm creates polygonal surface from the segmented data
    CMarchingCubes mc;
    vpl::img::CSize3d voxelSize = vpl::img::CSize3d(SmallVolume.getDX(), SmallVolume.getDY(), SmallVolume.getDZ());
    CThresholdFunctor<vpl::img::CDensityVolume, vpl::img::tDensityPixel> ThresholdFunc(1, 1, &SmallVolume, voxelSize);

    if( !mc.generateMesh(*pMesh, &ThresholdFunc, false) )
    {
        return;
    }

    // Remove all isolated submeshes and keep only the largest one
    CSmallSubmeshReducer rc;
    rc.reduceNonMax(*pMesh);

    // Smooth the surface
    CSmoothing sm;
//    double p1 = 0.6073;
//    double p2 = 0.1;
//    double p1 = 0.999;
//    double p2 = 0.001;
    //if (!sm.Smooth(*pMesh, 10, p1, p2))
    if (!sm.Smooth(*pMesh, 10))
    {
        return;
    }

    // Center of the mesh
    double offset = 0.5;
    double dX = offset, dY = offset, dZ = offset;
    for (geometry::CMesh::VertexIter vit = pMesh->vertices_sbegin(); vit != pMesh->vertices_end(); ++vit)
	{
        dX += pMesh->point(vit)[0];
        dY += pMesh->point(vit)[1];
        dZ += pMesh->point(vit)[2];
	}
    double dInvCount = 1.0 / (1 + pMesh->n_vertices());
    dX *= dInvCount;
    dY *= dInvCount;
    dZ *= dInvCount;

    // Center the mesh
	for (geometry::CMesh::VertexIter vit = pMesh->vertices_sbegin(); vit != pMesh->vertices_end(); ++vit)
	{
        pMesh->point(vit)[0] += offset - dX;
        pMesh->point(vit)[1] += offset - dY;
        pMesh->point(vit)[2] += offset - dZ;
	}

    // Store the mesh
    setMesh(pMesh.release());
    setVisibility(true);
}
