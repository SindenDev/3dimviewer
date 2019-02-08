///////////////////////////////////////////////////////////////////////////////
// $Id: CGeometryGenerator.h 1266 2011-04-17 23:00:36Z stancl$
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

#ifndef CGeometryGenerator_H_included
#define CGeometryGenerator_H_included

#include <osg/Geode>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Array>
#include <osg/MatrixTransform>

namespace geometry
{
    template <typename T>
    T swizzle(const T &v, const std::string &s)
    {
        T o;
        for (int i = 0; i < s.length(); ++i)
        {
            o[i] = v[((s[i] - 'w') + 3) % 4];
        }
        return o;
    }

    class CMesh;
}

namespace osg
{
    //! \class  CProceduralGeometry
    //!
    //! \brief  Interface for all proceduraly generated geometries.
    class CProceduralGeometry : public osg::Geode
    {
    public:
        //! Setup geometry, or force updates
        virtual void update() {};

        void setColor(const osg::Vec4 &color);

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Donut geometry. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CDonutGeometry : public CProceduralGeometry
    {
    public:
        //! Default constructor
        CDonutGeometry(unsigned int num_of_segments = 16, unsigned int num_of_segments2 = 5);

        //! Modify geometry settings
        void setSize(float r1, float r2);

        //! Setup geometry, or force updates
        virtual void update() override;

        osg::ref_ptr<osg::Geometry> getGeometry() { return m_geometry; }

    protected:
        //! Allocate needed data
        bool allocateArrays(unsigned int num_segments1, unsigned int num_segments2);

    protected:
        //! Ring radius
        float m_radius1;

        //! Profile radius
        float m_radius2;

        //! Number of ring segments
        unsigned int m_num_segments1;
        //! Number of profile segments
        unsigned int m_num_segments2;

        osg::ref_ptr<osg::Vec3Array> m_points;
        osg::ref_ptr<osg::Vec3Array> m_normals;
        osg::ref_ptr<osg::Vec4Array> m_colors;

        //! Geometry
        osg::ref_ptr<osg::Geometry> m_geometry;
        
        std::vector<osg::ref_ptr<osg::DrawElementsUInt>> m_drawElements;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Ring geometry - . 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CRingGeometry : public osg::Geometry
    {
    public:
        //! Default constructor
        CRingGeometry(unsigned int num_of_segments = 16);

        //! Modify geometry settings and update it
        void setSize(float r1, float r2);

        //! Set axis around which is ring
        void setAxis(const osg::Vec3 & axis) { m_axis = axis; }

        //! Get axis
        const osg::Vec3 & getAxis() const { return m_axis; }

        //! Setup geometry, or force updates
        void update();

    protected:
        //! Allocate needed data
        bool allocateArrays(unsigned int num_segments);

    protected:
        //! Inner radius
        float m_radius_inner;

        //! Outer radius
        float m_radius_outer;

        //! Number of segments
        unsigned int m_num_segments;

        //! Ring geometry points
        osg::ref_ptr<osg::Vec3Array> m_points;

        //! Normals
        osg::ref_ptr<osg::Vec3Array> m_normals;

        //! Colors
        osg::ref_ptr<osg::Vec4Array> m_colors;

        //! Draw elements bindings
        osg::ref_ptr<osg::DrawElementsUInt> m_drawElements;

        //! Used axis
        osg::Vec3 m_axis;

    }; // class CRingGeometry

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Cone geode. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CFrustrumGeometry : public osg::Geometry
    {
    public:
        //! Constructor
        CFrustrumGeometry(unsigned num_segments);

        //! Set radii
        void setRadii(float r1, float r2) { m_r1 = r1; m_r2 = r2; }

        //! Set height
        void setHeight(float height) { m_height = height; }

        float getHeight() { return m_height; }

        //! Set use capping
        void setCapping(bool bCap1 = true, bool bCap2 = true);

        //! Set all geometry offset
        void setOffset(const osg::Vec3 &offset) { m_offset = offset; }

        void setColor(const osg::Vec4 &color);

        //! Update geometry
        void update(bool centerShift = false);

        //! Set used axis
        void setAxis(const osg::Vec3 &axis) { m_axis = axis; }

        //! 
        std::unique_ptr<geometry::CMesh> createCMesh();

        void setNumSegments(int numSegments) { m_num_segments = numSegments; }

        osg::Vec3 getSize() {
            return osg::Vec3(m_r1, m_r2, m_height);
        }

    protected:
        //! Allocate arrays
        void allocateArrays(unsigned num_segments);

        //! Used segments
        unsigned m_num_segments;

        //! Radii
        float m_r1, m_r2;

        //! Cone height
        float m_height;

        //! Use caps?
        bool m_bcap1, m_bcap2;

        //! Used offset
        osg::Vec3 m_offset;

        //! Used axis
        osg::Vec3 m_axis;

        //! Points array
        osg::ref_ptr<osg::Vec3Array> m_points;

        //! Normals array
        osg::ref_ptr<osg::Vec3Array> m_normals;

        //! Colors
        osg::ref_ptr<osg::Vec4Array> m_colors;

        //! Draw elements
        osg::ref_ptr<osg::DrawElementsUInt> m_de_faces, m_de_cap1, m_de_cap2;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Arrow 3 d geometry. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CArrow3DGeometry : public CProceduralGeometry
    {
    public:
        //! Default constructor
        CArrow3DGeometry(unsigned int num_segments);

        //! Modify geometry settings and update it
        void setSize(float cylinder_radius, float cone_radius, float cylinder_length, float cone_length);

        //! Set arrow axis
        void setAxis(const osg::Vec3 & axis) { m_axis = axis; }

        //! Setup geometry, or force updates
        virtual void update() override;

    protected:
        //! Cylinder radius
        float m_radius_cylinder;

        //! Cone radius
        float m_radius_cone;

        //! Cylinder height
        float m_cylinder_length;

        //! Cone height
        float m_cone_length;

        //! Used axis
        osg::Vec3 m_axis;

        //! Geometry
        osg::ref_ptr<osg::CFrustrumGeometry> m_cone_geometry, m_cylinder_geometry;

    }; // class CArrow3DGeometry

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Ring 3 d geometry. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CRing3DGeometry : public osg::Geometry
    {
    public:
        //! Default constructor
        CRing3DGeometry(unsigned int num_of_segments = 16);

        //! Modify geometry settings
        void setSize(float r1, float r2);

        //! Set ring width
        void setWidth(float width) { m_width = width; }

        //! Set offset of ring
        void setOffset(osg::Vec3 offset) { m_offset = offset; }

        //! Setup geometry, or force updates
        void update();

        //! Set axis around which is ring
        void setAxis(const osg::Vec3 & axis) { m_axis = axis; }

        //! Get used axis
        const osg::Vec3 & getAxis() const { return m_axis; }

    protected:
        //! Allocate needed data
        bool allocateArrays(unsigned int num_segments);

    protected:
        //! Inner radius
        float m_radius_inner;

        //! Outer radius
        float m_radius_outer;

        //! Ring width
        float m_width;

        //! Offset
        osg::Vec3 m_offset;

        //! Number of segments
        unsigned int m_num_segments;

        //! Ring geometry points
        osg::ref_ptr<osg::Vec3Array> m_points;

        //! Normals
        osg::ref_ptr<osg::Vec3Array> m_normals;

        //! Colors
        osg::ref_ptr<osg::Vec4Array> m_colors;

        //! Draw elements bindings
        osg::ref_ptr<osg::DrawElementsUInt> m_de_cap1, m_de_cap2, m_de_inner, m_de_outer;

        //! Geometry
        osg::ref_ptr<osg::Geometry> m_geometry;

        //! Used axis
        osg::Vec3 m_axis;

    }; // class CRing3DGeometry

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Ring 3 d geometry. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CPill2DGeometry : public osg::Geometry
    {
    public:
        //! Default constructor
        CPill2DGeometry(unsigned int num_of_segments = 8);

        //! Modify geometry settings
        void setSize(float length, float radius, float width);

        //! Setup geometry, or force updates
        void update();

    protected:
        //! Allocate needed data
        bool allocateArrays(unsigned int num_segments);

    protected:
        //! Length
        float m_length;

        //! Radius
        float m_radius;

        //! Width
        float m_width;

        //! Number of segments
        unsigned int m_segments;

        //! Ring geometry points
        osg::ref_ptr<osg::Vec3Array> m_points;

        //! Normals
        osg::ref_ptr<osg::Vec3Array> m_normals;

        //! Colors
        osg::ref_ptr<osg::Vec4Array> m_colors;

        //! Draw elements bindings
        osg::ref_ptr<osg::DrawElementsUInt> m_de_cap1, m_de_cap2, m_de_per;

        //! Geometry
        osg::ref_ptr<osg::Geometry> m_geometry;
    }; // class CPill2DGeometry

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Sphere geometry. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CSphereGeometry_old : public CProceduralGeometry
    {
    public:
        //! Default constructor
        CSphereGeometry_old(unsigned int num_of_segments = 16, float radius = 1.0f);

        //! Modify geometry settings
        void setRadius(float radius) { m_radius = radius; }

        //! Setup geometry, or force updates
        void update();

        osg::ref_ptr<osg::Geometry> getGeometry() { return m_geometry; }

        std::unique_ptr<geometry::CMesh> createCMesh();

        void setColor(const osg::Vec4 &color);

    protected:
        //! Allocate needed data
        bool allocateArrays(unsigned int num_segments);

    protected:
        //! Sphere radius
        float m_radius;

        //! Ring geometry points
        osg::ref_ptr<osg::Vec3Array> m_points;

        //! Normals
        osg::ref_ptr<osg::Vec3Array> m_normals;

        //! Colors
        osg::ref_ptr<osg::Vec4Array> m_colors;

        //! Draw elements bindings
        std::vector<osg::ref_ptr<osg::DrawElementsUInt>> m_de_array;

        //! Number of segments
        unsigned int m_num_segments;

        //! Geometry
        osg::ref_ptr<osg::Geometry> m_geometry;

    }; // class CSphereGeometry

    class CSphereGeometry : public CProceduralGeometry
    {
    public:
        //! Default constructor
        CSphereGeometry(unsigned int num_of_segments = 16, float radius = 1.0f);

        //! Modify geometry settings
        void setRadius(float radius) { m_radius = radius; }

        //! Setup geometry, or force updates
        void update();

        osg::ref_ptr<osg::Geometry> getGeometry() { return m_geometry; }

        std::unique_ptr<geometry::CMesh> createCMesh();

        void setColor(const osg::Vec4 &color);

    protected:
        //! Allocate needed data
        bool allocateArrays(unsigned int num_segments);

    protected:
        //! Sphere radius
        float m_radius;

        //! Ring geometry points
        osg::ref_ptr<osg::Vec3Array> m_points;

        //! Normals
        osg::ref_ptr<osg::Vec3Array> m_normals;

        //! Colors
        osg::ref_ptr<osg::Vec4Array> m_colors;

        //! Draw elements bindings
        osg::ref_ptr<osg::DrawElementsUInt> m_de_array;

        //! Number of segments
        unsigned int m_num_segments;

        //! Geometry
        osg::ref_ptr<osg::Geometry> m_geometry;

    }; // class CSphereGeometry

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Sphere geometry. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CSemiCircularArrowGeometry : public osg::Geometry
    {
    public:
        //! Default constructor
        CSemiCircularArrowGeometry(unsigned int num_of_segments = 16);

        //! Modify geometry settings
        void setRadius(float radius)
        {
            m_radius = radius;
        }

        //! Sets angular length of circular part of arrow (in degrees)
        void setAngularLength(float angularLength)
        {
            m_angularLength = angularLength;
        }

        void setAngularOffset(float angularOffset)
        {
            m_angularOffset = angularOffset;
        }

        //! Sets width of arrow
        void setWidth(float width)
        {
            m_width = width;
        }

        //! Sets thickness of the geometry
        void setThickness(float thickness)
        {
            m_thickness = thickness;
        }

        //! Sets angular length of arrow part itself (in degrees)
        void setArrowLength(float arrowLength)
        {
            m_arrow1Length = arrowLength;
            m_arrow2Length = arrowLength;
        }

        //! Sets different angular length of each arrow part (in degrees)
        void setDifferentArrowLengths(float arrow1Length, float arrow2Length)
        {
            m_arrow1Length = arrow1Length;
            m_arrow2Length = arrow2Length;
        }

        //! Setup geometry, or force updates
        void update();

    protected:
        //! Allocate needed data
        bool allocateArrays(unsigned int num_segments);

    protected:
        //! Sphere radius
        float m_radius;
        float m_angularLength;
        float m_angularOffset;
        float m_width;
        float m_arrow1Length;
        float m_arrow2Length;
        float m_thickness;

        //! Geometry offset
        osg::Vec3 m_offset;

        //! Ring geometry points
        osg::ref_ptr<osg::Vec3Array> m_points;

        //! Normals
        osg::ref_ptr<osg::Vec3Array> m_normals;

        //! Colors
        osg::ref_ptr<osg::Vec4Array> m_colors;

        //! Draw elements bindings
        osg::ref_ptr<osg::DrawElementsUInt> m_drawElements;

        //! Number of segments
        unsigned int m_num_segments;

    }; // class CSemiCircularArrowGeometry

    class CCubeGeometry : public osg::Geometry
    {
    public:
        //! Simple constructor (unit size is set)
        CCubeGeometry();

        //! Initializing constructor
        CCubeGeometry(const osg::Vec3 &size);

        //! Set size
        void setSize(const osg::Vec3 &size);

        void setColor(const osg::Vec4& color);

        //! Setup geometry, or force updates
        void update();

        std::unique_ptr<geometry::CMesh> createCMesh();

        //! Current cube size
        osg::Vec3 m_size;

    protected:
        //! Allocate needed data
        bool allocateArrays();

    protected:


        //! Ring geometry points
        osg::ref_ptr<osg::Vec3Array> m_points;

        //! Normals
        osg::ref_ptr<osg::Vec3Array> m_normals;

        //! Colors
        osg::ref_ptr<osg::Vec4Array> m_colors;
    }; // class CCubeGeometry


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Cylinder geometry. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CCylinderGeometry : public CFrustrumGeometry
    {
    public:
        //! Default constructor
        CCylinderGeometry(unsigned int num_of_segments = 32, float radius = 1.0f);

        //! Modify geometry settings
        void setRadius(float radius) { m_r1 = radius; m_r2 = radius; }

        //! Set used axis
        void setAxis(const osg::Vec3 &axis = osg::Vec3(0.0f, 0.0f, 1.0f)) { m_axis = axis; }
    protected:

        //forbid some methods..
        using CFrustrumGeometry::setRadii;
        using CFrustrumGeometry::setCapping;

    }; // class CCylinderGeometry

}

// CGeometryGenerator_H_included
#endif