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

#include <osg/CDraggableGeometry.h>
#include <osg/Matrix>
#include <osg/Quat>
#include <osg/CBoundingBoxVisitor.h>
#include <sstream>


///////////////////////////////////////////////////////////////////////////////
// Constructor
osg::CDraggableGeometry::CDraggableGeometry(const osg::Matrix & placement, const long & id, bool isVisible /*= true*/, bool scaleDraggers/* = false*/)
	: m_id(id)
	, m_isVisible(isVisible)
    , m_draggersVisible( true )
    , m_scaleDraggers(scaleDraggers)
{
    std::stringstream ss;
    ss << "CDraggableGeometry " << id;
    setName(ss.str());
	buildTree( placement );
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
osg::CDraggableGeometry::~CDraggableGeometry()
{
    // because dragger event handler can contain reference to compositeDragger, fix its state in destructor
    if (nullptr != m_compositeDragger.get())
    {
        if (nullptr!= m_geometryCallback.get())
            m_compositeDragger->removeDraggerCallback(m_geometryCallback);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Add geometry
void osg::CDraggableGeometry::addGeometry(osg::Node *node)
{
	if(node == nullptr)
		return;

	m_geometrySwitch->addChild(node);

	// recalculate bounding boxes
	computeBoundingBoxes();
}

//////////////////////////////////////////////////////////////////////////
// Remove geometry
void osg::CDraggableGeometry::removeGeometry(Node * node)
{
	if(node == nullptr)
		return;

	if(m_geometrySwitch->containsNode(node))
		m_geometrySwitch->removeChild(node);

	// recalculate bounding boxes
	computeBoundingBoxes();

}

void osg::CDraggableGeometry::removeAllDraggers()
{
    m_compositeDragger->removeChildren(0, m_compositeDragger->getNumChildren());
    while(m_compositeDragger->getNumDraggers() > 0)
        m_compositeDragger->removeDragger(m_compositeDragger->getDragger(0));
}

//////////////////////////////////////////////////////////////////////////
// Add dragger
void osg::CDraggableGeometry::addDragger(osgManipulator::Dragger * dragger, bool bManaged /*= true*/)
{
    if (dragger == nullptr)
        return;

    // Add dragger to the scene
    m_compositeDragger->addChild(dragger);

    if (bManaged)
    {
        // Add dragger to the composite dragger
        m_compositeDragger->addDragger(dragger);

        dragger->setParentDragger(m_compositeDragger);
    }

    // recalculate bounding boxes
    computeBoundingBoxes();
}

void osg::CDraggableGeometry::addSpecialDragger(osgManipulator::Dragger *dragger)
{
    m_compositeDragger->addChild(dragger);
    // Add dragger to the composite dragger
    m_compositeDragger->addDragger(dragger);
}

/*!
 * \fn  void osg::CDraggableGeometry::removeSpecialDragger(osgManipulator::Dragger *dragger)
 *
 * \brief   Removes the "special" dragger 
 *
 * \param [in,out]  dragger If non-null, the dragger.
 */
void osg::CDraggableGeometry::removeSpecialDragger(osgManipulator::Dragger *dragger)
{
    if (dragger == nullptr)
        return;

    if (m_compositeDragger->containsNode(dragger))
    {
        m_compositeDragger->removeChild(dragger);
        m_compositeDragger->removeDragger(dragger);
        dragger->setParentDragger(dragger);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Recalculate dragger scales
void osg::CDraggableGeometry::rescaleDraggers(float scale)
{
	osgManipulator::Dragger * dragger( 0 );

	for( unsigned int i = 0; i < m_compositeDragger->getNumChildren(); ++i )
	{
		dragger = dynamic_cast< osgManipulator::Dragger * >( m_compositeDragger->getChild( i ) );

		if( dragger )
		{
			// scale dragger
			dragger->setMatrix(osg::Matrix::scale(scale, scale, scale));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Remove dragger
void osg::CDraggableGeometry::removeDragger(osgManipulator::Dragger * dragger)
{
	if(dragger == nullptr)
		return;

	// Try to remove dragger from the scene and composite dragger
	if(m_draggersSwitch->containsNode(dragger))
	{
		m_draggersSwitch->removeChild(dragger);
		m_compositeDragger->removeDragger(dragger);
        m_compositeDragger->removeChild(dragger);
		dragger->setParentDragger(dragger);
	}

	// recalculate bounding boxes
	computeBoundingBoxes();
}

//////////////////////////////////////////////////////////////////////////
// Dispatch incoming matrix, if not sent by me
void osg::CDraggableGeometry::sigDispatchMatrix(const osg::Matrix & matrix, long id)
{
	if(id != m_id)
	{
		relocate(matrix);
	}
}

///////////////////////////////////////////////////////////////////////////
// Relocate draggable geometry to the other position - by matrix
void osg::CDraggableGeometry::relocate(const osg::Matrix & m)
{

	// set new geometry position
	m_geometryMatrixTransform->setMatrix(m);
	
	// set new dragger position
	setDraggersMatrix( m );

}

//////////////////////////////////////////////////////////////////////////
// Show/Hide node
void osg::CDraggableGeometry::show(bool isVisible /* = true */)
{
    if (isVisible)
        setAllChildrenOn();
    else
        setAllChildrenOff();

    m_isVisible = isVisible;

    // Modify draggers visibility too...
    showDraggers(m_draggersVisible);
}  

//////////////////////////////////////////////////////////////////////////
// Show/hide draggers
void osg::CDraggableGeometry::showDraggers(bool draggersVisible /* = true */, long id /* = ALL_ID */)
{
	if(id == ALL_ID || id == m_id)
	{

		if(m_isVisible && draggersVisible)
			m_draggersSwitch->setAllChildrenOn();
		else
			m_draggersSwitch->setAllChildrenOff();

		m_draggersVisible = draggersVisible;
	}

}

//////////////////////////////////////////////////////////////////////////
// Get local to world matrix from geometry
osg::Matrix osg::CDraggableGeometry::getGeometryLocalToWorldMatrix()
{
	osg::NodePathList pathList = m_geometrySwitch->getParentalNodePaths();

	osg::NodePath path;

	if(pathList.size() > 0)
	{
		path = *pathList.begin();

		osg::Matrix m(m_geometryMatrixTransform->getMatrix());

		m = Matrix::inverse(m);

		osg::Matrix lw(computeLocalToWorld(path));

		return lw;
	}

	return osg::Matrix::identity();

}

// Recompute bounding boxes
void osg::CDraggableGeometry::computeBoundingBoxes()
{
	osg::CBoundingBoxVisitor visitor;

	// Compute geometry bounding box
	m_geometrySwitch->accept(visitor);
	m_geometryBoundingBox = visitor.getBoundingBox();

	// reset visitor
	visitor.reset();

	// Compute overall bounding box
	this->accept(visitor);
    m_overallBoundingBox = visitor.getBoundingBox();

}

// Get relocated geometry bounding box
osg::BoundingBox osg::CDraggableGeometry::getTransformedGeometryBoundingBox(const osg::Matrix initialMatrix)
{
	osg::CBoundingBoxVisitor visitor;
	visitor.setInitialMatrix(osg::Matrix(initialMatrix));

	m_geometryMatrixTransform->accept(visitor);

    return visitor.getBoundingBox();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Builds the scene tree. 
//
//		DG root node - geometry matrix transform - switch	- geometry 1
//															- geometry 2
//															- ...
//												 - switch	- dragger 1
//															- dragger 2
//															- ...
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CDraggableGeometry::buildTree( const osg::Matrix & initialMatrix )
{
	// Create geometry matrix transform
	m_geometryMatrixTransform = new MatrixTransform;
    m_geometryScaleTransform = new MatrixTransform;

    m_geometryMatrixTransform->setName("m_geometryMatrixTransform");
    m_geometryScaleTransform->setName("m_geometryScaleTransform");

	// Create geometries switch
	m_geometrySwitch = new Switch;
    m_geometrySwitch->setName("m_geometrySwitch");

	// add switch as a child to the matrix transform
	m_geometryMatrixTransform->addChild( m_geometryScaleTransform );
    m_geometryScaleTransform->addChild(m_geometrySwitch);

	// Add mt as a child to the root node
	addChild( m_geometryMatrixTransform );

	// Create draggers branch
	m_draggersSwitch = new Switch;
    m_draggersSwitch->setName("m_draggersSwitch");

    int mask = m_scaleDraggers ? tDGCallback::HANDLE_ALL : tDGCallback::HANDLE_ALL ^ tDGCallback::HANDLE_SCALED_1D ^ tDGCallback::HANDLE_SCALED_2D ^ tDGCallback::HANDLE_SCALED_UNIFORM;

    m_compositeDragger = new CMyCompositeDragger(this, mask);
    m_compositeDragger->setName("m_compositeDragger");
	m_draggersSwitch->addChild( m_compositeDragger );

	addChild( m_draggersSwitch );

	// Connect geometry and draggers
	m_geometryCallback = new tDGCallback( m_geometryMatrixTransform, m_geometryMatrixTransform, m_geometryScaleTransform );
    m_geometryCallback->setName("m_geometryCallback");
	m_compositeDragger->addDraggerCallback( m_geometryCallback );

	// Set position
	setDraggersMatrix( initialMatrix );
	m_geometryMatrixTransform->setMatrix( initialMatrix );
	
	// Store dragger initial matrix
	m_draggerInitialMatrix = initialMatrix;

	// Set id as a parameter to the geometry callback
	m_geometryCallback->parameter() = m_id;

	// Store geometry initial matrix
	m_geometryInitialMatrix = initialMatrix;

}

void osg::CDraggableGeometry::setGeometryPosition(osg::Vec3 position) 
{

    osg::Vec3 translation, scale;
    osg::Quat rotation, so;
    m_geometryMatrixTransform->getMatrix().decompose(translation, rotation, scale, so);

    osg::Matrix resultMatrix, translateMatrix, rotateMatrix;

    translateMatrix.makeTranslate(position); //this one does it
    rotateMatrix.makeRotate(rotation);

    //put all back again
    resultMatrix = rotateMatrix * translateMatrix;

    m_geometryMatrixTransform->setMatrix(resultMatrix);
    //m_compositeDragger->setMatrix(translateMatrix);
}


void osg::CDraggableGeometry::setDraggerPosition(osg::Vec3 position)
{

    osg::Vec3 translation, scale;
    osg::Quat rotation, so;
    m_compositeDragger->getMatrix().decompose(translation, rotation, scale, so);

    osg::Matrix resultMatrix, translateMatrix, rotateMatrix;

    translateMatrix.makeTranslate(position); //this one does it
    rotateMatrix.makeRotate(rotation);

    //put all back again
    resultMatrix = rotateMatrix * translateMatrix;

    //m_geometryMatrixTransform->setMatrix(resultMatrix);
     m_compositeDragger->setMatrix(resultMatrix);
}

void osg::CDraggableGeometry::setGeometryRotation(osg::Quat rotation) 
{

    osg::Vec3 translation, scale;
    osg::Quat rotations, so;
    m_geometryMatrixTransform->getMatrix().decompose(translation, rotations, scale, so);

    osg::Matrix resultMatrix, translateMatrix, rotateMatrix;

    translateMatrix.makeTranslate(translation);
    rotateMatrix.makeRotate(rotation); //this one does it

    //put all back again
    resultMatrix = rotateMatrix * translateMatrix;

    m_geometryMatrixTransform->setMatrix(resultMatrix);
    //m_compositeDragger->setMatrix(resultMatrix); //dont rotate draggers..
}

void osg::CDraggableGeometry::setDraggerRotation(osg::Quat rotation)
{

    //leaves translation as is and adds to it rotation

    osg::Vec3 translation, scale;
    osg::Quat rotations, so;
    m_compositeDragger->getMatrix().decompose(translation, rotations, scale, so);

    osg::Matrix resultMatrix, translateMatrix, rotateMatrix;

    translateMatrix.makeTranslate(translation);
    rotateMatrix.makeRotate(rotation); //this one does it

    //put all back again
    resultMatrix = rotateMatrix * translateMatrix;

    //m_geometryMatrixTransform->setMatrix(resultMatrix);
    m_compositeDragger->setMatrix(resultMatrix); 
}


void osg::CDraggableGeometry::setScale(const osg::Vec3 & s)
{
    m_geometryScaleTransform->setMatrix(osg::Matrix::scale(s));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Sets the draggers matrix. 
//!
//!\param	matrix	The matrix. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void osg::CDraggableGeometry::setDraggersMatrix( const osg::Matrix & matrix )
{
	// Simply set the matrix
	m_compositeDragger->setMatrix( matrix );
}

osg::CDraggableGeometry::CMyCompositeDragger::CMyCompositeDragger(CDraggableGeometry * dg, int handleCommandMask)
    : m_dg(dg)
{
    _selfUpdater = new osgManipulator::DraggerTransformCallback(this, handleCommandMask);
}
