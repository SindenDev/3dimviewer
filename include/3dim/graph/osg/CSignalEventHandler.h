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

#ifndef CSIGNALEVENTHANDLER_H
#define CSIGNALEVENTHANDLER_H

#include <osg/CEventHandlerBase.h>
#include <VPL/Module/Signal.h>
#include <map>

namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//

class CSignalEventHandler : public CEventHandlerBase
{
public:
    //! Event signal
    typedef vpl::mod::CSignal<bool, const osgGA::GUIEventAdapter& , osgGA::GUIActionAdapter& > tSigHandle;

    //! Map of signals. Event type is used as the key.
    typedef std::map<osgGA::GUIEventAdapter::EventType, tSigHandle *> tEventSignalsMap;

    //! Signals map iterator
    typedef tEventSignalsMap::iterator tIterEventSignalsMap;

    //! Constructor
    CSignalEventHandler(OSGCanvas * canvas);

    //! Destructor
    ~CSignalEventHandler();

    //! Handle event
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

    //! Add event to handle
    tSigHandle & addEvent(const osgGA::GUIEventAdapter::EventType & eventType);

    //! Get event signal reference. If not included, create it.
    tSigHandle & getSignal(const osgGA::GUIEventAdapter::EventType & eventType);

protected:
    //! Type of pair in map
    std::pair<osgGA::GUIEventAdapter::EventType, tSigHandle *> tEventTypeSignalPair;

    //! Map of signals
    tEventSignalsMap signalsMap;
};


} // namespace osgGA

#endif // CSIGNALEVENTHANDLER_H
