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

#include <osg/MouseDraw.h>
#include <osgViewer/View>
#include <app/Signals.h>
#include <osg/Version>

using namespace osg;

/******************************************************************************
	CLASS CLineGeode
******************************************************************************/

//=============================================================================
// Constructor
CLineGeode::CLineGeode( int flags ) :
	m_color( 0.8, 0.0, 0.0, 1.0 )
{
	// Create geometry
	m_geometry = new Geometry;

	// Vertex array
	m_vertices = new Vec3Array;
	m_geometry->setVertexArray( m_vertices.get() );

	// Color array
	m_colors = new Vec4Array;
	m_colors->push_back( m_color );
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	m_geometry->setColorArray(m_colors.get(), osg::Array::BIND_OVERALL);
#else
	m_geometry->setColorArray(m_colors.get());
#endif
	m_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	// set the normal in the same way color.
	Vec3Array* normals = new Vec3Array;
	normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	m_geometry->setNormalArray(normals, osg::Array::BIND_OVERALL);
#else
	m_geometry->setNormalArray(normals);
#endif
	m_geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	// Do not use display lists. Its slow for realtime graphics...
	m_geometry->setUseDisplayList( false );

	// Create draw arrays 
	m_drawArrays = new DrawArrays( osg::PrimitiveSet::LINE_STRIP, m_geometry->getVertexArray()->getNumElements(), 0 );
	m_geometry->addPrimitiveSet( m_drawArrays.get() );

	// Add geometry to the geode
	addDrawable( m_geometry.get() );

	// State set
	m_stateSet = new StateSet;
	setStateSet( m_stateSet.get() );

	// Line width
	m_lineWidth = new LineWidth;
	m_lineWidth->setWidth( 1.0f );

	// Add line width to the state set
	m_stateSet->setAttributeAndModes( m_lineWidth.get(), osg::StateAttribute::ON );

	// Point size
	m_pointSize = new Point;
	m_pointSize->setSize( 1.0f );

	// Add point size to the state set
	m_stateSet->setAttributeAndModes( m_pointSize.get(), osg::StateAttribute::ON );

    // Disable depth test if it is necessary to draw everything all the time
    if( flags & DISABLE_DEPTH_TEST )
	{
		m_stateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
		m_stateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
	}

	// Need to make sure this geometry is drawn last. RenderBins are handled
	// in numerical order so we set bin number to 111
	if( flags & USE_RENDER_BIN )
	{
		m_stateSet->setRenderBinMode(osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
		m_stateSet->setRenderBinDetails( 111, "RenderBin");
	}

	// Disable culling
	setCullingActive(false);
}

//=============================================================================
// Add line point
void CLineGeode::AddPoint(const osg::Vec3 &point)
{
// Add point
	osg::Vec3Array * arr = dynamic_cast<osg::Vec3Array *>( m_geometry->getVertexArray() );
	arr->push_back(point);

	// Set new count of vertices to draw array
	m_drawArrays->setCount( m_drawArrays->getCount() + 1 );
  
	// Set draw arrays dirty
	m_drawArrays->dirty();
	// Dirty display list
	m_geometry->dirtyDisplayList();
}

/******************************************************************************
	CLASS CDrawingsGroup - group used to store drawings.
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Constructor
CDrawingsGroup::CDrawingsGroup()
: m_clearingEnabled( true )
{
	// connect to the clearing signal
	m_conClear = VPL_SIGNAL( SigClearAllGizmos ).connect( this, &CDrawingsGroup::clearDrawings );
}

///////////////////////////////////////////////////////////////////////////////
// Signal response - clear drawings
void CDrawingsGroup::clearDrawings()
{
	if( m_clearingEnabled && ( getNumChildren() > 0 ) )
		removeChildren( 0, getNumChildren() );
}

//=============================================================================
//=============================================================================
using namespace osgGA;

/******************************************************************************
	CLASS CMouseDrawHandler - base class for drawing
******************************************************************************/

//=============================================================================
// Constructor
CMouseDrawHandler::CMouseDrawHandler(OSGCanvas * canvas ) 
    : CEventHandlerBase( canvas ),
	m_drawLine( true ),
	m_linesNode( NULL ),
    bDrawing(false),
	m_lineFlags( CLineGeode::DISABLE_DEPTH_TEST | CLineGeode::USE_RENDER_BIN )
{
	
}


//=============================================================================
// Handle events
bool CMouseDrawHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Object *, osg::NodeVisitor *)
{
	// Test application mode
	if( UseHandler() )
	{
		CMousePoint point;
		point.m_modKeyMask = ea.getModKeyMask();
		point.m_buttonMask = ea.getButtonMask();
		point.m_buttonEvent = ea.getButton();
	
		switch ( ea.getEventType() )
		{
		case osgGA::GUIEventAdapter::PUSH :
			// Try to get intersection
			if( ! GetIntersection( point, ea, aa ) )
				return false;

			initDraw(point);

			// Call callback
			OnMousePush( point );

			return true;

		case osgGA::GUIEventAdapter::DRAG :
			// Try to get intersection
			if( ! GetIntersection( point, ea, aa ) )
				return false;

			if (!bDrawing)
            {
                initDraw(point);
            }
            
            m_line->AddPoint( point.m_point );

			// Call callback
			OnMouseDrag( point );

			return true;

		case osgGA::GUIEventAdapter::RELEASE :
			// Try to get intersection
			if( ! GetIntersection( point, ea, aa ) )
				return false;

            if (!bDrawing)
            {
                initDraw(point);
            }
            
            m_line->AddPoint( point.m_point );

			// Call callback
			OnMouseRelease( point, true );

			return true;

		} // switch

		return true;
	} // if mode ==

	return false;
}

//=============================================================================
//  Starts drawing
void CMouseDrawHandler::initDraw(const CMousePoint &point)
{
    bDrawing = true;

    // Create new line
	m_line = new osg::CLineGeode( m_lineFlags );
	m_line->AddPoint( point.m_point );

    // If should be drawn, draw it...
	if( m_drawLine && ( m_linesNode.get() != NULL ) )
		m_linesNode->addChild( m_line.get() );

	// Store line
	m_linesVector.push_back( m_line.get() );	
}

//=============================================================================
//  Stops drawing
void CMouseDrawHandler::stopDraw(const CMousePoint &point)
{
	bDrawing = false;
}

//=============================================================================
//  Clear drawn lines from node
void CMouseDrawHandler::ClearLines()
{
	if( m_linesVector.size() > 0 && m_linesNode != NULL )
	{
		std::vector< osg::ref_ptr< osg::CLineGeode > >::iterator iLine;

		for( iLine = m_linesVector.begin(); iLine != m_linesVector.end(); ++iLine )
		{
			m_linesNode->removeChild( iLine->get() );
		}

		m_linesVector.clear();

	}
}


/******************************************************************************
	CLASS CScreenDrawHandler - On screen draw handler
******************************************************************************/

//=============================================================================
// Constructor
CScreenDrawHandler::CScreenDrawHandler( OSGCanvas * canvas ) :
	CMouseDrawHandler( canvas ),
	m_z( 0.5 )
{

}


//=============================================================================
// 
bool CScreenDrawHandler::GetIntersection( CMousePoint &intersection, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
	// get viewer pointer, if not possible, chicken out
	osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View *>( &aa );
	if ( !view )  return false;

	// get camera
	osg::Viewport* viewport = view->getCamera()->getViewport();
		
	// Recompute coordinates
	float mx = viewport->x() + (int)((float)viewport->width()*(ea.getXnormalized()*0.5f+0.5f));
	float my = viewport->y() + (int)((float)viewport->height()*(ea.getYnormalized()*0.5f+0.5f));

	osg::Matrixd ipm, pm( view->getCamera()->getProjectionMatrix() );
	ipm.invert(pm);

	osg::Matrixd iwm, wm( viewport->computeWindowMatrix() );
	iwm.invert(wm);

	osg::Matrixd ivm, vm( view->getCamera()->getViewMatrix() );
	ivm.invert(vm);

	osg::Matrixd matrix = iwm*ipm*ivm;
	intersection.m_point = osg::Vec3(mx, my, m_z)*matrix;

    intersection.m_pointWindow = osg::Vec2(ea.getX(), ea.getY());

	return true;
}

/******************************************************************************
		CLASS CGeometryDrawHandler
******************************************************************************/

//=============================================================================
// Constructor
CGeometryDrawHandler::CGeometryDrawHandler(OSGCanvas * canvas) :
	CMouseDrawHandler(canvas)
{
	m_drawableGeometry = new tDrawableGeometry;
	m_prospector.addDesiredRule( m_drawableGeometry );
	m_prospector.useDesired( true );
}

//=============================================================================
// Compute intersection
bool CGeometryDrawHandler::GetIntersection( CMousePoint & intersection, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa )
{

	tIntersections allIntersections, usableIntersections;
	tIntersection intersect;
//*
	osgViewer::View * view = dynamic_cast<osgViewer::View*>( &aa );
	
	if( view == NULL )
		return false;

	view->computeIntersections(ea.getX(), ea.getY(), allIntersections);

	// sort intersections
//	std::stable_sort(allIntersections.begin(), allIntersections.end());
//*
	
	if(m_prospector.prospect(allIntersections, usableIntersections) )
	{
		// intersection found
		intersect = *usableIntersections.begin();
		intersection.m_point = intersect.getWorldIntersectPoint();
		intersection.m_pointLocal = intersect.getLocalIntersectPoint();
		intersection.m_normal = intersect.getWorldIntersectNormal();
		intersection.m_normalLocal = intersect.getLocalIntersectNormal();
        intersection.m_pointWindow = osg::Vec2(ea.getX(), ea.getY());
		return true;
	}
//*/
	return false;
}




