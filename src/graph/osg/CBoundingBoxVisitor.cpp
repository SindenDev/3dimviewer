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

#include <osg/CBoundingBoxVisitor.h>
#include <osg/osgcompat.h>

using namespace osg; 


CBoundingBoxVisitor::CBoundingBoxVisitor() 
	: NodeVisitor( NodeVisitor::TRAVERSE_ALL_CHILDREN ) 
{
	m_transformMatrix.makeIdentity();
}

            
CBoundingBoxVisitor::~CBoundingBoxVisitor() 
{
}


void CBoundingBoxVisitor::apply( osg::Geode &geode )
{
    osg::BoundingBox drawableBBox, bbox;
//	osg::Drawable *drawable;

    // update bounding box for each drawable
	for(  unsigned int i = 0; i < geode.getNumDrawables(); ++i )
	{
//		drawable = geode.getDrawable( i );

		// expand the overall bounding box
        drawableBBox = OSGGETBOUND(geode.getDrawable( i ));
        if (drawableBBox.valid())
        {
		    bbox.expandBy(drawableBBox);
        }
    }

	// transform corners by current matrix
	
    if (bbox.valid())
    {
	    osg::BoundingBox bboxTrans;

	    for( unsigned int i = 0; i < 8; ++i ) 
	    {
		    osg::Vec3 xvec = bbox.corner( i ) * m_transformMatrix;
		    bboxTrans.expandBy( xvec );
	    }

	    // update the overall bounding box size
	    m_boundingBox.expandBy( bboxTrans );
    }

    // continue traversing through the graph
	traverse( geode );

} // ::apply(osg::Geode &geode)

        
void CBoundingBoxVisitor::apply( osg::MatrixTransform &node )
{
    osg::Matrix tmpMatrix = m_transformMatrix;
	m_transformMatrix = node.getMatrix() * m_transformMatrix;
	
	// continue traversing through the graph
	traverse( node );

    // restore matrix
    m_transformMatrix = tmpMatrix;
} // ::apply(osg::MatrixTransform &node)


void CBoundingBoxVisitor::apply( osg::Billboard &node ){
	
	// important to handle billboard so that its size will
	// not affect the geode size continue traversing the graph
	
	traverse( node );
} // ::apply(osg::MatrixTransform &node)

   
// Get computed bounding box
osg::BoundingBox &CBoundingBoxVisitor::getBoundBox() { return m_boundingBox; }  

// reset visitor
void CBoundingBoxVisitor::reset()
{
	NodeVisitor::reset();
	m_boundingBox.init();
	m_transformMatrix.makeIdentity();
}

