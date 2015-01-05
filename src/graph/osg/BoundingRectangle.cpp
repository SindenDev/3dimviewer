///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#include <osg/BoundingRectangle.h>
#include <algorithm>

osg::BoundingRectangle::BoundingRectangle()
    : m_valid(false)
{ }

osg::BoundingRectangle::BoundingRectangle(const BoundingRectangle &other)
    : m_valid(other.m_valid)
    , m_min(other.m_min)
    , m_max(other.m_max)
{ }

osg::BoundingRectangle::~BoundingRectangle()
{ }

osg::BoundingRectangle::BoundingRectangle(const osg::Vec2d min, const osg::Vec2d max)
    : m_valid(false)
{
    expandBy(min);
    expandBy(max);
}

osg::BoundingRectangle &osg::BoundingRectangle::operator=(const BoundingRectangle &other)
{
    if (this == &other)
    {
        return *this;
    }

    m_valid = other.m_valid;
    m_min = other.m_min;
    m_max = other.m_max;

    return *this;
}

void osg::BoundingRectangle::expandBy(const osg::Vec2d &point)
{
    if (!m_valid)
    {
        m_min = m_max = point;
        m_valid = true;
    }
    else
    {
        m_min = osg::Vec2d(std::min(m_min.x(), point.x()), std::min(m_min.y(), point.y()));
        m_max = osg::Vec2d(std::max(m_max.x(), point.x()), std::max(m_max.y(), point.y()));
    }
}

bool osg::BoundingRectangle::intersects(const BoundingRectangle &other) const
{
    return (m_valid && other.m_valid && 
            !((m_max.x() < other.m_min.x()) || (m_min.x() > other.m_max.x()) || (m_max.y() < other.m_min.y()) || (m_min.y() > other.m_max.y())));
}

bool osg::BoundingRectangle::contains(const osg::Vec2d &point) const
{
    return (m_valid && 
            point.x() > m_min.x() && point.x() < m_max.x() && point.y() > m_min.y() && point.y() < m_max.y());
}

bool osg::BoundingRectangle::contains(const osg::BoundingRectangle &rectangle) const
{
    return (contains(rectangle.getMin()) && contains(rectangle.getMax()));
}

bool osg::BoundingRectangle::isValid() const
{
    return m_valid;
}

void osg::BoundingRectangle::invalidate()
{
    m_valid = false;
}

osg::Vec2d osg::BoundingRectangle::getMin() const
{
    return m_min;
}

osg::Vec2d osg::BoundingRectangle::getMax() const
{
    return m_max;
}

