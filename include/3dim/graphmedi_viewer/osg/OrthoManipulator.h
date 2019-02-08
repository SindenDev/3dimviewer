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

#ifndef ORTHOMANIPULATOR_H
#define ORTHOMANIPULATOR_H

#include <osgGA/CameraManipulator>
#include <VPL/Module/Signal.h>

namespace osgGA
{

///////////////////////////////////////////////////////////////////////////////
//

class OrthoManipulator : public CameraManipulator
{
public:
   //! Up/down signal type
   typedef vpl::mod::CSignal< void, int > tSigUpDown;

   //! Scroll signal type
   typedef vpl::mod::CSignal< bool, const osgGA::GUIEventAdapter::ScrollingMotion > tSigScroll;

public:
	//! Constructor
	OrthoManipulator();

	//! handle events, return true if handled, false otherwise.
	virtual bool handle(const GUIEventAdapter& ea,GUIActionAdapter& us);

	//! Set manipulator by matrix
	virtual void setByMatrix(const osg::Matrixd& matrix);

	//! Set the position of the matrix manipulator using a 4x4 Matrix. 
	virtual void setByInverseMatrix (const osg::Matrixd &matrix);

    //! Set default position
    virtual void computeHomePosition();

    //! Sets the reference model.
    virtual void setNode(osg::Node *node);

    //! Returns the reference model.
    virtual const osg::Node *getNode() const;
    virtual osg::Node *getNode();

    //! Set center
    void setCenter(osg::Vec2 center);

    //! Set size
    void setSize(osg::Vec2 size);

    //! Set position of manipulator
    void setByCenterSize(osg::Vec2 center, osg::Vec2 size);

	//! Get the position of the manipulator as 4x4 Matrix. 
	virtual osg::Matrixd getMatrix () const;

	//! Get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix. 
	virtual osg::Matrixd getInverseMatrix () const;

	//! Move the camera to the default position.
	virtual void home(const GUIEventAdapter &, GUIActionAdapter &);
	
	//! Move the camera to the default position
	virtual void home(double);

	virtual void computeHomePosition(const osg::Camera *camera, bool useBoundingBox);

    //! Width of field of view
    double width() { return _right - _left;}

    //! Height of field of view
    double height() { return _top - _bottom;}

    //! Up or down key pressed - true as a parameter means move up.
    tSigUpDown m_sigUpDown;

    //! Get scrolling signal
    tSigScroll & getSigScroll() { return m_sigScroll; }

    //! Use stored bounding box when computing home position
    void useStoredBox( bool bUse ) { m_bUseStoredBB = bUse; }

    //! Store bounding box 
    void storeBB( const osg::BoundingBox & bb ) { m_box = bb; }

    void zoomGesture(double zoomFactor);
    void panGesture(osg::Vec2 movement);
    void setEnabled(bool value) { m_enabled = value; }

protected:
	//! Compute position from eye, center and up vector.
	void computePosition(const osg::Vec3& eye, const osg::Vec3& center, const osg::Vec3& up);

	//! Compute position from window width and height.
	void computePosition(int width, int height);

	//! Calculate move from stored events
	bool calcMovement();

	//! Is mouse moving?
	bool isMouseMoving();

	//! Store event.
	void addMouseEvent(const GUIEventAdapter& ea);

	//! Clear stack
	void clearStack();

	//! Move (slide)
	void move(osg::Vec2 direction);

	//! Zoom
	void zoom(double ammount);

protected:
	//! Internal event stack comprising last two mouse events.
	osg::ref_ptr<const GUIEventAdapter> event1, event2;

    //! Reference model.
    osg::ref_ptr<osg::Node> _node;

	// Thrown value
	bool _thrown;

	// Near plane
	double _near;

	// far plane
	double _far;

	// Window left
	double _left;

	// Window right
	double _right;

	// Bottom
	double _bottom;

	// Up
	double _top;

	//! "Zoom"
	double _zoom;

	//! Shift coefficient
	double _shiftScale;

	//! "Zoom" coefficient
	double _zoomScale;

    //! Scrolling signal
    tSigScroll m_sigScroll;

    //! Use stored bounding box when computing home position
    bool m_bUseStoredBB;

    //! Stored bounding box
    osg::BoundingBox m_box;

    bool m_wasPush;

    bool m_enabled;
};


} // namespace osgGA

#endif // ORTHOMANIPULATOR_H
