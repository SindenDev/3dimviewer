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

#ifndef CPolytopePlaneTester_H
#define CPolytopePlaneTester_H

#include <osgUtil/PolytopeIntersector>
//#include <osg/CIntersectionProspector>

//////////////////////////////////////////////////////////////////////////
//

class CPolytopeByPlane : public osg::Polytope
{
public:
	//! Initialize by plane and radius
	void init( const osg::Plane & plane, double radius );
};


//////////////////////////////////////////////////////////////////////////
//

class CPolytopeTester : osg::Referenced
{
public:
	//! Compute intersections with scene
	bool computeIntersections( osg::Node * scene );

	//! Does stored polytope intersect given geometry?
	template< class tpNodeType > bool intersects( tpNodeType * node);

	//! Get polytope intersector last used
	osgUtil::PolytopeIntersector * getIntersector()
	{
		return m_intersector.get();
	}

	//! Set used polytope
	void setPolytope( osg::Polytope * polytope )
	{
		m_polytope = polytope;
	}

	//! Get stored polytope
	osg::Polytope * getPolytope()
	{
		return m_polytope;//.get();
	}

protected:
	//! Polytope used for testing
	osg::Polytope * m_polytope;

	//! Polytope intersector used for last testing
	osg::ref_ptr<osgUtil::PolytopeIntersector> m_intersector;

	//! Intersection visitor
	osgUtil::IntersectionVisitor m_visitor;

}; // class CPolytopeTester


//////////////////////////////////////////////////////////////////////////
// Does stored polytope intersect given geometry?

template< class tpNodeType > bool CPolytopeTester::intersects(tpNodeType *node)
{
	if( node == NULL || m_intersector == NULL /*|| maxIntersection == 0 */ )
		return false;

	// are any intersections?
	if( !m_intersector->containsIntersections() )
		return false;

	osgUtil::PolytopeIntersector::Intersections intersections = m_intersector->getIntersections();
	
	osgUtil::PolytopeIntersector::Intersections::iterator i;

	// go through intersections
	//for(

}


#endif // CPolytopePlaneTester_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

