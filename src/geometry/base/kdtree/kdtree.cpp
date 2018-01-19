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

#include <geometry/base/kdtree/kdtree.h>
#include <geometry/base/kdtree/kdtree_pc.h>

using namespace geometry;

/**
 * \brief	Constructor.
 *
 * \param [in,out]	model	If non-null, the model.
 */
CPointCloudAdaptorOM::CPointCloudAdaptorOM(const CMesh &model)
	: m_mesh(new CMesh(model))
{
	VPL_ASSERT(m_mesh != nullptr);

	// Create indexing
	m_indexing = new CMeshIndexing(m_mesh);
}

CPointCloudAdaptorOM::CPointCloudAdaptorOM()
{
}



/**
 * \brief   Gets a vertex handle.
 *
 * \param   id  The identifier.
 *
 * \return  The vertex handle.
 */
CMesh::VertexHandle &CPointCloudAdaptorOM::getVH(size_t id)
{
    VPL_ASSERT(id >= 0 && id < m_indexing->size());
    return (*m_indexing)[id].vh;
}

void geometry::CPointCloudAdaptorOM::clear()
{
    if (NULL!=m_indexing.get())
        m_indexing->clear();
}

//=================================================================================================
//	CLASS CKDTreeOM
//=================================================================================================

/**
 * \brief	Default constructor.
 */
CKDTreeOM::CKDTreeOM()
{

}

/**
 * \brief	Constructor.
 *
 * \param [in,out]	mesh	If non-null, the mesh.
 */
CKDTreeOM::CKDTreeOM(const CMesh &mesh)
{
    VPL_ASSERT(mesh.n_vertices() > 0);

    init(mesh);
}

/**
 * \brief	Destructor.
 */
CKDTreeOM::~CKDTreeOM()
{
	clear();
}

/**
 * \brief   Creates this object.
 *
 * \param [in,out]  mesh    If non-null, the mesh.
 *
 * \return  true if it succeeds, false if it fails.
 */
bool CKDTreeOM::init(const CMesh &mesh)
{
    // Clear all previous data
    clear();

    if (mesh.n_vertices() == 0)
        return false;

    // Create mesh adaptor
    m_pc = new CPointCloudAdaptorOM(mesh);

    m_tree.init(mesh);
    return true;
}

/**
 * \brief   Clears this object to its blank/initial state.
 */
void CKDTreeOM::clear()
{
    m_tree.clear();
    m_pc->clear();
}

/**
 * \brief   Gets a closest points.
 *
 * \param   point           The point.
 * \param [in,out]  result  The result.
 *
 * \return  true if it succeeds, false if it fails.
 */
bool CKDTreeOM::getClosestPoints(const tVec &point, SIndexDistancePairs &result) const 
{
    if (!hasData())
        return false;

    // Do search
    return(m_tree.nearestSearch(point, result.distances, result.indexes, std::max<size_t>(result.indexes.size(), 1)) > 0);
}

/**
* \brief   Gets a closest points in a given radius.
*
* \param   point           The point.
* \param   radius          The search radius.
* \param [out]  result     The result.
*
* \return  true if it succeeds, false if it fails.
*/
bool CKDTreeOM::getClosestPointsInRadius(const tVec &point, const double radius, SIndexDistancePairs &result, size_t max_results) const
{
    if (!hasData())
        return false;

    // Do search
    return(m_tree.radiusSearch(point, radius, result.distances, result.indexes, max_results));
}



