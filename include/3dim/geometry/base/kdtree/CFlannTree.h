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

#pragma once
#ifndef CFlannTree_H_included
#define CFlannTree_H_included

#include <flann/flann.h>
#include <geometry/base/CMesh.h>
#include <memory>

namespace geometry
{

    class CFlannTree
        : public vpl::base::CObject
    {
    public:
        VPL_SHAREDPTR(CFlannTree);

    private:
        //! KD tree type
        typedef flann::Index<flann::L2<double> > tIndexTree;

        //! Pointer on the index tree type
        typedef std::unique_ptr<tIndexTree> tIndexTreePtr;

        //! Vector of double values type
        typedef std::vector<double> tDoubleVec;

        //! Vector of indices type
        typedef std::vector<size_t> tIndexVec;

        //! Flann doubles matrix type
        typedef flann::Matrix<double> tFlannDMatrix;

        //! Flann indices matrix type
        typedef flann::Matrix<size_t> tFlannIMatrix;
    public:
        //! Simple constructor (without initialization)
        CFlannTree() {}

        //! Initializing constructor - by mesh.
        CFlannTree(const CMesh &mesh) { init(mesh); }

        //! Initializing constructor - by array of points.
        CFlannTree(const Vec3Array &points) { init(points); }

        //! Destructor
        virtual ~CFlannTree() {}

        //! Initialize by mesh. Index returned with search methods is order number of the vertex in the mesh.
        virtual bool init(const CMesh &mesh) { return init(meshToPoints(mesh)); }

        //! Initialize by array of points. Index returned with search method is position of the point in this array.
        virtual bool init(const Vec3Array &points);

        /*!
         * \fn  int CFlannTree::nearestSearch(const Vec3 &point, tDoubleVec &distances, tIndexVec &indexes, size_t maximal_points_returned);
         *
         * \brief   Nearest point(s) search
         *
         * \param           point                   The point.
         * \param [in,out]  distances               The found points distances.
         * \param [in,out]  indexes                 The found points indexes.
         * \param           maximal_points_returned The maximal number points that can be returned.
         *
         * \return  An number of points found (input vectors are resized to this value).
         */
        int nearestSearch(const Vec3 &point, tDoubleVec &distances, tIndexVec &indexes, size_t maximal_points_returned) const;

        /*!
         * \fn  int CFlannTree::radiusSearch(const Vec3 &point, double radius, tDoubleVec &distances, tIndexVec &indexes, size_t maximal_points_returned);
         *
         * \brief   Search for points in the given radius from the point.
         *
         * \param           point                   The point.
         * \param           radius                  The radius.
         * \param [in,out]  distances               The found points distances.
         * \param [in,out]  indexes                 The found points indexes.
         * \param           maximal_points_returned The maximal number of points that can be returned.
         *
         * \return  An number of points found (input vectors are resized to this value).
         */
        int radiusSearch(const Vec3 &point, double radius, tDoubleVec &distances, tIndexVec &indexes, size_t maximal_points_returned) const;

        //! Does tree contain any data (is initialized)?
        bool hasData() const { return m_tree.get() != nullptr && m_tree->size() > 0; }

        //! Clear all data (and delete tree).
        virtual void clear();

        //! Get size of tree (number of elements stored).
        size_t size() const { if (!hasData()) return 0; return m_tree->size(); }

    private:


        //! Convert mesh to the array of points
        Vec3Array meshToPoints(const CMesh &mesh) const;

        //! Convert geometry vector to the vector of double values
        tDoubleVec geometryVecToDoubles(const Vec3 &point) const;

        //! Convert vector of double values to the flann matrix format
        tFlannDMatrix doublesToFlann(tDoubleVec &data, size_t rows) const;

        //! Convert vector of indices to the Flann matrix format
        tFlannIMatrix indicesToFlann(tIndexVec &data, size_t rows) const;

    private:
        tIndexTreePtr m_tree;
    };

    typedef CFlannTree::tSmartPtr CFlannTreePtr;
} // namespace geometry

// CFlannTree_H_included
#endif