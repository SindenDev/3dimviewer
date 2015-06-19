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

#ifndef COrthoSceneOSG_H
#define COrthoSceneOSG_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osg/Group>
// CP macro redefinition in the osgDB/Serializer problem
//#undef CP 
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osg/CoordinateSystemNode>
#include <osgText/Text>
#include <osg/OSGOrtho2DCanvas.h>

// Dragger
#include <osgManipulator/Translate1DDragger>
#include <osgManipulator/CommandManager>
#include <osg/Constraints.h>

// Line
#include <osg/Geode>
#include <osg/Vec3>
#include <osg/Geometry>
#include <osg/LineWidth>

#include "osg/CPlaneConstraint.h"
#include "osg/CPlaneUpdateSelection.h"
#include "osg/CSignalEventHandler.h"
#include "osg/CTranslateOtherLineDragger.h"

#include "osg/CObjectObserverOSG.h"
#include "osg/CForceCullCallback.h"
#include <data/COrthoSlice.h>
#include <graph/osg/CForceCullCallback.h>


///////////////////////////////////////////////////////////////////////////////
// Constants

#define SELECTOR_MULTIPLIER 4

namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! class description
//
class COrthoSceneOSG : public osg::Group, public CObjectObserverOSG<data::COrthoSliceXY, OSGOrtho2DCanvas>, public osg::CInvisibleObjectInterface
{
protected:
	//--------------------
	// Main parts

	//! Scene events handler
	osg::ref_ptr<CSignalEventHandler> signalEventHandler;

	//! Ortho manipulator - view manipulation
	osg::ref_ptr<osgGA::OrthoManipulator> manipulator;

	//! Pointer info is structure used to provide informations to draggers in scene. 
	osgManipulator::PointerInfo m_pointerInfo;

	//! Currently used dragger
	osg::ref_ptr<osgManipulator::Dragger> m_activeDragger;

	//--------------------
	// Scene parts

	//! Geometry.
	osg::ref_ptr<osg::Geometry> geom;

	//! Line geode
	osg::ref_ptr<osg::Geode> lineGeode;

    //! Line geometry
	osg::ref_ptr<osg::Geometry> lineGeometry;

    //! Width
	osg::ref_ptr<osg::LineWidth> linewidth;

    //! Invisible cylinder
	osg::ref_ptr<osg::Cylinder> cylinder;

	//! Line dragger
	osg::ref_ptr<osgManipulator::Translate1DDragger> lineDragger;

	//! Plane dragger
	osg::ref_ptr<osgManipulator::CTranslateOtherLineDragger> planeDragger;

	//! Line constraint
	osg::ref_ptr<osgManipulator::CLineConstraint> lineConstraint;

	//! Plane constraint
	osg::ref_ptr<osgManipulator::CLineConstraint> planeConstraint;

public:
	//! Constructor
	COrthoSceneOSG(OSGOrtho2DCanvas * pCanvas);

	//! Create scene geometry.
	void createGeometry();

protected:
	//! Handle event - try to find and call all draggers.
	virtual bool handleDraggersPress(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	//! Handle event - use active dragger.
	virtual bool handleDraggersContinue(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	//! Handle event - window resize
	virtual bool handleWindowResize(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	//! Handle event - plane drag
	virtual bool handlePlaneMove(const osgManipulator::TranslateInLineCommand & command);

    //! Method called on OSG update callback.
    virtual void updateFromStorage();
};


} // namespace scene

#endif // COrthoSceneOSG_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
