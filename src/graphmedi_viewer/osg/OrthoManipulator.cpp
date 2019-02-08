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

#include <osg/OrthoManipulator.h>

#include <osg/CAppMode.h>
#include <VPL/Math/Base.h>
#include <base/Defs.h>

using namespace osgGA;

#ifdef __APPLE__
#define SCROLL_STEP 0.035
#else
#define SCROLL_STEP 0.2
#endif

// Constructor
OrthoManipulator::OrthoManipulator()
    : event1(NULL),
    event2(NULL),
    _thrown(false),
    _near(0),
    _far(0),
    _left(0),
    _right(0),
    _bottom(0),
    _top(0),
    _zoom(1.0),
    _shiftScale(1.0),
    _zoomScale(1.0),
    m_bUseStoredBB(false),
    m_wasPush(false),
    m_enabled(true)
{
}

void OrthoManipulator::zoomGesture(double zoomFactor)
{
    zoom(zoomFactor - 1.0);
}

void OrthoManipulator::panGesture(osg::Vec2 movement)
{
    move(movement);
}

// Handle events
bool OrthoManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    // Check the application mode
    if( !APP_MODE.check(scene::CAppMode::MODE_TRACKBALL) )
    {
        return false;
    }

	switch(ea.getEventType())
	{
	// Mouse down
	case(GUIEventAdapter::PUSH):
		{
            m_wasPush = true;
			clearStack();
			addMouseEvent(ea);
			if (calcMovement()) us.requestRedraw();
			us.requestContinuousUpdate(false);
			_thrown = false;
			return true;
		}

	case(GUIEventAdapter::RELEASE):
		{
            if (!m_wasPush)
            {
                return false;
            }
            else
            {
                m_wasPush = false;
            }

			// Left mouse button - rotate
			if (ea.getButtonMask()==0)
			{
/*
				// Isn't it click?
				double timeSinceLastRecordEvent = event1.valid() ? (ea.getTime() - event1->getTime()) : DBL_MAX;
				if (timeSinceLastRecordEvent>0.02) clearStack();

				if (isMouseMoving())
				{
					if (calcMovement())
					{
						us.requestRedraw();
						us.requestContinuousUpdate(true);
						_thrown = true;
					}
				}
				else
				{
					clearStack();
					addMouseEvent(ea);
					if (calcMovement()) us.requestRedraw();
					us.requestContinuousUpdate(false);
					_thrown = false;
				}
*/
				_thrown = false;
				return false;
			}
			else
			{
				clearStack();
				addMouseEvent(ea);
				if (calcMovement()) us.requestRedraw();
				us.requestContinuousUpdate(false);
				_thrown = false;
			}
			return true;
		}

	case(GUIEventAdapter::DRAG):
		{
            if (!m_wasPush)
            {
                return false;
            }

			addMouseEvent(ea);
			if (calcMovement()) us.requestRedraw();
			us.requestContinuousUpdate(false);
			_thrown = false;
			return true;
		}

	case(GUIEventAdapter::MOVE):
		{
			return false;
		}

    case(GUIEventAdapter::SCROLL):
        {
            if( m_sigScroll.getNumOfConnections() > 0 )
            {
                if( m_sigScroll.invoke2( ea.getScrollingMotion() ) )
                    return true;
            }

            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            us.requestContinuousUpdate(false);
            _thrown = false;
			
            return true;
        }

	case(GUIEventAdapter::KEYDOWN):
		if (ea.getKey()== GUIEventAdapter::KEY_Space)
		{
			clearStack();
			_thrown = false;
	//		home(ea,us);
			return true;
		}

      // Move up
	  if ( ea.getKey() == UIK_UP || ea.getKey() == GUIEventAdapter::KEY_Page_Up )
      {
         clearStack();
         _thrown = false;
		 if (ea.getKey() == GUIEventAdapter::KEY_Page_Up)
			m_sigUpDown.invoke( 5 );
		 else
			m_sigUpDown.invoke( 1 );

//         us.requestRedraw();
//			us.requestContinuousUpdate(false);
         return true;
      }

      // Move down
      if ( ea.getKey() == UIK_DOWN || ea.getKey() == GUIEventAdapter::KEY_Page_Down )
      {
         clearStack();
         _thrown = false;
		 if (ea.getKey() == GUIEventAdapter::KEY_Page_Down)
			m_sigUpDown.invoke( -5 );
		 else
			m_sigUpDown.invoke( -1 );
//         us.requestRedraw();
//			us.requestContinuousUpdate(false);
         return true;
      }

		return false;
	case(GUIEventAdapter::FRAME):
		if (_thrown)
		{
			if (calcMovement()) us.requestRedraw();
		}
		return false;
	default:
		return false;
	}
	return false;
}

// Clear stack
void OrthoManipulator::clearStack()
{
	event1 = NULL;
	event2 = NULL;
}

// Push value to stack
void OrthoManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
	event2 = event1;
	event1 = &ea;
}


// Is mouse moving?
bool OrthoManipulator::isMouseMoving()
{
	if (event1.get()==NULL || event2.get()==NULL) return false;

	static const float velocity = 0.1f;

	float dx = event1->getXnormalized()-event2->getXnormalized();
	float dy = event1->getYnormalized()-event2->getYnormalized();
	float len = sqrtf(dx*dx+dy*dy);
	float dt = event1->getTime()-event2->getTime();

	return (len>dt*velocity);
}


//! Move (slide)
void OrthoManipulator::move(osg::Vec2 direction)
{
	osg::Vec2 window_size(_right - _left, _top - _bottom);

	float ndx = (direction[0])*(window_size[0]/2);
	float ndy = (-direction[1])*(window_size[1]/2);

	osg::Vec3 dv(ndx*_shiftScale,ndy*_shiftScale,0.0f);

	osg::Vec3 shift = dv;

	_left -= shift.x();
	_right -= shift.x();
	_top += shift.y();
	_bottom += shift.y();
}

//! Zoom
void OrthoManipulator::zoom(double ammount)
{
	osg::Vec2 window_size((_right - _left), (_top - _bottom));

	osg::Vec2 center((_right + _left)/2, (_top + _bottom) / 2);

	osg::Vec2 zoom_step = window_size*_zoomScale;
    zoom_step *= ammount;

//	if(ammount < 0)
//		zoom_step *= -1;

	window_size[0] += zoom_step[0];
	window_size[1] += zoom_step[1];

   
	window_size /= 2;

	_left = center[0] - window_size[0];
	_right = center[0] + window_size[0];
	_top = center[1] + window_size[1];
	_bottom = center[1] - window_size[1];
}

//! Set manipulator by matrix
void OrthoManipulator::setByMatrix(const osg::Matrixd& matrix)
{
	matrix.getOrtho(_left, _right, _bottom, _top, _near, _far);
}

//! Set the position of the matrix manipulator using a 4x4 Matrix. 
void OrthoManipulator::setByInverseMatrix (const osg::Matrixd &matrix)
{
	osg::Matrixd m(matrix);

	m.invert(m);

    m.getOrtho(_left, _right, _bottom, _top, _near, _far);
}

//! Get the position of the manipulator as 4x4 Matrix. 
osg::Matrixd OrthoManipulator::getMatrix () const
{
	osg::Matrixd matrix;

	matrix.makeOrtho2D(_left, _right, _bottom, _top);

	return matrix;
}

//! Get the position of the manipulator as a inverse matrix of the manipulator, typically used as a model view matrix. 
osg::Matrixd OrthoManipulator::getInverseMatrix() const
{
//	return osg::Matrixd::invert(getMatrix());

	osg::Matrixd matrix = getMatrix();
//	matrix.invert(getMatrix());

	return matrix;
}

void OrthoManipulator::setNode(osg::Node *node)
{
    _node = node;            
    if( getAutoComputeHomePosition() ) computeHomePosition();
}

const osg::Node *OrthoManipulator::getNode() const
{
    return _node.get();
}

osg::Node *OrthoManipulator::getNode()
{
    return _node.get();
}

void OrthoManipulator::computeHomePosition()
{
    setHomePosition(osg::Vec3d(0,0,-5), osg::Vec3d(0,0,0), osg::Vec3d(0,1,0));
}

//! Move the camera to the default position.
void OrthoManipulator::home(const GUIEventAdapter &ea, GUIActionAdapter &us)
{
    home(ea.getTime());
	us.requestRedraw();
	us.requestContinuousUpdate(false);
}

//! Move the camera to the default position
void OrthoManipulator::home(double)
{
    double dRatio( 1.0 );

	// That ugly (a * 0) == 0 part is portable isnan(a)
    if( _top != _bottom && (_top * 0 ) == 0 && (_bottom * 0 ) == 0 )
        dRatio = vpl::math::getAbs( ( _right - _left ) / ( _top - _bottom ));

    if( getNode() )
    {
        osg::BoundingSphere boundingSphere;

        if( m_bUseStoredBB )
        {
            if(m_box.valid())
                boundingSphere.expandBy( m_box );
        }
        else
        {
            // compute bounding sphere
            boundingSphere = getNode()->getBound();
        }

        osg::Node * node( getNode() );

        if( node->getName() == "NormalScene" )
            dRatio = 1.0;

        double wsize = boundingSphere._radius * 0.75;
        
        _right = boundingSphere._center[0] + wsize * dRatio;
        _left  = boundingSphere._center[0] - wsize * dRatio;
        
        _bottom = boundingSphere._center[1] - wsize;
        _top    = boundingSphere._center[1] + wsize;
    }

	

    if (getAutoComputeHomePosition()) 
		computeHomePosition();

	computePosition(_homeEye, _homeCenter, _homeUp);

  

	_thrown = false;
}

void OrthoManipulator::computePosition(int width, int height)
{
	osg::Matrix ortho;

	double l,r,t,b, tmp;

	if(width > height)
	{
		l = -1.0; r = 1.0;
		tmp = static_cast<double>(height)/static_cast<double>(width);
		t = tmp; b = -tmp;
	}else{

		t = 1.0; b = -1.0;
		tmp = static_cast<double>(width)/static_cast<double>(height);
		l = -tmp; r = tmp;
	}

	ortho.makeOrtho2D(l, r, b, t);
	//ortho.makeOrtho2D(-1, 1, -1, 1);
	//setByMatrix(ortho);
}

void OrthoManipulator::computePosition(const osg::Vec3& eye,const osg::Vec3& center,const osg::Vec3& up)
{
	osg::Vec3 lv(center-eye);

	osg::Vec3 f(lv);
	f.normalize();
	osg::Vec3 s(f^up);
	s.normalize();
	osg::Vec3 u(s^f);
	u.normalize();
	//f *= -1;

	osg::Matrix rotation_matrix(s[0],     u[0],     f[0],     0.0f,
		s[1],     u[1],     f[1],     0.0f,
		s[2],     u[2],     f[2],     0.0f,
		0.0f,     0.0f,     0.0f,      1.0f);

	//_center = center;
	//_distance = lv.length();
	//_rotation = rotation_matrix.getRotate().inverse();

	//osg::Vec2 wsize(_right - _left, _top - _bottom);

	osg::Matrix ortho;
	ortho.makeOrtho2D(_left, _right, _bottom, _top);

	ortho *= rotation_matrix;

	setByMatrix(ortho);
}

//! Set center
void OrthoManipulator::setCenter(osg::Vec2 center)
{
    osg::Vec2 wsize(_right - _left, _top - _bottom);

    wsize /= 2;

    _right = center[0] + wsize[0];
    _left  = center[0] - wsize[0];

    _bottom = center[1] - wsize[1];
    _top    = center[1] + wsize[1];    
}

//! Set size
void OrthoManipulator::setSize(osg::Vec2 size)
{
    osg::Vec2 wcenter(_right + _left, _top + _bottom);

    wcenter /= 2;
    osg::Vec2 wsize(size/2);

    _right = wcenter[0] + wsize[0];
    _left  = wcenter[0] - wsize[0];

    _bottom = wcenter[1] - wsize[1];
    _top    = wcenter[1] + wsize[1];    
}

//! Set position of manipulator
void OrthoManipulator::setByCenterSize(osg::Vec2 center, osg::Vec2 size)
{
    osg::Vec2 wsize(size/2);

    _right = center[0] + wsize[0];
    _left  = center[0] - wsize[0];

    _bottom = center[1] - wsize[1];
    _top    = center[1] + wsize[1];    
}

void osgGA::OrthoManipulator::computeHomePosition( const osg::Camera * VPL_UNUSED(camera), bool VPL_UNUSED(useBoundingBox) )
{
	home(0.1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Calculates the movement. 
//!
//!\return  true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool OrthoManipulator::calcMovement()
{

    // Are some events stored?
    if(event1.get() == NULL || event2.get() == NULL)
        return false;

    double	wxs(event2->getXmax() - event2->getXmin()),
        wys(event2->getYmax() - event2->getYmin());

    if(wxs == 0.0 || wys == 0.0)
        return false;

    if(wxs > wys)
    {
        wxs = wxs/wys;
        wys = 1.0;
    }else{
        wys = wys/wxs;
        wxs = 1.0;				
    }

    // Compute distance
    float dx = (event1->getXnormalized()-event2->getXnormalized());
    float dy = (event1->getYnormalized()-event2->getYnormalized());

    float distance;// = dx*dx + dy*dy;

    // return if movement is too fast, indicating an error in event values or change in screen.
    //	if (distance>0.5*0.5)
    //	{
    //		return false;
    //	}

    dx *= wxs; dy *= wys;
    distance = sqrtf(dx*dx + dy*dy);

    // return if there is no movement.
    if (distance==0.0f && (event1->getEventType() != GUIEventAdapter::SCROLL))
    {
        return false;
    }

    unsigned int buttonMask = event1->getButtonMask();

    if(buttonMask == GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        // No rotation - so return not handled
        return false;
    }

    if(buttonMask == GUIEventAdapter::RIGHT_MOUSE_BUTTON )
    {
        // pan
        move(osg::Vec2(dx, dy));
        // invert y-axis
        //        move(osg::Vec2(dx, -dy));

        return true;
    }

    if(buttonMask == GUIEventAdapter::MIDDLE_MOUSE_BUTTON 
        || buttonMask == (GUIEventAdapter::LEFT_MOUSE_BUTTON | GUIEventAdapter::RIGHT_MOUSE_BUTTON))
    {
        // zoom
        zoom(dy);

        return true;
    }

    if(event1->getEventType() == GUIEventAdapter::SCROLL)
    {
        if(event1->getScrollingMotion() == GUIEventAdapter::SCROLL_UP)
            zoom(SCROLL_STEP);
        else
            zoom(-SCROLL_STEP);

        return true;
    }

    return false;

} // calcMovement()
