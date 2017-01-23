///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2014-2016 3Dim Laboratory s.r.o.
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

#ifndef CBoundingBox_H_included
#define CBoundingBox_H_included

#include "types.h"

namespace geometry
{
/** 
 * \brief Class declares axis aligned bounding box
 */
class CBoundingBox : public vpl::base::CObject
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CBoundingBox);

    //! Minimum and maximum corners
    Vec3 _min, _max;

    //! Simple constructor initializes invalid bounding box
    CBoundingBox() : _min(MAX_VECTOR), _max(MIN_VECTOR) { }

    //! Copy constructor
    CBoundingBox(const CBoundingBox &b) : _min(b._min), _max(b._max) { }

    //! Initializing constructor - by two vectors
    CBoundingBox(const Vec3 &min, const Vec3 &max) : _min(min), _max(max) { }

    //! Initializing constructor - by six values
    CBoundingBox(double xmin, double ymin, double zmin, double xmax, double ymax, double zmax) : _min(xmin, ymin, zmin), _max(xmax, ymax, zmax) { }

    //! Initialize bounding box to the invalid state
    void init() {_min = MAX_VECTOR; _max = MIN_VECTOR; }

    //! Initialize bounding box by two vectors
    void set(const Vec3 &min, const Vec3 &max) { _min = min; _max = max; }

    //! Initialize bounding box by six values
    void set(double xmin, double ymin, double zmin, double xmax, double ymax, double zmax) { _min = Vec3(xmin, ymin, zmin); _max = Vec3(xmax, ymax, zmax); }

    //! Access components
    double xMin() const { return _min[0]; }
    double &xMin() { return _min[0]; }

    double xMax() const { return _max[0]; }
    double &xMax() { return _max[0]; }

    double yMin() const { return _min[1]; }
    double &yMin() { return _min[1]; }

    double yMax() const { return _max[1]; }
    double &yMax() { return _max[1]; }

    double zMin() const { return _min[2]; }
    double &zMin() { return _min[2]; }

    double zMax() const { return _max[2]; }
    double &zMax() { return _max[2]; }

    //! Test if bounding box is valid
    inline bool valid() const { return _max[0] >= _min[0] &&  _max[1] >= _min[1] &&  _max[2] >= _min[2];
    }

    //! Calculate bounding box center
    Vec3 center() const { return (_max + _min) * 0.5; }

    //! Calculate bounding box radius (radius of the minimal possible bounding sphere)
    double radius() const { return sqrtf(radius2()); }

    //! Calculate bounding box squared length of radius (radius of the minimal possible bounding sphere)
    double radius2() const { return ((_max-_min).length2()) * 0.25; }

    //! Expand bounding box by vector
    inline void expandBy(const Vec3 &v) 
    {
        _min[0] = std::min(_min[0], v[0]); _min[1] = std::min(_min[1], v[1]); _min[2] = std::min(_min[2], v[2]); 
        _max[0] = std::max(_max[0], v[0]); _max[1] = std::max(_max[1], v[1]); _max[2] = std::max(_max[2], v[2]); 
    }

    //! Expand bounding box by vector given by three values
    inline void expandBy(double x, double y, double z) { expandBy(Vec3(x, y, z)); }

    //! Expand by another bounding box
    inline void expandBy(const CBoundingBox &bb)
    {
        _min[0] = std::min(_min[0], bb._min[0]); _min[1] = std::min(_min[1], bb._min[1]); _min[2] = std::min(_min[2], bb._min[2]); 
        _max[0] = std::max(_max[0], bb._max[0]); _max[1] = std::max(_max[1], bb._max[1]); _max[2] = std::max(_max[2], bb._max[2]); 
    }

    //! Compute common part of two bounding boxes
    CBoundingBox intersect(const CBoundingBox &bb) const
    {
        // If any of boxes invalid, return the second one
        if(!bb.valid())
            return *this;
        if(!valid())
            return bb;

        // Compute common part
        return CBoundingBox( std::max(_min[0], bb._min[0]), std::max(_min[1], bb._min[1]), std::max(_min[2], bb._min[2]),
                             std::min(_min[0], bb._min[0]), std::min(_min[1], bb._min[1]), std::min(_min[2], bb._min[2]));
    }

    //! Test if two bounding boxes intersect
    bool intersects(const CBoundingBox &bb) const 
    {    
        return  std::max(_min[0], bb._min[0]) <= std::min(_max[0],bb._max[0]) &&
                std::max(_min[1], bb._min[1]) <= std::min(_max[1],bb._max[1]) &&
                std::max(_min[2], bb._min[2]) <= std::min(_max[2],bb._max[2]);

    }

    //! Test if point is inside bounding box
    bool contains(const Vec3 &p)
    {
        return valid() &&
            (p[0] >= _min[0] && p[0] <= _max[0]) &&
            (p[1] >= _min[1] && p[1] <= _max[1]) &&
            (p[2] >= _min[2] && p[2] <= _max[2]);
    }

    //! Test if point is inside or near bounding box
    bool contains(const Vec3 &p, double epsilon)
    {
        return valid() &&
            ((p[0] >= _min[0] - epsilon) && (p[0] <= _max[0] + epsilon)) &&
            ((p[1] >= _min[1] - epsilon) && (p[1] <= _max[1] + epsilon)) &&
            ((p[2] >= _min[2] - epsilon) && (p[2] <= _max[2] + epsilon));
    }

    //! Compare bounding boxes
    bool operator == (const CBoundingBox &bb) { return _min == bb._min && _max == bb._max; }
    bool operator != (const CBoundingBox &bb) { return _min != bb._min || _max != bb._max; }

}; // class CBoundingBox

}

// CBoundingBox_H_included
#endif