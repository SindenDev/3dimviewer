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

#ifndef _CMESHINDEXING_H
#define _CMESHINDEXING_H

#include <geometry/base/CMesh.h>

struct CMeshIndexDataSimple
{
    typedef vpl::math::CDVector3 tVec;

    geometry::CMesh::VertexHandle vh;
    tVec point, normal;
};

/** 
 *  Mesh indexing vector
 */
class CMeshIndexing : public vpl::base::CObject, public std::vector<CMeshIndexDataSimple>
{
public:
    VPL_SHAREDPTR(CMeshIndexing);

    //! Vector type
    typedef CMeshIndexDataSimple::tVec tVec;

    //! Vertex handle to index map
    typedef std::map<geometry::CMesh::VertexHandle, size_t> tVHIdMap;

public:
    //! Constructor simple
    CMeshIndexing() {}

    //! Constructor initializing
    CMeshIndexing(geometry::CMesh *mesh) {create(mesh);}

    //! Construct indexing from the mesh
    void create(geometry::CMesh *mesh);

    //! Actualize indexing
    void actualize();

    //! Map from vertex handle to index
    tVHIdMap vhid_map;

    //! Source mesh pointer
    geometry::CMesh* meshPtr;

    //! Compute mesh point again
    tVec meshPoint(size_t id) 
    {
        const geometry::CMesh::Point &p(meshPtr->point((*this)[id].vh));
        return tVec(p[0], p[1], p[2]);
    }
};

//typedef CMeshIndexing::tSmartPtr CMeshIndexingPtr;
typedef vpl::base::CScopedPtr<CMeshIndexing> CMeshIndexingPtr;




#endif