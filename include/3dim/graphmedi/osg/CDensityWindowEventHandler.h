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

#ifndef CDensityWindowEventHandler_H
#define CDensityWindowEventHandler_H

//#include <osg/OSGCanvas.h> // nothin' more needed
#include <osg/CAppMode.h>

#include <osgGA/GUIEventHandler>
#include <data/CDensityWindow.h>
//#include <VPL/module/Signal.h>


namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! Class handles mouse events in the OSG window and allowes to adjust
//! density window width and center.
//
class CDensityWindowEventHandler : public osgGA::GUIEventHandler
{	
public:
	//! Constructor, sort of doesn't do anything
	CDensityWindowEventHandler();

    //! Virtual destructor.
    virtual ~CDensityWindowEventHandler() {}

	//! How am i supposed to know what this does
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*);

    //! Set used density window id
    void setWindowId(int id) { m_id = id; }

    //! Get used density window id
    int getWindowId() const { return m_id; }

protected:
    //! Mouse position at the begining of the mouse dragging.
    float m_fPushX, m_fPushY;

    //! Density window at the begining of mouse dragging.
    data::SDensityWindow m_OldDW;

    //! Previously set density window.
    int m_OldCenter, m_OldWidth;

    //! Density window id
    int m_id;
};


} // namespace scene

#endif // CDensityWindowEventHandler_H
