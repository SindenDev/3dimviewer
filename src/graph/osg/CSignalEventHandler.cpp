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

#include <osg/CSignalEventHandler.h>
#include <algorithm>

using namespace scene;

// Constructor
CSignalEventHandler::CSignalEventHandler(OSGCanvas * canvas)
    : CEventHandlerBase( canvas )
{
}

// Destructor
CSignalEventHandler::~CSignalEventHandler()
{
    // delete signals in map
    for(tIterEventSignalsMap i = signalsMap.begin(); i != signalsMap.end(); i++)
        if(i->second != NULL)
            delete i->second;

    // clear map
    signalsMap.clear();
}

// Add event to handle
CSignalEventHandler::tSigHandle & CSignalEventHandler::addEvent(const osgGA::GUIEventAdapter::EventType & eventType)
{
    
    // Try to find event type. If found, exit, else add it.
    tIterEventSignalsMap i = signalsMap.find(eventType);

    if(i == signalsMap.end())
        signalsMap.insert(std::make_pair(eventType, new tSigHandle));

    return getSignal(eventType);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Gets a signal. 
//!
//!\param   eventType   Type of the event. 
//!
//!\return  The signal handle. 
////////////////////////////////////////////////////////////////////////////////////////////////////
CSignalEventHandler::tSigHandle & CSignalEventHandler::getSignal(const osgGA::GUIEventAdapter::EventType & eventType)
{
    tIterEventSignalsMap i = signalsMap.find(eventType);

    if(i == signalsMap.end())
    {
        return addEvent(eventType);
    }

    return *(i->second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Handles. 
//!
//!\param   ea          The event adapter. 
//!\param [in,out]  aa  The action adapter. 
//!
//!\return  true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSignalEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{

    tIterEventSignalsMap i = signalsMap.find(ea.getEventType());

    // Try to find apropriate signal and invoke it.
    if(i != signalsMap.end())
        return i->second->invoke2(ea, aa);

    return false;
}

