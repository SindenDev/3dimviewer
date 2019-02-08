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

#ifndef CMarchingCubesFast_H
#define CMarchingCubesFast_H

////////////////////////////////////////////////////////////
// Includes
#include <geometry/base/types.h>

// VPL
#include <VPL/Image/DensityVolume.h>
#include <VPL/Module/Serializable.h>
#include <VPL/Module/Progress.h>
#include <VPL/Image/Vector3.h>

// STL
#include <vector>
#include <set>
#include <map>

////////////////////////////////////////////////////////////
//! Fast marching cubes functor
class IMarchingCubesFastFunctor
{
public:
    virtual unsigned char operator()(int x, int y, int z) const = 0;
    virtual vpl::img::CSize3i getVolumeDimensions() const = 0;
    virtual vpl::img::CSize3d getVoxelSize() const = 0;
};

////////////////////////////////////////////////////////////
//! Marching cubes worker - MC algorithm implementation class.
//! Simplified MC algorithm, that only processes given volume and fills coords a indicies vectors.
//! Doesn't use OpenMesh.
class CMarchingCubesWorkerFast
{
private:
    vpl::img::CSize3d m_voxelSize;

    int m_index;

    //! Pointer on up codes of cubes.
    unsigned char *m_cube_code_matrix;

    //! Pointer on down codes of cubes.
    unsigned char *m_down_cube_code_matrix;

    //! Pointer on voxels state matrix, up and down.
    unsigned char *m_state_matrix_up;
    unsigned char *m_state_matrix_down;

    //! Actual cube vertices coordinates.
    geometry::Vec3 m_cube_vertices[8];

    //! Pointer on up work matrices of edges vertices and indicies.
    bool *m_node_matrix_up_h;
    bool *m_node_matrix_up_v;
    int *m_node_matrix_up_h_index;
    int *m_node_matrix_up_v_index;

    //! Pointer on down work matrices of edges vertices and indicies.
    bool *m_node_matrix_down_h;
    bool *m_node_matrix_down_v;
    int *m_node_matrix_down_h_index;
    int *m_node_matrix_down_v_index;

    //! Pointer on middle work matrices of edges vertices and indicies.
    bool *m_node_matrix_middle;
    int *m_node_matrix_middle_index;

    //! Size of help work matrices.
    int m_work_matrices_size_x;
    int m_work_matrices_size_y;

    //! actual volume grid size
    vpl::tSize m_volume_start_x;
    vpl::tSize m_volume_start_y;
    vpl::tSize m_volume_start_z;
    vpl::tSize m_volume_end_x;
    vpl::tSize m_volume_end_y;
    vpl::tSize m_volume_end_z;

public:
    CMarchingCubesWorkerFast();
    ~CMarchingCubesWorkerFast();

    //! Set work area - in voxels.
    void setVolumeOfInterest(vpl::tSize startX, vpl::tSize startY, vpl::tSize startZ, vpl::tSize endX, vpl::tSize endY, vpl::tSize endZ);

    //! Marching cubes mesh generation.
    //! param xCoords Vector of x coordinates of final vertices.
    //! param yCoords Vector of y coordinates of final vertices.
    //! param zCoords Vector of z coordinates of final vertices.
    //! param indicies Vector of indicies marking final triangles.
    bool generateMesh(std::vector<double> &xCoords, std::vector<double> &yCoords, std::vector<double> &zCoords, std::vector<int> &indicies, const IMarchingCubesFastFunctor *volume_functor);

protected:
    //! (De)Allocation of work matrices.
    void allocWorktMem(int size_x, int size_y);
    void deallocWorktMem();

    //! Make tris for actual cube, given coordinates of processed planes and cube code.
    //! param xCoords Vector of x coordinates of final vertices.
    //! param yCoords Vector of y coordinates of final vertices.
    //! param zCoords Vector of z coordinates of final vertices.
    //! param indicies Vector of indicies marking final triangles.
    void makeTri(std::vector<double> &xCoords, std::vector<double> &yCoords, std::vector<double> &zCoords, std::vector<int> &indicies, int x, int y, unsigned char cube_code);

    void holeFilling(std::vector<double> &xCoords, std::vector<double> &yCoords, std::vector<double> &zCoords, std::vector<int> &indicies, int x, int y, unsigned char cube_code);

    //! Get/Set pointer on vertex of cube node by given coordinates and cube edge index.
    bool getCubeEdgeNode(int edge_index, int x, int y, int &index);
    void setCubeEdgeNode(int edge_index, int x, int y, bool new_vertex);

    //! Set cube node Z/Y/X coordinates.
    void setCodeCoordinateZ(double z, double dz);
    void setCodeCoordinateY(double y, double dy);
    void setCodeCoordinateX(double x, double dx);

    //! Get cube code of down/front/left cube.
    unsigned char getCubeCodeDown(int x, int y);
    unsigned char getCubeCodeFront(int x, int y);
    unsigned char getCubeCodeLeft(int x, int y);

    //! Create actual cube code and save it into code matrix
    unsigned char makeCubeCode(int x, int y);

    //! Calculate number of nodes for cube code.
    int cubeCodeNodeNumber(unsigned char cube_code);
};

////////////////////////////////////////////////////////////
//! Fast marching cubes class.
//! Generates mesh using parallel by using CMarchingCubesWorkerFast.
//! Simplified algorithm with no postprocessing.
//! Uses tpContainer for output and fill it with final vertices and indices of the triangles.
//! Output container must have methods addVertex(double, double, double) and addIndex(int).
template<class tpContainer>
class CMarchingCubesFast
{
private:
    //! actual volume voxel size
    vpl::img::CSize3d m_voxelSize;

public:
    //! Marching cubes class constructor.
    CMarchingCubesFast() {}

    //! Marching cubes class destructor.
    ~CMarchingCubesFast() {}

    //! Marching cubes triangle mesh generation.
    bool generateMesh(tpContainer *output, const IMarchingCubesFastFunctor *volumeFunctor)
    {
        const vpl::tSize desiredCubesOnEdge = 8;
        vpl::img::CSize3i volumeSize = volumeFunctor->getVolumeDimensions();

        vpl::img::CSize3i cubeSize;
        cubeSize.x() = (volumeSize.x() + desiredCubesOnEdge - 1) / desiredCubesOnEdge;
        cubeSize.y() = (volumeSize.y() + desiredCubesOnEdge - 1) / desiredCubesOnEdge;
        cubeSize.z() = (volumeSize.z() + desiredCubesOnEdge - 1) / desiredCubesOnEdge;

        vpl::img::CVector3i cubesOnEdge;
        cubesOnEdge.x() = volumeSize.x() / cubeSize.x() + 1;
        cubesOnEdge.y() = volumeSize.y() / cubeSize.y() + 1;
        cubesOnEdge.z() = volumeSize.z() / cubeSize.z() + 1;

        const int workerCount = cubesOnEdge.x() * cubesOnEdge.y() * cubesOnEdge.z();
        std::vector<CMarchingCubesWorkerFast> workers(workerCount);

        // initialize workers
        for (vpl::tSize z = 0; z < cubesOnEdge.z(); ++z)
        {
            for (vpl::tSize y = 0; y < cubesOnEdge.y(); ++y)
            {
                for (vpl::tSize x = 0; x < cubesOnEdge.x(); ++x)
                {
                    vpl::tSize index = z * cubesOnEdge.y() * cubesOnEdge.x() + y * cubesOnEdge.x() + x;
                    CMarchingCubesWorkerFast &worker = workers[index];
                    worker.setVolumeOfInterest(
                        std::min(x * cubeSize.x(), volumeSize.x()),
                        std::min(y * cubeSize.y(), volumeSize.y()),
                        std::min(z * cubeSize.z(), volumeSize.z()),
                        std::min((x + 1) * cubeSize.x(), volumeSize.x() + 1),
                        std::min((y + 1) * cubeSize.y(), volumeSize.y() + 1),
                        std::min((z + 1) * cubeSize.z(), volumeSize.z() + 1));
                }
            }
        }

        std::vector<std::vector<double> > xCoords;
        std::vector<std::vector<double> > yCoords;
        std::vector<std::vector<double> > zCoords;
        std::vector<std::vector<int> > indicies;
        xCoords.resize(workerCount);
        yCoords.resize(workerCount);
        zCoords.resize(workerCount);
        indicies.resize(workerCount);

        // generate submeshes
#pragma omp parallel for
        for (int i = 0; i < workerCount; ++i)
        {
            workers[i].generateMesh(xCoords[i], yCoords[i], zCoords[i], indicies[i], volumeFunctor);
        }

        int verticesSum = 0;

        for (int i = 0; i < workerCount; ++i)
        {
            size_t xCoordsSize = xCoords[i].size();
            size_t yCoordsSize = yCoords[i].size();
            size_t zCoordsSize = zCoords[i].size();
            assert(xCoordsSize == yCoordsSize && yCoordsSize == zCoordsSize);

            size_t indiciesSize = indicies[i].size();

            for (size_t x = 0; x < xCoordsSize; ++x)
            {
                output->addVertex(xCoords[i][x], yCoords[i][x], zCoords[i][x]);
            }

            for (size_t x = 0; x < indiciesSize; ++x)
            {
                output->addIndex(indicies[i][x] + verticesSum);
            }

            verticesSum += xCoordsSize;
        }

        return true;
    }
};

#endif // CMarchingCubesFast_H

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
