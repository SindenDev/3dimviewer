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

///////////////////////////////////////////////////////////////////////////////
// include files

#include <base/Defs.h>
#include <base/Macros.h>

#include <osg/COrthoSceneOSG.h>

#include <osg/ShapeDrawable>
#include <widgets/Widgets.h>
#include <osg/Version>
#include <app/Signals.h>

namespace scene
{

COrthoSceneOSG::COrthoSceneOSG(OSGOrtho2DCanvas *pCanvas)
{
    // turn off the lights
    this->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    setCanvas(pCanvas);

	// Initialize and set manipulator
    
	// create new manipulator
	manipulator = new osgGA::OrthoManipulator;

	// this should be rewritten in some way (it depends on model position and size).
#define ZOOM 1
	m_pCanvas->getView()->getCamera()->setProjectionMatrixAsOrtho2D(-ZOOM, ZOOM,-ZOOM, ZOOM);

	// set manipulator to camera
	pCanvas->setManipulator(manipulator.get());

	// compute home position
	manipulator->computeHomePosition();

	// Create and attach event handler
	signalEventHandler = new CSignalEventHandler( pCanvas );

	pCanvas->addEventHandler(signalEventHandler.get());

	// Add signals for push drag and release - invoke handlDraggers method
	CSignalEventHandler::tSigHandle &handlePush = signalEventHandler->addEvent(osgGA::GUIEventAdapter::PUSH);
	handlePush.connect(this, &COrthoSceneOSG::handleDraggersPress);

	CSignalEventHandler::tSigHandle &handleRelease = signalEventHandler->addEvent(osgGA::GUIEventAdapter::RELEASE);
	handleRelease.connect(this, &COrthoSceneOSG::handleDraggersContinue);
	handleRelease.connect(this, &COrthoSceneOSG::handleWindowResize);

	CSignalEventHandler::tSigHandle & handleDrag = signalEventHandler->addEvent(osgGA::GUIEventAdapter::DRAG);
	handleDrag.connect(this, &COrthoSceneOSG::handleDraggersContinue);

	CSignalEventHandler::tSigHandle & handleScroll = signalEventHandler->addEvent(osgGA::GUIEventAdapter::SCROLL);
	handleScroll.connect(this, &COrthoSceneOSG::handleWindowResize);

	CSignalEventHandler::tSigHandle & handleResize = signalEventHandler->addEvent(osgGA::GUIEventAdapter::RESIZE);
	handleResize.connect(this, &COrthoSceneOSG::handleWindowResize);

    // Set the update callback
    CGeneralObjectObserver<COrthoSceneOSG>::connect(APP_STORAGE.getEntry(data::Storage::SliceXY::Id).get());
    this->setupObserver(this);
}

//////////////////////////////////////////////////////////////////////////
//
bool COrthoSceneOSG::handleDraggersPress(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	bool bDraggerFound(false);

	if(ea.getButtonMask() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
		return false;

	osgViewer::Viewer * m_view = m_pCanvas->getView();

	assert(m_view != NULL);

	// compute intersections
	osgUtil::LineSegmentIntersector::Intersections intersections;

	if(m_view->computeIntersections(ea.getX(), ea.getY(), intersections))
	{
		// initialize pointer info
		m_pointerInfo.reset();
		m_pointerInfo.setCamera(m_view->getCamera());
		m_pointerInfo.setMousePosition(ea.getX(), ea.getYmax() - ea.getY() + 1);

		for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
			hitr != intersections.end();
			++hitr)
		{
			m_pointerInfo.addIntersection(hitr->nodePath, hitr->getLocalIntersectPoint());
		}

		// Transform Y coordinate
		//		osgGA::GUIEventAdapter localEA = ea;
		//		ea.setY(ea.getYmax() - ea.getY() + 1);

		// try to call handle of all intersected draggers 
		for (osg::NodePath::iterator itr = m_pointerInfo._hitList.front().first.begin();
			itr != m_pointerInfo._hitList.front().first.end();
			++itr)
		{
			osgManipulator::Dragger* dragger = dynamic_cast<osgManipulator::Dragger*>(*itr);
			if (dragger)
			{
				dragger->handle(m_pointerInfo, ea, aa);
				m_activeDragger = dragger;
				bDraggerFound = true;
			}                   
		}
		if(!bDraggerFound)
			m_activeDragger = NULL;

		return true;
	}

	m_activeDragger = NULL;
	return false;

} // handle

//////////////////////////////////////////////////////////////////////////
//
bool COrthoSceneOSG::handleDraggersContinue(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	// !!!!!!! What if this condition wasn't here ??????
	// sort of doesn't send FINISH motion commands when it is

	if( ea.getButtonMask() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON &&
		ea.getEventType() != osgGA::GUIEventAdapter::RELEASE )
		return false;

	osgViewer::Viewer * m_view = m_pCanvas->getView();

	assert(m_view != NULL);

	// Transform Y coordinate
	//	ea.setY(ea.getYmax() - ea.getY() + 1);

	if (m_activeDragger != NULL)
	{
		m_pointerInfo._hitIter = m_pointerInfo._hitList.begin();
		m_pointerInfo.setCamera(m_view->getCamera());
		//m_pointerInfo.setMousePosition(ea.getX(), ea.getY());
		m_pointerInfo.setMousePosition(ea.getX(), ea.getYmax() - ea.getY() + 1);
		return m_activeDragger->handle(m_pointerInfo, ea, aa);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Handle window resize event
bool COrthoSceneOSG::handleWindowResize(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    OSGOrtho2DCanvas *pCanvas = dynamic_cast<OSGOrtho2DCanvas *>(m_pCanvas);
    double px(pCanvas->getPixelSize()[0]), py(pCanvas->getPixelSize()[1]);
	float r((px < py)?py:px);
	cylinder->setRadius(r*SELECTOR_MULTIPLIER);
	return false;
}

//////////////////////////////////////////////////////////////////////////
//
bool COrthoSceneOSG::handlePlaneMove(const osgManipulator::TranslateInLineCommand & command)
{
	if(command.getStage() != osgManipulator::TranslateInLineCommand::START)
	{
		// compute integer slice position
		int position = 100*planeDragger->translation()[2];

        //_TRACEW(std::endl << "Translation: " << position);

        // Signal plane move
        VPL_SIGNAL(SigSetSliceXY).invoke(position);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//
void COrthoSceneOSG::updateFromStorage()
{
    data::CObjectPtr<data::COrthoSliceXY> spSlice( APP_STORAGE.getEntry(data::Storage::SliceXY::Id) );

	// Update the texture data
	osg::StateSet* stateset = geom->getOrCreateStateSet();
	if( stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE) != spSlice->getTexturePtr() )
	{
		stateset->setTextureAttributeAndModes(0, spSlice->getTexturePtr(), osg::StateAttribute::ON);
//        geom->setStateSet(stateset);
	}

	// Change the texture size
	osg::Vec2Array *texcoords = dynamic_cast<osg::Vec2Array *>(geom->getTexCoordArray(0));
	(*texcoords)[0].set(0.0f, spSlice->getTextureHeight());
	(*texcoords)[1].set(0.0f, 0.0f);
	(*texcoords)[2].set(spSlice->getTextureWidth(), 0.0f);
	(*texcoords)[3].set(spSlice->getTextureWidth(), spSlice->getTextureHeight());
}

//////////////////////////////////////////////////////////////////////////
//
void COrthoSceneOSG::createGeometry()
{
	osg::Group* group = new osg::Group;

	// Turn off lighting 
	group->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	// Create the geometry for the slice
	geom = new osg::Geometry;

	osg::Vec3Array* vertices = new osg::Vec3Array(4);
	(*vertices)[0] = osg::Vec3(0.0, 1.0, 0.0);
	(*vertices)[1] = osg::Vec3(0.0, 0.0, 0.0);
	(*vertices)[2] = osg::Vec3(1.0, 0.0, 0.0);
	(*vertices)[3] = osg::Vec3(1.0, 1.0, 0.0);
	geom->setVertexArray(vertices);

	osg::Vec2Array* texcoords = new osg::Vec2Array(4);
	(*texcoords)[0].set(0.0f,1.0f);
	(*texcoords)[1].set(0.0f,0.0f);
	(*texcoords)[2].set(1.0f,0.0f);
	(*texcoords)[3].set(1.0f,1.0f);
	geom->setTexCoordArray(0,texcoords);

	osg::Vec3Array* normals = new osg::Vec3Array(1);
	(*normals)[0].set(0.0f,0.0f,1.0f);
	geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

	osg::Vec4Array* colors = new osg::Vec4Array(1);
	(*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
	geom->setColorArray(colors, osg::Array::BIND_OVERALL);

	geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	osg::Geode* GeomGeode = new osg::Geode;
	GeomGeode->addDrawable(geom.get());

	GeomGeode->setName("Plane");

	//    group->addChild(GeomGeode);
	group->setName("Scene group");

	//---------------------------
	// Add line

	// create geode
	lineGeode = new osg::Geode;

	// create geometry
	lineGeometry = new osg::Geometry;

	// create array of vertices
	osg::Vec3Array * lineVertices = new osg::Vec3Array;

	// add vertices
	lineVertices->push_back(osg::Vec3(0.0, 0.0, 0.0));
	lineVertices->push_back(osg::Vec3(0.0, 1.0, 0.0));

	// Set vertices to geometry
	lineGeometry->setVertexArray(lineVertices);

	// Create array
	osg::DrawArrays * drawArrays = new osg::DrawArrays(osg::PrimitiveSet::LINES,0,lineVertices->size());

	// Set primitive set
	lineGeometry->addPrimitiveSet(drawArrays);

	// set the colors
	osg::Vec4Array * color = new osg::Vec4Array;
	color->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
	lineGeometry->setColorArray(color, osg::Array::BIND_OVERALL);

	// set the normal in the same way color.
	//    osg::Vec3Array* lineNormals = new osg::Vec3Array;
	//    normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
	//    normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));

	//    lineGeometry->setNormalArray(lineNormals);
	//    lineGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);

	// Add geometry to geode
	lineGeode->addDrawable(lineGeometry.get());
	lineGeode->setName("Line");

	// Set rendering priorities etc.

	// Create and set up a state set using the texture from above:
	osg::StateSet* stateSet = new osg::StateSet();
	// Set state set to geode
	lineGeode->setStateSet(stateSet);

	// Shading etc
	stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	// Set line width
	linewidth = new osg::LineWidth();
	linewidth->setWidth(2.0f);
	stateSet->setAttributeAndModes(linewidth.get(), osg::StateAttribute::ON);

	// Disable depth testing so geometry is draw regardless of depth values
	// of geometry already draw.
	stateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
	stateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

	// Need to make sure this geometry is drawn last. RenderBins are handled
	// in numerical order so set bin number to 11
	stateSet->setRenderBinMode(osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);//:USE_RENDERBIN_DETAILS);
	stateSet->setRenderBinDetails( 111, "RenderBin");

	// Create an invisible cylinder for picking the line.
	//    {
    OSGOrtho2DCanvas *pCanvas = dynamic_cast<OSGOrtho2DCanvas *>(m_pCanvas);
	cylinder = new osg::Cylinder (osg::Vec3(0.0, 0.5, 0.0), pCanvas->getPixelSize()[0]*SELECTOR_MULTIPLIER, 1);
	osg::Quat rotation;
	rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, 1.0f, 0.0f));
	cylinder->setRotation(rotation);
	osg::Drawable* cylinderGeom = new osg::ShapeDrawable(cylinder.get());

	// set drawable as invisible
	//osgManipulator::setDrawableToAlwaysCull(*cylinderGeom);
    setForceCullCallback( cylinderGeom );

	osg::Geode* cylinderGeode = new osg::Geode;
	cylinderGeode->addDrawable(cylinderGeom);
	//    }

	// Disable culling
	lineGeode->setCullingActive(false);

	// Create line dragger
	lineDragger = new osgManipulator::Translate1DDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(1.0, 0.0, 0.0));

	osg::ref_ptr< osg::MatrixTransform > transform = new osg::MatrixTransform;
	transform->addChild(lineGeode.get());
	lineDragger->addChild(cylinderGeode);

	// Create line constraint
	lineConstraint = new osgManipulator::CLineConstraint(*lineGeode.get());

	/////////////////
	// Plane dragging 
	planeDragger = new osgManipulator::CTranslateOtherLineDragger(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0), osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 1.0, 0.0));

	// Add plane to dragger
	planeDragger->addChild(GeomGeode);
	planeDragger->setName("Plane dragger");

	// Create plane constraint
	planeConstraint = new osgManipulator::CLineConstraint(*GeomGeode);

	// connect line dragger and selection
	lineDragger->addTransformUpdating( transform );
	lineDragger->addConstraint(lineConstraint);
	planeDragger->addConstraint(planeConstraint);

	// Add geode to scene
	group->addChild(transform);

	// Add dragger to scene
	group->addChild(lineDragger.get());

	// Add plane dragger to scene
	group->addChild(planeDragger.get());

	//---------------------------
	this->addChild(group);
}

} // namespace scene
