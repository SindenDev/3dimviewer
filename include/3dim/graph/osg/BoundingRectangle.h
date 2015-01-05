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

#ifndef BoundingRectangle_H
#define BoundingRectangle_H

#include <osg/Vec2d>

namespace osg
{
    class BoundingRectangle
    {
    private:
        bool m_valid;
        osg::Vec2d m_min;
        osg::Vec2d m_max;

    public:
        BoundingRectangle();
        BoundingRectangle(const BoundingRectangle &other);
        BoundingRectangle(const osg::Vec2d min, const osg::Vec2d max);
        ~BoundingRectangle();
        BoundingRectangle &operator=(const BoundingRectangle &other);
        void expandBy(const osg::Vec2d &point);
        bool intersects(const BoundingRectangle &other) const;
        bool contains(const osg::Vec2d &point) const;
        bool contains(const BoundingRectangle &rectangle) const;
        bool isValid() const;
        void invalidate();
        osg::Vec2d getMin() const;
        osg::Vec2d getMax() const;
    };
}

#endif // BoundingRectangle_H
