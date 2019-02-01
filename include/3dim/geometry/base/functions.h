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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

////////////////////////////////////////////////////////////
// include
#include <geometry/base/types.h>
#include <geometry/base/CPlane.h>
#include <geometry/base/CMesh.h>

namespace geometry
{
    int isnan(double x);
    int isnan(const geometry::Vec2 &pt);
    int isnan(const geometry::Vec3 &pt);
    bool triOnPlane(const geometry::CPlane &plane, const geometry::Vec3 &pointA, const geometry::Vec3 &pointB, const geometry::Vec3 &pointC, double snapThreshold);
    bool checkSelfIntersections(std::vector<geometry::Vec2> &polygon, double snapThreshold, double angleThreshold);
    bool calculateIntersection2d(const geometry::Vec2 &l0a, const geometry::Vec2 &l0b, const geometry::Vec2 &l1a, const geometry::Vec2 &l1b, geometry::Vec2 &i, double snapThreshold, double angleThreshold, bool includePoints = true);
    bool calculateIntersection3d(const geometry::Vec3 &l0a, const geometry::Vec3 &l0b, const geometry::Vec3 &l1a, const geometry::Vec3 &l1b, geometry::Vec3 &i, double snapThreshold, double angleThreshold);
    bool pointInPolygon(const geometry::Vec2 &p, const std::vector<geometry::Vec2> &polygon);
    double polygonArea(const std::vector<geometry::Vec2> &polygon);
    bool isPolygon(const std::vector<geometry::Vec2> &polygon);
    bool isPolygon(const geometry::Vec2 &v0, const geometry::Vec2 &v1, const geometry::Vec2 &v2);
    void dummyOffset(double offset, std::vector<geometry::Vec2> &polygon);
    geometry::Vec3 toVec3d(const geometry::Vec4 &v);
    geometry::Vec3 lerp(const geometry::Vec3 &v0, const geometry::Vec3 &v1, double w);
    void getLookAt(const geometry::Matrix &matrix, geometry::Vec3 &eye, geometry::Vec3 &center, geometry::Vec3 &up, double lookDistance = 1.0);
    // Principal Component Analysis
    void pca(const std::vector<geometry::Vec3>& pc, std::array<geometry::Vec3, 3>& eigenVectors, std::array<double, 3>& eigenValues);
    void getConnectedGeometryInRadius(const geometry::CMesh& mesh, const geometry::CMesh::VertexHandle seed, const geometry::Vec3 center, const float radius, std::vector<geometry::CMesh::VertexHandle>& handles, std::vector<geometry::Vec3>& neighbors, std::vector<double>& distances);
    geometry::Vec3 closestPointOnLine(const geometry::Vec3& pointOnLine, const geometry::Vec3& lineDirection, const geometry::Vec3& queryPoint, double& distance);
    geometry::Vec3 projectPointOnPlane(const geometry::Vec3& pointOnPlane, const geometry::Vec3& planeNormal, const geometry::Vec3& queryPoint, double& distance);
    bool linePlaneIntersection(const geometry::Vec3& pointOnPlane, const geometry::Vec3& planeNormal, const geometry::Vec3& pointOnLine, const geometry::Vec3& lineDirection, geometry::Vec3& intersectionPoint, double& distance);
    bool pointInTriangle(const geometry::Vec2& point, const std::vector<geometry::Vec2>& triangle);
    bool pointInTriangle(const geometry::Vec3& point, const std::vector<geometry::Vec3>& triangle, const geometry::Vec3& triangleNormal);
    bool lineTriangleIntersection(const std::vector<geometry::Vec3>& triangle, const geometry::Vec3& triangleNormal, const geometry::Vec3& pointOnLine, const geometry::Vec3& lineDirection, geometry::Vec3& intersectionPoint, double& distance);
    bool isPointInsideMesh(geometry::CMesh& mesh, const geometry::Vec3& point, const geometry::Vec3& upVector);
    geometry::CMesh* cutMeshByVerticesSubset(const geometry::CMesh& mesh, std::vector<geometry::CMesh::VertexHandle>& handles);
    geometry::Vec3 toEuler(const geometry::Quat &q);
    bool compareMatrices(const geometry::Matrix &a, const geometry::Matrix &b);
    bool isGuideTubeInCollisionWithMesh(geometry::CMesh& mesh, const geometry::Matrix &modelMatrix, const double GTOffset, const double GTHeight, const double GTDistance, const double GTAllowance, const double MCHeight, const double MCDiameter, const double MCLipWidth, const geometry::Vec3& IPosition, const geometry::Vec3& IDirection);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Geometric Tools, LLC
    // Copyright (c) 1998-2013
    // Distributed under the Boost Software License, Version 1.0.
    // http://www.boost.org/LICENSE_1_0.txt
    // http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
    //
    // File Version: 5.5.0 (2010/11/02)
    //
    // found on https://github.com/timprepscius/GeometricTools

    // Triangle cylinder intersection, algorithm description: https://www.geometrictools.com/Documentation/IntersectionTriangleCylinder.pdf
    bool triangleCylinderIntersection(const std::vector<geometry::Vec3> &triangle, const geometry::Vec3 &cylinderCenter, const geometry::Vec3 &cylinderDirection, const double cylinderHeight, const double cylinderRadius);
    // generate orthonormal basis from 1 vector, vector w must be normalized, otherwise it generates only complement basis
    void generateOrthonormalBasis(geometry::Vec3 &u, geometry::Vec3 &v, const geometry::Vec3 &w);
    // Support for the static test query.
    bool diskOverlapsPoint(const double diskRadius, const geometry::Vec2 &point);
    bool diskOverlapsSegment(const double diskRadius, const geometry::Vec2 &point0, const geometry::Vec2 &point1);
    bool diskOverlapsPolygon(const double diskRadius, const std::vector<geometry::Vec2> &polygon);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace geometry

#endif
