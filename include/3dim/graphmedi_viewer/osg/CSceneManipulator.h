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

#ifndef CSceneManipulator_H
#define CSceneManipulator_H

#include <osgGA/TrackballManipulator>

#include <iostream>


namespace osg
{

///////////////////////////////////////////////////////////////////////////////
//! A modified osgGA::TrackballManipulator

class CSceneManipulator : public osgGA::TrackballManipulator
{
public:
    CSceneManipulator() : osgGA::TrackballManipulator(), m_bUseStoredBB( false ) {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

    //! Use stored bounding box when computing home position
    void useStoredBox( bool bUse ) { m_bUseStoredBB = bUse; }

    //! Store bounding box 
    void storeBB( const osg::BoundingBox & bb ) { m_box = bb; }

protected:
    void flipMouseButtons()
    {
	    std::cerr << " flipping mouse buttons " << std::endl;
	    unsigned int buttonMask;
	    buttonMask = _ga_t1->getButtonMask();
	    if ( buttonMask == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON )		
		    const_cast< osgGA::GUIEventAdapter* >(_ga_t1.get())->setButton( osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON );
	    else if ( buttonMask == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON )	
		    const_cast< osgGA::GUIEventAdapter* >(_ga_t1.get())->setButton( osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON );
    }

    //! calculate movement
    bool calcMovement();

    //! Move the camera to the default position
    virtual void computeHomePosition(const osg::Camera *camera, bool useBoundingBox);


    //! Use stored bounding box when computing home position
    bool m_bUseStoredBB;

    //! Stored bounding box
    osg::BoundingBox m_box;
};


} // namespace osg

#endif // CSceneManipulator_H
