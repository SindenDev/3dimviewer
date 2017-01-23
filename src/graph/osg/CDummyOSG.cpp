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

#include <osg/CDummyOSG.h>
#include <iostream>
#include <osg/PositionAttitudeTransform>
#include <osg/Version>

//====================================================================================================================
void scene::CDummyGeometry::squarePlaneXY(osg::Geometry * geometry, bool thin)
{
	osg::Vec3Array * plane_vertices = new osg::Vec3Array;

	double thickness(thin ? 0.001 : 0.01);

    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, thickness ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 0.0, thickness ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 1.0, thickness ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 1.0, thickness ) );

    plane_vertices->push_back( osg::Vec3( 0.0, 0.0, -thickness ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 0.0, -thickness ) );
    plane_vertices->push_back( osg::Vec3( 1.0, 1.0, -thickness ) );
    plane_vertices->push_back( osg::Vec3( 0.0, 1.0, -thickness ) );

	geometry->setVertexArray( plane_vertices );

	osg::DrawElementsUInt* plane_ps = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 24 );	

    (*plane_ps)[0] = 0;
	(*plane_ps)[1] = 1;
	(*plane_ps)[2] = 2;
	(*plane_ps)[3] = 3;

	(*plane_ps)[4] = 4;
	(*plane_ps)[5] = 5;
	(*plane_ps)[6] = 6;
	(*plane_ps)[7] = 7;

	(*plane_ps)[8] = 0;
	(*plane_ps)[9] = 1;
	(*plane_ps)[10] = 5;
	(*plane_ps)[11] = 4;

	(*plane_ps)[12] = 1;
	(*plane_ps)[13] = 2;
	(*plane_ps)[14] = 6;
	(*plane_ps)[15] = 5;

	(*plane_ps)[16] = 2;
	(*plane_ps)[17] = 3;
	(*plane_ps)[18] = 7;
	(*plane_ps)[19] = 6;

	(*plane_ps)[20] = 3;
	(*plane_ps)[21] = 0;
	(*plane_ps)[22] = 4;
	(*plane_ps)[23] = 7;

	geometry->addPrimitiveSet( plane_ps );

	osg::Vec4Array * vertex_colors = new osg::Vec4Array(1);
	(*vertex_colors)[0] = osg::Vec4( 0.0, 0.0, 1.0, 0.2 );
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	geometry->setColorArray( vertex_colors, osg::Array::BIND_OVERALL );
#else
	geometry->setColorArray( vertex_colors );
#endif
	geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
}

//====================================================================================================================
void scene::CDummyGeometry::squarePlaneXZ( osg::Geometry * geometry, bool thin )
{
    osg::Vec3Array * plane_vertices = new osg::Vec3Array;

	double thickness(thin ? 0.001 : 0.01);

    plane_vertices->push_back( osg::Vec3( 0.0, thickness, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, thickness, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, thickness, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, thickness, 1.0 ) );

    plane_vertices->push_back( osg::Vec3( 0.0, -thickness, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, -thickness, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( 1.0, -thickness, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( 0.0, -thickness, 1.0 ) );

	geometry->setVertexArray( plane_vertices );

	osg::DrawElementsUInt* plane_ps = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 24 );	

    (*plane_ps)[0] = 0;
	(*plane_ps)[1] = 1;
	(*plane_ps)[2] = 2;
	(*plane_ps)[3] = 3;

	(*plane_ps)[4] = 4;
	(*plane_ps)[5] = 5;
	(*plane_ps)[6] = 6;
	(*plane_ps)[7] = 7;

	(*plane_ps)[8] = 0;
	(*plane_ps)[9] = 1;
	(*plane_ps)[10] = 5;
	(*plane_ps)[11] = 4;

	(*plane_ps)[12] = 1;
	(*plane_ps)[13] = 2;
	(*plane_ps)[14] = 6;
	(*plane_ps)[15] = 5;

	(*plane_ps)[16] = 2;
	(*plane_ps)[17] = 3;
	(*plane_ps)[18] = 7;
	(*plane_ps)[19] = 6;

	(*plane_ps)[20] = 3;
	(*plane_ps)[21] = 0;
	(*plane_ps)[22] = 4;
	(*plane_ps)[23] = 7;

	geometry->addPrimitiveSet( plane_ps );

	osg::Vec4Array * vertex_colors = new osg::Vec4Array(1);
	(*vertex_colors)[0] = osg::Vec4( 0.0, 1.0, 0.0, 0.2 );

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	geometry->setColorArray( vertex_colors, osg::Array::BIND_OVERALL );
#else
	geometry->setColorArray( vertex_colors );
#endif
	geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
}

//====================================================================================================================
void scene::CDummyGeometry::squarePlaneYZ( osg::Geometry * geometry, bool thin )
{
	osg::Vec3Array * plane_vertices = new osg::Vec3Array;

	double thickness(thin ? 0.001 : 0.01);

    plane_vertices->push_back( osg::Vec3( thickness, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( thickness, 1.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( thickness, 1.0, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( thickness, 0.0, 1.0 ) );

    plane_vertices->push_back( osg::Vec3( -thickness, 0.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( -thickness, 1.0, 0.0 ) );
    plane_vertices->push_back( osg::Vec3( -thickness, 1.0, 1.0 ) );
    plane_vertices->push_back( osg::Vec3( -thickness, 0.0, 1.0 ) );

	geometry->setVertexArray( plane_vertices );

	osg::DrawElementsUInt* plane_ps = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 24 );	

    (*plane_ps)[0] = 0;
	(*plane_ps)[1] = 1;
	(*plane_ps)[2] = 2;
	(*plane_ps)[3] = 3;

	(*plane_ps)[4] = 4;
	(*plane_ps)[5] = 5;
	(*plane_ps)[6] = 6;
	(*plane_ps)[7] = 7;

	(*plane_ps)[8] = 0;
	(*plane_ps)[9] = 1;
	(*plane_ps)[10] = 5;
	(*plane_ps)[11] = 4;

	(*plane_ps)[12] = 1;
	(*plane_ps)[13] = 2;
	(*plane_ps)[14] = 6;
	(*plane_ps)[15] = 5;

	(*plane_ps)[16] = 2;
	(*plane_ps)[17] = 3;
	(*plane_ps)[18] = 7;
	(*plane_ps)[19] = 6;

	(*plane_ps)[20] = 3;
	(*plane_ps)[21] = 0;
	(*plane_ps)[22] = 4;
	(*plane_ps)[23] = 7;

	geometry->addPrimitiveSet( plane_ps );

	osg::Vec4Array * vertex_colors = new osg::Vec4Array(1);
	(*vertex_colors)[0]	= osg::Vec4( 1.0, 0.0, 0.0, 0.2 );

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	geometry->setColorArray( vertex_colors, osg::Array::BIND_OVERALL );
#else
	geometry->setColorArray( vertex_colors );
#endif
	geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
}

//====================================================================================================================

void scene::CDummyGeometry::cube( osg::Geometry * geometry, bool generateNormals )
{
	if (generateNormals)
	{
		osg::Vec3Array * vertices = new osg::Vec3Array;
		vertices->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
		vertices->push_back( osg::Vec3( 1.0, 0.0, 0.0 ) );
		vertices->push_back( osg::Vec3( 1.0, 1.0, 0.0 ) );
		vertices->push_back( osg::Vec3( 0.0, 1.0, 0.0 ) );

		vertices->push_back( osg::Vec3( 0.0, 0.0, 1.0 ) );
		vertices->push_back( osg::Vec3( 1.0, 0.0, 1.0 ) );
		vertices->push_back( osg::Vec3( 1.0, 1.0, 1.0 ) );
		vertices->push_back( osg::Vec3( 0.0, 1.0, 1.0 ) );

		vertices->push_back((*vertices)[0]);
		vertices->push_back((*vertices)[1]);
		vertices->push_back((*vertices)[5]);
		vertices->push_back((*vertices)[4]);

		vertices->push_back((*vertices)[1]);
		vertices->push_back((*vertices)[2]);
		vertices->push_back((*vertices)[6]);
		vertices->push_back((*vertices)[5]);

		vertices->push_back((*vertices)[2]);
		vertices->push_back((*vertices)[3]);
		vertices->push_back((*vertices)[7]);
		vertices->push_back((*vertices)[6]);

		vertices->push_back((*vertices)[3]);
		vertices->push_back((*vertices)[0]);
		vertices->push_back((*vertices)[4]);
		vertices->push_back((*vertices)[7]);

		geometry->setVertexArray( vertices );

		osg::DrawElementsUInt* cube = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 24 );

		for(int i = 0; i < 24; i++)
			(*cube)[i] = i;

		geometry->addPrimitiveSet( cube );

		osg::Vec4Array * vertex_colors = new osg::Vec4Array(1);
		(*vertex_colors)[0] = osg::Vec4( 0.0, 1.0, 0.0, 0.2 );

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
		geometry->setColorArray( vertex_colors, osg::Array::BIND_OVERALL );
#else
		geometry->setColorArray( vertex_colors );
#endif
		geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
		
		{
			osg::Vec3Array * normals = new osg::Vec3Array;
			for(int i=0;i<4;i++)
				normals->push_back( osg::Vec3(  0.0,  0.0, -1.0 ) );
			for(int i=0;i<4;i++)
				normals->push_back( osg::Vec3(  0.0,  0.0,  1.0 ) );
			for(int i=0;i<4;i++)
				normals->push_back( osg::Vec3(  0.0, -1.0,  0.0 ) );
			for(int i=0;i<4;i++)
				normals->push_back( osg::Vec3(  1.0,  0.0,  0.0 ) );
			for(int i=0;i<4;i++)
				normals->push_back( osg::Vec3(  0.0,  1.0,  0.0 ) );
			for(int i=0;i<4;i++)
				normals->push_back( osg::Vec3( -1.0,  0.0,  0.0 ) );
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
			geometry->setNormalArray( normals, osg::Array::BIND_PER_VERTEX );
#else
			geometry->setNormalArray( normals );
#endif
			geometry->setNormalBinding(Geometry::BIND_PER_VERTEX);
		}
	}
	else
	{
		osg::Vec3Array * vertices = new osg::Vec3Array;
		vertices->push_back( osg::Vec3( 0.0, 0.0, 0.0 ) );
		vertices->push_back( osg::Vec3( 1.0, 0.0, 0.0 ) );
		vertices->push_back( osg::Vec3( 1.0, 1.0, 0.0 ) );
		vertices->push_back( osg::Vec3( 0.0, 1.0, 0.0 ) );

		vertices->push_back( osg::Vec3( 0.0, 0.0, 1.0 ) );
		vertices->push_back( osg::Vec3( 1.0, 0.0, 1.0 ) );
		vertices->push_back( osg::Vec3( 1.0, 1.0, 1.0 ) );
		vertices->push_back( osg::Vec3( 0.0, 1.0, 1.0 ) );

		geometry->setVertexArray( vertices );

		osg::DrawElementsUInt* cube = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 24 );

		(*cube)[0] = 0;
		(*cube)[1] = 1;
		(*cube)[2] = 2;
		(*cube)[3] = 3;

		(*cube)[4] = 4;
		(*cube)[5] = 5;
		(*cube)[6] = 6;
		(*cube)[7] = 7;

		(*cube)[8] = 0;
		(*cube)[9] = 1;
		(*cube)[10] = 5;
		(*cube)[11] = 4;

		(*cube)[12] = 1;
		(*cube)[13] = 2;
		(*cube)[14] = 6;
		(*cube)[15] = 5;

		(*cube)[16] = 2;
		(*cube)[17] = 3;
		(*cube)[18] = 7;
		(*cube)[19] = 6;

		(*cube)[20] = 3;
		(*cube)[21] = 0;
		(*cube)[22] = 4;
		(*cube)[23] = 7;

		geometry->addPrimitiveSet( cube );

		osg::Vec4Array * vertex_colors = new osg::Vec4Array(1);
		(*vertex_colors)[0] = osg::Vec4( 0.0, 1.0, 0.0, 0.2 );

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
		geometry->setColorArray( vertex_colors, osg::Array::BIND_OVERALL );
#else
		geometry->setColorArray( vertex_colors );
#endif
		geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
	}
}

//====================================================================================================================
void scene::CDummyGeometry::setColor( const osg::Vec4 & color )
{
	osg::Vec4Array * vertex_colors = new osg::Vec4Array(1);
	(*vertex_colors)[0] = color;

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	this->setColorArray( vertex_colors, osg::Array::BIND_OVERALL );
#else
	this->setColorArray( vertex_colors );
#endif
	this->setColorBinding( osg::Geometry::BIND_OVERALL );
}

//====================================================================================================================
scene::CDummyDraggableGeode::CDummyDraggableGeode() 
    : osg::Geode()
	, CInvisibleObjectInterface( false )
    , p_Dragger( 0 )
{
	// setup transparency 
	osg::StateSet * state_set = new osg::StateSet();
	state_set->setMode( GL_BLEND, osg::StateAttribute::ON );
	state_set->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );	
	state_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask( false );
	state_set->setAttributeAndModes( depth, osg::StateAttribute::ON );
	state_set->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

	// setup polygon offser
	osg::PolygonOffset * offset = new osg::PolygonOffset( -2.0f, -2.0f );
	state_set->setAttributeAndModes( offset, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
	
	this->setStateSet( state_set );
}

//====================================================================================================================
scene::CDummyDraggableGeode::~CDummyDraggableGeode()
{
}

//====================================================================================================================
void scene::CDummyDraggableGeode::setUpSquarePlaneXY(bool thin)
{
	if (p_Geometry.valid())
		this->removeDrawable(p_Geometry);

	p_Geometry = new CDummyGeometry;
	scene::CDummyGeometry::squarePlaneXY( p_Geometry.get(), thin );
	setGeometry( p_Geometry.get() );

}

//====================================================================================================================
void scene::CDummyDraggableGeode::setUpSquarePlaneXZ(bool thin)
{
	if (p_Geometry.valid())
		this->removeDrawable(p_Geometry);

	p_Geometry = new CDummyGeometry;
	scene::CDummyGeometry::squarePlaneXZ( p_Geometry.get(), thin );
	setGeometry( p_Geometry.get() );
}

//====================================================================================================================
void scene::CDummyDraggableGeode::setUpSquarePlaneYZ(bool thin)
{
	if (p_Geometry.valid())
		this->removeDrawable(p_Geometry);

	p_Geometry = new CDummyGeometry;
	scene::CDummyGeometry::squarePlaneYZ( p_Geometry.get(), thin );
	setGeometry( p_Geometry.get() );
}

//====================================================================================================================
void scene::CDummyDraggableGeode::setUpCube()
{
	p_Geometry = new CDummyGeometry;
	scene::CDummyGeometry::cube( p_Geometry.get() );
	setGeometry( p_Geometry.get() );
}

//====================================================================================================================
void scene::CDummyDraggableGeode::setGeometry( osg::Geometry * geometry )
{
	this->addDrawable( geometry );

    // Set cull callback to the geometry
    setForceCullCallback( geometry );
}		

//====================================================================================================================
osgManipulator::Dragger * scene::CDummyDraggableGeode::getDragger()
{
	return p_Dragger.get();
}

//====================================================================================================================
void scene::CDummyDraggableGeode::setDragger( osgManipulator::Dragger * d )
{
	p_Dragger = d;
}	

//====================================================================================================================
void scene::CDummyDraggableGeode::setColor( float r, float g, float b )
{
	assert( p_Geometry );

	osg::Vec4Array * color = dynamic_cast<osg::Vec4Array *>( p_Geometry->getColorArray() );
	(*color)[0][0] = r;
	(*color)[0][1] = g;
	(*color)[0][2] = b;
}

//====================================================================================================================
void scene::CDummyDraggableGeode::visible( bool v )
{
/*
    assert( p_Geometry );
	float alpha = ( v ? 0.2 : 0.0 );
	osg::Vec4Array * color = dynamic_cast<osg::Vec4Array *>( p_Geometry->getColorArray() );
	(*color)[0][3] = alpha;
	p_Geometry->dirtyDisplayList();
    */
    setIObjectVisible( v );
}


//====================================================================================================================
scene::CDummyCubeGeode::CDummyCubeGeode()
    : CInvisibleObjectInterface( false )
{
	// setup transparency 
	osg::StateSet * state_set = new osg::StateSet();
	state_set->setMode( GL_BLEND, osg::StateAttribute::ON );
	state_set->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );	
	state_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask( false );
	state_set->setAttributeAndModes( depth, osg::StateAttribute::ON );
	state_set->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

	// setup polygon offser
	osg::PolygonOffset * offset = new osg::PolygonOffset( -2.0f, -2.0f );
	state_set->setAttributeAndModes( offset, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
	
	this->setStateSet( state_set );

    // Create geometry
}

//====================================================================================================================
scene::CDummyCubeGeode::~CDummyCubeGeode()
{
}

//====================================================================================================================
void scene::CDummyCubeGeode::setUpCube()
{
	p_Geometry = new CDummyGeometry;
	scene::CDummyGeometry::cube( p_Geometry.get() );
	setGeometry( p_Geometry.get() );
}

//====================================================================================================================
void scene::CDummyCubeGeode::setGeometry( osg::Geometry * geometry )
{
	this->addDrawable( geometry );

    // Set cull callback to the geometry
    setForceCullCallback( geometry );
}

//====================================================================================================================
void scene::CDummyCubeGeode::setColor( float r, float g, float b )
{
    osg::Vec4Array * color = dynamic_cast<osg::Vec4Array *>( p_Geometry->getColorArray() );
	(*color)[0][0] = r;
	(*color)[0][1] = g;
	(*color)[0][2] = b;
	p_Geometry->dirtyDisplayList();
}

void scene::CDummyCubeGeode::setAlpha( float a )
{
    osg::Vec4Array * color = dynamic_cast<osg::Vec4Array *>( p_Geometry->getColorArray() );
	(*color)[0][3] = a;
	p_Geometry->dirtyDisplayList();
}

//====================================================================================================================
void scene::CDummyCubeGeode::visible( bool v )
{
    /*
	float alpha = ( v ? 0.1 : 0.0 );
	osg::Vec4Array * color = dynamic_cast<osg::Vec4Array *>( p_Geometry->getColorArray() );
	(*color)[0][3] = alpha;
	p_Geometry->dirtyDisplayList();
    */

    setIObjectVisible( v );
}

//====================================================================================================================
scene::CGizmo::CGizmo( double size, const osg::Vec3 & offset )
{
	d_Size		= size;
	p_Geode		= new osg::Geode();
	p_Geometry  = new osg::Geometry();
	p_StateSet  = new osg::StateSet();
	p_LineWidth = new osg::LineWidth();
    setName("CGizmo");

	//this->setAutoScaleToScreen( true );

	//osg::PositionAttitudeTransform	* trans = new osg::PositionAttitudeTransform();
	//osg::Vec3 modelScale;
	//double value = 1.0;
 	//modelScale.set(value,value,value); 
 	//trans->setScale( modelScale );
	//this->addChild( trans );

	// create color array for gizmo
	osg::Vec4Array * color_array = new osg::Vec4Array( 1 );
	(*color_array)[0] = osg::Vec4( 1.0, 1.0, 1.0, 1.0 );
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	p_Geometry->setColorArray( color_array, osg::Array::BIND_OVERALL );
#else
	p_Geometry->setColorArray( color_array );
#endif
	p_Geometry->setColorBinding( osg::Geometry::BIND_OVERALL );

	// set up line width
	p_LineWidth->setWidth( 1.0 );
	p_StateSet->setAttributeAndModes( p_LineWidth.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );		

	// set up always-render property
    p_StateSet->setMode( GL_DEPTH_TEST,osg::StateAttribute::OFF );
    p_StateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    p_StateSet->setRenderBinMode( osg::StateSet::OVERRIDE_RENDERBIN_DETAILS );//:USE_RENDERBIN_DETAILS);
    p_StateSet->setRenderBinDetails( 111, "RenderBin" );
	
	p_Geometry->setStateSet( p_StateSet.get() );
	p_Geode->addDrawable( p_Geometry.get() );
	
//	trans->addChild( p_Geode.get() );
    this->addChild( p_Geode.get() );
}

//====================================================================================================================
void scene::CGizmo::createGeometry( const osg::Vec3 & offset )
{
}

//====================================================================================================================
void scene::CGizmo::createDummy( const osg::Vec3 & offset )
{
}

//====================================================================================================================
void scene::CGizmo::setColor( const osg::Vec4 & color )
{
	osg::Vec4Array * c = dynamic_cast< osg::Vec4Array* >( p_Geometry->getColorArray() );
	if (NULL!=c)
	{
		for(int i = 0; i < c->size(); i++)
			(*c)[i] = color;
		p_Geometry->dirtyDisplayList();
	}
}

//====================================================================================================================
osg::Vec4 scene::CGizmo::getColor()
{
	osg::Vec4Array * c = static_cast< osg::Vec4Array* >( p_Geometry->getColorArray() );
	return (*c)[0];
}

//====================================================================================================================
void scene::CGizmo::setSize( double size )
{
	d_Size = size;
}

//====================================================================================================================
double scene::CGizmo::getSize()
{
	return d_Size;
}

//====================================================================================================================
void scene::CGizmo::setThickness( double thickness )
{
	p_LineWidth->setWidth( static_cast< float >( thickness ) );
}

//====================================================================================================================
double scene::CGizmo::getThickness()
{
	return p_LineWidth->getWidth();
}

//====================================================================================================================
scene::CTriangleGizmo::CTriangleGizmo( double size, const osg::Vec3 & offset ) : CGizmo( size ), m_Orientation(1,1,1)
{
	createGeometry( offset );
	createDummy( offset );
}

//====================================================================================================================
void scene::CTriangleGizmo::createGeometry( const osg::Vec3 & offset )
{
    m_Offset = offset;

	osg::Vec3Array * vertices = new osg::Vec3Array( 3 );
	(*vertices)[0] =  m_Offset + osg::Vec3( 0.5 * d_Size * m_Orientation.x(), -0.5 * d_Size * m_Orientation.y(), 0.0 );
	(*vertices)[1] =  m_Offset + osg::Vec3( 0.5 * d_Size * m_Orientation.x(), 0.5 * d_Size  * m_Orientation.y(), 0.0 );
	(*vertices)[2] =  m_Offset + osg::Vec3( -0.5 * d_Size* m_Orientation.x(), 0.0, 0.0 );

	osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt( osg::PrimitiveSet::LINE_LOOP, 3 );
	(*primitives)[0] = 0;
	(*primitives)[1] = 1;
	(*primitives)[2] = 2;

	p_Geometry->setVertexArray( vertices );
	p_Geometry->addPrimitiveSet( primitives );
}

//====================================================================================================================
void scene::CTriangleGizmo::createDummy( const osg::Vec3 & offset )
{
	osg::Box *			 dummy_box		=	new osg::Box( offset, 1.0 * d_Size, 1.0 * d_Size, 0.1 * d_Size );
	osg::ShapeDrawable * dummy_drawable	=   new osg::ShapeDrawable( dummy_box );

	osg::StateSet * dummy_set = new osg::StateSet();
	dummy_set->setMode( GL_BLEND, osg::StateAttribute::ON );
	dummy_set->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );	
	dummy_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask( false );
	dummy_set->setAttributeAndModes( depth, osg::StateAttribute::ON );
	dummy_set->setMode( GL_LIGHTING, osg::StateAttribute::OFF );			
	dummy_drawable->setStateSet( dummy_set );
	dummy_drawable->setColor( osg::Vec4( 1.0, 1.0, 1.0, 0.0 ) );
    osgManipulator::setDrawableToAlwaysCull(*dummy_drawable);

	p_Geode->addDrawable( dummy_drawable );
}

//====================================================================================================================
void scene::CTriangleGizmo::setSizeAndOffset( double size, const osg::Vec3 &offset, const osg::Vec3 & orientation )
{
    for ( unsigned int i = 0; i < p_Geometry->getNumPrimitiveSets(); i++ ) p_Geometry->removePrimitiveSet( i );
    p_Geode->removeDrawables( 0, p_Geode->getNumDrawables() );
    p_Geode->addDrawable( p_Geometry.get() );

    d_Size = size;
    m_Offset = offset;
    m_Orientation = orientation;

    createGeometry( offset );
    createDummy( offset );
}

void scene::CTriangleGizmo::setOrientation(const osg::Vec3 & orientation)
{
    m_Orientation = orientation;
    createGeometry( m_Offset );
    createDummy( m_Offset );
}

//====================================================================================================================
scene::CCircleGizmo::CCircleGizmo( double size, const osg::Vec3 & offset ) :
    CGizmo( size )
{
    createGeometry( offset );
	createDummy( offset );
}

//====================================================================================================================
void scene::CCircleGizmo::createGeometry( const osg::Vec3 & offset )
{
    m_Offset = offset;

    int segs    =   50;

    double da   =   2 * 3.1415926   /   segs;
    double sda  =   sin( da );
    double cda  =   cos( da );

    double x    =   0.5 * d_Size;
    double y    =   0.0;

    double nx, ny;

    osg::Vec3Array * vertices           = new osg::Vec3Array( segs );
    osg::DrawElementsUInt* primitives   = new osg::DrawElementsUInt( osg::PrimitiveSet::LINE_LOOP, segs );

    for ( int i = 0; i < segs; i++ )
    {
        (*vertices)[i]      =  offset + osg::Vec3( x, y, 0.0 );
        (*primitives)[i]    = i;

        nx = cda * x + -sda * y;
        ny = sda * x + cda * y; 

        x = nx;
        y = ny;
    }

	p_Geometry->setVertexArray( vertices );
	p_Geometry->addPrimitiveSet( primitives );
}

//====================================================================================================================
void scene::CCircleGizmo::createDummy( const osg::Vec3 & offset )
{
	osg::Box *			 dummy_box		=	new osg::Box( offset, 1.0 * d_Size, 1.0 * d_Size, 0.2 * d_Size );
	osg::ShapeDrawable * dummy_drawable	=   new osg::ShapeDrawable( dummy_box );

	osg::StateSet * dummy_set = new osg::StateSet();
	dummy_set->setMode( GL_BLEND, osg::StateAttribute::ON );
	dummy_set->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );	
	dummy_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask( false );
	dummy_set->setAttributeAndModes( depth, osg::StateAttribute::ON );
	dummy_set->setMode( GL_LIGHTING, osg::StateAttribute::OFF );			
	dummy_drawable->setStateSet( dummy_set );
	dummy_drawable->setColor( osg::Vec4( 1.0, 1.0, 1.0, 0.0 ) );
    osgManipulator::setDrawableToAlwaysCull(*dummy_drawable);

	p_Geode->addDrawable( dummy_drawable );
}

//====================================================================================================================
void scene::CCircleGizmo::setSize( double size )
{
    for ( unsigned int i = 0; i < p_Geometry->getNumPrimitiveSets(); i++ ) p_Geometry->removePrimitiveSet( i );
    p_Geode->removeDrawables( 0, p_Geode->getNumDrawables() );
    p_Geode->addDrawable( p_Geometry.get() );

    d_Size = size;

    createGeometry( m_Offset );
    createDummy( m_Offset );
}

//====================================================================================================================
void scene::CCircleGizmo::setSizeAndOffset( double size, const osg::Vec3 &offset )
{
    for ( unsigned int i = 0; i < p_Geometry->getNumPrimitiveSets(); i++ ) p_Geometry->removePrimitiveSet( i );
    p_Geode->removeDrawables( 0, p_Geode->getNumDrawables() );
    p_Geode->addDrawable( p_Geometry.get() );

    d_Size = size;
    m_Offset = offset;

    createGeometry( m_Offset );
    createDummy( m_Offset );
}

//====================================================================================================================
scene::CLineGizmo::CLineGizmo( double size, const osg::Vec3 & offset, const osg::Vec3 & orientation ) : CGizmo( size )
{
	createGeometry( offset, orientation );
	createDummy( offset, orientation );
}

//====================================================================================================================
void scene::CLineGizmo::createGeometry( const osg::Vec3 & offset, const osg::Vec3 & orientation )
{
	m_Offset = offset;
	m_Orientation	=	orientation;

	osg::Vec3Array * vertices = new osg::Vec3Array( 2 );

	(*vertices)[0]	=	m_Offset - ( m_Orientation * 0.5 * d_Size );
	(*vertices)[1]	=	m_Offset + ( m_Orientation * 0.5 * d_Size );

	osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt( osg::PrimitiveSet::LINES, 2 );
	(*primitives)[0] = 0;
	(*primitives)[1] = 1;

	p_Geometry->setVertexArray( vertices );
	p_Geometry->addPrimitiveSet( primitives );
}

//====================================================================================================================
void scene::CLineGizmo::createDummy( const osg::Vec3 & offset, const osg::Vec3 & orientation )
{
	
	//osg::MatrixTransform	*	transform = new osg::MatrixTransform;
	//osg::Geode				*	geode	  = new osg::Geode;
    this->removeChild(p_DummyTransform.get());

	p_DummyGeode		= new osg::Geode;
	p_DummyTransform	= new osg::MatrixTransform;

	osg::Box *			 dummy_box		=	new osg::Box( osg::Vec3( 0.0, 0.0, 0.0 ), 0.1 * d_Size, 1.1 * d_Size, 0.1 * d_Size );
	osg::ShapeDrawable * dummy_drawable	=   new osg::ShapeDrawable( dummy_box );

	osg::StateSet * dummy_set = new osg::StateSet();
	dummy_set->setMode( GL_BLEND, osg::StateAttribute::ON );
	dummy_set->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );	
	dummy_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask( false );
	dummy_set->setAttributeAndModes( depth, osg::StateAttribute::ON );
	dummy_set->setMode( GL_LIGHTING, osg::StateAttribute::OFF );			
	dummy_drawable->setStateSet( dummy_set );
	dummy_drawable->setColor( osg::Vec4( 1.0, 1.0, 1.0, 0.0 ) );

	osg::Matrix	m	= osg::Matrix::rotate( osg::Vec3( 0.0, 1.0, 0.0 ), m_Orientation );
	m = m * osg::Matrix::translate( m_Offset );

	p_DummyTransform->setMatrix( m );
	p_DummyGeode->addDrawable( dummy_drawable );
	p_DummyTransform->addChild( p_DummyGeode.get() );
	this->addChild( p_DummyTransform.get() );
}

void scene::CLineGizmo::setOffset( const osg::Vec3 & offset )
{
	m_Offset = offset;

	osg::Vec3Array * vertices = static_cast< osg::Vec3Array* >( p_Geometry->getVertexArray() );

	(*vertices)[0]	=	m_Offset - ( m_Orientation * 0.5 * d_Size );
	(*vertices)[1]	=	m_Offset + ( m_Orientation * 0.5 * d_Size );

	p_Geometry->dirtyBound();
	p_Geometry->dirtyDisplayList();
}

//====================================================================================================================
void scene::CLineGizmo::setSize( double size )
{
	////////////////////////////////////////////////////
	d_Size = size;

	osg::Vec3Array * vertices = static_cast< osg::Vec3Array* >( p_Geometry->getVertexArray() );

	(*vertices)[0]	=	m_Offset - ( m_Orientation * 0.5 * d_Size );
	(*vertices)[1]	=	m_Offset + ( m_Orientation * 0.5 * d_Size );

	p_Geometry->dirtyBound();
	p_Geometry->dirtyDisplayList();

    createDummy( m_Offset, m_Orientation );
}

//====================================================================================================================
scene::CSquareGizmo::CSquareGizmo( double size, const osg::Vec3 & offset, double dummyScale /* = 1.2 */ ) : CGizmo( size )
{					
	createGeometry( offset );
	createDummy( offset, dummyScale );
}

//====================================================================================================================
void	scene::CSquareGizmo::createGeometry( const osg::Vec3 & offset )
{
    m_Offset = offset;

	osg::Vec3Array * vertices = new osg::Vec3Array( 4 );
	(*vertices)[0] =  osg::Vec3( -0.5 * d_Size, -0.5 * d_Size, 0.0 ) + offset;
	(*vertices)[1] =  osg::Vec3( 0.5 * d_Size, -0.5 * d_Size, 0.0 ) + offset;
	(*vertices)[2] =  osg::Vec3( 0.5 * d_Size, 0.5 * d_Size, 0.0 ) + offset;
	(*vertices)[3] =  osg::Vec3( -0.5 * d_Size, 0.5 * d_Size, 0.0 ) + offset;

	osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt( osg::PrimitiveSet::LINE_LOOP, 4 );
	(*primitives)[0] = 0;
	(*primitives)[1] = 1;
	(*primitives)[2] = 2;
	(*primitives)[3] = 3;

	p_Geometry->setVertexArray( vertices );
    //p_Geometry->setPrimitiveSet( 0, primitives ); 
	p_Geometry->addPrimitiveSet( primitives );
}

//====================================================================================================================
void	scene::CSquareGizmo::createDummy( const osg::Vec3 & offset, double dummyScale )
{
	osg::Box *			 dummy_box		=	new osg::Box( offset, dummyScale * d_Size, dummyScale * d_Size, 0.1 * d_Size );
	osg::ShapeDrawable * dummy_drawable	=   new osg::ShapeDrawable( dummy_box );

	osg::StateSet * dummy_set = new osg::StateSet();
	dummy_set->setMode( GL_BLEND, osg::StateAttribute::ON );
	dummy_set->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );	
	dummy_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask( false );
	dummy_set->setAttributeAndModes( depth, osg::StateAttribute::ON );
	dummy_set->setMode( GL_LIGHTING, osg::StateAttribute::OFF );			
	dummy_drawable->setStateSet( dummy_set );
	dummy_drawable->setColor( osg::Vec4( 1.0, 1.0, 1.0, 0.0 ) );

	p_Geode->addDrawable( dummy_drawable );
}

//====================================================================================================================
void scene::CSquareGizmo::setSize( double size, double dummyScale )
{
    for ( unsigned int i = 0; i < p_Geometry->getNumPrimitiveSets(); i++ ) p_Geometry->removePrimitiveSet( i );
    p_Geode->removeDrawables( 0, p_Geode->getNumDrawables() );
    p_Geode->addDrawable( p_Geometry.get() );

    d_Size = size;

    createGeometry( m_Offset );
    createDummy( m_Offset, dummyScale );
}

//====================================================================================================================
scene::COrientableLineGizmo::COrientableLineGizmo( double size, const osg::Vec3 & offset, const osg::Vec3 & orientation ) :
	CGizmo( size )
{
	d_Size			=	size;
	m_Offset		=	offset;
	m_Orientation	=	orientation;

	this->createGeometry( offset, orientation );
	this->createDummy( offset, orientation );
}

//====================================================================================================================
void	scene::COrientableLineGizmo::createGeometry( const osg::Vec3 & offset, const osg::Vec3 & orientation )
{
	osg::Vec3Array * vertices = new osg::Vec3Array( 2 );

	(*vertices)[0]	=	m_Offset;
	(*vertices)[1]	=	( m_Offset + m_Orientation );

	osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt( osg::PrimitiveSet::LINES, 2 );
	(*primitives)[0] = 0;
	(*primitives)[1] = 1;

	p_Geometry->setVertexArray( vertices );
	p_Geometry->addPrimitiveSet( primitives );
}

//====================================================================================================================
void	scene::COrientableLineGizmo::createDummy( const osg::Vec3 & offset, const osg::Vec3 & orientation )
{
	p_DummyGeometry = new osg::Geometry();
	p_DummyGeode	= new osg::Geode();

	osg::Vec3Array * vertices = new osg::Vec3Array( 4 );

	osg::Vec3	normal = orientation;
	normal[2] = 0.0;
	normal[1] = -orientation[0];
	normal[0] = orientation[1];
	normal.normalize();
	normal = normal * d_Size;

	(*vertices)[0] =	offset + normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[1] =	offset + orientation + normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[2] =	offset + orientation - normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[3] =	offset - normal + osg::Vec3( 0.0,0.0,-1.0 );

	p_DummyGeometry->setVertexArray( vertices );

	osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt( osg::PrimitiveSet::QUADS, 4 );
	(*primitives)[0] = 0;
	(*primitives)[1] = 1;
	(*primitives)[2] = 2;
	(*primitives)[3] = 3;

	p_DummyGeometry->addPrimitiveSet( primitives );
	p_DummyGeode->addDrawable( p_DummyGeometry.get() );

	osg::StateSet * dummy_set = new osg::StateSet();
	dummy_set->setMode( GL_BLEND, osg::StateAttribute::ON );
	dummy_set->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );	
	dummy_set->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

	osg::Depth* depth = new osg::Depth;
	depth->setWriteMask( false );
	dummy_set->setAttributeAndModes( depth, osg::StateAttribute::ON );
	dummy_set->setMode( GL_LIGHTING, osg::StateAttribute::OFF );			
	p_DummyGeometry->setStateSet( dummy_set );

	osg::Vec4Array	* color = new osg::Vec4Array( 1 );
	(*color)[0] = osg::Vec4( 1.0, 1.0, 1.0, 0.0 );
	
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	p_DummyGeometry->setColorArray( color, osg::Array::BIND_OVERALL );
#else
	p_DummyGeometry->setColorArray( color );
#endif
	p_DummyGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );

	this->addChild( p_DummyGeode.get() );
}

//====================================================================================================================
void	scene::COrientableLineGizmo::setSize( double t )
{
	d_Size = t;

	osg::Vec3Array * vertices = static_cast< osg::Vec3Array* >( p_DummyGeometry->getVertexArray() );
	osg::Vec3	normal = m_Orientation;
	normal[2] = 0.0;
	normal[1] = -m_Orientation[0];
	normal[0] = m_Orientation[1];
	normal.normalize();
	normal = normal * d_Size;
	(*vertices)[0] =	m_Offset + normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[1] =	m_Offset + m_Orientation + normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[2] =	m_Offset + m_Orientation - normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[3] =	m_Offset - normal + osg::Vec3( 0.0,0.0,-1.0 );
	p_DummyGeometry->dirtyBound();
	p_DummyGeometry->dirtyDisplayList();
}

//====================================================================================================================
void	scene::COrientableLineGizmo::setOffset( const osg::Vec3 & offset )
{
	m_Offset = offset;
	osg::Vec3Array * vertices = static_cast< osg::Vec3Array* >( p_Geometry->getVertexArray() );
	(*vertices)[ 0 ]		  = m_Offset;
	(*vertices)[ 1 ]		  = m_Offset + m_Orientation;
	p_Geometry->dirtyDisplayList();
	p_Geometry->dirtyBound();

	vertices = static_cast< osg::Vec3Array* >( p_DummyGeometry->getVertexArray() );
	osg::Vec3	normal = m_Orientation;
	normal[2] = 0.0;
	normal[1] = -m_Orientation[0];
	normal[0] = m_Orientation[1];
	normal.normalize();
	normal = normal * d_Size;
	(*vertices)[0] =	m_Offset + normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[1] =	m_Offset + m_Orientation + normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[2] =	m_Offset + m_Orientation - normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[3] =	m_Offset - normal + osg::Vec3( 0.0,0.0,-1.0 );
	p_DummyGeometry->dirtyBound();
	p_DummyGeometry->dirtyDisplayList();
}

//====================================================================================================================
void	scene::COrientableLineGizmo::setOrientation( const osg::Vec3 & orientation )
{
	m_Orientation = orientation;
	osg::Vec3Array * vertices = static_cast< osg::Vec3Array* >( p_Geometry->getVertexArray() );
	(*vertices)[ 0 ]		  = m_Offset;
	(*vertices)[ 1 ]		  = m_Offset + m_Orientation;
	p_Geometry->dirtyDisplayList();
	p_Geometry->dirtyBound();

	vertices = static_cast< osg::Vec3Array* >( p_DummyGeometry->getVertexArray() );
	osg::Vec3	normal = m_Orientation;
	normal[2] = 0.0;
	normal[1] = -m_Orientation[0];
	normal[0] = m_Orientation[1];
	normal.normalize();
	normal = normal * d_Size;
	(*vertices)[0] =	m_Offset + normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[1] =	m_Offset + m_Orientation + normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[2] =	m_Offset + m_Orientation - normal + osg::Vec3( 0.0,0.0,-1.0 );
	(*vertices)[3] =	m_Offset - normal + osg::Vec3( 0.0,0.0,-1.0 );
	p_DummyGeometry->dirtyBound();
	p_DummyGeometry->dirtyDisplayList();
}

