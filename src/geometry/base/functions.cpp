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
#include <array>
#include <unordered_set>
#include <unordered_map>

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

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Calculates Principal Component Analysis
//!
//!\param   pc              Point cloud
//!\param   eigenVectors    Result eigen vectors
//!\param   eigenValues     Result eigen values
//!
////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Finds connected geometry of given mesh in defined radius
//!
//!\param   mesh        Input mesh
//!\param   seed        Starting vertex
//!\param   center      center of search radius
//!\param   radius      search radius
//!\param   handles     found vertex handles
//!\param   neighbors   found vertices
//!\param   distances   distances of vertices from center
//!
////////////////////////////////////////////////////////////////////////////////////////////////////
void getConnectedGeometryInRadius(const geometry::CMesh& mesh, const geometry::CMesh::VertexHandle seed, const geometry::Vec3 center, const float radius, std::vector<geometry::CMesh::VertexHandle>& handles, std::vector<geometry::Vec3>& neighbors, std::vector<double>& distances)
{
    handles.clear();
    neighbors.clear();
    distances.clear();
    geometry::CMesh copy(mesh);
    geometry::CMesh::Point s = copy.point(seed);
    geometry::CMesh::Point c(center[0], center[1], center[2]);
    double cdist = (s - c).length();
    if (cdist > radius)
    {
        return;
    }

    OpenMesh::VPropHandleT< bool > visited;
    copy.add_property(visited);

    geometry::CMesh::VertexIter v_it, v_end;
    v_end = copy.vertices_end();

    // iterate over all vertices
    for (v_it = copy.vertices_begin(); v_it != v_end; ++v_it)
    {
        copy.property(visited, v_it) = false;
    }

    handles.push_back(seed);
    neighbors.push_back(geometry::Vec3(s[0], s[1], s[2]));
    distances.push_back(cdist);
    copy.property(visited, seed) = true;
    int i = 0;
    while (i < handles.size())
    {
        geometry::CMesh::VertexHandle current = handles[i];
        for (geometry::CMesh::VertexVertexIter vv_it = copy.vv_iter(current); vv_it.is_valid(); ++vv_it)
        {
            if (copy.property(visited, vv_it))
            {
                continue;
            }

            geometry::CMesh::Point p = copy.point(vv_it.handle());
            double dist = (p - c).length();
            if (dist < radius)
            {
                handles.push_back(vv_it.handle());
                neighbors.push_back(geometry::Vec3(p[0], p[1], p[2]));
                distances.push_back(dist);
            }
            copy.property(visited, vv_it) = true;
        }
        i++;
    }
    copy.remove_property(visited);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Projects point on line
//!
//!\param   pointOnLine     Point on line
//!\param   lineDirection   Line direction
//!\param   queryPoint      Point of intersection
//!\param   distance        Signed distance from pointOnLine to projected point
//!
//!\return  Projected point
////////////////////////////////////////////////////////////////////////////////////////////////////
geometry::Vec3 closestPointOnLine(const geometry::Vec3& pointOnLine, const geometry::Vec3& lineDirection, const geometry::Vec3& queryPoint, double& distance)
{
    geometry::Vec3 pq = queryPoint - pointOnLine;
    geometry::Vec3 l(lineDirection);
    distance = (pq * l) / (l * l);
    geometry::Vec3 result = pointOnLine + (l * vpl::CScalar<double>(distance));
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Projects point on plane
//!
//!\param   pointOnPlane    Point on plane
//!\param   planeNormal     Normal vector of plane
//!\param   queryPoint      Point of intersection
//!\param   distance        Signed distance from queryPoint to plane
//!
//!\return  Projected point
////////////////////////////////////////////////////////////////////////////////////////////////////
geometry::Vec3 projectPointOnPlane(const geometry::Vec3& pointOnPlane, const geometry::Vec3& planeNormal, const geometry::Vec3& queryPoint, double& distance)
{
    closestPointOnLine(pointOnPlane, planeNormal, queryPoint, distance);
    geometry::Vec3 result = queryPoint - (planeNormal * vpl::CScalar<double>(distance));
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if line intesects plane
//! this treats edge case of line coincident with plane as no intersection
//!
//!\param   pointOnPlane        Point on plane
//!\param   planeNormal         Normal vector of plane
//!\param   pointOnLine         Point on line
//!\param   lineDirection       Line direction
//!\param   intersectionPoint   Point of intersection
//!\param   distance            Signed distance from pointOnLine to intersectionPoint
//!
//!\return  True if there is any intersection found
////////////////////////////////////////////////////////////////////////////////////////////////////
bool linePlaneIntersection(const geometry::Vec3& pointOnPlane, const geometry::Vec3& planeNormal, const geometry::Vec3& pointOnLine, const geometry::Vec3& lineDirection, geometry::Vec3& intersectionPoint, double& distance)
{
    double dot = lineDirection * planeNormal;
    double dist2plane = 0.0;
    closestPointOnLine(pointOnPlane, planeNormal, pointOnLine, dist2plane);
    if (std::fabs(dot) > 0.000001)
    { 
        // point intersection
        distance = -dist2plane / dot;
        intersectionPoint = pointOnLine + (lineDirection * vpl::CScalar<double>(distance));
        return true;
    }

    // no intersection
    intersectionPoint = geometry::Vec3(0.0, 0.0, 0.0);
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if point lies inside of triangle
//!
//!\param   point           Point
//!\param   triangle        Vector of three triangle vertices
//!
//!\return  True if the point lies inside of the triangle
////////////////////////////////////////////////////////////////////////////////////////////////////
bool pointInTriangle(const geometry::Vec2 & point, const std::vector<geometry::Vec2>& triangle)
{
    // check triangle size
    if (triangle.size() != 3)
        return false;

    // compute barycentric coordinates of point
    double ABlen = (triangle[1] - triangle[0]).length();
    double AClen = (triangle[2] - triangle[0]).length();
    double PAlen = (triangle[0] - point).length();
    double PBlen = (triangle[1] - point).length();
    double PClen = (triangle[2] - point).length();
    double inv2areaABC = 1.0 / (ABlen * AClen);
    double alpha = PBlen * PClen * inv2areaABC;
    double beta = PClen * PAlen * inv2areaABC;
    double gamma = 1.0 - alpha - beta;

    return alpha >= 0.0 && alpha <= 1.0 && beta >= 0.0 && beta <= 1.0 && gamma >= 0.0 && gamma <= 1.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if point lies inside of triangle
//!
//!\param   point           Point
//!\param   triangle        Vector of three triangle vertices
//!\param   triangleNormal  Normal vector of triangle
//!
//!\return  True if the point lies inside of the triangle
////////////////////////////////////////////////////////////////////////////////////////////////////
bool pointInTriangle(const geometry::Vec3& point, const std::vector<geometry::Vec3>& triangle, const geometry::Vec3& triangleNormal)
{
    // check triangle size
    if (triangle.size() != 3)
        return false;

    // check if point lies on triangle plane
    double distance = 0.0;
    closestPointOnLine(triangle[0], triangleNormal, point, distance);
    if (std::fabs(distance) > 0.000001)
        return false;

    // find biggest component of normal
    geometry::Vec3 N = geometry::Vec3(triangleNormal).abs();
    int idx = -1;
    if (N[0] > N[1])
    {
        if (N[0] > N[2])
            idx = 0;
        else
            idx = 2;
    }
    else if (N[1] > N[2])
        idx = 1;
    else
        idx = 2;

    // project point and triangle on one of the main planes (XY, YZ, XZ) that is most parallel with triangle plane
    geometry::Vec2 P2;
    std::vector<geometry::Vec2> T2(3);
    for (int i = 0, c = 0; i < 3; ++i)
    {
        if (i == idx)
            continue;
        P2[c] = point[i];
        T2[0][c] = triangle[0][i];
        T2[1][c] = triangle[1][i];
        T2[2][c] = triangle[2][i];
        c++;
    }
    // test in 2D
    return pointInTriangle(P2, T2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if line intersects triangle
//!
//!\param   triangle            Vector of three triangle vertices
//!\param   triangleNormal      Normal vector of triangle
//!\param   pointOnLine         Point on line
//!\param   lineDirection       Line direction
//!\param   intersectionPoint   Point of intersection
//!\param   distance            Signed distance from pointOnLine to intersection
//!
//!\return  True if there is any intersection found
////////////////////////////////////////////////////////////////////////////////////////////////////
bool lineTriangleIntersection(const std::vector<geometry::Vec3>& triangle, const geometry::Vec3& triangleNormal, const geometry::Vec3& pointOnLine, const geometry::Vec3& lineDirection, geometry::Vec3& intersectionPoint, double& distance)
{
    if (triangle.size() != 3)
        return false;

    if (!linePlaneIntersection(triangle[0], triangleNormal, pointOnLine, lineDirection, intersectionPoint, distance))
        return false;

    return pointInTriangle(intersectionPoint, triangle, triangleNormal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if point is inside of mesh
//! if the mesh is opened, the algorithm assumes the maximal dimensions in the given direction (upVector) as mesh boundaries
//! octree of mesh needs to be updated
//!
//!\param   mesh        Input mesh
//!\param   point       Point in question
//!\param   upVector    Vector pointing upwards
//!
//!\return  True if the point is inside of mesh
////////////////////////////////////////////////////////////////////////////////////////////////////
bool isPointInsideMesh(geometry::CMesh& mesh, const geometry::Vec3& point, const geometry::Vec3& upVector)
{
    // find dimensions of model in direction of upVector
    double maxDist = -std::numeric_limits<double>::max(), minDist = std::numeric_limits<double>::max();
    geometry::Vec3 maxVec, minVec;
    geometry::CMesh::VertexIter v_it = mesh.vertices_begin(), v_end = mesh.vertices_end();
    for (; v_it != v_end; ++v_it)
    {
        geometry::Vec3 p = geometry::convert3<geometry::Vec3>(mesh.point(v_it.handle()));
        double distance = 0.0;
        geometry::Vec3 q = closestPointOnLine(point, upVector, p, distance);
        if (distance > maxDist)
        {
            maxDist = distance;
            maxVec = q;
        }
        if (distance < minDist)
        {
            minDist = distance;
            minVec = q;
        }
    }

    // point lies outside of model dimensions
    if (maxDist < 0.0 || minDist > 0.0)
        return false;

    // get nodes from octree
    osg::BoundingBox bb;
    bb.expandBy(minVec[0], minVec[1], minVec[2]);
    bb.expandBy(maxVec[0], maxVec[1], maxVec[2]);
    // make sure the bounding box is valid
    bb.expandBy(minVec[0] - 0.1, minVec[1] - 0.1, minVec[2] - 0.1);
    bb.expandBy(maxVec[0] + 0.1, maxVec[1] + 0.1, maxVec[2] + 0.1);
    const std::vector<geometry::CMeshOctreeNode *> &nodes = mesh.getOctree()->getIntersectedNodes(bb);

    // get triangles from octree
    std::vector<geometry::CMesh::FaceHandle> faceHandles;

    // use reverse iterators because the last nodes are the leaf nodes and should include closer triangles 
    std::vector<geometry::CMeshOctreeNode *>::const_reverse_iterator itn = nodes.rbegin(), itnEnd = nodes.rend();
    for (; itn != itnEnd; ++itn)
    {
        faceHandles.insert(faceHandles.end(), (*itn)->faces.begin(), (*itn)->faces.end());
    }

    // structure for intersections data
    typedef struct SIntersection {
        geometry::Vec3 point;
        double distance;
        bool faceUp;
        bool invalid;
    } SIntersection;

    std::vector<SIntersection> intersections(faceHandles.size());

    // find all faces that intersects with the line
    SIntersection invalidIntersection = { geometry::Vec3(), 0.0, false, true };
#pragma omp parallel for shared (intersections)
    for (int i = 0; i < static_cast<int>(faceHandles.size()); ++i)
    {
        geometry::CMesh::FaceHandle fh = faceHandles[i];
        std::vector<geometry::Vec3> triangle;
        triangle.reserve(3);
        for (geometry::CMesh::ConstFaceVertexIter fv_it = mesh.cfv_iter(fh); fv_it.is_valid(); ++fv_it)
        {
            geometry::CMesh::Point p = mesh.point(fv_it);
            triangle.push_back(geometry::convert3<geometry::Vec3>(p));
        }
        if (triangle.size() != 3)
        {
            intersections[i] = invalidIntersection;
            continue;
        }

        geometry::Vec3 triangleNormal = geometry::convert3<geometry::Vec3>(mesh.normal(fh));
        triangleNormal.normalize();

        // find intersection with triangle
        geometry::Vec3 intersection;
        double distance = 0.0;
        if (!lineTriangleIntersection(triangle, triangleNormal, point, upVector, intersection, distance))
        {
            intersections[i] = invalidIntersection;
            continue;
        }

        // save intersection data
        bool faceUp = (upVector * triangleNormal) > 0.0;
        intersections[i] = { intersection, distance, faceUp, false };
    }

    // clear invalid intersections
    intersections.erase(std::remove_if(intersections.begin(), intersections.end(), [](SIntersection intersection) {return intersection.invalid; }), intersections.end());

    // no intersection, point is outside
    double size = intersections.size();
    if (size <= 0)
        return false;

    // sort intersections from bottom up
    std::sort(intersections.begin(), intersections.end(), [](SIntersection a, SIntersection b) { return a.distance < b.distance; });

    std::vector<SIntersection> copy;
    copy.reserve(intersections.size()+2);

    // Go through intersections and store only change of state
    ////////////////////////////////////////////////////////////////////////////////
    //  intersections:
    //          min          point                 max  
    //        ---|------)--)---0----(-(-(--)--------|---
    //                                                  
    //  copy:                                           
    //        ---(---------)---0----(------)------------
    //        out    in       out      in      out      
    ////////////////////////////////////////////////////////////////////////////////
    bool inside = false;
    std::vector<SIntersection>::iterator it = intersections.begin(), itEnd = intersections.end();
    for (; it != itEnd; ++it)
    {
        if (inside)
        {
            if (!it->faceUp)
            { // when "inside" ignore other inside region openings
                continue;
            }
            else
            { // closeing of last inside region
                copy.push_back(*it);
                inside = false;
            }
        }
        else
        {
            if (!it->faceUp)
            { // opening of new inside region
                copy.push_back(*it);
                inside = true;
            }
            else
            {
                if (copy.empty())
                { 
                    // insert cap to "close" the model
                    copy.push_back({ (point + upVector * vpl::CScalar<double>(minDist)), minDist, false });
                    // insert current intersectin  to close the inside reguion
                    copy.push_back(*it);
                }
                else
                { // overwrite the closing of last inside region
                    copy.back() = *it;
                }
            }
        }
        
    }
    if (!copy.empty() && !copy.back().faceUp)
    { // insert cap to "close" the model
        copy.push_back({ (point + upVector * vpl::CScalar<double>(maxDist)), maxDist, true });
    }

    // go through intersections vector and check if point is inside, 0.0 is position of point
    std::vector<SIntersection>::iterator it2 = copy.begin(), it2End = copy.end();
    for (; it2 != it2End; ++it2)
    {
        if (it2->faceUp && it2->distance >= 0.0)
            return true; // point inside
        if (!it2->faceUp && it2->distance > 0.0)
            return false; // point outside
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Creates new mesh from given mesh including only selected vertices and faces betweeen them
//!
//!\param   mesh    Source mesh
//!\param   handles Collection of vertex handles
//!
//!\return  New mesh
////////////////////////////////////////////////////////////////////////////////////////////////////
geometry::CMesh * cutMeshByVerticesSubset(const geometry::CMesh& mesh, std::vector<geometry::CMesh::VertexHandle>& handles)
{
    if (handles.size() < 3)
        return nullptr;

    geometry::CMesh* cutoutMesh = new geometry::CMesh();

    std::unordered_map<geometry::CMesh::VertexHandle, geometry::CMesh::VertexHandle> vhandlesMap;
    for (auto& vh : handles)
    { // copy vertices into new CMesh
        geometry::CMesh::Point p = mesh.point(vh);
        vhandlesMap[vh] = cutoutMesh->add_vertex(p);
    }

    if (cutoutMesh->n_vertices() < 3)
    {
        delete cutoutMesh;
        return nullptr;
    }

//     for (geometry::CMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it)
//     { // copy faces into new CMesh
//         std::vector<geometry::CMesh::VertexHandle> fvhandles;
//         for (geometry::CMesh::ConstFaceVertexIter fv_it = mesh.cfv_iter(f_it.handle()); fv_it.is_valid(); ++fv_it)
//         {
//             if (vhandlesMap.find(fv_it.handle()) != vhandlesMap.end())
//             {
//                 fvhandles.push_back(vhandlesMap[fv_it.handle()]);
//             }
//         }
//         if (fvhandles.size() == 3)
//         { // all three face vertices are in defined subset
//             cutoutMesh->add_face(fvhandles);
//         }
//     }

    std::unordered_set<geometry::CMesh::FaceHandle> faces_created;
    std::vector<geometry::CMesh::VertexHandle> fvhandles;
    for (auto &vh : handles)
    {
        // For all faces around this vertex
        for (geometry::CMesh::ConstVertexFaceIter vf_it = mesh.cvf_iter(vh); vf_it.is_valid(); ++vf_it)
        {
            const geometry::CMesh::FaceHandle fh = vf_it.handle();
            // If face not created yet
            if (faces_created.find(fh) == faces_created.end())
            {
                // Circulate around face and test if all vertices are in the enabled vertices 
                for (geometry::CMesh::ConstFaceVertexIter fv_it = mesh.cfv_iter(fh); fv_it.is_valid(); ++fv_it)
                {
                    std::unordered_map<geometry::CMesh::VertexHandle, geometry::CMesh::VertexHandle>::const_iterator it = vhandlesMap.find(fv_it.handle());
                    if (it == vhandlesMap.end())
                        break;
                    fvhandles.push_back(it->second);
                }
                // all three face vertices are in defined subset
                if (fvhandles.size() == 3)
                    cutoutMesh->add_face(fvhandles);

                // clear handles vector for reuse
                fvhandles.clear();

                // Set face as tested (and possibly created)
                faces_created.insert(fh);
            }
        }
    }

    if (cutoutMesh->n_faces() < 1)
    {
        delete cutoutMesh;
        return nullptr;
    }

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

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Calculates Euler angles from given quaternion
//! Quaternion does not need to be normalized
//! Resulting rotation is supposed to be applied in this order: X firts, Y second and Z last
//! Right hand coordinate system
//! Angles are in radians
//!
//!\param   v0 Vertex #0
//!\param   v1 Vertex #1
//!\param   v2 Vertex #2
//!
//!\return  True if vertices form a polygon (CW order), false if it is a hole (CCW order)
////////////////////////////////////////////////////////////////////////////////////////////////////
geometry::Vec3 toEuler(const geometry::Quat &q)
{
    double sqx = q.x() * q.x();
    double sqy = q.y() * q.y();
    double sqz = q.z() * q.z();
    double sqw = q.w() * q.w();

    double unit = sqx + sqy + sqz + sqw;
    double test = q.x() * q.y() + q.z() * q.w();
    double x, y, z;

    if (test > 0.499)
    {
        y = 2 * std::atan2(q.x(), q.w());
        z = vpl::math::getPi<double>() * 0.5;
        x = 0.0;
    }
    else if (test < -0.499)
    {
        y = -2 * std::atan2(q.x(), q.w());
        z = -vpl::math::getPi<double>() * 0.5;
        x = 0.0;
    }
    else
    {
        y = atan2(2 * q.y() * q.w() - 2 * q.x() * q.z(), sqx - sqy - sqz + sqw);
        z = asin(2 * test / unit);
        x = atan2(2 * q.x() * q.w() - 2 * q.y() * q.z(), -sqx + sqy - sqz + sqw);
    }

    return geometry::Vec3(x, y, z);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Compares two matrices
//!
//!\param   a   Matrix a
//!\param   b   Matrix b
//!
//!\return  True if matrices are the same, false otherwise
////////////////////////////////////////////////////////////////////////////////////////////////////
bool compareMatrices(const geometry::Matrix &a, const geometry::Matrix &b)
{
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            if (a.at(y, x) != b.at(y, x))
            {
                return false;
            }
        }
    }

    return true;
}

bool isGuideTubeInCollisionWithMesh(geometry::CMesh& mesh, const geometry::Matrix& modelMatrix, const double GTOffset, const double GTHeight, const double GTDistance, const double GTAllowance, const double MCHeight, const double MCDiameter, const double MCLipWidth, const geometry::Vec3& IPosition, const geometry::Vec3& IDirection)
{
    // get implant position and direction in model coordinates
    geometry::Matrix invModelMat = geometry::Matrix::inverse(modelMatrix);
    geometry::Vec3 position(invModelMat * IPosition);
    geometry::Vec3 translation(invModelMat.getTrans<double>());
    invModelMat.setTrans(translation);
    geometry::Vec3 direction((invModelMat * IDirection) - translation);
    direction.normalize();

    // compute center point and up vector
    geometry::Vec3 center = position - direction * geometry::Scalar(GTOffset);
    geometry::Vec3 upVector((invModelMat * geometry::Vec3(0.0, 0.0, 1.0)) - translation);
    upVector.normalize();

    // Compute dimmension of first collision cylinder (inside of guide tube)
    double cyl1radius = (MCDiameter + GTDistance) * 0.5;
    double cyl1height = std::max(0.0, GTHeight - GTAllowance);
    geometry::Vec3 cyl1center = position - direction * geometry::Scalar(GTOffset - (cyl1height * 0.5));

    // Compute dimmension of second collision cylinder (on top of guide tube)
    double cyl2radius = ((MCDiameter + GTDistance) * 0.5) + MCLipWidth;
    double cyl2height = MCHeight;
    geometry::Vec3 cyl2center = position - direction * geometry::Scalar(GTOffset + (cyl2height * 0.5));

    // Compute bounding box of collision cylinders
    geometry::Vec3 bbcenter = center;
    double bbhside = std::sqrt((cyl2radius * cyl2radius) + (cyl2height * cyl2height));
    geometry::Vec3 bbhdist(bbhside, bbhside, bbhside);
    osg::BoundingBox bb(geometry::convert3<osg::Vec3>(bbcenter - bbhdist), geometry::convert3<osg::Vec3>(bbcenter + bbhdist));

    if (nullptr == mesh.getOctree())
    {
        assert(false); // bug?
        return false;
    }

    // get nodes from octree
    const std::vector<geometry::CMeshOctreeNode *> &nodes = mesh.getOctree()->getIntersectedNodes(bb);

    // get triangles from octree
    std::vector<geometry::CMesh::FaceHandle> faceHandles;

    // use reverse iterators because the last nodes are the leaf nodes and should include closer triangles 
    std::vector<geometry::CMeshOctreeNode *>::const_reverse_iterator itn = nodes.rbegin(), itnEnd = nodes.rend();
    for (; itn != itnEnd; ++itn)
    {
        faceHandles.insert(faceHandles.end(), (*itn)->faces.begin(), (*itn)->faces.end());
    }

    bool intersectionFound = false;
    // For all faces
    int facesSize = static_cast<int>(faceHandles.size());
#pragma omp parallel for
    for (int i = 0; i < facesSize; ++i)
    {
        // check if intersection was already found
#pragma omp flush (intersectionFound)
        if (!intersectionFound)
        {
            geometry::CMesh::FaceHandle fh = faceHandles[i];
            std::vector<geometry::Vec3> triangle;
            triangle.reserve(3);

            // Circulate around face and save all vertices into triangles vector
            for (geometry::CMesh::ConstFaceVertexIter fv_it = mesh.cfv_iter(fh); fv_it.is_valid(); ++fv_it)
            {
                geometry::CMesh::Point p = mesh.point(*fv_it);
                triangle.push_back(geometry::convert3<geometry::Vec3>(p));
            }

            //check if face intersects any ot the cylinders
            if (geometry::triangleCylinderIntersection(triangle, cyl1center, direction, cyl1height, cyl1radius))
            {
                intersectionFound = true;
#pragma omp flush (intersectionFound)
                continue;
            }

            if (geometry::triangleCylinderIntersection(triangle, cyl2center, direction, cyl2height, cyl2radius))
            {
                intersectionFound = true;
#pragma omp flush (intersectionFound)
                continue;
            }
        }
    }

    // if some intersection was found
    if (intersectionFound)
        return true;

    // triangleCylinderIntersection handles only mesh surface, we approximate the metal cylinder as one point and test if it lies inside of the mesh
    if (geometry::isPointInsideMesh(mesh, center, upVector))
        return true;

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Generates orthonormal basis
//! The input vector must be normalized to create orthonormal basis
//!
//!\param   u first output base vector
//!\param   v second output base vector
//!\param   w input base vector
//!
////////////////////////////////////////////////////////////////////////////////////////////////////
void generateOrthonormalBasis(geometry::Vec3 &u, geometry::Vec3 &v, const geometry::Vec3 &w)
{
    double invLength;

    if (std::fabs(w[0]) >= std::fabs(w[1]))
    {
        // W.x or W.z is the largest magnitude component, swap them
        invLength = 1.0 / std::sqrt(w[0] * w[0] + w[2] * w[2]);
        u[0] = -w[2] * invLength;
        u[1] = 0.0;
        u[2] = +w[0] * invLength;
        v[0] =  w[1] * u[2];
        v[1] =  w[2] * u[0] - w[0] * u[2];
        v[2] = -w[1] * u[0];
    }
    else
    {
        // W.y or W.z is the largest magnitude component, swap them
        invLength = 1.0 / std::sqrt(w[1] * w[1] + w[2] * w[2]);
        u[0] = 0.0;
        u[1] = +w[2] * invLength;
        u[2] = -w[1] * invLength;
        v[0] =  w[1] * u[2] - w[2] * u[1];
        v[1] = -w[0] * u[2];
        v[2] =  w[0] * u[1];
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks i there is any intersection between cylinder and triangle
//!
//!\param   triangle            Vector with three vertices of triangle
//!\param   cylinderCenter      Center of cylinder
//!\param   cylinderDirection   orientation of cylinder
//!\param   cylinderHeight      height of cylinder
//!\param   cylinderRadius      radius of cylinder
//!
//!\return  True if there is any intersection found
////////////////////////////////////////////////////////////////////////////////////////////////////
bool triangleCylinderIntersection(const std::vector<geometry::Vec3> &triangle, const geometry::Vec3 &cylinderCenter, const geometry::Vec3 &cylinderDirection, const double cylinderHeight, const double cylinderRadius)
{
    if (triangle.size() < 3)
        return false;

    // Get an orthonormal basis {U,V,D}
    geometry::Vec3 U, V, D = cylinderDirection;
    D.normalize();
    generateOrthonormalBasis(U, V, D);

    // Compute coordinates of P[i] in system {C;U,V,D}.
    geometry::Vec3 temp[3];
    int i;
    for (i = 0; i < 3; ++i)
    {
        geometry::Vec3 delta = triangle[i] - cylinderCenter;
        temp[i][0] = U * delta;
        temp[i][1] = V * delta;
        temp[i][2] = cylinderDirection * delta;
    }

    // Sort by z-value.
    int j0, j1, j2;
    if (temp[0][2] < temp[1][2])
    {
        if (temp[2][2] < temp[0][2])
        {
            j0 = 2; j1 = 0; j2 = 1;
        }
        else if (temp[2][2] < temp[1][2])
        {
            j0 = 0; j1 = 2; j2 = 1;
        }
        else
        {
            j0 = 0; j1 = 1; j2 = 2;
        }
    }
    else
    {
        if (temp[2][2] < temp[1][2])
        {
            j0 = 2; j1 = 1; j2 = 0;
        }
        else if (temp[2][2] < temp[0][2])
        {
            j0 = 1; j1 = 2; j2 = 0;
        }
        else
        {
            j0 = 1; j1 = 0; j2 = 2;
        }
    }

    // Maintain the xy-components and z-components separately.  The
    // z-components are used for clipping against bottom and top of
    // cylinder.  The xy-components are used for containment tests
    // in disk x*x+y*y <= r*r.
    std::vector<geometry::Vec2> Q(3);
    Q[0] = geometry::Vec2(temp[j0][0], temp[j0][1]);
    Q[1] = geometry::Vec2(temp[j1][0], temp[j1][1]);
    Q[2] = geometry::Vec2(temp[j2][0], temp[j2][1]);

    // From sorting we know that z[0] <= z[1] <= z[2].
    double z[3] =
    {
        temp[j0][2],
        temp[j1][2],
        temp[j2][2]
    };

    // Fast no-intersection.
    const double hhalf = 0.5f*cylinderHeight;
    if (z[2] < -hhalf || z[0] > hhalf)
    {
        // Triangle strictly below or above the cylinder.
        return false;
    }

    // Fast no-clipping.
    if (-hhalf <= z[0] && z[2] <= hhalf)
    {
        return diskOverlapsPolygon(cylinderRadius, Q);
    }

    // Clip against |z| <= h/2.  At this point we know that z2 >= -h/2 and
    // z0 <= h/2 with either z0 < -h/2 or z2 > h/2 (or both).
    std::vector<geometry::Vec2> polygon;
    polygon.reserve(5);
    double t, numer0, numer1, invDenom0, invDenom1;

    if (z[0] < -hhalf)
    {
        if (z[2] > hhalf)
        {
            if (z[1] >= hhalf)
            {
                numer0 = -hhalf - z[0];
                numer1 = +hhalf - z[0];
                invDenom0 = (1.0) / (z[1] - z[0]);
                invDenom1 = (1.0) / (z[2] - z[0]);
                t = numer0 * invDenom1;
                polygon.push_back(Q[0] + ((Q[2] - Q[0]) * vpl::CScalar<double>(t)));
                t = numer0 * invDenom0;
                polygon.push_back(Q[0] + ((Q[1] - Q[0]) * vpl::CScalar<double>(t)));
                t = numer1 * invDenom0;
                polygon.push_back(Q[0] + ((Q[1] - Q[0]) * vpl::CScalar<double>(t)));
                t = numer1 * invDenom1;
                polygon.push_back(Q[0] + ((Q[2] - Q[0]) * vpl::CScalar<double>(t)));
                polygon.shrink_to_fit();
                return diskOverlapsPolygon(cylinderRadius, polygon);
            }
            else if (z[1] <= -hhalf)
            {
                numer0 = -hhalf - z[2];
                numer1 = +hhalf - z[2];
                invDenom0 = (1.0) / (z[1] - z[2]);
                invDenom1 = (1.0) / (z[0] - z[2]);
                t = numer0 * invDenom1;
                polygon.push_back(Q[2] + ((Q[0] - Q[2]) * vpl::CScalar<double>(t)));
                t = numer0 * invDenom0;
                polygon.push_back(Q[2] + ((Q[1] - Q[2]) * vpl::CScalar<double>(t)));
                t = numer1 * invDenom0;
                polygon.push_back(Q[2] + ((Q[1] - Q[2]) * vpl::CScalar<double>(t)));
                t = numer1 * invDenom1;
                polygon.push_back(Q[2] + ((Q[0] - Q[2]) * vpl::CScalar<double>(t)));
                polygon.shrink_to_fit();
                return diskOverlapsPolygon(cylinderRadius, polygon);
            }
            else
            {
                numer0 = -hhalf - z[0];
                numer1 = +hhalf - z[2];
                invDenom0 = (1.0) / (z[1] - z[0]);
                invDenom1 = (1.0) / (z[2] - z[0]);
                t = numer0 * invDenom1;
                polygon.push_back(Q[0] + ((Q[2] - Q[0]) * vpl::CScalar<double>(t)));
                t = numer0 * invDenom0;
                polygon.push_back(Q[0] + ((Q[1] - Q[0]) * vpl::CScalar<double>(t)));
                polygon.push_back(Q[1]);
                invDenom0 = (1.0) / (z[1] - z[2]);
                invDenom1 = (1.0) / (z[0] - z[2]);
                t = numer1 * invDenom0;
                polygon.push_back(Q[2] + ((Q[1] - Q[2]) * vpl::CScalar<double>(t)));
                t = numer1 * invDenom1;
                polygon.push_back(Q[2] + ((Q[0] - Q[2]) * vpl::CScalar<double>(t)));
                polygon.shrink_to_fit();
                return diskOverlapsPolygon(cylinderRadius, polygon);
            }
        }
        else if (z[2] > -hhalf)
        {
            if (z[1] <= -hhalf)
            {
                numer0 = -hhalf - z[2];
                invDenom0 = (1.0) / (z[1] - z[2]);
                invDenom1 = (1.0) / (z[0] - z[2]);
                t = numer0 * invDenom0;
                polygon.push_back(Q[2] + ((Q[1] - Q[2]) * vpl::CScalar<double>(t)));
                t = numer0 * invDenom1;
                polygon.push_back(Q[2] + ((Q[0] - Q[2]) * vpl::CScalar<double>(t)));
                polygon.push_back(Q[2]);
                polygon.shrink_to_fit();
                return diskOverlapsPolygon(cylinderRadius, polygon);
            }
            else
            {
                numer0 = -hhalf - z[0];
                invDenom0 = (1.0) / (z[2] - z[0]);
                invDenom1 = (1.0) / (z[1] - z[0]);
                t = numer0 * invDenom0;
                polygon.push_back(Q[0] + ((Q[2] - Q[0]) * vpl::CScalar<double>(t)));
                t = numer0 * invDenom1;
                polygon.push_back(Q[0] + ((Q[1] - Q[0]) * vpl::CScalar<double>(t)));
                polygon.push_back(Q[1]);
                polygon.push_back(Q[2]);
                polygon.shrink_to_fit();
                return diskOverlapsPolygon(cylinderRadius, polygon);
            }
        }
        else
        {
            if (z[1] < -hhalf)
            {
                return diskOverlapsPoint(cylinderRadius, Q[2]);
            }
            else
            {
                return diskOverlapsSegment(cylinderRadius, Q[2], Q[1]);
            }
        }
    }
    else if (z[0] < hhalf)
    {
        if (z[1] >= hhalf)
        {
            numer0 = hhalf - z[0];
            invDenom0 = (1.0) / (z[2] - z[0]);
            invDenom1 = (1.0) / (z[1] - z[0]);
            t = numer0 * invDenom0;
            polygon.push_back(Q[0] + ((Q[2] - Q[0]) * vpl::CScalar<double>(t)));
            t = numer0 * invDenom1;
            polygon.push_back(Q[0] + ((Q[1] - Q[0]) * vpl::CScalar<double>(t)));
            polygon.push_back(Q[0]);
            polygon.shrink_to_fit();
            return diskOverlapsPolygon(cylinderRadius, polygon);
        }
        else
        {
            numer0 = hhalf - z[2];
            invDenom0 = (1.0) / (z[1] - z[2]);
            invDenom1 = (1.0) / (z[0] - z[2]);
            t = numer0 * invDenom0;
            polygon.push_back(Q[2] + ((Q[1] - Q[2]) * vpl::CScalar<double>(t)));
            t = numer0 * invDenom1;
            polygon.push_back(Q[2] + ((Q[0] - Q[2]) * vpl::CScalar<double>(t)));
            polygon.push_back(Q[0]);
            polygon.push_back(Q[1]);
            polygon.shrink_to_fit();
            return diskOverlapsPolygon(cylinderRadius, polygon);
        }
    }
    else
    {
        if (z[1] > hhalf)
        {
            return diskOverlapsPoint(cylinderRadius, Q[0]);
        }
        else
        {
            return diskOverlapsSegment(cylinderRadius, Q[0], Q[1]);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if disk overlaps point
//!
//!\param   diskRadius  Radius of a disk centered at 0,0
//!\param   point       Point
//!
//!\return  True if there is any overlaping parts between disc and point
////////////////////////////////////////////////////////////////////////////////////////////////////
bool diskOverlapsPoint(const double diskRadius, const geometry::Vec2 &point)
{
    return point[0] * point[0] + point[1] * point[1] <= diskRadius * diskRadius;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Computes dot product with perpendicular vector: v0 dot perp(v1)
//!
//!\param   v0  Vector #0
//!\param   v1  Vector #1
//!
//!\return  Result
////////////////////////////////////////////////////////////////////////////////////////////////////
double dotPerp(const geometry::Vec2 &v0, const geometry::Vec2 &v1)
{
    return v0[0] * v1[1] - v0[1] * v1[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if disk overlaps line segment
//!
//!\param   diskRadius  Radius of a disk centered at 0,0
//!\param   point0      First endpoint of line segment
//!\param   point1      Second endpoint of line segment
//!
//!\return  True if there is any overlaping parts between disc and line segment
////////////////////////////////////////////////////////////////////////////////////////////////////
bool diskOverlapsSegment(const double diskRadius, const geometry::Vec2 &point0, const geometry::Vec2 &point1)
{
    double rSqr = diskRadius * diskRadius;
    geometry::Vec2 D = point0 - point1;
    double dot = point0 * D;
    if (dot <= 0.0)
    {
        return point0 * point0 <= rSqr;
    }

    double lenSqr = D * D;
    if (dot >= lenSqr)
    {
        return point1 * point1 <= rSqr;
    }

    dot = dotPerp(D, point0);
    return dot * dot <= lenSqr * rSqr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Checks if disk overlaps polygon
//!
//!\param   diskRadius  radius of a disk centered at 0,0
//!\param   polygon     vertices of a polygon
//!
//!\return  True if there is any overlaping parts between disc and polygon
////////////////////////////////////////////////////////////////////////////////////////////////////
bool diskOverlapsPolygon(const double diskRadius, const std::vector<geometry::Vec2> &polygon)
{
    // Test whether the polygon contains (0,0).
    int positive = 0, negative = 0;
    int i0, i1;
    int numVertices = polygon.size();
    for (i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        double dot = dotPerp(polygon[i0], (polygon[i0] - polygon[i1]));
        if (dot > 0.0)
        {
            ++positive;
        }
        else if (dot < 0.0)
        {
            ++negative;
        }
    }
    if (positive == 0 || negative == 0)
    {
        // The polygon contains (0,0), so the disk and polygon overlap.
        return true;
    }

    // Test whether any edge is overlapped by the polygon.
    for (i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        if (diskOverlapsSegment(diskRadius, polygon[i0], polygon[i1]))
        {
            return true;
        }
    }

    return false;
}

} // namespace geometry
