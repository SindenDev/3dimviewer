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

#include <graph/osg/CGeometryGenerator.h>
#include <osg/Geometry>
#include <osg/Version>
#include <new>
#include <VPL/Base/Logging.h>

#define PI 3.14159265f

osg::CDonutGeometry::CDonutGeometry(unsigned int num_of_segments/* = 16*/, unsigned int num_of_segments2/* = 5*/)
    : m_radius1(1.0f)
    , m_radius2(0.2f)
    , m_num_segments1(0)
    , m_num_segments2(0)
{
    setName("CDonutGeometry");
    allocateArrays(num_of_segments, num_of_segments2);
}

void osg::CDonutGeometry::setSize(float r1, float r2)
{
    m_radius1 = r1;
    m_radius2 = r2;
}

bool osg::CDonutGeometry::allocateArrays(unsigned int num_segments1, unsigned int num_segments2)
{
    if (num_segments1 == m_num_segments1 && num_segments2 == m_num_segments2)
    {
        return false;
    }

    try
    {
        m_points = new osg::Vec3Array((num_segments1 + 1) * num_segments2);
        m_normals = new osg::Vec3Array((num_segments1 + 1) * num_segments2);
        m_drawElements = std::vector<osg::ref_ptr<osg::DrawElementsUInt>>(num_segments2);

        for (unsigned i = 0; i < num_segments2; ++i)
        {
            m_drawElements[i] = new osg::DrawElementsUInt(osg::PrimitiveSet::QUAD_STRIP, 2 * (num_segments1 + 1));
        }
    }
    catch (std::bad_alloc& e)
    {
        VPL_LOG_ERROR(e.what());

        return false;
    }

    m_num_segments1 = num_segments1;
    m_num_segments2 = num_segments2;

    return true;
}

void osg::CDonutGeometry::update()
{
    float segment2Angle = 2.0f * PI / float(m_num_segments2);
    float a2 = 0.0f;

    std::vector<osg::Vec3> points(m_num_segments2);
    std::vector<osg::Vec3> normals(m_num_segments2);

    for (unsigned int s = 0; s < m_num_segments2; ++s)
    {
        float x = cos(a2) * m_radius2;
        float y = sin(a2) * m_radius2;

        points[s] = osg::Vec3(x, y, 0.0f);

        normals[s] = points[s];
        normals[s].normalize();

        a2 += segment2Angle;
    }

    osg::Matrix m = osg::Matrix::rotate(osg::Y_AXIS, osg::Z_AXIS);

    float segment1Angle = (2.0f * PI / float(m_num_segments1));
    float a1 = 0.0f;

    for (unsigned int s1 = 0; s1 <= m_num_segments1; ++s1)
    {
        for (unsigned int s2 = 0; s2 < m_num_segments2; ++s2)
        {
            (*m_points)[s1*m_num_segments2 + s2] = points[s2] * osg::Matrix::translate(osg::Vec3(m_radius1, 0.0, 0.0)) * osg::Matrix::rotate(a1, osg::Y_AXIS) * m;
            (*m_normals)[s1*m_num_segments2 + s2] = normals[s2] * osg::Matrix::rotate(a1, osg::Y_AXIS) * m;

            (*m_drawElements[s2])[2 * s1 + 0] = s1*m_num_segments2 + s2;
            (*m_drawElements[s2])[2 * s1 + 1] = s1*m_num_segments2 + (s2 + 1) % m_num_segments2;
        }

        a1 += segment1Angle;
    }

    // Remove all previous primitive sets
    int num(this->getNumPrimitiveSets());
    this->removePrimitiveSet(0, num);

     //Finalize geometry settings
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    this->setNormalArray(m_normals, osg::Array::BIND_PER_VERTEX);
#else
    this->setNormalArray(m_normals);
#endif
    this->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    this->setVertexArray(m_points);

    for (unsigned i = 0; i < m_num_segments2; ++i)
    {
        this->addPrimitiveSet(m_drawElements[i]);
    }
}


osg::CRingGeometry::CRingGeometry( unsigned int num_of_segments /*= 16 */ )
	: m_radius_inner( 0.0f )
	, m_radius_outer( 1.0f )
	, m_num_segments( 0 )
	, m_axis( 0.0, 0.0, 1.0 )
{
    setName("CRingGeometry");
	allocateArrays( num_of_segments );
	
	if( num_of_segments )
		update();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Allocate arrays. 
//!
//!\param	num_segments	Number of segments. 
//!
//!\return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool osg::CRingGeometry::allocateArrays( unsigned int num_segments )
{
	if( num_segments == 0 )
		return false;

	if( num_segments == m_num_segments )
		return true; // Already done...

	try
	{
		// Allocate points
		m_points = new osg::Vec3Array( 2 * num_segments );

		// Allocate normals
		m_normals = new osg::Vec3Array( 1 );

		// Allocate draw elements
		m_drawElements = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP, 2 * num_segments + 2 );
	
		// Allocate colors array
		m_colors = new osg::Vec4Array( 1 );

	}
	catch( std::bad_alloc &  )
	{
//		std::cerr << e.what() << std::endl;

		return false;
	}

	m_num_segments = num_segments;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Updates this object. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CRingGeometry::update()
{
	// Compute angle increment
	float dangle( 2.0f * PI / float( m_num_segments ) );
	float a1( 0.0f ), a2( dangle / 2.0f );
	float x, y;

	//! Transform matrix - rotate ring axis
	osg::Matrix m( osg::Matrix::rotate( osg::Vec3( 0.0, 0.0, 1.0 ), m_axis ));

	// Fill points array
	for( unsigned int s = 0; s < m_num_segments; ++s )
	{
		x = cos( a1 ) * m_radius_inner;
		y = sin( a1 ) * m_radius_inner;

		(*m_points)[ 2*s ] = osg::Vec3( x, y, 0.0f ) * m;

		x = cos( a2 ) * m_radius_outer;
		y = sin( a2 ) * m_radius_outer;

		(*m_points)[ 2*s + 1 ] = osg::Vec3( x, y, 0.0 ) * m;

		a1 += dangle;
		a2 += dangle;
	}

	// Fill draw elements array
	for( unsigned int s = 0; s < m_num_segments * 2 + 2; ++s )
	{
		(*m_drawElements)[ s ] = s % (2*m_num_segments);
	}

	// Fill normals array
	(*m_normals)[0] = -m_axis;

	// Fill colors array
	( *m_colors )[0] = osg::Vec4( 0.0, 1.0, 0.0, 1.0 );

	// Set all to the geometry
	this->setVertexArray( m_points );
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	this->setNormalArray( m_normals, osg::Array::BIND_OVERALL );
#else
	this->setNormalArray( m_normals );
#endif
	this->setNormalBinding( osg::Geometry::BIND_OVERALL);
	this->addPrimitiveSet( m_drawElements );
//	this->setColorArray( m_colors );
//	this->setColorBinding( osg::Geometry::BIND_OVERALL );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets a size. 
//!
//!\param	r1	The first float. 
//!\param	r2	The second float. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CRingGeometry::setSize( float r1, float r2 )
{
	m_radius_inner = r1;
	m_radius_outer = r2;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Constructor. 
//!
//!\param	num_segments	Number of segments. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osg::CArrow3DGeometry::CArrow3DGeometry( unsigned int num_segments )
	: m_radius_cylinder( 0.0f )
	, m_radius_cone( 1.0f )
	, m_cylinder_length( 1.0f )
	, m_cone_length( 1.0f )
	, m_axis( 0.0, 0.0, 1.0 )
{
    setName("CArrow3DGeometry");
	// Create geometry node
	m_cone_geometry = new osg::CFrustrumGeometry( num_segments );
	m_cylinder_geometry = new osg::CFrustrumGeometry( num_segments );

	// Create tree
	this->addDrawable( m_cone_geometry );
	this->addDrawable( m_cylinder_geometry );

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets a size. 
//!
//!\param	cylinder_radius	The cylinder radius. 
//!\param	cone_radius			The cone radius. 
//!\param	cylinder_length	Length of the cylinder. 
//!\param	cone_length			Length of the cone. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CArrow3DGeometry::setSize( float cylinder_radius, float cone_radius, float cylinder_length, float cone_length )
{
	m_radius_cylinder = cylinder_radius;
	m_radius_cone = cone_radius;
	m_cylinder_length = cylinder_length;
	m_cone_length = cone_length;

	m_cylinder_geometry->setHeight( m_cylinder_length );
	m_cylinder_geometry->setRadii( m_radius_cylinder, m_radius_cylinder );

	m_cone_geometry->setHeight( m_cone_length );
	m_cone_geometry->setRadii( m_radius_cone, 0.001f );
	m_cone_geometry->setOffset( osg::Vec3( 0.0, 0.0, m_cylinder_length ) );
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Updates this object. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CArrow3DGeometry::update()
{
	m_cone_geometry->setAxis( m_axis );
	m_cylinder_geometry->setAxis( m_axis );
	m_cone_geometry->update();
	m_cylinder_geometry->update();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Constructor. 
//!
//!\param	num_segments	Number of segments. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osg::CFrustrumGeometry::CFrustrumGeometry( unsigned num_segments )
	: m_num_segments( 0 )
	, m_r1( 1.0 )
	, m_r2( 1.0 )
	, m_height( 1.0 )
	, m_bcap1( true )
	, m_bcap2( true )
	, m_offset( 0.0, 0.0, 0.0 )
	, m_axis( 0.0, 0.0, 1.0 )
{
    setName("CFrustrumGeometry");
	allocateArrays( num_segments );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Allocate arrays. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CFrustrumGeometry::allocateArrays( unsigned num_segments )
{
	if( num_segments == m_num_segments )
		return;

	try
	{
		m_points = new osg::Vec3Array( 4 * (1 + num_segments) + 2);
		m_normals = new osg::Vec3Array( 4 * (1 + num_segments) + 2);
		m_de_cap1 = new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLE_FAN, num_segments + 2 );
		m_de_cap2 = new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLE_FAN, num_segments + 2 );
		m_de_faces = new osg::DrawElementsUInt( osg::PrimitiveSet::QUAD_STRIP, 2*num_segments + 2 ); 

	}
	catch (std::bad_alloc& e)
	{
		VPL_LOG_ERROR( e.what() );

		return;
	}

	m_num_segments = num_segments;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Updates this object. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CFrustrumGeometry::update()
{
	osg::Vec3 v1, v2, n;
	float r( m_r1 - m_r2 );
	float normalz = r/(sqrtf(r*r+m_height*m_height));
	float normalRatio = 1.0f/(sqrtf(1.0f+normalz*normalz));

	// Create rotation matrix
	osg::Quat q;
	q.makeRotate( osg::Vec3( 0.0, 0.0, 1.0 ), m_axis );
	osg::Matrix m( osg::Matrix::rotate(q));

	unsigned off_cap1( 2 * ( m_num_segments + 1 ) )
		, off_cap2( 3 * ( m_num_segments + 1 ) )
		, off_centers( 4 * ( m_num_segments + 1 ) );

	// Generate points
	for( unsigned i = 0; i <= m_num_segments; ++i )
	{
		float si = sin( 2 * PI  * ( float ) i / ( float ) m_num_segments );
		float co = cos( 2 * PI  * ( float ) i / ( float ) m_num_segments );


		// Store two facet points
		(*m_points)[ 2*i ] = v1 = (osg::Vec3( co * m_r1, si * m_r1, 0.0 ) + m_offset) * m; 
		(*m_points)[ 2*i+1 ] = v2 = (osg::Vec3( co * m_r2, si * m_r2, m_height ) + m_offset) * m;

		// Compute facet normal vector
		n = osg::Vec3( co * normalRatio, si * normalRatio, normalz ) * m;

		(*m_normals)[ 2*i ] = n;
		(*m_normals)[ 2*i+1 ] = n;

		// Store point and normal - cap 1
		(*m_points)[ i + off_cap1 ] = v1;
		(*m_normals)[ i + off_cap1 ] = osg::Vec3( 0.0, 0.0, -1.0 ) * m;

		// Store point and normal - cap 2
		(*m_points)[ i + off_cap2 ] = v2;
		(*m_normals)[ i + off_cap2 ] = osg::Vec3( 0.0, 0.0, 1.0 ) * m;

		// Add faces strip draw elements
		(*m_de_faces)[ i * 2 ] = 2*i;
		(*m_de_faces)[ i * 2 + 1 ] = 2*i+1;

		// Add caps draw elements
		(*m_de_cap1)[i + 1] = i + off_cap1;
		(*m_de_cap2)[i + 1] = i + off_cap2;

	}

	// Add center points
	(*m_points)[ off_centers ] = (osg::Vec3( 0.0, 0.0, 0.0 ) + m_offset) * m;
	(*m_points)[ off_centers + 1 ] = (osg::Vec3( 0.0, 0.0, m_height ) + m_offset) * m;

	// Add center normals
	(*m_normals)[ off_centers ] = osg::Vec3( 0.0, 0.0, -1.0 ) * m;
	(*m_normals)[ off_centers + 1 ] = osg::Vec3( 0.0, 0.0, 1.0 ) * m;

	// Add this center point to the caps drawable elements
	(*m_de_cap1)[0] = off_centers;
	(*m_de_cap2)[0] = off_centers + 1;

	// Remove all previous primitive sets
	int num( this->getNumPrimitiveSets() );
	this->removePrimitiveSet(0, num );

	// Finalize geometry settings
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	this->setNormalArray( m_normals, osg::Array::BIND_PER_VERTEX );
#else
	this->setNormalArray( m_normals );
#endif
	this->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
	this->setVertexArray( m_points );
	
	// Add caps only if wanted
	if( m_bcap1 )
		this->addPrimitiveSet( m_de_cap1 );
	
	if( m_bcap2 )
		this->addPrimitiveSet( m_de_cap2 );
	
	this->addPrimitiveSet( m_de_faces );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets a capping. 
//!
//!\param	bCap1	true to cap 1. 
//!\param	bCap2	true to cap 2. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CFrustrumGeometry::setCapping( bool bCap1 /*= true*/, bool bCap2 /*= true */ )
{
	m_bcap1 = bCap1; m_bcap2 = bCap2;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Constructor. 
//!
//!\param	num_of_segments	Number of segments. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osg::CRing3DGeometry::CRing3DGeometry( unsigned int num_of_segments /*= 16 */ )
	: m_radius_inner( 0.0f )
	, m_radius_outer( 1.0f )
	, m_width( 1.0f )
    , m_offset(0.0f, 0.0f, 0.0f)
    , m_num_segments( 0 )
    , m_axis( 0.0, 0.0, 1.0 )
{
    setName("CRing3DGeometry");
	allocateArrays( num_of_segments );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets a size. 
//!
//!\param	r1	The first float. 
//!\param	r2	The second float. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CRing3DGeometry::setSize( float r1, float r2 )
{
	m_radius_inner = r1;
	m_radius_outer = r2;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Allocate arrays. 
//!
//!\param	num_segments	Number of segments. 
//!
//!\return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool osg::CRing3DGeometry::allocateArrays( unsigned int num_segments )
{
	if( num_segments == m_num_segments )
		return false;

	try
	{
		m_points = new osg::Vec3Array( 8 * (1 + num_segments) );
		m_normals = new osg::Vec3Array( 8 * (1 + num_segments) );
		m_de_cap1 = new osg::DrawElementsUInt( osg::PrimitiveSet::QUAD_STRIP, 2 * (1 + num_segments) );
		m_de_cap2 = new osg::DrawElementsUInt( osg::PrimitiveSet::QUAD_STRIP, 2 * (1 + num_segments) );
		m_de_inner = new osg::DrawElementsUInt( osg::PrimitiveSet::QUAD_STRIP, 2 * (1 + num_segments) ); 
		m_de_outer = new osg::DrawElementsUInt( osg::PrimitiveSet::QUAD_STRIP, 2 * (1 + num_segments) ); 

	}
	catch (std::bad_alloc& e)
	{
		VPL_LOG_ERROR( e.what() );

		return false;
	}

	m_num_segments = num_segments;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Updates this object. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CRing3DGeometry::update()
{
	unsigned numinc( m_num_segments + 1 );
	unsigned off_cap1( 0 )
			 , off_cap2( 2 * numinc )
			 , off_inner( 4 * numinc )
			 , off_outer( 6 * numinc );

	// Rotation matrix
	osg::Quat rotate;
	rotate.makeRotate( osg::Vec3( 0.0, 0.0, 1.0 ), m_axis );
    osg::Matrix m( osg::Matrix::rotate(rotate) * osg::Matrix::translate(m_offset) );
    osg::Matrix nm( osg::Matrix::rotate(rotate) );

	float si, co;
	osg::Vec3 p1, p2, p3, p4, po, n;

	for( unsigned i = 0; i <= m_num_segments; ++i )
	{
		// Compute current angle
		float angle( 2 * PI  * ( float ) i / ( float ) m_num_segments );

		// Compute sin and cos
		si = sin( angle );
		co = cos( angle );

		// Compute inner and outer points
		p1 = osg::Vec3( co * m_radius_inner, si * m_radius_inner, -m_width/2.0f ) * m;
		p2 = osg::Vec3( co * m_radius_inner, si * m_radius_inner, m_width/2.0f ) * m;
		p3 = osg::Vec3( co * m_radius_outer, si * m_radius_outer, -m_width/2.0f ) * m;
		p4 = osg::Vec3( co * m_radius_outer, si * m_radius_outer, m_width/2.0f ) * m;

		// Cap1 points
		(*m_points)[ 2 * i + off_cap1 ]		= p1;	
		(*m_points)[ 2 * i + 1 + off_cap1 ]	= p3;
		// Cap1 normals
		(*m_normals)[ 2 * i + off_cap1]		= osg::Vec3( 0.0, 0.0, -1.0 ) * nm;
		(*m_normals)[ 2 * i + 1 + off_cap1]	= osg::Vec3( 0.0, 0.0, -1.0 ) * nm;
		// Draw elements
		(*m_de_cap1)[ 2 * i ]				= 2 * i + off_cap1;
		(*m_de_cap1)[ 2 * i + 1 ]			= 2 * i + off_cap1 + 1;

		// Cap2 points
		(*m_points)[ 2 * i + off_cap2 ]		= p2;	
		(*m_points)[ 2 * i + 1 + off_cap2 ]	= p4;
		// Cap2 normals
		(*m_normals)[ 2 * i + off_cap2]		= osg::Vec3( 0.0, 0.0, 1.0 ) * nm;
		(*m_normals)[ 2 * i + 1 + off_cap2]	= osg::Vec3( 0.0, 0.0, 1.0 ) * nm;
		// Draw elements
		(*m_de_cap2)[ 2 * i ]				= 2 * i + off_cap2;
		(*m_de_cap2)[ 2 * i + 1 ]			= 2 * i + off_cap2 + 1;

		// Inner ring points
		(*m_points)[ 2 * i + off_inner ]		= p1;	
		(*m_points)[ 2 * i + off_inner + 1 ]	= p2;
		// Inner normals
		(*m_normals)[ 2 * i + off_inner]		= osg::Vec3( -co, -si, 0.0 ) * nm;
		(*m_normals)[ 2 * i + 1 + off_inner]	= osg::Vec3( -co, -si, 0.0 ) * nm;
		// Draw elements
		(*m_de_inner)[ 2 * i ]					= 2 * i + off_inner;
		(*m_de_inner)[ 2 * i + 1 ]				= 2 * i + off_inner + 1;

		// Outer ring points
		(*m_points)[ 2 * i + off_outer ]		= p3;	
		(*m_points)[ 2 * i + off_outer + 1 ]	= p4;
		// Outer normals
		(*m_normals)[ 2 * i + off_outer]		= osg::Vec3( co, si, 0.0 ) * nm;
		(*m_normals)[ 2 * i + 1 + off_outer]	= osg::Vec3( co, si, 0.0 ) * nm;
		// Draw elements
		(*m_de_outer)[ 2 * i ]					= 2 * i + off_outer;
		(*m_de_outer)[ 2 * i + 1 ]				= 2 * i + off_outer + 1;
	}

	// Remove all previous primitive sets
	int num( this->getNumPrimitiveSets() );
	this->removePrimitiveSet(0, num );

	// Finalize geometry settings
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	this->setNormalArray( m_normals, osg::Array::BIND_PER_VERTEX );
#else
	this->setNormalArray( m_normals );
#endif
	this->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
	this->setVertexArray( m_points );

	this->addPrimitiveSet( m_de_cap1 );
	this->addPrimitiveSet( m_de_cap2 );
	this->addPrimitiveSet( m_de_inner );
	this->addPrimitiveSet( m_de_outer );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! Ctor. 
osg::CPill2DGeometry::CPill2DGeometry(unsigned int num_of_segments /*= 8 */)
    : m_length(0.0f)
    , m_radius(1.0f)
    , m_width(1.0f)
    , m_segments(0)
{
    setName("CPill2DGeometry");
    allocateArrays(num_of_segments);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! Sets a size
void osg::CPill2DGeometry::setSize(float length, float radius, float width)
{
    m_length = length;
    m_radius = radius;
    m_width = width;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//! Allocate arrays
bool osg::CPill2DGeometry::allocateArrays(unsigned int num_segments)
{
    if (num_segments == m_segments)
    {
        return false;
    }

    try
    {
        m_points = new osg::Vec3Array(4 * (2 + 2 * num_segments));
        m_normals = new osg::Vec3Array(4 * (2 + 2 * num_segments));
        m_de_cap1 = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 3 * (2 + 2 * (num_segments - 1)));
        m_de_cap2 = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 3 * (2 + 2 * (num_segments - 1)));
        m_de_per = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES,  3 * (2 * (num_segments * 2 + 2)));

    }
    catch (std::bad_alloc& e)
    {
        VPL_LOG_ERROR(e.what());

        return false;
    }

    m_segments = num_segments;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Updates this object. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CPill2DGeometry::update()
{
    // vertices
    for (int i = 0; i < m_segments + 1; ++i)
    {
        float angle = osg::DegreesToRadians(i * 180.0 / m_segments);
        float y = sin(angle);
        float x = cos(angle);

        // cap 1 vertices
        (*m_points)[i + 0 * (m_segments + 1)] = osg::Vec3(-x * m_radius, -y * m_radius,            -m_width * 0.5);
        (*m_points)[i + 1 * (m_segments + 1)] = osg::Vec3( x * m_radius,  y * m_radius + m_length, -m_width * 0.5);
        (*m_normals)[i + 0 * (m_segments + 1)] = osg::Vec3(0.0, 0.0, -1.0);
        (*m_normals)[i + 1 * (m_segments + 1)] = osg::Vec3(0.0, 0.0, -1.0);
        // cap 2 vertices
        (*m_points)[i + 2 * (m_segments + 1)] = osg::Vec3(-x * m_radius, -y * m_radius,             m_width * 0.5);
        (*m_points)[i + 3 * (m_segments + 1)] = osg::Vec3( x * m_radius,  y * m_radius + m_length,  m_width * 0.5);
        (*m_normals)[i + 2 * (m_segments + 1)] = osg::Vec3(0.0, 0.0, 1.0);
        (*m_normals)[i + 3 * (m_segments + 1)] = osg::Vec3(0.0, 0.0, 1.0);
        // per vertices
        (*m_points)[i + 4 * (m_segments + 1)] = osg::Vec3(-x * m_radius, -y * m_radius,            -m_width * 0.5);
        (*m_points)[i + 5 * (m_segments + 1)] = osg::Vec3( x * m_radius,  y * m_radius + m_length, -m_width * 0.5);
        (*m_points)[i + 6 * (m_segments + 1)] = osg::Vec3(-x * m_radius, -y * m_radius,             m_width * 0.5);
        (*m_points)[i + 7 * (m_segments + 1)] = osg::Vec3( x * m_radius,  y * m_radius + m_length,  m_width * 0.5);
        (*m_normals)[i + 4 * (m_segments + 1)] = osg::Vec3(-x, -y, 0.0);
        (*m_normals)[i + 5 * (m_segments + 1)] = osg::Vec3( x,  y, 0.0);
        (*m_normals)[i + 6 * (m_segments + 1)] = osg::Vec3(-x, -y, 0.0);
        (*m_normals)[i + 7 * (m_segments + 1)] = osg::Vec3( x,  y, 0.0);
    }

    // faces for cap1
    for (int i = 0; i < m_segments - 1; ++i)
    {
        (*m_de_cap1)[i * 3 + 0] = 0;
        (*m_de_cap1)[i * 3 + 1] = i + 1;
        (*m_de_cap1)[i * 3 + 2] = i + 2;

        (*m_de_cap1)[(m_segments - 1) * 3 + i * 3 + 0] = m_segments + 1 + 0;
        (*m_de_cap1)[(m_segments - 1) * 3 + i * 3 + 1] = m_segments + 1 + i + 1;
        (*m_de_cap1)[(m_segments - 1) * 3 + i * 3 + 2] = m_segments + 1 + i + 2;
    }

    (*m_de_cap1)[(m_segments - 1) * 2 * 3 + 0] = 0;
    (*m_de_cap1)[(m_segments - 1) * 2 * 3 + 1] = m_segments;
    (*m_de_cap1)[(m_segments - 1) * 2 * 3 + 2] = m_segments + 1;
    (*m_de_cap1)[(m_segments - 1) * 2 * 3 + 3] = 0;
    (*m_de_cap1)[(m_segments - 1) * 2 * 3 + 4] = m_segments + 1;
    (*m_de_cap1)[(m_segments - 1) * 2 * 3 + 5] = m_segments * 2 + 1;

    // faces for cap2
    for (int i = 0; i < m_segments - 1; ++i)
    {
        (*m_de_cap2)[i * 3 + 0] = (m_segments * 2 + 2) + 0;
        (*m_de_cap2)[i * 3 + 1] = (m_segments * 2 + 2) + i + 1;
        (*m_de_cap2)[i * 3 + 2] = (m_segments * 2 + 2) + i + 2;

        (*m_de_cap2)[(m_segments - 1) * 3 + i * 3 + 0] = (m_segments * 2 + 2) + m_segments + 1 + 0;
        (*m_de_cap2)[(m_segments - 1) * 3 + i * 3 + 1] = (m_segments * 2 + 2) + m_segments + 1 + i + 1;
        (*m_de_cap2)[(m_segments - 1) * 3 + i * 3 + 2] = (m_segments * 2 + 2) + m_segments + 1 + i + 2;
    }

    (*m_de_cap2)[(m_segments - 1) * 2 * 3 + 0] = (m_segments * 2 + 2) + 0;
    (*m_de_cap2)[(m_segments - 1) * 2 * 3 + 1] = (m_segments * 2 + 2) + m_segments;
    (*m_de_cap2)[(m_segments - 1) * 2 * 3 + 2] = (m_segments * 2 + 2) + m_segments + 1;
    (*m_de_cap2)[(m_segments - 1) * 2 * 3 + 3] = (m_segments * 2 + 2) + 0;
    (*m_de_cap2)[(m_segments - 1) * 2 * 3 + 4] = (m_segments * 2 + 2) + m_segments + 1;
    (*m_de_cap2)[(m_segments - 1) * 2 * 3 + 5] = (m_segments * 2 + 2) + m_segments * 2 + 1;

    // faces for sides
    for (int i = 0; i < m_segments * 2 + 2; ++i)
    {
        (*m_de_per)[i * 6 + 0] = (m_segments * 2 + 2) * 2 + i;
        (*m_de_per)[i * 6 + 1] = (m_segments * 2 + 2) * 2 + (i + 1) % (m_segments * 2 + 2);
        (*m_de_per)[i * 6 + 2] = (m_segments * 2 + 2) * 3 + i;
        (*m_de_per)[i * 6 + 3] = (m_segments * 2 + 2) * 2 + (i + 1) % (m_segments * 2 + 2);
        (*m_de_per)[i * 6 + 4] = (m_segments * 2 + 2) * 3 + (i + 1) % (m_segments * 2 + 2);
        (*m_de_per)[i * 6 + 5] = (m_segments * 2 + 2) * 3 + i;
    }

    // Remove all previous primitive sets
    removePrimitiveSet(0, getNumPrimitiveSets());

    // Finalize geometry settings
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    setNormalArray(m_normals, osg::Array::BIND_PER_VERTEX);
#else
    setNormalArray(m_normals);
#endif
    setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    setVertexArray(m_points);

    addPrimitiveSet(m_de_cap1);
    addPrimitiveSet(m_de_cap2);
    addPrimitiveSet(m_de_per);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Constructor. 
//!
//!\param	num_of_segments	Number of segments. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osg::CSphereGeometry::CSphereGeometry( unsigned int num_of_segments /*= 16 */ )
	: m_radius( 1.0 )
	, m_de_array( 0 )
    , m_num_segments( 0 )
{
    setName("CSphereGeometry");
	allocateArrays( num_of_segments );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Allocate arrays. 
//!
//!\param	num_segments	Number of segments. 
//!
//!\return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool osg::CSphereGeometry::allocateArrays( unsigned int num_segments )
{
	if( num_segments == 0 )
		return false;

	if( num_segments == m_num_segments )
		return true; // Already done...

	try
	{
		// Allocate points
		m_points = new osg::Vec3Array( (1 + num_segments ) * ( 1 + num_segments ) );

		// Allocate normals
		m_normals = new osg::Vec3Array( (1 + num_segments ) * ( 1 + num_segments ) );

		// Allocate draw elements
		if( m_de_array != 0 )
			delete [] m_de_array;

		// Create array of arrays
		m_de_array = new osg::ref_ptr< osg::DrawElementsUInt >[ num_segments ];

		// Initialize arrays
		for( unsigned s = 0; s < num_segments; ++s )
			m_de_array[ s ] = new osg::DrawElementsUInt( osg::DrawElements::TRIANGLE_STRIP, 2 * (num_segments + 1) );

		// Allocate colors array
		m_colors = new osg::Vec4Array( 1 );

	}
	catch( std::bad_alloc &  )
	{
		//		std::cerr << e.what() << std::endl;

		return false;
	}

	m_num_segments = num_segments;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Updates this object. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CSphereGeometry::update()
{
	// Angle increment
	float dangle( 2.0f * PI / float( m_num_segments ) );
	// "Height" increment
	float dz( 2.0f*m_radius / float( m_num_segments ) );
	float x, y, z( -m_radius );
	float a(0.0);
	// "Height" angle increment
	float dzaangle( PI / float( m_num_segments ) );
	// "Height" angle
	float zangle( -0.5f * PI );

	unsigned numinc( m_num_segments + 1 );

	// For all rows
	for( unsigned uz = 0; uz <= m_num_segments; ++uz )
	{
		for( unsigned ua = 0; ua <= m_num_segments; ++ua )
		{
			float si( sin( a ) );
			float co( cos( a ) );
			float coz( cos(zangle) );
			float siz( sin(zangle) );

			x = co * m_radius * coz;
			y = si * m_radius * coz;
			z = m_radius * siz;

			osg::Vec3 n( x, y, z );

			// Store point and normal
			(*m_points)[ uz * numinc + ua ]	= n;
			n.normalize();
			(*m_normals)[ uz * numinc + ua ]	= n;

			// Increment angle
			a += dangle;
		}
	
		// Height increment
		z += dz;

		// Height angle increment
		zangle += dzaangle;
	}

	this->setVertexArray( m_points );
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	this->setNormalArray( m_normals, osg::Array::BIND_PER_VERTEX );
#else
	this->setNormalArray( m_normals );
#endif
	this->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

	// Remove all previous primitive sets
	unsigned int num( this->getNumPrimitiveSets() );
	this->removePrimitiveSet( 0, num );

	// Store quad strips
	for( unsigned h = 0; h < m_num_segments; ++h )
	{
		for( unsigned a = 0; a < numinc; ++a )
		{
			(*m_de_array[ h ])[ 2*a ] = h * numinc + a; 
			(*m_de_array[ h ])[ 2*a + 1 ] = ( h + 1 ) * numinc + a;
		}
		
		this->addPrimitiveSet( m_de_array[h] );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Constructor. 
//!
//!\param	num_of_segments	Number of segments. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osg::CSemiCircularArrowGeometry::CSemiCircularArrowGeometry(unsigned int num_of_segments)
    : m_radius(1.0)
    , m_angularLength(45.0)
    , m_angularOffset(0.0)
    , m_width(0.1)
    , m_arrowLength(5.0)
    , m_num_segments(0)
    , m_offset(0.0, 0.0, 0.0)
{
    setName("CSemiCircularArrowGeometry");
    allocateArrays(num_of_segments);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Allocate arrays. 
//!
//!\param	num_segments	Number of segments. 
//!
//!\return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool osg::CSemiCircularArrowGeometry::allocateArrays(unsigned int num_segments)
{
    if (num_segments == 0)
    {
        return false;
    }

    if (num_segments == m_num_segments)
    {
        return true;
    }

    try
    {
        // Allocate points
        m_points = new osg::Vec3Array(8 * (num_segments + 1) + 36);

        // Allocate normals
        m_normals = new osg::Vec3Array(8 * (num_segments + 1) + 36);

        // Allocate draw elements
        m_drawElements = new osg::DrawElementsUInt(osg::DrawElements::TRIANGLES, 24 * num_segments + 48);

        // Allocate colors array
        m_colors = new osg::Vec4Array(1);

    }
    catch (std::bad_alloc &)
    {
        return false;
    }

    m_num_segments = num_segments;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Updates this object. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CSemiCircularArrowGeometry::update()
{
    {
        float angle = m_angularOffset + 90.0;
        m_offset = osg::Vec3(std::cos(osg::DegreesToRadians(angle)), std::sin(osg::DegreesToRadians(angle)), 0.0) * (-m_radius);
        m_offset = geometry::swizzle(m_offset, "xzy");
    }

    for (int v = 0; v < m_num_segments + 1; ++v)
    {
        float angle = m_angularOffset + 90.0 - m_angularLength * 0.5 + v * (m_angularLength / m_num_segments);
        osg::Vec2 vec = osg::Vec2(std::cos(osg::DegreesToRadians(angle)), std::sin(osg::DegreesToRadians(angle)));

        osg::Vec3 v0 = osg::Vec3(vec * (m_radius - m_width * 0.5), 0.1);
        osg::Vec3 v1 = osg::Vec3(vec * (m_radius + m_width * 0.5), 0.1);
        osg::Vec3 v2 = osg::Vec3(vec * (m_radius - m_width * 0.5), -0.1);
        osg::Vec3 v3 = osg::Vec3(vec * (m_radius + m_width * 0.5), -0.1);

        osg::Vec3 n0 = osg::Vec3(0.0, 0.0, 1.0);
        osg::Vec3 n1 = osg::Vec3(0.0, 0.0, -1.0);
        osg::Vec3 n2 = osg::Vec3(vec, 0.0);
        osg::Vec3 n3 = osg::Vec3(-vec, 0.0);

        v0 = geometry::swizzle(v0, "xzy");
        v1 = geometry::swizzle(v1, "xzy");
        v2 = geometry::swizzle(v2, "xzy");
        v3 = geometry::swizzle(v3, "xzy");

        n0 = geometry::swizzle(n0, "xzy");
        n1 = geometry::swizzle(n1, "xzy");
        n2 = geometry::swizzle(n2, "xzy");
        n3 = geometry::swizzle(n3, "xzy");

        (*m_points)[v + 0 * (m_num_segments + 1)] = v0 + m_offset;
        (*m_points)[v + 1 * (m_num_segments + 1)] = v1 + m_offset;

        (*m_points)[v + 2 * (m_num_segments + 1)] = v1 + m_offset;
        (*m_points)[v + 3 * (m_num_segments + 1)] = v3 + m_offset;

        (*m_points)[v + 4 * (m_num_segments + 1)] = v3 + m_offset;
        (*m_points)[v + 5 * (m_num_segments + 1)] = v2 + m_offset;

        (*m_points)[v + 6 * (m_num_segments + 1)] = v2 + m_offset;
        (*m_points)[v + 7 * (m_num_segments + 1)] = v0 + m_offset;


        (*m_normals)[v + 0 * (m_num_segments + 1)] = n0;
        (*m_normals)[v + 1 * (m_num_segments + 1)] = n0;

        (*m_normals)[v + 2 * (m_num_segments + 1)] = n2;
        (*m_normals)[v + 3 * (m_num_segments + 1)] = n2;

        (*m_normals)[v + 4 * (m_num_segments + 1)] = n1;
        (*m_normals)[v + 5 * (m_num_segments + 1)] = n1;

        (*m_normals)[v + 6 * (m_num_segments + 1)] = n3;
        (*m_normals)[v + 7 * (m_num_segments + 1)] = n3;
    }

    {
        int indexOffset = 8 * (m_num_segments + 1);
        float angle0 = m_angularOffset + 90.0 - m_angularLength * 0.5;
        float angle1 = m_angularOffset + 90.0 - m_angularLength * 0.5 - m_arrowLength;
        osg::Vec2 vec0 = osg::Vec2(std::cos(osg::DegreesToRadians(angle0)), std::sin(osg::DegreesToRadians(angle0)));
        osg::Vec2 vec1 = osg::Vec2(std::cos(osg::DegreesToRadians(angle1)), std::sin(osg::DegreesToRadians(angle1)));

        osg::Vec3 v0 = osg::Vec3(vec0 * (m_radius - m_width * 1.5), 0.1);
        osg::Vec3 v1 = osg::Vec3(vec0 * (m_radius + m_width * 1.5), 0.1);
        osg::Vec3 v2 = osg::Vec3(vec1 * (m_radius + m_width * 0.0), 0.1);
        osg::Vec3 v3 = osg::Vec3(vec0 * (m_radius - m_width * 1.5), -0.1);
        osg::Vec3 v4 = osg::Vec3(vec0 * (m_radius + m_width * 1.5), -0.1);
        osg::Vec3 v5 = osg::Vec3(vec1 * (m_radius + m_width * 0.0), -0.1);

        osg::Vec3 n0 = osg::Vec3(0.0, 0.0, 1.0);
        osg::Vec3 n1 = osg::Vec3(0.0, 0.0, -1.0);
        osg::Vec3 n2 = v1 - v0; n2.normalize(); n2 = osg::Vec3(n2.y(), -n2.x(), 0.0);
        osg::Vec3 n3 = v2 - v1; n3.normalize(); n3 = osg::Vec3(n3.y(), -n3.x(), 0.0);
        osg::Vec3 n4 = v0 - v2; n4.normalize(); n4 = osg::Vec3(n4.y(), -n4.x(), 0.0);

        v0 = geometry::swizzle(v0, "xzy");
        v1 = geometry::swizzle(v1, "xzy");
        v2 = geometry::swizzle(v2, "xzy");
        v3 = geometry::swizzle(v3, "xzy");
        v4 = geometry::swizzle(v4, "xzy");
        v5 = geometry::swizzle(v5, "xzy");

        n0 = geometry::swizzle(n0, "xzy");
        n1 = geometry::swizzle(n1, "xzy");
        n2 = geometry::swizzle(n2, "xzy");
        n3 = geometry::swizzle(n3, "xzy");
        n4 = geometry::swizzle(n4, "xzy");

        (*m_points)[indexOffset + 0] = v0 + m_offset;
        (*m_points)[indexOffset + 1] = v1 + m_offset;
        (*m_points)[indexOffset + 2] = v2 + m_offset;

        (*m_points)[indexOffset + 3] = v3 + m_offset;
        (*m_points)[indexOffset + 4] = v4 + m_offset;
        (*m_points)[indexOffset + 5] = v5 + m_offset;

        (*m_points)[indexOffset + 6] = v0 + m_offset;
        (*m_points)[indexOffset + 7] = v1 + m_offset;
        (*m_points)[indexOffset + 8] = v4 + m_offset;
        (*m_points)[indexOffset + 9] = v3 + m_offset;

        (*m_points)[indexOffset + 10] = v1 + m_offset;
        (*m_points)[indexOffset + 11] = v2 + m_offset;
        (*m_points)[indexOffset + 12] = v5 + m_offset;
        (*m_points)[indexOffset + 13] = v4 + m_offset;

        (*m_points)[indexOffset + 14] = v0 + m_offset;
        (*m_points)[indexOffset + 15] = v2 + m_offset;
        (*m_points)[indexOffset + 16] = v5 + m_offset;
        (*m_points)[indexOffset + 17] = v3 + m_offset;


        (*m_normals)[indexOffset + 0] = n0;
        (*m_normals)[indexOffset + 1] = n0;
        (*m_normals)[indexOffset + 2] = n0;

        (*m_normals)[indexOffset + 3] = n1;
        (*m_normals)[indexOffset + 4] = n1;
        (*m_normals)[indexOffset + 5] = n1;

        (*m_normals)[indexOffset + 6] = n2;
        (*m_normals)[indexOffset + 7] = n2;
        (*m_normals)[indexOffset + 8] = n2;
        (*m_normals)[indexOffset + 9] = n2;

        (*m_normals)[indexOffset + 10] = n3;
        (*m_normals)[indexOffset + 11] = n3;
        (*m_normals)[indexOffset + 12] = n3;
        (*m_normals)[indexOffset + 13] = n3;

        (*m_normals)[indexOffset + 14] = n4;
        (*m_normals)[indexOffset + 15] = n4;
        (*m_normals)[indexOffset + 16] = n4;
        (*m_normals)[indexOffset + 17] = n4;
    }

    {
        int indexOffset = 8 * (m_num_segments + 1) + 18;
        float angle0 = m_angularOffset + 90.0 + m_angularLength * 0.5;
        float angle1 = m_angularOffset + 90.0 + m_angularLength * 0.5 + m_arrowLength;
        osg::Vec2 vec0 = osg::Vec2(std::cos(osg::DegreesToRadians(angle0)), std::sin(osg::DegreesToRadians(angle0)));
        osg::Vec2 vec1 = osg::Vec2(std::cos(osg::DegreesToRadians(angle1)), std::sin(osg::DegreesToRadians(angle1)));

        osg::Vec3 v0 = osg::Vec3(vec0 * (m_radius - m_width * 1.5), 0.1);
        osg::Vec3 v1 = osg::Vec3(vec0 * (m_radius + m_width * 1.5), 0.1);
        osg::Vec3 v2 = osg::Vec3(vec1 * (m_radius + m_width * 0.0), 0.1);
        osg::Vec3 v3 = osg::Vec3(vec0 * (m_radius - m_width * 1.5), -0.1);
        osg::Vec3 v4 = osg::Vec3(vec0 * (m_radius + m_width * 1.5), -0.1);
        osg::Vec3 v5 = osg::Vec3(vec1 * (m_radius + m_width * 0.0), -0.1);

        osg::Vec3 n0 = osg::Vec3(0.0, 0.0, 1.0);
        osg::Vec3 n1 = osg::Vec3(0.0, 0.0, -1.0);
        osg::Vec3 n2 = v1 - v0; n2.normalize(); n2 = osg::Vec3(n2.y(), -n2.x(), 0.0);
        osg::Vec3 n3 = v2 - v1; n3.normalize(); n3 = osg::Vec3(n3.y(), -n3.x(), 0.0);
        osg::Vec3 n4 = v0 - v2; n4.normalize(); n4 = osg::Vec3(n4.y(), -n4.x(), 0.0);

        v0 = geometry::swizzle(v0, "xzy");
        v1 = geometry::swizzle(v1, "xzy");
        v2 = geometry::swizzle(v2, "xzy");
        v3 = geometry::swizzle(v3, "xzy");
        v4 = geometry::swizzle(v4, "xzy");
        v5 = geometry::swizzle(v5, "xzy");

        n0 = geometry::swizzle(n0, "xzy");
        n1 = geometry::swizzle(n1, "xzy");
        n2 = geometry::swizzle(n2, "xzy");
        n3 = geometry::swizzle(n3, "xzy");
        n4 = geometry::swizzle(n4, "xzy");

        (*m_points)[indexOffset + 0] = v0 + m_offset;
        (*m_points)[indexOffset + 1] = v1 + m_offset;
        (*m_points)[indexOffset + 2] = v2 + m_offset;

        (*m_points)[indexOffset + 3] = v3 + m_offset;
        (*m_points)[indexOffset + 4] = v4 + m_offset;
        (*m_points)[indexOffset + 5] = v5 + m_offset;

        (*m_points)[indexOffset + 6] = v0 + m_offset;
        (*m_points)[indexOffset + 7] = v1 + m_offset;
        (*m_points)[indexOffset + 8] = v4 + m_offset;
        (*m_points)[indexOffset + 9] = v3 + m_offset;

        (*m_points)[indexOffset + 10] = v1 + m_offset;
        (*m_points)[indexOffset + 11] = v2 + m_offset;
        (*m_points)[indexOffset + 12] = v5 + m_offset;
        (*m_points)[indexOffset + 13] = v4 + m_offset;

        (*m_points)[indexOffset + 14] = v0 + m_offset;
        (*m_points)[indexOffset + 15] = v2 + m_offset;
        (*m_points)[indexOffset + 16] = v5 + m_offset;
        (*m_points)[indexOffset + 17] = v3 + m_offset;


        (*m_normals)[indexOffset + 0] = n0;
        (*m_normals)[indexOffset + 1] = n0;
        (*m_normals)[indexOffset + 2] = n0;

        (*m_normals)[indexOffset + 3] = n1;
        (*m_normals)[indexOffset + 4] = n1;
        (*m_normals)[indexOffset + 5] = n1;

        (*m_normals)[indexOffset + 6] = n2;
        (*m_normals)[indexOffset + 7] = n2;
        (*m_normals)[indexOffset + 8] = n2;
        (*m_normals)[indexOffset + 9] = n2;

        (*m_normals)[indexOffset + 10] = n3;
        (*m_normals)[indexOffset + 11] = n3;
        (*m_normals)[indexOffset + 12] = n3;
        (*m_normals)[indexOffset + 13] = n3;

        (*m_normals)[indexOffset + 14] = n4;
        (*m_normals)[indexOffset + 15] = n4;
        (*m_normals)[indexOffset + 16] = n4;
        (*m_normals)[indexOffset + 17] = n4;
    }

    for (int j = 0; j < 4; ++j)
    {
        for (int i = 0; i < m_num_segments; ++i)
        {
            int triangleOffset = j * m_num_segments * 6 + i * 6;

            (*m_drawElements)[triangleOffset + 0] = (0 + 2 * j) * (m_num_segments + 1) + i + 0;
            (*m_drawElements)[triangleOffset + 1] = (0 + 2 * j) * (m_num_segments + 1) + i + 1;
            (*m_drawElements)[triangleOffset + 2] = (1 + 2 * j) * (m_num_segments + 1) + i + 0;
            (*m_drawElements)[triangleOffset + 3] = (0 + 2 * j) * (m_num_segments + 1) + i + 1;
            (*m_drawElements)[triangleOffset + 4] = (1 + 2 * j) * (m_num_segments + 1) + i + 0;
            (*m_drawElements)[triangleOffset + 5] = (1 + 2 * j) * (m_num_segments + 1) + i + 1;
        }
    }

    {
        int triangleOffset = 24 * m_num_segments;
        int vertexOffset = 8 * (m_num_segments + 1);

        (*m_drawElements)[triangleOffset + 0] = vertexOffset + 0;
        (*m_drawElements)[triangleOffset + 1] = vertexOffset + 1;
        (*m_drawElements)[triangleOffset + 2] = vertexOffset + 2;

        (*m_drawElements)[triangleOffset + 3] = vertexOffset + 3;
        (*m_drawElements)[triangleOffset + 4] = vertexOffset + 4;
        (*m_drawElements)[triangleOffset + 5] = vertexOffset + 5;

        (*m_drawElements)[triangleOffset + 6] = vertexOffset + 6;
        (*m_drawElements)[triangleOffset + 7] = vertexOffset + 7;
        (*m_drawElements)[triangleOffset + 8] = vertexOffset + 8;
        (*m_drawElements)[triangleOffset + 9] = vertexOffset + 6;
        (*m_drawElements)[triangleOffset + 10] = vertexOffset + 8;
        (*m_drawElements)[triangleOffset + 11] = vertexOffset + 9;

        (*m_drawElements)[triangleOffset + 12] = vertexOffset + 10;
        (*m_drawElements)[triangleOffset + 13] = vertexOffset + 11;
        (*m_drawElements)[triangleOffset + 14] = vertexOffset + 12;
        (*m_drawElements)[triangleOffset + 15] = vertexOffset + 10;
        (*m_drawElements)[triangleOffset + 16] = vertexOffset + 12;
        (*m_drawElements)[triangleOffset + 17] = vertexOffset + 13;

        (*m_drawElements)[triangleOffset + 18] = vertexOffset + 14;
        (*m_drawElements)[triangleOffset + 19] = vertexOffset + 15;
        (*m_drawElements)[triangleOffset + 20] = vertexOffset + 16;
        (*m_drawElements)[triangleOffset + 21] = vertexOffset + 14;
        (*m_drawElements)[triangleOffset + 22] = vertexOffset + 16;
        (*m_drawElements)[triangleOffset + 23] = vertexOffset + 17;
    }

    {
        int triangleOffset = 24 * m_num_segments + 24;
        int vertexOffset = 8 * (m_num_segments + 1) + 18;

        (*m_drawElements)[triangleOffset + 0] = vertexOffset + 0;
        (*m_drawElements)[triangleOffset + 1] = vertexOffset + 1;
        (*m_drawElements)[triangleOffset + 2] = vertexOffset + 2;

        (*m_drawElements)[triangleOffset + 3] = vertexOffset + 3;
        (*m_drawElements)[triangleOffset + 4] = vertexOffset + 4;
        (*m_drawElements)[triangleOffset + 5] = vertexOffset + 5;

        (*m_drawElements)[triangleOffset + 6] = vertexOffset + 6;
        (*m_drawElements)[triangleOffset + 7] = vertexOffset + 7;
        (*m_drawElements)[triangleOffset + 8] = vertexOffset + 8;
        (*m_drawElements)[triangleOffset + 9] = vertexOffset + 6;
        (*m_drawElements)[triangleOffset + 10] = vertexOffset + 8;
        (*m_drawElements)[triangleOffset + 11] = vertexOffset + 9;

        (*m_drawElements)[triangleOffset + 12] = vertexOffset + 10;
        (*m_drawElements)[triangleOffset + 13] = vertexOffset + 11;
        (*m_drawElements)[triangleOffset + 14] = vertexOffset + 12;
        (*m_drawElements)[triangleOffset + 15] = vertexOffset + 10;
        (*m_drawElements)[triangleOffset + 16] = vertexOffset + 12;
        (*m_drawElements)[triangleOffset + 17] = vertexOffset + 13;

        (*m_drawElements)[triangleOffset + 18] = vertexOffset + 14;
        (*m_drawElements)[triangleOffset + 19] = vertexOffset + 15;
        (*m_drawElements)[triangleOffset + 20] = vertexOffset + 16;
        (*m_drawElements)[triangleOffset + 21] = vertexOffset + 14;
        (*m_drawElements)[triangleOffset + 22] = vertexOffset + 16;
        (*m_drawElements)[triangleOffset + 23] = vertexOffset + 17;
    }

    setVertexArray(m_points);
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    setNormalArray(m_normals, osg::Array::BIND_PER_VERTEX);
#else
    setNormalArray(m_normals);
#endif
    setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    // Remove all previous primitive sets
    removePrimitiveSet(0, getNumPrimitiveSets());
    addPrimitiveSet(m_drawElements);
}
