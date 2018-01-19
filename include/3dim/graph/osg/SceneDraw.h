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

#ifndef SCENEDRAW_H
#define SCENEDRAW_H

#include <osg/MouseDraw.h>
#include <osg/CSceneOSG.h>
#include <osg/COrthoSceneOSG.h>


namespace osgGA
{

///////////////////////////////////////////////////////////////////////////////
//! CLASS  CSceneWindowDrawEH - scene window drawing handler

class CSceneWindowDrawEH : public CScreenDrawHandler
{
public:
	//! Constructor
    CSceneWindowDrawEH( OSGCanvas * canvas, scene::CSceneBase * scene );

protected:
	//! Compute intersection
	virtual bool GetIntersection( CMousePoint & intersection, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa );

	//! Test if handler should be used
	virtual bool UseHandler() { return APP_MODE.get() == scene::CAppMode::COMMAND_DRAW_WINDOW; }

	//! Delete all lines on mouse release
	virtual void OnMouseRelease( const osg::Vec3 & intersection, bool bUsePoint ) 
	{ 
		ClearLines(); 
	}

    virtual void OnMouseRelease( const CMousePoint & intersection, bool bUsePoint ) 
	{ 
		ClearLines(); 
	}

protected:
	//! Scene pointer
	osg::ref_ptr< scene::CSceneBase > m_scene;

};	// class COrthoWindowDraw


///////////////////////////////////////////////////////////////////////////////
//! CLASS  CSceneWindowDrawEH - scene window drawing handler

class CSceneGeometryDrawEH : public CGeometryDrawHandler
{
public:
	//! Constructor
	CSceneGeometryDrawEH( OSGCanvas * canvas, scene::CSceneBase * scene );

protected:
	//! Compute intersection
	virtual bool GetIntersection(  CMousePoint & intersection, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa );

	//! Test if handler should be used
	virtual bool UseHandler() { return APP_MODE.get() == scene::CAppMode::COMMAND_DRAW_GEOMETRY; }

	//! Delete all lines on mouse release
	virtual void OnMouseRelease( const osg::Vec3 & intersection, bool bUsePoint ) 
	{ 
        if(m_bClearLinesAutomatically)
		    ClearLines(); 
	}

	virtual void OnMouseRelease( const CMousePoint & intersection, bool bUsePoint ) 
	{ 
        if(m_bClearLinesAutomatically)
		    ClearLines(); 
	}

protected:
	//! Scene pointer
	osg::ref_ptr< scene::CSceneBase > m_scene;

    //! Lines should be cleared after mouse release
    bool m_bClearLinesAutomatically;


};	// class COrthoWindowDraw


} // namespace osgGA

#endif // SCENEDRAW_H
