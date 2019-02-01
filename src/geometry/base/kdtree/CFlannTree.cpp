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

#include <geometry/base/kdtree/CFlannTree.h>

//! Simple constructor (without initialization)
geometry::CFlannTree::CFlannTree() 
{
    initHelperBuffers();
}

//! Initializing constructor - by mesh.
geometry::CFlannTree::CFlannTree(const CMesh &mesh) 
{ 
    initHelperBuffers();
    init(mesh); 
}

//! Initializing constructor - by array of points.
geometry::CFlannTree::CFlannTree(const Vec3Array &points) 
{ 
    initHelperBuffers();
    init(points); 
}

//! Destructor
geometry::CFlannTree::~CFlannTree()
{
    // delete helper buffers
    if (nullptr != m_result_distances)
    {
        delete[] m_result_distances;
        m_result_distances = nullptr;
    }
    if (nullptr != m_result_indices)
    {
        delete[] m_result_indices;
        m_result_indices = nullptr;
    }
    m_result_buffers_count = 0;
}

void geometry::CFlannTree::initHelperBuffers()
{
#ifdef _OPENMP
    const int nThreads = omp_get_max_threads();
#else
    const int nThreads = 1;
#endif
    m_result_distances = new tDoubleVec[nThreads];
    m_result_indices = new tIndexVec[nThreads];
    m_result_buffers_count = nThreads;
}

/*!
 * \fn  bool geometry::CFlannTree::init(const Vec3Array &points)
 *
 * \brief   Initializes this object
 *
 * \param   points  The points used for initialization.
 *
 * \return  True if it succeeds, false if it fails.
 */

bool geometry::CFlannTree::init(const Vec3Array &points)
{
    clear();

    if (points.size() == 0)
        return false;

    tDoubleVec data(points.size() * 3);
    auto itd(data.begin());
    for (auto itp = points.begin(); itp != points.end(); ++itp)
    {
        *itd = (*itp)[0]; ++itd;
        *itd = (*itp)[1]; ++itd;
        *itd = (*itp)[2]; ++itd;
    }

    tFlannDMatrix flann_data(doublesToFlann(data, points.size()));

    m_tree = tIndexTreePtr(new tIndexTree(flann_data, flann::KDTreeSingleIndexParams(10)));
    m_tree->buildIndex();
    m_tree->size();

    return m_tree->veclen() > 0;
}

/*!
 * \fn  int geometry::CFlannTree::nearestSearch(const Vec3 &point, tDoubleVec &distances, tIndexVec &indexes, size_t maximal_points_returned)
 *
 * \brief   Search for the nearest vertices to the given point
 *
 * \param           point                   The point.
 * \param [in,out]  distances               The vector of found vertices distances.
 * \param [in,out]  indexes                 The vector of found vertices indexes.
 * \param           maximal_points_returned The maximal points returned.
 *
 * \return  Number of points found.
 */

int geometry::CFlannTree::nearestSearch(const Vec3 &point, tDoubleVec &distances, tIndexVec &indexes, size_t maximal_points_returned) const
{
    if (m_tree == nullptr || maximal_points_returned == 0)
        return 0;

    VPL_ASSERT(maximal_points_returned > 0);

    tDoubleVec result_distances(maximal_points_returned);
    tIndexVec result_indices(maximal_points_returned);

    // Convert input to the flann format
    std::vector<double> pt(geometryVecToDoubles(point));
    flann::Matrix<double> query_mat(doublesToFlann(pt, 1));
    flann::Matrix<double> dists_mat(doublesToFlann(result_distances, 1));
    flann::Matrix<size_t> indices_mat(indicesToFlann(result_indices, 1));

    // Do search
    int num_found = m_tree->knnSearch(query_mat, indices_mat, dists_mat, maximal_points_returned, flann::SearchParams(128));

    if (num_found == 0)
    {
        distances.clear();
        indexes.clear();
        return 0;
    }

    distances.assign(result_distances.begin(), result_distances.begin() + num_found);
    indexes.assign(result_indices.begin(), result_indices.begin() + num_found);

    return num_found;
}

/*!
 * \fn  int geometry::CFlannTree::radiusSearch(const Vec3 &point, double radius, tDoubleVec &distances, tIndexVec &indexes, size_t maximal_points_returned)
 *
 * \brief   Search for vertices in the given radius from the point.
 *
 * \param           point                   The point.
 * \param           radius                  The radius.
 * \param [in,out]  distances               The vector of found vertices distances.
 * \param [in,out]  indexes                 The vector of found vertices indexes.
 * \param           maximal_points_returned The maximal points returned.
 *
 * \return  Number of points found.
 */

int geometry::CFlannTree::radiusSearch(const Vec3 &point, double radius, tDoubleVec &distances, tIndexVec &indexes, size_t maximal_points_returned) const
{
    if (m_tree == nullptr || maximal_points_returned == 0)
        return 0;

    VPL_ASSERT(maximal_points_returned > 0);
    
    // get shared buffer object
#ifdef _OPENMP
    const int iThread = omp_get_thread_num();
#else
    const int iThread = 0;
#endif    
    tDoubleVec& result_distances = m_result_distances[iThread];
    tIndexVec& result_indices = m_result_indices[iThread];

    // adjust buffer object size
    if (result_distances.size() < maximal_points_returned)
        result_distances.resize(maximal_points_returned);
    if (result_indices.size() < maximal_points_returned)
        result_indices.resize(maximal_points_returned);

    // Convert input to the flann format
    std::vector<double> pt(geometryVecToDoubles(point));
    flann::Matrix<double> query_mat(doublesToFlann(pt, 1));
    flann::Matrix<double> dists_mat(doublesToFlann(result_distances, 1, maximal_points_returned));
    flann::Matrix<size_t> indices_mat(indicesToFlann(result_indices, 1, maximal_points_returned));

    flann::SearchParams params = flann::SearchParams(128);
    params.sorted = true;

    // Do search
    int num_found = m_tree->radiusSearch(query_mat, indices_mat, dists_mat, radius, flann::SearchParams(128));

    if (num_found == 0)
    {
        distances.clear();
        indexes.clear();
        return 0;
    }

    distances.assign(result_distances.begin(), result_distances.begin() + num_found);
    indexes.assign(result_indices.begin(), result_indices.begin() + num_found);

    // reset buffer object to original state as it will be reused
    for (int i = 0; i < num_found; i++)
    {
        result_distances[i] = 0.0;
        result_indices[i] = 0;
    }

    return num_found;
}

/*!
 * \fn  void geometry::CFlannTree::clear()
 *
 * \brief   Clears this object to its blank/initial state
 */

void geometry::CFlannTree::clear()
{
    m_tree = tIndexTreePtr(nullptr);
}

/*!
 * \fn  geometry::Vec3Array geometry::CFlannTree::meshToPoints(const CMesh &mesh) const
 *
 * \brief   Convert mesh vertices to the array of geometry points.
 *
 * \param   mesh    The mesh.
 *
 * \return  A geometry::Vec3Array.
 */

geometry::Vec3Array geometry::CFlannTree::meshToPoints(const CMesh &mesh) const
{
    if (mesh.n_vertices() == 0)
        return Vec3Array();
    
    Vec3Array points;
    points.reserve(mesh.n_vertices());

    for (CMesh::CVIter it = mesh.vertices_sbegin(); it != mesh.vertices_end(); ++it)
        points.push_back(convert3<Vec3, CMesh::Point>(mesh.point(it)));

    return points;
}

/*!
 * \fn  geometry::CFlannTree::tDoubleVec geometry::CFlannTree::geometryVecToDoubles(const Vec3 &point) const
 *
 * \brief   Convert geometry vector to the std vector of doubles
 *
 * \param   point   The point.
 *
 * \return  A geometry::CFlannTree::tDoubleVec.
 */

geometry::CFlannTree::tDoubleVec geometry::CFlannTree::geometryVecToDoubles(const Vec3 &point) const
{
    tDoubleVec data(3);
    data[0] = point[0];
    data[1] = point[1];
    data[2] = point[2];

    return data;
}

/*!
 * \fn  geometry::CFlannTree::tFlannDMatrix geometry::CFlannTree::doublesToFlann(tDoubleVec &data, size_t rows ) const
 *
 * \brief   Use std vector of doubles to initialize Flann matrix
 *
 * \param [in,out]  data    The data.
 * \param           rows    The number of rows.
 *
 * \return  A geometry::CFlannTree::tFlannDMatrix.
 */

geometry::CFlannTree::tFlannDMatrix geometry::CFlannTree::doublesToFlann(tDoubleVec &data, size_t rows) const 
{
    size_t columns(data.size() / rows);

    return tFlannDMatrix(&data[0], rows, columns);
}

/*!
 * \fn  geometry::CFlannTree::tFlannIMatrix geometry::CFlannTree::indicesToFlann(tIndexVec &data, size_t rows ) const
 *
 * \brief   Use std vector of indices to initialize Flann matrix
 *
 * \param [in,out]  data    The data.
 * \param           rows    The number of rows.
 *
 * \return  A geometry::CFlannTree::tFlannIMatrix.
 */

geometry::CFlannTree::tFlannIMatrix geometry::CFlannTree::indicesToFlann(tIndexVec &data, size_t rows) const
{
    size_t columns(data.size() / rows);

    return tFlannIMatrix(&data[0], rows, columns);
}

/*!
* \fn  geometry::CFlannTree::tFlannDMatrix geometry::CFlannTree::doublesToFlann(tDoubleVec &data, size_t rows, size_t cols ) const
*
* \brief   Use std vector of doubles to initialize Flann matrix
*
* \param [in,out]  data    The data.
* \param           rows    The number of rows.
* \param           cols    The number of columns.
*
* \return  A geometry::CFlannTree::tFlannDMatrix.
*/

geometry::CFlannTree::tFlannDMatrix geometry::CFlannTree::doublesToFlann(tDoubleVec &data, size_t rows, size_t cols) const
{
    assert(data.size() >= rows * cols);
    return tFlannDMatrix(&data[0], rows, cols);
}

/*!
* \fn  geometry::CFlannTree::tFlannIMatrix geometry::CFlannTree::indicesToFlann(tIndexVec &data, size_t rows, size_t cols ) const
*
* \brief   Use std vector of indices to initialize Flann matrix
*
* \param [in,out]  data    The data.
* \param           rows    The number of rows.
* \param           cols    The number of columns.
*
* \return  A geometry::CFlannTree::tFlannIMatrix.
*/

geometry::CFlannTree::tFlannIMatrix geometry::CFlannTree::indicesToFlann(tIndexVec &data, size_t rows, size_t cols) const
{
    assert(data.size() >= rows * cols);
    return tFlannIMatrix(&data[0], rows, cols);
}
