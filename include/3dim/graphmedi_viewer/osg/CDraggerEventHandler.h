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

#ifndef CDraggerEventHandler_H
#define CDraggerEventHandler_H

#include <osg/CDummyOSG.h>
#include <osg/CAppMode.h>

#include <osg/CEventHandlerBase.h>


namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! Class handles mouse events in the OSG window. It allows you to move any
//! draggable slice in the scene.
//
class CDraggerEventHandler : public CEventHandlerBase
{
public:
    //! Operational mode
    enum EMode
    {
        MODE_NO_MASK,           // Do not use masking
        MODE_MASKED_FIRST,      // Masked draggers are used first ( if found, else use any dragger )
        MODE_MASKED_LAST,       // Masked draggers are used last ( if no other dragger is found )
        MODE_MASKED_ONLY,       // Use only masked draggers
        MODE_MASKED_EXCLUDED    // Do not use masked draggers
    };

protected:
    //! Activated dragger
    osg::ref_ptr<osgManipulator::Dragger> p_Dragger;

    //! Mouse pointer for computing intersections
    osgManipulator::PointerInfo m_Pointer;

    //! Event adapter pointer
    osgGA::GUIEventAdapter * p_EventAdapter;

    //! Action adapter pointer
    osgGA::GUIActionAdapter * p_ActionAdapter;

    //! Actual id of draggable slice
    unsigned u_ActualId;

    //! True if mouse pointer is over draggable slice
    bool b_IsOverDraggable;

	//! Intersections mask currently used
	osg::Node::NodeMask m_mask;

    //! Use node mask?
    EMode m_mode;

    //! distance limit - what is the maximal distance between first and used intersection (when masking is used).
    double m_maxDistance;

    bool   m_bHandle;
    
public:
    //! Constructor, sort of doesn't do anything
    CDraggerEventHandler(OSGCanvas * canvas);

    //! Search the scene for intersection with draggable object and return its dragger pointer
    osgManipulator::Dragger * getDragger(osg::Node::NodeMask _mask);

    //! How am i supposed to know what this does
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* );

    //! Returns true if mouse pointer is over draggable plane
    bool isOverDraggable() { return b_IsOverDraggable; };

    //! Returns index of draggable plane
    unsigned getDraggableId() { return u_ActualId; }

    //! Returns true if dragger is currently activated
    bool activeDragger() { return ( p_Dragger != 0 ); }

    //! Check if dummy geometry was hit
    bool checkDummyHit();

	//! Get currently used node mask.
	osg::Node::NodeMask getVisitorMask() { return m_mask; }

	//! Set intersection visitor node mask
	void setVisitorMask( osg::Node::NodeMask mask ) { m_mask = mask; }

    //! Set masking mode
    void setMaskingMode( EMode mode ) { m_mode = mode; }

    // Access the MaxDistance
    double getMaxDistance(void) const		{ return(m_maxDistance);		};
    void setMaxDistance(double MaxDistance)	{ m_maxDistance = MaxDistance;	};

protected:
    //! Search the scene for intersection with draggable object and return its dragger pointer
    osgManipulator::Dragger * getDraggerByMode();
};


} // namespace scene

#endif // CDraggerEventHandler_H
