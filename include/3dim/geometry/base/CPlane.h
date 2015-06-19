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


#ifndef Plane_H_included
#define Plane_H_included

#include "types.h"

namespace geometry
{
    class CPlane : public vpl::base::CObject
    {
    public:
        //! Smart pointer type.
        //! - Declares type tSmartPtr.
        VPL_SHAREDPTR(CPlane);
    
        //! Simple constructor
        CPlane() : m_p(0.0, 0.0, 0.0, 0.0) {}

        //! Copy constructor
        CPlane(const CPlane &p) { set(p); }

        //! Initializing constructor
        CPlane(const Vec4 &v) { set(v); }

        //! Initializing constructor
        CPlane(double a, double b, double c, double d) { set(Vec4(a, b, c, d)); }

        //! Initializing constructor
        CPlane(const Vec3 &normal, double d) { set(Vec4(normal[0], normal[1], normal[2], d)); }

        //! Initializing constructor - by three plane points
        CPlane(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2) { set(v0, v1, v2); }

        //! Initializing constructor - by normal and point
        CPlane(const Vec3 &normal, const Vec3 &point) { set(normal, point); }

        //! Assignment operator
        inline CPlane &operator=(const CPlane &p) {if(&p != this) set(p); return *this; }

        //! Initialize by 4D vector
        void set(const Vec4 &v) { m_p = v; }

        //! Initialize by four values
        void set(double a, double b, double c, double d) { m_p = Vec4(a, b, c, d); }

        //! Initialize by another plane
        void set(const CPlane &p) { m_p = p.m_p; }

        //! Initialize by normal and point
        void set(const Vec3 &normal, const Vec3 &point);

        //! Initialize by three (not collinear) points
        void set(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2);

        //! Flip plane orientation
        void flip() { m_p = -m_p; }

        //!
        void makeUnitLength() { m_p.normalize(); }

        //! Test if plane parameters are valid
        bool valid() const { return !isNaN(); }

        //! Test if plane is invalid
        bool isNaN() const { return geometry::isNaN(m_p[0]) || geometry::isNaN(m_p[1]) || geometry::isNaN(m_p[2]) || geometry::isNaN(m_p[3]);}

        //! Compare two planes.
        bool operator == (const CPlane &p);

        //! Compare two planes
        bool operator != (const CPlane &p) {return !(operator==(p));}

        //! Access vector elements - const version
        double operator[] (int i) const { return m_p[i]; }

        //! Access vector elements
        double &operator[] (int i) { return m_p[i]; }

        //! Get plane normal
        Vec3 getNormal() const { return Vec3(m_p[0], m_p[1], m_p[2]); }

        //! Compute point-plane distance
        inline double distance(const Vec3 &point) const
        {
            return m_p[0]*point[0] + m_p[1]*point[1] + m_p[2]*point[2] + m_p[3];
        }

        //! Calculate dot product of plane normal and given vector
        inline double dotProductNormal(const Vec3 &v) const
        {
            return m_p[0]*v[0] + m_p[1]*v[1] + m_p[2]*v[2];
        }

        //! Calculates if vertex array intersects plane. 
        //! Returns 1 if vertices are completely above the plane, 
        //!  0 if they intersects or 
        //! -1 if vertices are completely below the plane.
        int intersect(const Vec3Array &vertices) const;

        //! Transform the plane by matrix. If inverse transform was already calculated, 
        //! use transformProvidingInverse to evade inverse matrix calculation
        inline void transform(const Matrix &matrix)
        {
            Matrix inverse;
            inverse.invert(matrix);
            transformProvidingInverse(inverse);
        }

        //! Transform the plane by providing a pre inverted matrix.
        inline void transformProvidingInverse(const Matrix &matrix)
        {
            Vec4 vec(matrix.operator*(m_p));
            set(vec);
            makeUnitLength();
        }

    protected:
        //! Plane definition 
        Vec4 m_p;

        //! Plane bounding box corners
    }; // class CPlane

    //! Smart pointer type
    typedef CPlane::tSmartPtr CPlanePtr;
}

// Plane_H_included
#endif