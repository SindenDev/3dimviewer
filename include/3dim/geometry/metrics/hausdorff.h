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

#pragma once
#include <3dim/geometry/base/CMesh.h>
#include <random>

namespace geometry
{
//! Hausdorff distance which compute difference between two meshes.
class HausdorffDistance
{
public:
    //! Constructor initialize default values for computing distance.
    HausdorffDistance(CMesh& meshA, CMesh& meshB, size_t sampleNumber);

    //! Enable vertex sampling for computation distance
    void enableVertexSampling(bool isEnabled);

    //! Should be use in computation vertex sampling?
    bool isEnabledVertexSampling() const;

    //! Enable face sampling for computation distance
    void enableFaceSampling(bool isEnabled);

    //! Should be use in computation face sampling?
    bool isEnabledFaceSampling() const;

    //! Set in once all sampling methods.
    void setSampleType(bool isVertexSampling, bool isFaceSampling);

    //! Set radius for search closest faces from point.
    void setDistanceSearch(double distance);

    //! Starting comparison of meshes and compute Hausdorff distance. 
    bool compute();

    //! Minimal different distance between sampled points on meshA and any points on meshB.
    double getMinDistance() const;
    //! Maximal different distance between sampled points on meshA and any points on meshB.
    double getMaxDistance() const;
    //! Mean different distance between sampled points on meshA and any points on meshB.
    double getMeanDistance() const;
    //! Root mean square distance between sampled points on meshA and any points on meshB.
    double getRmsDistance() const;

    //! Compute diagonal length of bounding box from mesh.
    double computeBBoxDiagonalLength(CMesh& mesh) const;


private:
    //! For the newly acquired sample, locate it on second mesh and compute distance.  
    //! Face sampling
    void addFaceSample(const CMesh::Point& point);

    //! For the newly acquired sample, locate it on second mesh and compute distance. 
    //! Vertex sampling
    void addSample(const CMesh::Point& point);

    //! Compute triangle area, used for random selection triangle in sampling.
    double getTriangleArea(CMesh::FaceHandle face);

    //! Get a sample from the vertices.
    bool vertexCompute();

    //! Get a sample from the faces.
    bool faceCompute();

    //! Points on the meshA are obtained using vertices.
    bool m_isVertexSample;

    //! Points on the meshA are obtained using face sampling.
    bool m_isFaceSample;

    //! Actual minimal distance.
    double m_min;
    //! Actual maximal distance.
    double m_max;
    //! Actual mean distance.
    double m_mean;
    //! Actual RMS distance.
    double m_rms;

    //! Maximal distance in searching closest points.
    double m_searchDistance;

    //! Count of sample points on meshA.
    size_t m_samplesCount;

    //! Upper bound for sampling points on meshA.
    size_t m_maxSamples;

    //! Save reference to meshA.
    CMesh m_meshA;

    //! Save reference to meshB.
    CMesh m_meshB;

    //! Octree reference which is used in face sampling.
    CMeshOctree* m_octree;

    //! Random generation barycentric coordinates on triangle, used in face sampling.
    CMesh::Point generateBarycentric();

    //! Compute barycentric coordinates from point in euclidean space and triangle
    CMesh::Point barycentric(const CMesh::Point& point, const CMesh::Point& a, const CMesh::Point& b, const CMesh::Point& c) const;

    //! Generators
    std::mt19937 m_mt;
    std::uniform_real_distribution<double> m_unifGenerate01;

};
}
