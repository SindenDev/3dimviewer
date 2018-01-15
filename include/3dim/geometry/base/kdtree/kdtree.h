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

#ifndef kdtree_h_included
#define kdtree_h_included

#include "CFlannTree.h"
#include <vector>
#include "CMeshIndexing.h"

namespace geometry
{

    /**
        Interface between nanoflann and openmesh model
        */
    class CPointCloudAdaptorOM : public vpl::base::CObject
    {
    public:
        VPL_SHAREDPTR(CPointCloudAdaptorOM);

        typedef vpl::math::CDVector3 tVec;

    public:
        //! Simple constructor
        CPointCloudAdaptorOM();

        //! Constructor
        CPointCloudAdaptorOM(const CMesh &model);

        //! Set object to its initial state
        void clear();

        //! Get vertex handle by id
        CMesh::VertexHandle &getVH(size_t id);

        //! Get vertex coordinates in vpl format
        const tVec &getPointVPL(size_t id){ return (*m_indexing)[id].point; }

        //! Get normal coordinates in VPL format
        const tVec &getNormalVPL(size_t id){ return (*m_indexing)[id].normal; }

        //! Get indexing
        const CMeshIndexing &getIndexing() const { return *m_indexing; }

        //! Get mesh indexing pointer - full access, dangerous!
        CMeshIndexing *getIndexingPtr() { return m_indexing; }

        //! Does it have data?
        bool hasData() const { return m_indexing != 0 && m_indexing->size() > 0; }

    protected:
        //! Model pointer
        CMesh* m_mesh;

        //! Mesh vertices 
        CMeshIndexingPtr m_indexing;
    };

    //! Smart pointer to the point cloud adaptor
    typedef CPointCloudAdaptorOM::tSmartPtr CPointCloudAdaptorOMPtr;

    /**
     *	KD tree for OpenMesh
     */
    class CKDTreeOM : public vpl::base::CObject
    {
    public:
        // Smart pointer definition
        VPL_SHAREDPTR(CKDTreeOM);

        //! Index vector type
        typedef std::vector<size_t> tIndexVec;

        //! Distance vector type
        typedef std::vector<double> tDistanceVec;

        //! Neighbor searches output type
        struct SIndexDistancePairs
        {
            // Constructors
            SIndexDistancePairs(){}
            SIndexDistancePairs(size_t allocated) : indexes(allocated), distances(allocated) {}

            tIndexVec indexes;
            tDistanceVec distances;
        };

        //! Coordinate vector type
        typedef vpl::math::CDVector3 tVec;

    public:
        //! Simple constructor
        CKDTreeOM();

        //! Destructor
        ~CKDTreeOM();

        //! Initializing constructor
        CKDTreeOM(const CMesh &mesh);

        //! Initialize tree
        bool init(const CMesh &mesh);

        //! Has tree data?
        bool hasData() const { return m_tree.hasData() && m_pc != nullptr && m_pc->hasData(); }

        //! Get closest point index and distance
        bool getClosestPoints(const tVec &point, SIndexDistancePairs &result) const;

        //! Get closest point index and distance in radius
        bool getClosestPointsInRadius(const tVec &point, const double radius, SIndexDistancePairs &result, size_t max_results) const;

        //! Get point vertex handle
        const CMesh::VertexHandle getPointVH(size_t idx) const { return m_pc->getVH(idx); }

        //! Get point coordinates in vpl format
        const tVec &getPointVPL(size_t idx) const { return m_pc->getPointVPL(idx); }

        //! Get normal coordinates
        const tVec &getNormalVPL(size_t idx) const { return m_pc->getNormalVPL(idx); }

        //! Get indexing 
        const CMeshIndexing &getIndexing() const { return m_pc->getIndexing(); }

        //! Get mesh indexing pointer - full access, dangerous!
        CMeshIndexing *getIndexingPtr() { return m_pc->getIndexingPtr(); }

        //! Get tree size - number of elements stored
        size_t treeSize() const { return m_tree.size(); }
    
        //! Delete all previous data
        void clear();

    protected:
        //! Mesh adaptor
        CPointCloudAdaptorOMPtr m_pc;

        //! KD tree
        CFlannTree m_tree;
    };

    //! Smart pointer for KD tree 
    typedef CKDTreeOM::tSmartPtr CKDTreeOMPtr;

}

// kdtree_h_included
#endif