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


#include <geometry/base/CPlane.h>

/**
 * \brief   Sets plane parameters - from normal and point.
 *
 * \param   normal  The normal.
 * \param   point   The point.
 */
void geometry::CPlane::set(const Vec3 &normal, const Vec3 &point)
{
    // Compute last parameter
    double d(-normal[0]*point[0] - normal[1]*point[1] - normal[2]*point[2]);
    set(normal[0], normal[1], normal[2],d);
}

/**
 * \brief   Sets plane parameters - from three (not colinear) points.
 *
 * \param   v0  The point.
 * \param   v1  The point.
 * \param   v2  The point.
 */

void geometry::CPlane::set(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2)
{
    // Compute normal vector from cross product
    Vec3 norm((v1-v0)^(v2-v1));

    double length(norm.length());
    VPL_ASSERT(length>1e-6);
    norm /= vpl::CScalar<double>(length);

    // Set values
    set(Vec4(norm[0],norm[1],norm[2],-(v0*norm)));
}

/**
 * \brief   == casting operator.
 *
 * \param   p   The const CPlane &amp; to test.
 *
 * \return  The result of the operation.
 */
bool geometry::CPlane::operator==(const CPlane &p)
{
    return p.m_p == m_p;
}

/**
* \brief   Calculates if vertex array intersects plane.
*          Returns 1 if vertices are completely above the plane,
*           0 if they intersects or
*          -1 if vertices are completely below the plane.Intersects.
 *
 * \param   vertices    The vertices array.
 *
 * \return  1/0/-1.
 */
int geometry::CPlane::intersect(const Vec3Array &vertices) const
{
    // If no vertices, return -1
    if(vertices.empty())
        return -1;

    size_t num_above(0), num_below(0), num_on(0);

    // For all vertices
    Vec3Array::const_iterator it(vertices.begin()), itEnd(vertices.end());
    for(; it != itEnd; ++it)
    {
        // Compute point distance
        double d(distance(*it));

        // Test with plane
        if(d < 0.0) 
            ++num_below;
        else if(d > 0.0)
            ++num_above;
        else
            ++num_on;
    }

    if(num_on > 0)
        return 0;

    if(num_below > 0)
    {
        if(num_above > 0)
            return 0;   // Some are above, some below

        return -1;  // Anything is below and nothing is above - all is below
    }

    return 1;   // Nothing is below and nothing is on - all is above
}
