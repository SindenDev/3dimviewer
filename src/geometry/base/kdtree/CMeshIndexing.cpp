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

#include <geometry/base/kdtree/CMeshIndexing.h>

/**
 * \brief   Creates indexing object.
 *
 * \param   mesh    The mesh.
 */

void CMeshIndexing::create(geometry::CMesh *mesh)
{
    assert(mesh != nullptr);
    meshPtr = mesh;

    // Resize vector
    reserve(mesh->n_vertices());

    vhid_map.clear();

    // Request normals
    mesh->request_vertex_normals();
    if(!mesh->has_vertex_normals())
        VPL_ERROR("Mesh has not normals.");

    mesh->update_normals();

    // For all vertices
    geometry::CMesh::VertexIter it(mesh->vertices_sbegin()), itEnd(mesh->vertices_end());
    size_t ind(0);
    for (; it != itEnd; ++it, ++ind) 
    {
        CMeshIndexDataSimple data;

        // Store handle
        data.vh = it.handle();

        // Get and store vertex in VPL format
        const geometry::CMesh::Point &p(mesh->point(it));
        data.point = tVec(p[0], p[1], p[2]);

        // Get and store normal in VPL format
        const geometry::CMesh::Normal &n(mesh->normal(it));
        data.normal = tVec(n[0], n[1], n[2]);
        data.normal.normalize();

        // Store index to map
        vhid_map.insert(std::pair<geometry::CMesh::VertexHandle, size_t>(data.vh, size()));

        // Store data
        push_back(data);
    }
}

/**
 * \brief   Actualizes this object.
 */
void CMeshIndexing::actualize()
{
    assert(meshPtr != 0);

    meshPtr->update_normals();

    // For all vertices
    geometry::CMesh::VertexIter it(meshPtr->vertices_sbegin()), itEnd(meshPtr->vertices_end());
    size_t ind(0);
    for (; it != itEnd; ++it, ++ind) 
    {
        CMeshIndexDataSimple data;

        // Store handle
        data.vh = it.handle();

        // Get and store vertex in VPL format
        const geometry::CMesh::Point &p(meshPtr->point(it));
        data.point = tVec(p[0], p[1], p[2]);

        // Get and store normal in VPL format
        const geometry::CMesh::Normal &n(meshPtr->normal(it));
        data.normal = tVec(n[0], n[1], n[2]);
        data.normal.normalize();

//         // Store index to map
//         vhid_map.insert(std::pair<CMesh::VertexHandle, size_t>(data.vh, size()));

        // Store data
        (*this)[ind] = data;
    }
}
