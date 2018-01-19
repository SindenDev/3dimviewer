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

////////////////////////////////////////////////////////////
// Includes

#include <alg/CDecimator.h>

#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <OpenMesh/Tools/Decimater/ModAspectRatioT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalDeviationT.hh>
#include <OpenMesh/Tools/Decimater/ModEdgeLengthT.hh>
#include <OpenMesh/Tools/Decimater/ModIndependentSetsT.hh>

////////////////////////////////////////////////////////////
//

bool CDecimator::Reduce(geometry::CMesh &mesh, int final_vert_number, int final_tri_number)
{
#if (OM_VERSION<0x020300)
    OpenMesh::Decimater::DecimaterT<geometry::CMesh> decimater(mesh);
    OpenMesh::Decimater::ModQuadricT<OpenMesh::Decimater::DecimaterT<geometry::CMesh> >::Handle modHandle0;
    OpenMesh::Decimater::ModAspectRatioT<OpenMesh::Decimater::DecimaterT<geometry::CMesh> >::Handle modHandle1;
    OpenMesh::Decimater::ModNormalDeviationT<OpenMesh::Decimater::DecimaterT<geometry::CMesh> >::Handle modHandle2;
    OpenMesh::Decimater::ModNormalFlippingT<OpenMesh::Decimater::DecimaterT<geometry::CMesh> >::Handle modHandle3;
    CDecimatorProgressModule<OpenMesh::Decimater::DecimaterT<geometry::CMesh> >::Handle progressModuleHandle;

    decimater.add(modHandle0);
    decimater.add(modHandle1);
    decimater.add(modHandle2);
    decimater.add(modHandle3);
    decimater.add(progressModuleHandle);

    decimater.initialize();

	CDecimatorProgressModule<OpenMesh::Decimater::DecimaterT<geometry::CMesh> > & meh = decimater.module(progressModuleHandle);
	meh.setDecimator(this);

    int start_tri_number = mesh.n_faces();
    int target = start_tri_number - final_tri_number;
    m_step = target / 1000;
    m_progress = 0;
    m_success = true;
    this->setProgressMax(1000 + 1);
    this->beginProgress();
    this->progress();

    decimater.decimate_to_faces(final_vert_number, final_tri_number);

    mesh.garbage_collection();

    this->endProgress();
#else
    OpenMesh::Decimater::DecimaterT<geometry::CMesh> decimater(mesh);
    OpenMesh::Decimater::ModQuadricT<geometry::CMesh>::Handle modHandle0;
    OpenMesh::Decimater::ModAspectRatioT<geometry::CMesh>::Handle modHandle1;
    //OpenMesh::Decimater::ModNormalDeviationT<geometry::CMesh >::Handle modHandle2;
    OpenMesh::Decimater::ModNormalFlippingT<geometry::CMesh>::Handle modHandle3;
    //OpenMesh::Decimater::ModIndependentSetsT<geometry::CMesh>::Handle modHandle4;
    CDecimatorProgressModule<geometry::CMesh>::Handle progressModuleHandle;

    decimater.add(modHandle0);
    decimater.add(modHandle1);
    //decimater.add(modHandle2);
    decimater.add(modHandle3);
    //decimater.add(modHandle4);
    decimater.add(progressModuleHandle);	
	
    decimater.initialize();

	CDecimatorProgressModule<geometry::CMesh> & meh = decimater.module(progressModuleHandle);
	meh.setDecimator(this);

    int start_tri_number = mesh.n_faces();
    int target = start_tri_number - final_tri_number;
    m_step = target / 1000;
    m_progress = 0;
    m_success = true;
    this->setProgressMax(1000 + 1);
    this->beginProgress();
    this->progress();

    decimater.decimate_to_faces(final_vert_number, final_tri_number);

    mesh.garbage_collection();

    this->endProgress();
#endif
    return m_success;
}

bool CDecimator::makeProgress(int step)
{
    m_progress += step;
    while (m_progress > m_step)
    {
        m_progress -= m_step;
        if (!progress())
        {
            m_success = false;
            return false;
        }
        if (m_step<=0)
            return true;
    }

    return true;
}
