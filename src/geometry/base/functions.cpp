///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2015 3Dim Laboratory s.r.o.
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

#include <geometry/base/functions.h>
#include <geometry/base/CBoundingRectangle.h>
#include <Eigen/Eigenvalues>

namespace geometry
{

int isnan(double x)
{
    return x != x;
}

int isnan(const geometry::Vec2 &pt)
{
    return isnan(pt[0]) || isnan(pt[1]);
}

int isnan(const geometry::Vec3 &pt)
{
    return isnan(pt[0]) || isnan(pt[1]) || isnan(pt[2]);
}

geometry::Vec3 toVec3d(const geometry::Vec4 &v)
{
    return geometry::Vec3(v[0], v[1], v[2]);
}

geometry::Vec3 lerp(const geometry::Vec3 &v0, const geometry::Vec3 &v1, double w)
{
    return v0 + (v1 - v0) * geometry::Scalar(w);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Calculates intersection between two line segments
//!
//!\param   l0a             Line0's first point
//!\param   l0b             Line0's second point
//!\param   l1a             Line1's first point
//!\param   l1b             Line1's second point
//!\param   i               Intersection point
//!\param   includePoints   Flag if lines' points can touch without causing intersection
//!
//!\return  True if lines intersect each other, false otherwise
////////////////////////////////////////////////////////////////////////////////////////////////////
bool calculateIntersection2d(const geometry::Vec2 &l0a, const geometry::Vec2 &l0b, const geometry::Vec2 &l1a, const geometry::Vec2 &l1b, geometry::Vec2 &i, double snapThreshold, double angleThreshold, bool includePoints)
{
    if (!includePoints && ((l0a == l1a) || (l0b == l1b) || (l0a == l1b) || (l1a == l0b)))
    {
        return false;
    }

    if ((l0b == l0a) || (l1b == l1a))
        return false;

#ifdef _DEBUG
    if (isnan(l0a) || isnan(l0b) || isnan(l1a) || isnan(l1b))
    {
        std::cout << "nan!";
        return false;
    }
#endif

    geometry::CBoundingRectangle l0r, l1r;
    l0r.expandBy(l0a);
    l0r.expandBy(l0b);
    l0r.expandBy(l0r.getMin() - geometry::Vec2(snapThreshold, snapThreshold));
    l0r.expandBy(l0r.getMax() + geometry::Vec2(snapThreshold, snapThreshold));
    l1r.expandBy(l1a);
    l1r.expandBy(l1b);
    l1r.expandBy(l1r.getMin() - geometry::Vec2(snapThreshold, snapThreshold));
    l1r.expandBy(l1r.getMax() + geometry::Vec2(snapThreshold, snapThreshold));

    if (!l0r.intersects(l1r))
    {
        return false;
    }

    geometry::Vec2 v0 = l0b - l0a;
    geometry::Vec2 v0n = v0;
    v0n.normalize();
    geometry::Vec2 n0 = geometry::Vec2(-v0n[1], v0n[0]);
    double a0 = n0[0];
    double b0 = n0[1];
    double c0 = -1 * (a0 * l0a[0] + b0 * l0a[1]);

    geometry::Vec2 v1 = l1b - l1a;
    geometry::Vec2 v1n = v1;
    v1n.normalize();
    geometry::Vec2 n1 = geometry::Vec2(-v1n[1], v1n[0]);
    double a1 = n1[0];
    double b1 = n1[1];
    double c1 = -1 * (a1 * l1a[0] + b1 * l1a[1]);

    // check if ray and segment are parallel
    if (1.0 - std::fabs(v0n * v1n) < angleThreshold)
    {
        return false;
    }

    double denom = a1 * b0 - a0 * b1;
    if (0.0 == denom)
    {
        return false;
    }

    // find intersection point
    double iy = (a0 * c1 - a1 * c0) / denom;
    double ix = (b0 * c1 - b1 * c0) / (-denom);

    // calculate "t" parameters
    double t0 = std::fabs(v0n[0]) < snapThreshold ? (iy - l0a[1]) / v0[1] : (ix - l0a[0]) / v0[0];
    double t1 = std::fabs(v1n[0]) < snapThreshold ? (iy - l1a[1]) / v1[1] : (ix - l1a[0]) / v1[0];

    // check if intersection is in valid area
    if ((t0 < -snapThreshold || t0 > 1.0 + snapThreshold) || (t1 < -snapThreshold || t1 > 1.0 + snapThreshold))
    {
        return false;
    }

    i = l0a + v0 * geometry::Scalar(t0);
#ifdef _DEBUG
    assert(!isnan(i[0]));
    assert(!isnan(i[1]));
#endif
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Calculates intersection between two line segments
//!
//!\param   l0a             Line0's first point
//!\param   l0b             Line0's second point
//!\param   l1a             Line1's first point
//!\param   l1b             Line1's second point
//!\param   i               Intersection point
//!\param   includePoints   Flag if lines' points can touch without causing intersection
//!
//!\return  True if lines intersect each other, false otherwise
////////////////////////////////////////////////////////////////////////////////////////////////////
bool calculateIntersection3d(const geometry::Vec3 &l0a, const geometry::Vec3 &l0b, const geometry::Vec3 &l1a, const geometry::Vec3 &l1b, geometry::Vec3 &i, double snapThreshold, double angleThreshold)
{
#if(1)
    geometry::CPlane plane = geometry::CPlane(l0a, l0b, l1a);
    if (std::fabs(plane.distance(l1b)) > snapThreshold)
    {
        return false;
    }

    geometry::Vec3 u = l0b - l0a;
    geometry::Vec3 v = l1b - l1a;

    geometry::Vec3 un = u;
    geometry::Vec3 vn = v;
    un.normalize();
    vn.normalize();

    // check if ray and segment are parallel
    if (1.0 - std::fabs(un * vn) < angleThreshold)
    {
        return false;
    }

    int i0 = 0;
    i0 = (std::fabs(u[0]) > std::fabs(u[1]) ? 0 : 1);
    i0 = (std::fabs(u[2]) > std::fabs(u[i0]) ? 2 : i0);

    int i1 = 0;
    i1 = (std::max(std::fabs(u[(i0 + 1) % 3]), std::fabs(v[(i0 + 1) % 3])) > std::max(std::fabs(u[(i0 + 2) % 3]), std::fabs(v[(i0 + 2) % 3])) ? (i0 + 1) % 3 : (i0 + 2) % 3);

    double denominator = v[i0] * u[i1] - v[i1] * u[i0];
    if (std::fabs(denominator) < snapThreshold)
    {
        return false;
    }

    double t2 = (u[i0] * (l1a[i1] - l0a[i1]) - l1a[i0] * u[i1] + l0a[i0] * u[i1]) / denominator;
    double t1 = (l1a[i0] + t2 * v[i0] - l0a[i0]) / u[i0];

    double uLength = u.length();
    double vLength = v.length();

    if (t1 * uLength < -snapThreshold || t1 * uLength > uLength + snapThreshold || t2 * vLength < -snapThreshold || t2 * vLength > vLength + snapThreshold)
    {
        return false;
    }

    i = l0a + u * geometry::Scalar(t1);

    return true;

#else
#ifdef _DEBUG
    if (isnan(l0a) || isnan(l0b) || isnan(l1a) || isnan(l1b))
    {
        std::cout << "nan!";
        return false;
    }
#endif

    geometry::Vec3 u = l0b - l0a;
    geometry::Vec3 v = l1b - l1a;
    geometry::Vec3 w = l0a - l1a;

    double a = u * u;
    double b = u * v;
    double c = v * v;
    double d = u * w;
    double e = v * w;
    double D = a * c - b * b;
    double sc = D;
    double sN = D;
    double sD = D;
    double tc = D;
    double tN = D;
    double tD = D;

    if (D < snapThreshold)
    {
        sN = 0.0;
        sD = 1.0;
        tN = e;
        tD = c;
    }
    else
    {
        sN = (b * e - c * d);
        tN = (a * e - b * d);
        if (sN < 0.0)
        {
            sN = 0.0;
            tN = e;
            tD = c;
        }
        else if (sN > sD)
        {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0)
    {
        tN = 0.0;
        if (-d < 0.0)
        {
            sN = 0.0;
        }
        else if (-d > a)
        {
            sN = sD;
        }
        else
        {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD)
    {
        tN = tD;
        if ((-d + b) < 0.0)
        {
            sN = 0;
        }
        else if ((-d + b) > a)
        {
            sN = sD;
        }
        else
        {
            sN = (-d + b);
            sD = a;
        }
    }
    sc = (std::fabs(sN) < snapThreshold ? 0.0 : sN / sD);
    tc = (std::fabs(tN) < snapThreshold ? 0.0 : tN / tD);

    geometry::Vec3 usc = u * sc;
    geometry::Vec3 vtc = v * tc;

    geometry::Vec3 dP = w + usc - vtc;

    if (dP.length() > snapThreshold)
    {
        return false;
    }

    geometry::Vec3 i0 = l0a + usc;
    geometry::Vec3 i1 = l0a - w + vtc;
    geometry::Vec3 i2 = l1a + vtc;
    geometry::Vec3o i3 = l0a - w + vtc + dP;

    i = i0;

#ifdef _DEBUG
    assert(!isnan(i[0]));
    assert(!isnan(i[1]));
    assert(!isnan(i[2]));
#endif

    return true;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Performs very simple offset of polygon along point normals
//!         (! only use for tiny offsets as it does not solves overlaps, intersections etc.)
//!
//!\param   offset  Offset
//!\param   polygon Polygon
////////////////////////////////////////////////////////////////////////////////////////////////////
void dummyOffset(double offset, std::vector<geometry::Vec2> &polygon)
{
    polygon.resize(polygon.size() - 1);

    std::vector<geometry::Vec2> normals;
    for (int i = 0; i < polygon.size(); ++i)
    {
        geometry::Vec2 prev = polygon[(polygon.size() + i - 1) % polygon.size()];
        geometry::Vec2 curr = polygon[i];
        geometry::Vec2 next = polygon[(i + 1) % polygon.size()];

        geometry::Vec2 vec0 = curr - prev;
        geometry::Vec2 vec1 = next - curr;

        vec0.normalize();
        vec1.normalize();

        vec0 = geometry::Vec2(-vec0[1], vec0[0]);
        vec1 = geometry::Vec2(-vec1[1], vec1[0]);

        geometry::Vec2 normal = vec0 + vec1;
        normal.normalize();

        normals.push_back(normal);
    }

    for (int i = 0; i < polygon.size(); ++i)
    {
        polygon[i] += normals[i] * geometry::Scalar(offset);
    }

    polygon.push_back(polygon.front());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if point lies within polygon
//!
//!\param   p       Point
//!\param   polygon Polygon
//!
//!\return  True if point lies in polygon, false otherwise
////////////////////////////////////////////////////////////////////////////////////////////////////
bool pointInPolygon(const geometry::Vec2 &p, const std::vector<geometry::Vec2> &polygon)
{
    int i_first = 0;
    int i_last = polygon.size() - 1;
    int i_curr = 0;
    bool result = false;
    {
        do
        {
            geometry::Vec2 curr = polygon[i_curr];
            geometry::Vec2 prev = polygon[(polygon.size() - 1 + i_curr - 1) % (polygon.size() - 1)];

            if ((((curr.y() <= p.y()) && (p.y() < prev.y())) || ((prev.y() <= p.y()) && (p.y() < curr.y()))) &&
                (p.x() < (prev.x() - curr.x()) * (p.y() - curr.y()) / (prev.y() - curr.y()) + curr.x()))
            {
                result = !result;
            }
            i_curr = (i_curr + 1) % polygon.size();
        }
        while (i_curr != i_last);
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Calculates signed area of polygon
//!
//!\param   polygon Polygon
//!
//!\ return Signed area of polygon
////////////////////////////////////////////////////////////////////////////////////////////////////
double polygonArea(const std::vector<geometry::Vec2> &polygon)
{
    int last = polygon.size() - 1;

    double a = (polygon[last][0] + polygon[0][0]) * (polygon[0][1] - polygon[last][1]);
    for (int i = 1; i < polygon.size(); ++i)
    {
        geometry::Vec2 vertex0 = polygon[i];
        geometry::Vec2 vertex1 = polygon[i - 1];

        a += (vertex1[0] + vertex0[0]) * (vertex0[1] - vertex1[1]);
    }

    return a / 2;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks orientation of polygon
//!
//!\param   polygon List of vertices forming polygon or a hole
//!
//!\ return True if polygon is a polygon (CW order), false if it is a hole (CCW order)
////////////////////////////////////////////////////////////////////////////////////////////////////
bool isPolygon(const std::vector<geometry::Vec2> &polygon)
{
    return polygonArea(polygon) < 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks orientation of triangle
//!
//!\param   v0 Vertex #0
//!\param   v1 Vertex #1
//!\param   v2 Vertex #2
//!
//!\ return True if vertices form a polygon (CW order), false if it is a hole (CCW order)
////////////////////////////////////////////////////////////////////////////////////////////////////
bool isPolygon(const geometry::Vec2 &v0, const geometry::Vec2 &v1, const geometry::Vec2 &v2)
{
#if(0)
    std::vector<geometry::Vec2> polygon;
    polygon.push_back(v0);
    polygon.push_back(v1);
    polygon.push_back(v2);
    return isPolygon(polygon);
#else
    double a = (v2.x() + v0.x()) * (v0.y() - v2.y());
    a += (v1.x() + v0.x()) * (v1.y() - v0.y());
    a += (v2.x() + v1.x()) * (v2.y() - v1.y());
    return a < 0.0; //return (a / 2) > 0.0;
#endif
}

bool triOnPlane(const geometry::CPlane &plane, const geometry::Vec3 &pointA, const geometry::Vec3 &pointB, const geometry::Vec3 &pointC, double snapThreshold)
{
    double distanceA = std::fabs(plane.distance(pointA));
    double distanceB = std::fabs(plane.distance(pointB));
    double distanceC = std::fabs(plane.distance(pointC));

    if (distanceA + distanceB + distanceC < snapThreshold)
    {
        return true;
    }

    return false;
}

void getLookAt(const geometry::Matrix &matrix, geometry::Vec3 &eye, geometry::Vec3 &center, geometry::Vec3 &up, double lookDistance)
{
    geometry::Matrix inv = geometry::Matrix::inverse(matrix);

    // note: e and c variables must be used inside this method instead of eye and center
    // because eye and center are references and they may point to the same variable.
    geometry::Vec3 e = inv.operator*(geometry::Vec3(0.0, 0.0, 0.0));
    geometry::Vec3 u = inv.operator*(geometry::Vec3(0.0, 1.0, 0.0));
    geometry::Vec3 f = inv.operator*(geometry::Vec3(0.0, 0.0, -1.0));
    u = u - e;
    f = f - e;
    u.normalize();
    f.normalize();
    geometry::Vec3 c = e + f * geometry::Scalar(lookDistance);

    // assign the results
    eye = e;
    center = c;
    up = u;
}

void pca(const std::vector<geometry::Vec3> & pc, std::array<geometry::Vec3, 3>& eigenVectors, std::array<double, 3>& eigenValues)
{
    if (pc.empty())
    {
        return;
    }

    Eigen::MatrixXd mat(pc.size(), 3);

    for (unsigned i = 0; i < pc.size(); i++)
    {
        mat.row(i) = Eigen::Vector3d(pc[i].getPtr());
    }
    Eigen::MatrixXd centered = mat.rowwise() - mat.colwise().mean();
    Eigen::MatrixXd cov = centered.adjoint() * centered;

    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(cov);

    // Sort eigenvectors and eigenvalues in descendent order
    for (int i = 0; i < 3; ++i)
    {
        eigenValues[i] = eig.eigenvalues()[2 - i];
        eigenVectors[i][0] = eig.eigenvectors().col(2 - i)[0];
        eigenVectors[i][1] = eig.eigenvectors().col(2 - i)[1];
        eigenVectors[i][2] = eig.eigenvectors().col(2 - i)[2];
    }
}

void getConnectedGeometryInRadius(geometry::CMesh& mesh, geometry::CMesh::VertexHandle seed, geometry::Vec3 center, float radius, std::vector<geometry::CMesh::VertexHandle>& handles, std::vector<geometry::Vec3>& neighbors, std::vector<double>& distances)
{
    handles.clear();
    neighbors.clear();
    distances.clear();
    geometry::CMesh::Point s = mesh.point(seed);
    geometry::CMesh::Point c(center[0], center[1], center[2]);
    double cdist = (s - c).length();
    if (cdist > radius)
    {
        return;
    }

    OpenMesh::VPropHandleT< bool > visited;
    mesh.add_property(visited);

    geometry::CMesh::VertexIter v_it, v_end;
    v_end = mesh.vertices_end();

    // iterate over all vertices
    for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it)
    {
        mesh.property(visited, v_it) = false;
    }

    handles.push_back(seed);
    neighbors.push_back(geometry::Vec3(s[0], s[1], s[2]));
    distances.push_back(cdist);
    mesh.property(visited, seed) = true;
    int i = 0;
    while (i < handles.size())
    {
        geometry::CMesh::VertexHandle current = handles[i];
        for (geometry::CMesh::VertexVertexIter vv_it = mesh.vv_iter(current); vv_it; ++vv_it)
        {
            if (mesh.property(visited, vv_it))
            {
                continue;
            }

            geometry::CMesh::Point p = mesh.point(vv_it.handle());
            double dist = (p - c).length();
            if (dist < radius)
            {
                handles.push_back(vv_it.handle());
                neighbors.push_back(geometry::Vec3(p[0], p[1], p[2]));
                distances.push_back(dist);
            }
            mesh.property(visited, vv_it) = true;
        }
        i++;
    }
    mesh.remove_property(visited);
}

/**
 * Projects queryPoint on line defined by pointOnLine and lineDirection
 * distance - signed distance from pointOnLine to projected point
 */
geometry::Vec3 closestPointOnLine(geometry::Vec3 pointOnLine, geometry::Vec3 lineDirection, geometry::Vec3 queryPoint, double& distance)
{
    geometry::Vec3 pq = queryPoint - pointOnLine;
    geometry::Vec3 l(lineDirection);
    distance = (pq * l) / (l * l);
    geometry::Vec3 result = pointOnLine + (l * vpl::CScalar<double>(distance));
    return result;
}

geometry::CMesh * cutMeshByVerticesSubset(geometry::CMesh & mesh, std::vector<geometry::CMesh::VertexHandle>& handles)
{
    geometry::CMesh* cutoutMesh = new geometry::CMesh();

    if (handles.size() < 3)
        return nullptr;

    std::map<geometry::CMesh::VertexHandle, geometry::CMesh::VertexHandle> vhandlesMap;
    for (auto& vh : handles)
    { // copy vertices into new CMesh
        geometry::CMesh::Point p = mesh.point(vh);
        vhandlesMap[vh] = cutoutMesh->add_vertex(p);
    }

    if(cutoutMesh->n_vertices() < 3)
        return nullptr;

    for (geometry::CMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it)
    { // copy faces into new CMesh
        std::vector<geometry::CMesh::VertexHandle> fvhandles;
        for (geometry::CMesh::FaceVertexIter fv_it = mesh.fv_iter(f_it.handle()); fv_it; ++fv_it)
        {
            if (vhandlesMap.find(fv_it.handle()) != vhandlesMap.end())
            {
                fvhandles.push_back(vhandlesMap[fv_it.handle()]);
            }
        }
        if (fvhandles.size() == 3)
        { // all three face vertices are in defined subset
            cutoutMesh->add_face(fvhandles);
        }
    }

    if (cutoutMesh->n_faces() < 1)
        return nullptr;

    return cutoutMesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if polygon is self-intersecting
//!
//!\param   polygon     Polygon to check
//!
//!\return  true if contains self-intersection, false otherwise
////////////////////////////////////////////////////////////////////////////////////////////////////
bool checkSelfIntersections(std::vector<geometry::Vec2> &polygon, double snapThreshold, double angleThreshold)
{
    for (int l0 = 0; l0 < polygon.size() - 1; ++l0)
    {
        geometry::Vec2 l0a = polygon[l0 + 0];
        geometry::Vec2 l0b = polygon[l0 + 1];

        for (int l1 = l0 + 2; l1 < polygon.size() - 1; ++l1)
        {
            geometry::Vec2 l1a = polygon[l1 + 0];
            geometry::Vec2 l1b = polygon[l1 + 1];
            geometry::Vec2 i;

            if (calculateIntersection2d(l0a, l0b, l1a, l1b, i, snapThreshold, angleThreshold, false))
            {
                return true;
            }
        }
    }

    return false;
}

} // namespace geometry
