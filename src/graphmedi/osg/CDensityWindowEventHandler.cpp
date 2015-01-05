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
#include <osg/CDensityWindowEventHandler.h>
#include <app/Signals.h>

#include <osgViewer/Viewer>


scene::CDensityWindowEventHandler::CDensityWindowEventHandler()
    : m_fPushX(0.0f)
    , m_fPushY(0.0f)
    , m_OldDW(data::DEFAULT_DENSITY_WINDOW)
    , m_OldCenter(-10000)
    , m_OldWidth(-1)
{
}


bool scene::CDensityWindowEventHandler::handle(const osgGA::GUIEventAdapter& ea,
										 osgGA::GUIActionAdapter& aa, 
										 osg::Object*, 
										 osg::NodeVisitor*
                                         )
{
    osgGA::GUIEventAdapter *p_EventAdapter = const_cast< osgGA::GUIEventAdapter *>(&ea);
    osgGA::GUIActionAdapter *p_ActionAdapter = const_cast< osgGA::GUIActionAdapter*>(&aa);

    // get viewer pointer, if not possible, chicken out
    osgViewer::View * view = dynamic_cast<osgViewer::View*>(p_ActionAdapter);
    if( !view )
    {
        return false;
    }

    if( !APP_MODE.check(scene::CAppMode::MODE_DENSITY_WINDOW) )
    {
        return false;
    }

    // decide what to do according to how mouse behaves
    switch( p_EventAdapter->getEventType() )
    {
	    case osgGA::GUIEventAdapter::PUSH:
            // Remember the mouse position
            m_fPushX = p_EventAdapter->getX();
            m_fPushY = p_EventAdapter->getYmax() - p_EventAdapter->getY() + 1;

            // Remember the density window
            m_OldDW = VPL_SIGNAL(SigGetDensityWindow).invoke2();
            m_OldCenter = m_OldDW.m_Center;
            m_OldWidth = m_OldDW.m_Width;
            break;

        case osgGA::GUIEventAdapter::DRAG:
        {
            // Calculate difference of coordinates
            float dx = m_fPushX - p_EventAdapter->getX();
            float dy = m_fPushY - p_EventAdapter->getYmax() + p_EventAdapter->getY() - 1;

            // Calculate new density window
            int NewCenter = m_OldDW.m_Center + int(dx);
            int NewWidth = m_OldDW.m_Width + int(2 * dy);
            if( NewCenter == m_OldCenter && NewWidth == m_OldWidth )
            {
                return true;
            }

            // Modify the density window
            VPL_SIGNAL(SigSetDensityWindow).invoke(NewCenter, NewWidth);
            m_OldCenter = NewCenter;
            m_OldWidth = NewWidth;
            break;
        }

        case osgGA::GUIEventAdapter::RELEASE:
            break;

	    default: 
		    break;
    }

    // O.K.
    return true;
}

