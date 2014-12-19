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

///////////////////////////////////////////////////////////////////////////////
// include files
#include <osg/CPolytopePlaneTester.h>

/************************************************************************/
/* CLASS CPolytopeByPlane                                               */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
// Initialize by plane and radius
void CPolytopeByPlane::init(const osg::Plane &plane, double radius)
{
	// create two bounding planes parallel to the clipping plane. 
	// Distance from the clipping plane is set by radius parammeter.

	osg::Plane p1( plane ), p2( plane );

	// Compute plane normal
	osg::Vec3 normal( plane.getNormal() );
	normal.normalize();

	osg::Matrix MTranslation;

	// First plane is shifted in normal direction by radius/2
	MTranslation.setTrans( normal * radius / 2.0 );
	p1.transform( MTranslation );

	// First plane is shifted in -normal direction by radius/2
	MTranslation.setTrans( normal * ( -radius / 2.0 ) );
	p2.transform( MTranslation );

	// Clear current planes
	clear();

	// add planes
	add( p1 );
	add( p2 );
}

/************************************************************************/
/* CLASS CPolytopePlaneTester                                           */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
// Does stored polytope intersect any geometry in the scene?
bool CPolytopeTester::computeIntersections( osg::Node * scene )
{

	if( m_polytope == NULL || scene == NULL )
		return false;

	// create intersector
	m_intersector = new osgUtil::PolytopeIntersector( *m_polytope );

	// Send it to the scene...
	m_visitor.reset();
	m_visitor.setIntersector( m_intersector.get() );
	scene->accept( m_visitor );

	return true;
}

 
//////////////////////////////////////////////////////////////////////////
// 