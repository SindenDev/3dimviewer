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

#ifndef CDraggableGeometry_H
#define CDraggableGeometry_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osg/CSignalDraggerCallback.h>
#include <osg/Switch>
#include <osgManipulator/Dragger>
#include <osgManipulator/CommandManager>
#include <osg/CBoundingBoxVisitor.h>

#include <VPL/Module/Signal.h>


namespace osg
{

///////////////////////////////////////////////////////////////////////////////
// type definitions

//! Use this ID number, if you want to send something to all views.
static const long ALL_ID = -1;

///////////////////////////////////////////////////////////////////////////////
//! CDraggable geometry stores geometry and draggers used to move it.
//!
//! Tree:				CDraggableGeometry as a root
//!							|						|
//!				geometry matrix transform		draggers switch
//!							|						|
//!					  geometries				draggers
//
class CDraggableGeometry : public Switch
{
public:
	//! Type of selection used.
	typedef osgManipulator::CSignalDraggerCallback<long> tDGCallback;

public:
	//! Constructor. Set position and direction.
	CDraggableGeometry(const osg::Matrix & placement, const long & id, bool isVisible = true, bool scaleDraggers = false);

	//! Destructor.
	virtual ~CDraggableGeometry();

	//! Add dragger
	void addDragger(osgManipulator::Dragger * dragger, bool bManaged = true );

    //! Add dragger as is. This dragger is not stored in the geometry composite dragger. Use this routine for pivot draggers for example.
    void addSpecialDragger(osgManipulator::Dragger *dragger);

    //! Removes "special" dragger
    void removeSpecialDragger(osgManipulator::Dragger *dragger);

	//! Add geometry
	void addGeometry(Node * node);

	//! Remove dragger
	void removeDragger(osgManipulator::Dragger * dragger);

	//! Remove geometry
	void removeGeometry(Node * node);

    //! Remove all draggers
    void removeAllDraggers();

	//! Recalculate dragger scales
	virtual void rescaleDraggers(float scale = 1.0);

    //! Set dragger (in)visible (they will be hidden or visible, when showDraggers is called)
    void setDraggersVisible( bool bVisible ) { m_draggersVisible = bVisible; }

	//! Show/hide all draggers
	virtual void showDraggers(bool draggersVisible = true, long id = ALL_ID);

	//! Show/hide all
	virtual void show(bool isVisible = true);

	//! Is node visible?
	virtual bool isVisible()
	{
		return m_isVisible;
	}

    //! moves both dragger to given position
    virtual void setGeometryPosition(osg::Vec3 position);

    virtual void setDraggerPosition(osg::Vec3 position);

    //! consider moving these into some common base..
    virtual void setGeometryRotation(osg::Quat rotation);

    virtual void setDraggerRotation(osg::Quat rotation);

    //! \fn virtual void setScale(const osg::Vec3 &s);
    //!
    //! \brief  Set scaling
    //!
    //! \param  s   The scale.
    virtual void setScale(const osg::Vec3 &s);

	//! Set internal draggers matrix
	void setDraggersMatrix( const osg::Matrix & matrix );

	//! Get signal reference
	//tDGSelection::tCommandSignal & getCommandSignal()
	tDGCallback::tMatrixSignal & getSignal()
	{
		return m_geometryCallback->getMatrixSignal();
	}

	//! Set parameter to be send by signal

	//! Get parameter - my id
	long getID() const
	{
		return m_id;
	}


	//! Move by matrix
	virtual void relocate(const osg::Matrix & m);

	//! Get geometry switch node
	Switch * getGeometrySwitch()
	{
		return m_geometrySwitch.get();
	}

	//! Get local to world matrix } from geometry switch
	osg::Matrix getGeometryLocalToWorldMatrix();

	//! Get current position matrix
	osg::Matrix getGeometryPositionMatrix() const
	{
		return m_geometryMatrixTransform->getMatrix();
	}

	const osg::BoundingBox & getGeometryBoundingBox()
	{
		return m_geometryBoundingBox;
	}

	const osg::BoundingBox & getOverallBoundingBox()
	{
		return m_overallBoundingBox;
	}

	osg::BoundingBox getTransformedGeometryBoundingBox(const osg::Matrix initialMatrix);

	osg::BoundingBox getTransformedGeometryBoundingBox()
	{
		return getTransformedGeometryBoundingBox(osg::Matrix::identity());
	}

public:
    //! My composite dragger nested subclass
    class CMyCompositeDragger : public osgManipulator::CompositeDragger
    {
    protected:
        osg::observer_ptr<CDraggableGeometry> m_dg;
    public:
        //! Constructor
        CMyCompositeDragger(CDraggableGeometry* dg, int handleCommandMask);

        //! Get pointer to draggable geometry which holds this dragger
        CDraggableGeometry* getDraggableGeometry() const { return m_dg.get(); }
    };

protected:
	//! Dispatch incomming command, if not sent by me
	void sigDispatchMatrix(const osg::Matrix & matrix, long id);

	//! Compute bonding boxes
	virtual void computeBoundingBoxes();

	//! Build scene tree
	void buildTree( const osg::Matrix & initialMatrix );

protected:
	//! Draggers switch - used to set draggers on/off
	ref_ptr<Switch> m_draggersSwitch;

	//! Geometry selection
	ref_ptr<tDGCallback> m_geometryCallback;

	//! Geometry switch
	ref_ptr<Switch> m_geometrySwitch;

	//! Geometry transform matrix manipulated by callback
	ref_ptr<MatrixTransform> m_geometryMatrixTransform;

    //! Geometry scale transform
    ref_ptr<MatrixTransform> m_geometryScaleTransform;

	//! Composite dragger used to tie all included draggers together
	ref_ptr<CMyCompositeDragger> m_compositeDragger;

	//! Parameter send by selection signal - id of the draggable geometry object
	long m_id;

	//! Is this geometry visible?
	bool m_isVisible;

    //! Should I show draggers?
    bool m_draggersVisible;

    bool m_scaleDraggers;

	//! Initial dragger matrix
	osg::Matrix m_draggerInitialMatrix;

	//! Initial geometry selection matrix.
	osg::Matrix m_geometryInitialMatrix;

	//! Geometry bounding box.
	osg::BoundingBox m_geometryBoundingBox;

	//! Overall bounding box.
	osg::BoundingBox m_overallBoundingBox;
};


} // namespace osg

#endif // CDraggableGeometry_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
