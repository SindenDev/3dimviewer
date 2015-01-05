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

#include <base/Defs.h>
#include <osg/CDraggerEventHandler.h>
#include <iostream>
#include <osgViewer/Viewer>

// This header is here only for #ifdef qt_version line (determine if running on QT platform).
#include <osg/OSGCanvas.h>

// DELETETHIS:
#include <base/Macros.h>

scene::CDraggerEventHandler::CDraggerEventHandler(OSGCanvas * canvas) 
:	CEventHandlerBase( canvas )
,   p_Dragger( 0 )
,	p_EventAdapter( 0 )
,	p_ActionAdapter( 0 )
,	u_ActualId( 0 )
,   b_IsOverDraggable( false )
,	m_mask( -1 )
,   m_mode( MODE_NO_MASK )
,   m_maxDistance( 1.0 )
,   m_bHandle(true)
{
}


bool scene::CDraggerEventHandler::handle(const osgGA::GUIEventAdapter& ea,
                                         osgGA::GUIActionAdapter& aa, 
                                         osg::Object*, 
                                         osg::NodeVisitor*
                                         )
{
   p_EventAdapter = const_cast< osgGA::GUIEventAdapter *>( &ea );
   p_ActionAdapter = const_cast< osgGA::GUIActionAdapter *>( &aa );

   // get viewer pointer, if not possibel, chicken out
   osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View *>( p_ActionAdapter );
   if ( !view )  return false;

   //if( !APP_MODE.check(scene::CAppMode::MODE_SLICE_MOVE) )
   //{
   //    return false;
   //}

   // for highlight mode don't handle right mouse button the usual way
   const bool bRightButton = osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON==ea.getButton();
   if (osgGA::GUIEventAdapter::PUSH==p_EventAdapter->getEventType())
        m_bHandle=!(bRightButton && APP_MODE.getDraggerHitSignal().getNumOfConnections()>0 );

   // decide what to do according to how mouse behaves
   switch ( p_EventAdapter->getEventType() )
   {
		case osgGA::GUIEventAdapter::PUSH :

            if( !APP_MODE.check(scene::CAppMode::MODE_SLICE_MOVE) )  return false;
            _TRACEW( "->1<-" << std::endl );
            p_Dragger = this->getDraggerByMode();

        case osgGA::GUIEventAdapter::DRAG :
            if( !APP_MODE.check(scene::CAppMode::MODE_SLICE_MOVE) ) return false;

        case osgGA::GUIEventAdapter::RELEASE :

			// dragging with mouse pointer
            if( p_Dragger.get() )
            {
				m_Pointer._hitIter = m_Pointer._hitList.begin();
				m_Pointer.setCamera(view->getCamera());
#ifdef QT_VERSION
                m_Pointer.setMousePosition( p_EventAdapter->getX(), p_EventAdapter->getY() );
#else               
                m_Pointer.setMousePosition( p_EventAdapter->getX(), p_EventAdapter->getYmax() - p_EventAdapter->getY() + 1 );
#endif 
                if (m_bHandle)
                    p_Dragger->handle( m_Pointer, *p_EventAdapter, *p_ActionAdapter );

                // in highlight mode on right button up invoke signal
                if (osgGA::GUIEventAdapter::RELEASE==p_EventAdapter->getEventType() &&
                    osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON==ea.getButton())
                    APP_MODE.getDraggerHitSignal().invoke(p_Dragger, scene::CAppMode::RELEASE);

            }
            break;

		default : 
			//return true;
			
		break;
   }

   // check if mouse button released and release dragger
   if ( p_EventAdapter->getEventType() == osgGA::GUIEventAdapter::RELEASE )
   {
		p_Dragger = 0;
		m_Pointer.reset();
   }
   return false;
}

bool scene::CDraggerEventHandler::checkDummyHit()
{
	//assert( p_EventAdapter && p_ActionAdapter );

	//// prepare list of intersections
	//osgUtil::LineSegmentIntersector::Intersections intersections;
	//
	//m_Pointer.reset();
	//
	//osgViewer::View * view = dynamic_cast<osgViewer::View*>( p_ActionAdapter );

	//b_IsOverDraggable = false;

	//// compute intersections
	//if ( view->computeIntersections( p_EventAdapter->getX(), p_EventAdapter->getY(), intersections ) )
	//{
	//	m_Pointer.setCamera( view->getCamera() );
	//	//m_Pointer.setMousePosition( p_EventAdapter->getX(), p_EventAdapter->getY());
	//	m_Pointer.setMousePosition( p_EventAdapter->getX(), p_EventAdapter->getYmax() - p_EventAdapter->getY() + 1 );

	//	// find intersections and mark intersected object nodes
	//	osgUtil::LineSegmentIntersector::Intersections::iterator	 hit;		

	//	for ( hit = intersections.begin(); hit != intersections.end(); ++hit )
	//	{
	//		m_Pointer.addIntersection( hit->nodePath, hit->getLocalIntersectPoint() );			
	//		osg::NodePath::iterator		node;
	//		for ( node = hit->nodePath.begin(); node != hit->nodePath.end(); ++node )
	//		{
	//			scene::CDummyDraggableGeode * geometry = dynamic_cast< scene::CDummyDraggableGeode* > ( *node );
	//			if ( geometry ) 
	//			{
	//				b_IsOverDraggable = true;
	//				u_ActualId		  = geometry->getID();
	//				return true;
	//			}			
	//		}
	//	}
	//}
	return false;
}

osgManipulator::Dragger * scene::CDraggerEventHandler::getDragger( osg::Node::NodeMask _mask )
{
	assert( p_EventAdapter && p_ActionAdapter );

	// prepare list of intersections
	osgUtil::LineSegmentIntersector::Intersections intersections;
	
	m_Pointer.reset();
	
    osg::ref_ptr<osgViewer::View> view = dynamic_cast<osgViewer::View *>( p_ActionAdapter );

	b_IsOverDraggable = false;

	// compute intersections
	if ( view->computeIntersections( p_EventAdapter->getX(), p_EventAdapter->getY(), intersections, _mask ) )
	{
		m_Pointer.setCamera( view->getCamera() );
		
		//m_Pointer.setMousePosition( p_EventAdapter->getX(), p_EventAdapter->getY());
		m_Pointer.setMousePosition( p_EventAdapter->getX(), p_EventAdapter->getYmax() - p_EventAdapter->getY() + 1 );

		// find intersections and mark intersected object nodes
		osgUtil::LineSegmentIntersector::Intersections::iterator	 hit;		

        osg::Vec3d v, firstIntersection( intersections.begin()->getWorldIntersectPoint() );

//		bool	geometry_hit = false;
		for ( hit = intersections.begin(); hit != intersections.end(); ++hit )
		{
            v = hit->getWorldIntersectPoint() - firstIntersection;

     //       if( v.length() > m_maxDistance )
     //           continue;

		    // try to downcast node to dragger. if possible, dragger found
          osg::NodePath::const_iterator node;
		    osg::NodePath::const_iterator first_node = hit->nodePath.begin(); // m_Pointer._hitList.front().first.begin();
		    osg::NodePath::const_iterator last_node  = hit->nodePath.end(); //m_Pointer._hitList.front().first.end();	
		    //intersection list = nodepath, intersection list


		    for ( node = first_node; node != last_node; ++node )
		    {	
                // Is it dragger?
			    osgManipulator::Dragger * dragger = dynamic_cast< osgManipulator::Dragger* >( *node );
			    if (dragger)
			    {
           		    m_Pointer.addIntersection( hit->nodePath, hit->getLocalIntersectPoint() );			

				    // dragger selected by mouse
                    if (m_bHandle)
                        dragger->handle( m_Pointer, *p_EventAdapter, *p_ActionAdapter );
				    return dragger;
			    }
		    }
        }
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Search the scene for intersection with draggable object and return its dragger
//!         pointer. 
//!
//!\return  null if it fails, else the dragger by mode. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osgManipulator::Dragger * scene::CDraggerEventHandler::getDraggerByMode(void)
{
    osgManipulator::Dragger * dragger;
    osg::Node::NodeMask mask( m_mask );
    osg::Node::NodeMask invertedMask( ~m_mask );

    switch( m_mode )
    {
    case MODE_NO_MASK:           // Do not use masking
        return getDragger( -1 );

    case MODE_MASKED_FIRST:      // Masked draggers are used first ( if found, else use any dragger )
        dragger = getDragger( m_mask );

        if( !dragger )
            dragger = getDragger( -1 );

        return dragger;

    case MODE_MASKED_LAST:       // Masked draggers are used last ( if no other dragger is found )
        dragger = getDragger( invertedMask );

        if( ! dragger )
            dragger = getDragger( mask );

        return dragger;

    case MODE_MASKED_ONLY:       // Use only masked draggers
        return getDragger( mask );

    case MODE_MASKED_EXCLUDED:   // Do not use masked draggers

        return getDragger( invertedMask );

    default:
        break;
    }
	
	return(NULL);
}

