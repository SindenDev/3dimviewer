///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// This file comes from 3DimViewer software and was modified for 
// 
// BlueSkyPlan version 3.x
// Diagnostic and implant planning software for dentistry.
//
// The original 3DimViewer license can be found below.
//
// Changes are Copyright 2012 Blue Sky Bio, LLC
// All rights reserved 
//
// Changelog:
//    [2012/mm/dd] - ...
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
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


#include <osg/CActiveObjectBase.h>
#include <osg/dbout.h>


////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Constructor. 
//!
//!\param   name    The name of the object. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osg::CActiveObjectBase::CActiveObjectBase( const std::string & name /*= "ActiveObject" */, int priority /*= 0*/, EObjectClass oc /*= AO_DEFAULT*/ )
    : m_name( name )
    , m_priority( priority )
    , m_class( oc )
    , m_active(true)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Executes the mouse enter action. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CActiveObjectBase::onMouseEnter(const osgGA::GUIEventAdapter& ea, bool command_mode)
{
    // Just write info to the debug window
//    DBOUT( m_name.c_str() << ": onMouseEnter");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Executes the mouse exit action. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CActiveObjectBase::onMouseExit(const osgGA::GUIEventAdapter& ea, bool command_mode)
{
    // Just write info to the debug window
//    DBOUT( m_name.c_str() << ": onMouseExit");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Executes the mouse down action. 
//!
//!\param [in,out]  eh  If non-null, the event handler. 
//!\param   ea          The event adapter. 
//!\param [in,out]  aa  The action adapter. 
//!
//!\return  true if it succeeds, false if it fails.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool osg::CActiveObjectBase::onMouseDown(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    // Just write info to the debug window
//    DBOUT( m_name.c_str() << ": onMouseDown");

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Executes the mouse release action. 
//!
//!\param [in,out]  eh  If non-null, the event handler. 
//!\param   ea          The event adapter. 
//!\param [in,out]  aa  The action adapter. 
//!
//!\return  true if it succeeds, false if it fails.
////////////////////////////////////////////////////////////////////////////////////////////////////
bool osg::CActiveObjectBase::onMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    // Just write info to the debug window
//    DBOUT( m_name.c_str() << ": onMouseRelease");

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Executes the mouse scroll action. 
//!
//!\param [in,out]  eh  If non-null, the event handler. 
//!\param   ea          The event adapter. 
//!\param [in,out]  aa  The action adapter. 
//!
//!\return  true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool osg::CActiveObjectBase::onMouseScroll(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    // Just write info to the debug window
//    DBOUT( m_name.c_str() << ": onMouseScroll");

    return false;
}

/**
 * \fn  bool osg::CActiveObjectBase::onKeyDown( CActiveObjectEH * eh,
 *      const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
 *
 * \brief   Executes the key down action.
 *
 //!\param [in,out]  eh  If non-null, the event handler. 
 //!\param   ea          The event adapter. 
 //!\param [in,out]  aa  The action adapter.
 *
 * \return  true if it succeeds, false if it fails.
 */

bool osg::CActiveObjectBase::onKeyDown(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    // Just write info to the debug window
//    DBOUT( m_name.c_str() << ": onKeyDown" );

    return false;
}

/**
 * \fn  osg::CActiveObjectBase::~CActiveObjectBase()
 *
 * \brief   Destructor.
 */

osg::CActiveObjectBase::~CActiveObjectBase()
{
    m_destroySignal.invoke(this);
    m_destroySignal.disconnectAll();
}
