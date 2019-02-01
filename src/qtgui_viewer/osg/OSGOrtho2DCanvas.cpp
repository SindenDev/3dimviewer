///////////////////////////////////////////////////////////////////////////////
// $Id: OSGOrtho2DCanvas.cpp 2789 2012-12-17 17:39:48Z stancl $
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

#include <osg/OSGOrtho2DCanvas.h>

#include <data/COrthoSlice.h>
#include <coremedi/app/Signals.h>
#include <base/Macros.h>

// Includes for simple geometry
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/Texture2D>

OSGOrtho2DCanvas::OSGOrtho2DCanvas(QWidget* parent) : OSGCanvas(parent)
{
    // create new manipulator
    m_sceneManipulator = new osgGA::OrthoManipulator;

    // this should be rewritten in some way (it depends on model position and size).
#define ZOOM 1
    m_view->getCamera()->setProjectionMatrixAsOrtho(-ZOOM, ZOOM, -ZOOM, ZOOM, -1500.0, 1500.0);
    // set manipulator to camera
    setManipulator(m_sceneManipulator.get());

    // Try to set manipulator matrix
//	osg::Matrix m = m_sceneManipulator->getMatrix();
//	m.setRotate( osg::Quat( osg::DegreesToRadians( 90.0 ), osg::Vec3( 1.0, 0.0, 0.0 ) ) );
//	m_sceneManipulator->setByMatrix( m );

    // compute home position
    //m_sceneManipulator->computeHomePosition();

    // vt: bad view with above manipulator, ask why
    //m_view->setCameraManipulator(new osg::CSceneManipulator);

}

void OSGOrtho2DCanvas::setManipulator(osgGA::OrthoManipulator * manipulator)
{
    if( !manipulator )
    {
        return;
    }

    m_view->setCameraManipulator(manipulator);
    m_sceneManipulator = manipulator;
}

void OSGOrtho2DCanvas::setScene(osg::Node * node, bool bCenterView)
{
    if( !node )
    {
        return;
    }

    m_view->setSceneData(node);
    m_sceneManipulator->setNode(node);

    if( bCenterView )
    {
        centerAndScale();
    }
}

void OSGOrtho2DCanvas::centerAndScale()
{
    m_sceneManipulator->computeHomePosition();
    m_sceneManipulator->home(0.0);
    Refresh(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Center and scale. 
//!
//!\param   box The used bounding box. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void OSGOrtho2DCanvas::centerAndScale( const osg::BoundingBox & box )
{
    // Try to get camera manipulator
    osgGA::OrthoManipulator * sm( dynamic_cast< osgGA::OrthoManipulator * >( m_view->getCameraManipulator() ) );

    if( sm != 0 )
    {
        sm->storeBB( box );
        sm->useStoredBox( true );

        m_sceneManipulator->home(0.0);
        Refresh(false);
    }
}

osg::Vec2 OSGOrtho2DCanvas::getPixelSize()
{
    // compute window to local matrix
    osg::Matrix matrix;
    //m_view->getCamera()->computeWorldToLocalMatrix(matrix, NULL);
    matrix = osg::Matrix::inverse(m_view->getCamera()->getProjectionMatrix());// * matrix;

    QSize client=size();
    // create point in window coordinates, one pixel from origin
    osg::Vec3f vec = osg::Vec3f(1/((float)client.width()), -1/((float)client.height()), 0) * matrix;

    return osg::Vec2(vec[0], vec[1]);
}

