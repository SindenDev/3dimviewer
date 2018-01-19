///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2014-2016 3Dim Laboratory s.r.o.
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


#ifndef CBASEMESH_H
#define CBASEMESH_H

#define _USE_MATH_DEFINES

#ifndef OM_STATIC_BUILD
#define OM_STATIC_BUILD
#endif

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMeshT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#include <geometry/base/OMMesh.h>

namespace geometry
{

	class CBaseMesh : public OMMesh
	{
	public:
		//! Default constructor.
		CBaseMesh();

		//! Copy constructor.
		CBaseMesh(const CBaseMesh &mesh);

		//! Constructor
		CBaseMesh(const geometry::OMMesh &mesh);

		//! Assignment operator
		CBaseMesh &operator=(const CBaseMesh &mesh);

		//! Destructor.
		~CBaseMesh();

		//! calculates boundary count
		virtual int  boundaryCount();

		//! calculates component count
		virtual int  componentCount();

		//! is model closed
		virtual bool isClosed();

		//! is model 2-manifold
		virtual bool isTwoManifold();

		//! Calculate mesh volume
		virtual double meshVolume();

		//! Invert all normals
		virtual void invertNormals();

		//! Attempt to fix open components
		virtual int fixOpenComponents();
	};
} // namspace geometry

#endif // CBASEMESH_H