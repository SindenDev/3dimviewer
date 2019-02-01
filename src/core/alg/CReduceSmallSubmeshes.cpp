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

#include <alg/CReduceSmallSubmeshes.h>
#include <deque>

bool CSmallSubmeshReducer::reduce(geometry::CMesh &mesh, int triangle_threshold)
{
    // queue of triangles to visit
    std::deque<geometry::CMesh::FaceHandle> toVisit;

    // submesh index map with number of submeshes
    std::map<int, int> counts;

    // last used submesh index
    int lastused = 0;

    // add property and clear it
    OpenMesh::FPropHandleT<int> fProp_submeshIndex;
    //if (!mesh.get_property_handle(fProp_submeshIndex, "submeshIndex"))
        mesh.add_property<int>(fProp_submeshIndex, "submeshIndex"); // local property

    for (geometry::CMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        mesh.property<int>(fProp_submeshIndex, fit) = -1;
    }

    // get first triangle of the mesh
    geometry::CMesh::FaceIter faceIt = mesh.faces_begin();
    while (faceIt != mesh.faces_end())
    {
        geometry::CMesh::FaceHandle face = faceIt.handle();

        // start new submesh index
        counts[lastused] = 0;
        mesh.property<int>(fProp_submeshIndex, face) = lastused;

        // place the triangle into the visiting queue
        toVisit.push_back(face);

        // go through all triangles to visit ( that lie on the same submesh )
        while (toVisit.size() > 0)
        {
            geometry::CMesh::FaceHandle curr = toVisit.front();
            toVisit.pop_front();

            // get all neighbourhoods and place them in the queue
            for (geometry::CMesh::FaceFaceIter ffit = mesh.ff_begin(curr); ffit != mesh.ff_end(curr); ++ffit)
            {
                geometry::CMesh::FaceHandle neighbour = ffit.handle();
                if (mesh.property<int>(fProp_submeshIndex, neighbour) == -1)
                {
                    mesh.property<int>(fProp_submeshIndex, neighbour) = lastused;
                    toVisit.push_back(neighbour);
                    counts[lastused]++;
                }
            }
        }

        // end of continuous area, find next one
        while ((faceIt != mesh.faces_end()) && (mesh.property<int>(fProp_submeshIndex, faceIt.handle()) != -1))
        {
            ++faceIt;
        }

        lastused++;
    }

    // mark groups which are to be erased
    std::set<unsigned> toErase;
    for (std::map<int, int>::iterator it = counts.begin(); it != counts.end(); ++it)
    {
        if (it->second < triangle_threshold)
        {
            toErase.insert(it->first);
        }
    }

    // erase marked tris
    for (geometry::CMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        if (toErase.find(mesh.property<int>(fProp_submeshIndex, fit.handle())) != toErase.end())
        {
            mesh.delete_face(fit.handle());
        }
    }

    // clean    
    mesh.remove_property(fProp_submeshIndex);
    mesh.garbage_collection();
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//

bool CSmallSubmeshReducer::reduceNonMax(geometry::CMesh &mesh)
{
    // queue of triangles to visit
    std::deque<geometry::CMesh::FaceHandle> toVisit;

    // submesh index map with number of submeshes
    std::map<int, int> counts;

    // last used submesh index
    int lastused = 0;

    int maxGroupIndex = -1;

    // add property and clear it
    OpenMesh::FPropHandleT<int> fProp_submeshIndex;
    //if (!mesh.get_property_handle(fProp_submeshIndex, "submeshIndex"))
        mesh.add_property<int>(fProp_submeshIndex, "submeshIndex"); // local property

    for (geometry::CMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        mesh.property<int>(fProp_submeshIndex, fit) = -1;
    }

    // get first triangle of the mesh
    geometry::CMesh::FaceIter faceIt = mesh.faces_begin();
    while (faceIt != mesh.faces_end())
    {
        geometry::CMesh::FaceHandle face = faceIt.handle();

        // start new submesh index
        counts[lastused] = 0;
        mesh.property<int>(fProp_submeshIndex, face) = lastused;

        // place the triangle into the visiting queue
        toVisit.push_back(face);

        // go through all triangles to visit ( that lie on the same submesh )
        while (toVisit.size() > 0)
        {
            geometry::CMesh::FaceHandle curr = toVisit.front();
            toVisit.pop_front();

            // get all neighbourhoods and place them in the queue
            for (geometry::CMesh::FaceFaceIter ffit = mesh.ff_begin(curr); ffit != mesh.ff_end(curr); ++ffit)
            {
                geometry::CMesh::FaceHandle neighbour = ffit.handle();
                if (mesh.property<int>(fProp_submeshIndex, neighbour) == -1)
                {
                    mesh.property<int>(fProp_submeshIndex, neighbour) = lastused;
                    toVisit.push_back(neighbour);
                    counts[lastused]++;
                }
            }
        }

        if ((maxGroupIndex == -1) || (counts[lastused] > counts[maxGroupIndex]))
        {
            maxGroupIndex = lastused;
        }

        // end of continuous area, find next one
        while ((faceIt != mesh.faces_end()) && (mesh.property<int>(fProp_submeshIndex, faceIt.handle()) != -1))
        {
            ++faceIt;
        }

        lastused++;
    }

    // erase tris of nonmax groups
    for (geometry::CMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        if (mesh.property<int>(fProp_submeshIndex, fit.handle()) != maxGroupIndex)
        {
            mesh.delete_face(fit.handle());
        }
    }

    // clean    
    mesh.remove_property(fProp_submeshIndex);
    mesh.garbage_collection();
    
    return true;
}
