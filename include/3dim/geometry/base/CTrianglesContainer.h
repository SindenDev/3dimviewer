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

#ifndef CTrianglesContainer_H
#define CTrianglesContainer_H

#include <geometry/base/types.h>

namespace geometry
{

//! Container for storing vertices and indices for some geometry.
//! Now used for region 3D preview.
class CTrianglesContainer
{

public:
	//! Constructor.
	CTrianglesContainer();

	//! Destructor.
	~CTrianglesContainer();

    void init();

    //! Adds vertex.
    //! \param x, y, z Vertex coordinates.
    void addVertex(double x, double y, double z);

    //! Adds index.
    //! \param index Vertex index. Must be >= 0.
    void addIndex(int index);

    //! Returns vector of vertices.
    //! \return Const reference to vector of vertices.
    const std::vector<geometry::Vec3>& getVertices();

    //! Returns vector of indicies.
    //! \return Const reference to vector of indicies.
    const std::vector<int>& getIndicies();

private:
    //! Vector of vertices.
    std::vector<geometry::Vec3> m_vertices;

    //! Vector of indicies.
    std::vector<int> m_indicies;
};


} // namespace geometry

#endif // CTrianglesContainer_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
