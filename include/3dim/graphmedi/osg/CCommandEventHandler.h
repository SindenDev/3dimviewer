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

#ifndef CCommandEventHandler_H
#define CCommandEventHandler_H

#include <osg/OSGCanvas.h> // nothin' more needed
#include <osg/CAppMode.h>

#include <osgGA/GUIEventHandler>


namespace scene
{

////////////////////////////////////////////////////////////
//! Class handles mouse commands in the OSG scene.
//
class CCommandEventHandler : public osgGA::GUIEventHandler
{
public:
	//! Constructor, sort of doesn't do anything
	CCommandEventHandler();

    //! Virtual destructor.
    virtual ~CCommandEventHandler() {}

	//! How am i supposed to know what this does
	bool handle(const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& aa,
                osg::Object *o,
                osg::NodeVisitor *nv
                );
};


} // namespace scene

#endif // CCommandEventHandler_H
