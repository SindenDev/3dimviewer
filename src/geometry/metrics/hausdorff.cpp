///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2018 TESCAN 3DIM, s.r.o.
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

#include <geometry/metrics/hausdorff.h>
#include <cassert>
#include <random>
#include <geometry/base/kdtree/kdtree.h>
#include <geometry/base/functions.h>
#include <osg/Point>


namespace settings
{
//! Default searchDistance = bboxDiagonalLength / searchDistanceDivider
double searchDistanceDivider = 100;
double searchStopEpsilon = 1e-10;
}
namespace geometry
{
HausdorffDistance::HausdorffDistance(geometry::CMesh& meshA, geometry::CMesh& meshB, const size_t sampleNumber)
    : m_unifGenerate01(0, 1)
{
    //! Is meshes triangular models?
    assert(meshA.IsTriMesh && meshB.IsTriMesh);

    //! Save model meshA, generate KDTree.
    m_meshA = meshA;
    m_meshB = meshB;

    // Update mesh normals if not done yet
    m_meshB.request_face_normals();
    m_meshB.update_normals();

    // Try to get octree
    m_meshB.updateOctree();
    m_octree = m_meshB.getOctree();

    m_maxSamples = sampleNumber;

    //! Initialize default values
    m_min = std::numeric_limits<double>::max();
    m_max = 0;
    m_mean = 0;
    m_rms = 0;
    m_samplesCount = 0;

    m_isVertexSample = true;
    m_isFaceSample = true;

    m_searchDistance = computeBBoxDiagonalLength(m_meshB) / settings::searchDistanceDivider;

    const uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
    m_mt.seed(ss);
}

void HausdorffDistance::enableFaceSampling(const bool isEnabled)
{
    m_isFaceSample = isEnabled;
}
bool HausdorffDistance::isEnabledFaceSampling() const
{
    return m_isFaceSample;
}

void HausdorffDistance::enableVertexSampling(const bool isEnabled)
{
    m_isVertexSample = isEnabled;
}

bool HausdorffDistance::isEnabledVertexSampling() const
{
    return m_isVertexSample;
}


void HausdorffDistance::setSampleType(const bool isVertexSampling, const bool isFaceSampling)
{
    m_isVertexSample = isVertexSampling;
    m_isFaceSample = isFaceSampling;
}

void HausdorffDistance::setDistanceSearch(const double distance)
{
    m_searchDistance = distance;
}

bool HausdorffDistance::compute()
{
    //! Start vertex sampling, if is enabled.
    if (m_isVertexSample && !vertexCompute())
    {
        return false;
    }

    //! Start face sampling, if is enabled.
    if (m_isFaceSample && !faceCompute())
    {
        return false;
    }
    return true;
}
 
double HausdorffDistance::getMinDistance() const
{
    if (m_min < 1.0e-5)
    {
        return 0;
    }
    return m_min;
}

double HausdorffDistance::getMaxDistance() const
{
    return m_max;
}

double HausdorffDistance::getMeanDistance() const
{
    return m_mean / m_samplesCount;
}

double HausdorffDistance::getRmsDistance() const
{
    return sqrt(m_rms / m_samplesCount);
}

double HausdorffDistance::computeBBoxDiagonalLength(geometry::CMesh& mesh) const
{
    geometry::CMesh::Point min, max;
    mesh.calc_bounding_box(min, max);

    return sqrt(
        ((min[0] - max[0]) * (min[0] - max[0])) +
        ((min[1] - max[1]) * (min[1] - max[1])) +
        ((min[2] - max[2]) * (min[2] - max[2])));
}

void HausdorffDistance::addFaceSample(const CMesh::Point& point)
{
    //! Find closest point on meshB.
    const osg::BoundingSphere bSphere(geometry::convert3<osg::Vec3, CMesh::Point>(point), m_searchDistance);
    std::vector<CMeshOctreeNode*> nodes = m_octree->getIntersectedNodes(bSphere);

    double closestDistance(std::numeric_limits<double>::max());

    // For all returned nodes
    std::vector<geometry::CMeshOctreeNode *>::const_reverse_iterator itn = nodes.rbegin();
    const std::vector<geometry::CMeshOctreeNode *>::const_reverse_iterator itnEnd = nodes.rend();

    for (; itn != itnEnd; ++itn)
    {
        // For all node faces
        std::vector<CMesh::FaceHandle>::const_iterator itfh((*itn)->faces.begin()), itfhEnd((*itn)->faces.end());
        for (; itfh != itfhEnd; ++itfh)
        {
            //! Current face is not valid skip it.
            if (!itfh->is_valid())
            {
                continue;
            }

            //! Get vertex of triangle in second mesh.
            CMesh::FaceVertexIter fvIter  = m_meshB.fv_iter(*itfh);
            const CMesh::Point vA = m_meshB.point(*fvIter);
            ++fvIter;
            const CMesh::Point vB = m_meshB.point(*fvIter);
            ++fvIter;
            const CMesh::Point vC = m_meshB.point(*fvIter);
                                    

            CMesh::Point normal((vB - vA) % (vC - vA));
            normal.normalize();

            float t = dot(normal, vA) - dot(normal, point);

            CMesh::Point p = point + normal * t;

            const double tempDist = (point - p).length();

            if(tempDist < closestDistance)
            {
                closestDistance = tempDist;
            }
        }
        if(closestDistance < settings::searchStopEpsilon)
        {
            break;
        }
    }

    if (closestDistance != std::numeric_limits<double>::max())
    {
        if (closestDistance > m_max)
        {
            m_max = closestDistance;
        }
        if (closestDistance < m_min)
        {
            m_min = closestDistance;
        }
        //! Add new distance.
        m_mean += closestDistance;
        m_rms += closestDistance * closestDistance;
    }
    m_samplesCount++;
}

void HausdorffDistance::addSample(const CMesh::Point& point)
{
    //! Find closest point on meshB.
    OpenMesh::VertexHandle vHandle = m_octree->getClosestPointHandle(geometry::convert3<osg::Vec3, CMesh::Point>(point), m_searchDistance, m_meshB);

    if(vHandle.is_valid())
    {
        CMesh::Point mp = m_meshB.point(vHandle);
        const double dx(point[0] - mp[0]);
        const double dy(point[1] - mp[1]);
        const double dz(point[2] - mp[2]);
        const double distance(dx*dx + dy * dy + dz * dz);

        //! Toggle new distance.
        if (distance > m_max)
        {
            m_max = distance;
        }
        if (distance < m_min)
        {
            m_min = distance;
        }
        //! Add new distance.
        m_mean += distance;
        m_rms += distance * distance;

        m_samplesCount++;
    }
}


double HausdorffDistance::getTriangleArea(CMesh::FaceHandle face)
{
    CMesh::FaceVertexIter vertexIter = m_meshA.fv_iter(face);

    const CMesh::Point& vA = m_meshA.point(*vertexIter);
    ++vertexIter;
    const CMesh::Point& vB = m_meshA.point(*vertexIter);
    ++vertexIter;
    const CMesh::Point& vC = m_meshA.point(*vertexIter);

    return ((vB - vA) - (vC - vA)).norm() * 0.5f * 0.3333f;
}

bool HausdorffDistance::vertexCompute()
{
    if (m_maxSamples >= m_meshA.n_vertices())
    {
        //! The count of samples is greater than the total number of vertices, use all.
        for (geometry::CMesh::VertexIter vi = m_meshA.vertices_begin(); vi != m_meshA.vertices_end(); ++vi)
        {
            geometry::CMesh::Point point = m_meshA.point(*vi);
            addSample(point);
        }
    }
    else
    {
        //! Save vertices to helper buffer, and shuffle it.
        std::vector<geometry::CMesh::VertexHandle> vertexBuffer;
        for (geometry::CMesh::VertexIter vi = m_meshA.vertices_begin(); vi != m_meshA.vertices_end(); ++vi)
        {
            vertexBuffer.push_back(*vi);
        }
        assert(static_cast<size_t>(vertexBuffer.size()) == m_meshA.n_vertices());

        // obtain a time-based seed.
        const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(vertexBuffer.begin(), vertexBuffer.end(), std::mt19937(seed));

        //! Use sample until is equal with required count of samples. 
        for (int i = 0; i < vertexBuffer.size() && i < m_maxSamples; ++i)
        {
            geometry::CMesh::Point point = m_meshA.point(vertexBuffer[i]);
            addSample(point);
        }
    }
    return true;
}

bool HausdorffDistance::faceCompute()
{
    if (m_octree == nullptr)
    {
        m_meshB.updateOctree();
        m_octree = m_meshB.getOctree();
        return false;
    }

    typedef  std::pair<double, geometry::CMesh::FaceHandle> tInterval;
    std::vector<tInterval> intervals(m_meshA.n_faces() + 1);
    int i = 0;
    intervals[i] = std::make_pair(0, CMesh::FaceHandle(0));

    for (CMesh::FaceIter faceInterate = m_meshA.faces_begin(); faceInterate != m_meshA.faces_end(); ++faceInterate, ++i)
    {
        if (faceInterate->is_valid())
        {
            intervals[i + 1] = std::make_pair(intervals[i].first + getTriangleArea(*faceInterate), *faceInterate);
        }
    }

    const double meshArea = intervals.back().first;
    for (int sampleIndex = 0; sampleIndex < m_maxSamples; sampleIndex++)
    {
        double randomValue = meshArea * m_unifGenerate01(m_mt);
        const std::vector<tInterval>::iterator it = std::lower_bound(intervals.begin(), intervals.end(),
                                                                     std::make_pair(
                                                                         randomValue, OpenMesh::FaceHandle(0)));

        assert(it != intervals.begin());
        assert(it != intervals.end());
        assert((it - 1)->first < randomValue);
        assert(it->first >= randomValue);


        CMesh::Point barycentric = generateBarycentric();
        CMesh::FaceVertexIter vertexIter = m_meshA.fv_iter(it->second);

        const CMesh::Point vA = m_meshA.point(*vertexIter);
        ++vertexIter;           
        const CMesh::Point vB = m_meshA.point(*vertexIter);
        ++vertexIter;           
        const CMesh::Point vC = m_meshA.point(*vertexIter);

        const CMesh::Point point(
            vA * barycentric[0]
            + vB * barycentric[1]
            + vC * barycentric[2]);
        addFaceSample(point);
    }
    return true;
}

CMesh::Point HausdorffDistance::generateBarycentric()
{
    CMesh::Point barycentric;
    barycentric[1] = m_unifGenerate01(m_mt);
    barycentric[2] = m_unifGenerate01(m_mt);

    if (barycentric[1] + barycentric[2] > 1.0)
    {
        barycentric[1] = 1.0 - barycentric[1];
        barycentric[2] = 1.0 - barycentric[2];
    }
    assert(barycentric[1] + barycentric[2] <= 1.0);
    barycentric[0] = 1.0 - (barycentric[1] + barycentric[2]);

    return barycentric;
}

CMesh::Point HausdorffDistance::barycentric(const CMesh::Point& point, const CMesh::Point& a, const CMesh::Point& b, const CMesh::Point& c) const
{
    OpenMesh::Vec3d n((b - a) % (c - a));
    const OpenMesh::Vec3d na((c - b) % (point - b));
    const OpenMesh::Vec3d nb((a - c) % (point - c));
    const OpenMesh::Vec3d nc((b - a) % (point - a));
    const double denom(1.0 / n.length());
    const double u = dot(n, na) * denom;
    const double v = dot(n, nb) * denom;
    const double w = dot(n, nc) * denom;

    return {u, v, w};
}
}
