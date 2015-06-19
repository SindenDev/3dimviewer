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

#include "osg/CSceneManipulator.h"

#include <osg/CAppMode.h>
#include <data/CSceneManipulatorDummy.h>

#include <iostream>
#include <osg/OSGCanvas.h>
#include <osg/ComputeBoundsVisitor>

using namespace osgGA;

bool osg::CSceneManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
    // Check the application mode
    if( !APP_MODE.check(scene::CAppMode::MODE_TRACKBALL) )
    {
        return false;
    }
    
    switch(ea.getEventType())
    {
        case(GUIEventAdapter::FRAME):
            if (_thrown)
            {
                if (calcMovement()) us.requestRedraw();
            }
            return false;
        default:
            break;
    }

    if (ea.getHandled()) return false;

    switch(ea.getEventType())
    {
        case(GUIEventAdapter::PUSH):
        {
            flushMouseEventStack();
            addMouseEvent(ea);			
            if (calcMovement()) us.requestRedraw();
            us.requestContinuousUpdate(false);
            _thrown = false;
            return true;
        }

        case(GUIEventAdapter::RELEASE):
        {
            if (ea.getButtonMask()==0)
            {
            
                double timeSinceLastRecordEvent = _ga_t0.valid() ? (ea.getTime() - _ga_t0->getTime()) : DBL_MAX;
                if (timeSinceLastRecordEvent>0.02) flushMouseEventStack();

                {
                    flushMouseEventStack();
                    addMouseEvent(ea);
                    if (calcMovement()) us.requestRedraw();
                    us.requestContinuousUpdate(false);
                    _thrown = false;
                }

            }
            else
            {
                flushMouseEventStack();
                addMouseEvent(ea);
                if (calcMovement()) us.requestRedraw();
                us.requestContinuousUpdate(false);
                _thrown = false;
            }
            return true;
        }

        case(GUIEventAdapter::DRAG):
        {
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
//		case(GUIEventAdapter::SCROLL_DOWN):
//		case(GUIEventAdapter::SCROLL_UP):
        {		
            //flushMouseEventStack();
            addMouseEvent(ea);
            if (calcMovement()) us.requestRedraw();
            us.requestContinuousUpdate(false);
            _thrown = false;
            return true;
        }

        case(GUIEventAdapter::KEYDOWN):
            if (ea.getKey()== GUIEventAdapter::KEY_Space)
            {
                flushMouseEventStack();
                _thrown = false;
                home(ea,us);
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

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Calculates the movement. 
//!
//!\return  true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool osg::CSceneManipulator::calcMovement()
{
    // return if less then two events have been added.
    if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

    float sensitivity = 0.005f;
    float dragSensitivity = 0.5f;
    float rotateSensitivity = 2.0f;
    float zoomSensitivity = 1.0f;

    float ndx = _ga_t0->getXnormalized() - _ga_t1->getXnormalized();
    float ndy = _ga_t0->getYnormalized() - _ga_t1->getYnormalized();

    float dx = (_ga_t0->getX() - _ga_t1->getX()) * sensitivity;
#ifdef QT_VERSION
    float dy = (_ga_t0->getY() - _ga_t1->getY()) * sensitivity;
#else
    float dy = (_ga_t0->getY() - _ga_t1->getY()) * sensitivity * -1.0f;
#endif

    float distance = sqrtf(dx * dx + dy * dy);
    float ndistance = sqrtf(ndx * ndx + ndy * ndy);

    // return if movement is too fast, indicating an error in event values or change in screen.
    if (ndistance > 0.5)
    {
        return false;
    }


    //unsigned int buttonMask = _ga_t1->getButtonMask();
    if ( _ga_t0->getEventType() == GUIEventAdapter::SCROLL )
    {
        if ( _ga_t0->getScrollingMotion() == GUIEventAdapter::SCROLL_UP )
        {
            //osg::Matrix rotation_matrix(_rotation);
            //osg::Vec3 dv = (osg::Vec3(0.0f,0.0f,-1.0f)*rotation_matrix)*( -20.0 );
            //_center += dv;
            //double b = _distance;
            _distance *= zoomSensitivity * 1.1;

        }
        else if ( _ga_t0->getScrollingMotion() == GUIEventAdapter::SCROLL_DOWN )
        {
            //osg::Matrix rotation_matrix(_rotation);
            //osg::Vec3 dv = (osg::Vec3(0.0f,0.0f,-1.0f)*rotation_matrix)*( 20.0 );
            //_center += dv;
            //double b = _distance;
            _distance *= zoomSensitivity * 0.9;

        }
        // Invalidate dummy
        APP_STORAGE.invalidate( APP_STORAGE.getEntry( data::Storage::SceneManipulatorDummy::Id, data::Storage::NO_UPDATE ).get() );

        return true;
    }	

    // return if there is no movement.
    if (distance==0.0f)
    {
        return false;
    }

    unsigned int buttonMask = _ga_t1->getButtonMask();
    if (buttonMask == GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
        // rotate camera.
        osg::Vec3d axis;
        float angle;

        // trackball is deformed by window dimensions and rotating is unnatural if width to height ratio is not near 1.0f
        float px0n = _ga_t0->getXnormalized();
        float py0n = _ga_t0->getYnormalized();
        float px1n = _ga_t1->getXnormalized();
        float py1n = _ga_t1->getYnormalized();

        float width = _ga_t0->getWindowWidth();
        float height = _ga_t0->getWindowHeight();
        float ratio = height / width;

        // "undeform" trackball
        if (ratio > 1.0f) // height is bigger than width
        {
            px1n = px1n / ratio;
            px0n = px0n / ratio;
        }
        else // width is bigger than height
        {
            py1n = py1n * ratio;
            py0n = py0n * ratio;
        }

        trackball(axis, angle, px1n, py1n, px0n, py0n);

        osg::Quat new_rotate;
        new_rotate.makeRotate(rotateSensitivity * angle, axis);

        _rotation = _rotation * new_rotate;

        // Invalidate dummy
        APP_STORAGE.invalidate( APP_STORAGE.getEntry( data::Storage::SceneManipulatorDummy::Id, data::Storage::NO_UPDATE ).get() );

        return true;

    }
    else if (buttonMask==GUIEventAdapter::RIGHT_MOUSE_BUTTON )
    {

        // pan model.

        float scale = -0.3f * _distance * dragSensitivity;

        osg::Matrix rotation_matrix;
        rotation_matrix.makeRotate(_rotation);

        osg::Vec3 dv(dx*scale,dy*scale,0.0f);

        _center += dv*rotation_matrix;

        // Invalidate dummy
        APP_STORAGE.invalidate( APP_STORAGE.getEntry( data::Storage::SceneManipulatorDummy::Id, data::Storage::NO_UPDATE ).get() );

        return true;

    }
    else if (buttonMask==GUIEventAdapter::MIDDLE_MOUSE_BUTTON ||
        buttonMask==( GUIEventAdapter::LEFT_MOUSE_BUTTON | GUIEventAdapter::RIGHT_MOUSE_BUTTON ) )
    {

        // zoom model.

        float fd = _distance;
        float scale = 1.0f+dy;
        float _modelScale( 1.0 );
        float _minimumZoomScale( 0.001 );
        if (fd*scale>_modelScale*_minimumZoomScale)
        {

            _distance *= scale;

        }
        else
        {

            // notify(DEBUG_INFO) << "Pushing forward"<<std::endl;
            // push the camera forward.
            float scale = -fd;

            osg::Matrix rotation_matrix(_rotation);

            osg::Vec3 dv = (osg::Vec3(0.0f,0.0f,-1.0f)*rotation_matrix)*(dy*scale);

            _center += dv;

        }

        // Invalidate dummy
        APP_STORAGE.invalidate( APP_STORAGE.getEntry( data::Storage::SceneManipulatorDummy::Id, data::Storage::NO_UPDATE ).get() );

        return true;
    }

    return false;
}

/**
 * \fn  void osg::CSceneManipulator::computeHomePosition( const osg::Camera *camera,
 *      bool useBoundingBox )
 *
 * \brief   Calculates the home position.
 *
 * \param   camera          The camera.
 * \param   useBoundingBox  true to use bounding box.
 */

void osg::CSceneManipulator::computeHomePosition( const osg::Camera *camera, bool useBoundingBox )
{
    if (getNode())
    {
        osg::BoundingSphere boundingSphere;

        OSG_INFO<<" CameraManipulator::computeHomePosition("<<camera<<", "<<useBoundingBox<<")"<<std::endl;

        if( m_bUseStoredBB )
        {
            if(m_box.valid())
                boundingSphere.expandBy( m_box );
        }
        else
        {
            if (useBoundingBox)
            {
                // compute bounding box
                // (bounding box computes model center more precisely than bounding sphere)
                osg::ComputeBoundsVisitor cbVisitor;
                getNode()->accept(cbVisitor);
                osg::BoundingBox &bb = cbVisitor.getBoundingBox();

                if (bb.valid()) boundingSphere.expandBy(bb);
                else boundingSphere = getNode()->getBound();
            }
            else
            {
                getNode()->dirtyBound();
                // compute bounding sphere
                boundingSphere = getNode()->getBound();
            }
        }

        // set dist to default
        double dist = 1.0 * boundingSphere.radius();

        if (camera)
        {

            // try to compute dist from frustrum
            double left,right,bottom,top,zNear,zFar;
            if (camera->getProjectionMatrixAsFrustum(left,right,bottom,top,zNear,zFar))
            {
                double vertical2 = fabs(right - left) / zNear / 2.;
                double horizontal2 = fabs(top - bottom) / zNear / 2.;
                double dim = horizontal2 < vertical2 ? horizontal2 : vertical2;
                double viewAngle = atan2(dim,1.);
                dist = boundingSphere.radius() / sin(viewAngle);
            }
            else
            {
                // try to compute dist from ortho
                if (camera->getProjectionMatrixAsOrtho(left,right,bottom,top,zNear,zFar))
                {
                    dist = fabs(zFar - zNear) / 2.;
                }
            }
        }

        // set home position
        setHomePosition(boundingSphere.center() + osg::Vec3d(0.0,-dist,0.0f),
            boundingSphere.center(),
            osg::Vec3d(0.0f,0.0f,1.0f),
            true);
    }
}
