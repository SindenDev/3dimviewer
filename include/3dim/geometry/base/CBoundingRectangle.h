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

#ifndef CBoundingRectangle_H
#define CBoundingRectangle_H

#include <geometry/base/types.h>

namespace geometry
{
    class CBoundingRectangle
    {
    private:
        bool m_valid;
        geometry::Vec2 m_min;
        geometry::Vec2 m_max;

    public:
        CBoundingRectangle() : m_valid(false) { }
        CBoundingRectangle(const CBoundingRectangle &other) : m_valid(other.m_valid), m_min(other.m_min), m_max(other.m_max) { }
        CBoundingRectangle(const geometry::Vec2 &min, const geometry::Vec2 &max) : m_valid(false) { expandBy(min); expandBy(max); }
        ~CBoundingRectangle() { }
        CBoundingRectangle &operator=(const CBoundingRectangle &other) { if (this != &other) { m_valid = other.m_valid; m_min = other.m_min; m_max = other.m_max; } return *this; }
        void expandBy(const geometry::Vec2 &point) { if (!m_valid) { m_min = m_max = point; m_valid = true; } else { m_min.x() = std::min<double>(m_min.x(), point.x()); m_min.y() = std::min<double>(m_min.y(), point.y()); m_max.x() = std::max<double>(m_max.x(), point.x()); m_max.y() = std::max<double>(m_max.y(), point.y()); } }
        bool intersects(const CBoundingRectangle &other) const { return (m_valid && other.m_valid && !((m_max.x() < other.m_min.x()) || (m_min.x() > other.m_max.x()) || (m_max.y() < other.m_min.y()) || (m_min.y() > other.m_max.y()))); }
        bool contains(const geometry::Vec2 &point) const { return (m_valid && point.x() > m_min.x() && point.x() < m_max.x() && point.y() > m_min.y() && point.y() < m_max.y()); }
        bool contains(const CBoundingRectangle &rectangle) const { return (contains(rectangle.getMin()) && contains(rectangle.getMax())); }
        bool isValid() const { return m_valid; }
        void invalidate() { m_valid = false; }
        geometry::Vec2 getMin() const { return m_min; }
        geometry::Vec2 getMax() const { return m_max; }

		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };	
}

#endif // CBoundingRectangle_H
