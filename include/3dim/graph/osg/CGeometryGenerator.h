///////////////////////////////////////////////////////////////////////////////
// $Id: CGeometryGenerator.h 1266 2011-04-17 23:00:36Z stancl$
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

#ifndef CGeometryGenerator_H_included
#define CGeometryGenerator_H_included

#include <osg/Geode>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Array>
#include <osg/MatrixTransform>
namespace osg
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Ring geometry - . 
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRingGeometry : public osg::Geometry
{
public:
	//! Default constructor
	CRingGeometry( unsigned int num_of_segments = 16 );

	//! Modify geometry settings and update it
	void setSize( float r1, float r2 );
	
	//! Set axis around which is ring
	void setAxis( const osg::Vec3 & axis ) { m_axis = axis; }

	//! Get axis
	const osg::Vec3 & getAxis( ) const { return m_axis; }

	//! Setup geometry, or force updates
	void update();

protected:
	//! Allocate needed data
	bool allocateArrays( unsigned int num_segments );

protected:
	//! Inner radius
	float m_radius_inner;

	//! Outer radius
	float m_radius_outer;

	//! Number of segments
	unsigned int m_num_segments;

	//! Ring geometry points
	osg::ref_ptr< osg::Vec3Array > m_points;

	//! Normals
	osg::ref_ptr< osg::Vec3Array > m_normals;

	//! Colors
	osg::ref_ptr< osg::Vec4Array > m_colors;

	//! Draw elements bindings
	osg::ref_ptr< osg::DrawElementsUInt > m_drawElements;
	
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
	CFrustrumGeometry( unsigned num_segments );

	//! Set radii
	void setRadii( float r1, float r2 ) { m_r1 = r1; m_r2 = r2;}

	//! Set height
	void setHeight( float height ) { m_height = height; }

	//! Set use capping
	void setCapping( bool bCap1 = true, bool bCap2 = true );

	//! Set all geometry offset
	void setOffset( const osg::Vec3 & offset ) { m_offset = offset; }

	//! Update geometry
	void update();

	//! Set used axis
	void setAxis( const osg::Vec3 & axis ) { m_axis = axis; }

protected:
	//! Allocate arrays
	void allocateArrays( unsigned num_segments );

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
	osg::ref_ptr< osg::Vec3Array > m_points;

	//! Normals array
	osg::ref_ptr< osg::Vec3Array > m_normals;

	//! Draw elements
	osg::ref_ptr< osg::DrawElementsUInt > m_de_faces, m_de_cap1, m_de_cap2;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Arrow 3 d geometry. 
////////////////////////////////////////////////////////////////////////////////////////////////////
class CArrow3DGeometry : public osg::Geode
{
public:
	//! Default constructor
	CArrow3DGeometry( unsigned int num_segments );

	//! Modify geometry settings and update it
	void setSize( float cylinder_radius, float cone_radius, float cylinder_length, float cone_length );

	//! Set arrow axis
	void setAxis( const osg::Vec3 & axis ) { m_axis = axis; }

	//! Setup geometry, or force updates
	void update();

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
	osg::ref_ptr< osg::CFrustrumGeometry > m_cone_geometry, m_cylinder_geometry;

}; // class CArrow3DGeometry

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Ring 3 d geometry. 
////////////////////////////////////////////////////////////////////////////////////////////////////
class CRing3DGeometry : public osg::Geometry
{
public:
	//! Default constructor
	CRing3DGeometry( unsigned int num_of_segments = 16 );

	//! Modify geometry settings
	void setSize( float r1, float r2 );

	//! Set ring width
	void setWidth( float width ) { m_width = width; }

    //! Set offset of ring
    void setOffset(osg::Vec3 offset) { m_offset = offset; }

	//! Setup geometry, or force updates
	void update();

	//! Set axis around which is ring
	void setAxis( const osg::Vec3 & axis ) { m_axis = axis; }

	//! Get used axis
	const osg::Vec3 & getAxis( ) const { return m_axis; }

protected:
	//! Allocate needed data
	bool allocateArrays( unsigned int num_segments );

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
	osg::ref_ptr< osg::Vec3Array > m_points;

	//! Normals
	osg::ref_ptr< osg::Vec3Array > m_normals;

	//! Colors
	osg::ref_ptr< osg::Vec4Array > m_colors;

	//! Draw elements bindings
	osg::ref_ptr< osg::DrawElementsUInt > m_de_cap1, m_de_cap2, m_de_inner, m_de_outer;

	//! Geometry
	osg::ref_ptr< osg::Geometry > m_geometry;

	//! Used axis
	osg::Vec3 m_axis;

}; // class CRing3DGeometry

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sphere geometry. 
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSphereGeometry : public osg::Geometry
{
public:
	//! Default constructor
	CSphereGeometry( unsigned int num_of_segments = 16 );

	//! Modify geometry settings
	void setRadius( float radius ){ m_radius = radius; }

	//! Setup geometry, or force updates
	void update();

protected:
	//! Allocate needed data
	bool allocateArrays( unsigned int num_segments );

protected:
	//! Sphere radius
	float m_radius;

	//! Ring geometry points
	osg::ref_ptr< osg::Vec3Array > m_points;

	//! Normals
	osg::ref_ptr< osg::Vec3Array > m_normals;

	//! Colors
	osg::ref_ptr< osg::Vec4Array > m_colors;

	//! Draw elements bindings
	osg::ref_ptr< osg::DrawElementsUInt > * m_de_array;

	//! Number of segments
	unsigned int m_num_segments;

}; // class CSphereGeometry

}

// CGeometryGenerator_H_included
#endif