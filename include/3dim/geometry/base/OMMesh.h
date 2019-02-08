///////////////////////////////////////////////////////////////////////////////
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


#ifndef OMMESH_H
#define OMMESH_H

#ifndef OM_STATIC_BUILD
#define OM_STATIC_BUILD
#endif

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMeshT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

namespace geometry
{

// define our OM mesh
struct OMTraits : public OpenMesh::DefaultTraits
{
    VertexAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal | OpenMesh::Attributes::Color | OpenMesh::Attributes::TexCoord2D);
    FaceAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal);
    EdgeAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal);
    HalfedgeAttributes(OpenMesh::Attributes::Status);
};

typedef OpenMesh::TriMesh_ArrayKernelT<OMTraits> OMMesh;

}; // namespace data

#endif // OMMESH_H