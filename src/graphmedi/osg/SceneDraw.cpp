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

#include <osg/SceneDraw.h>
#include <app/Signals.h>

using namespace osgGA;

/******************************************************************************
	CLASS CSceneWindowDrawEH
******************************************************************************/

//=============================================================================
// Constructor
CSceneWindowDrawEH::CSceneWindowDrawEH( OSGCanvas * canvas, scene::CSceneBase * scene ) 
    : CScreenDrawHandler( canvas )
	, m_scene( scene )
{
	osg::ref_ptr< osg::CDrawingsGroup > group = new osg::CDrawingsGroup;
	scene->addChild( group.get() );
	SetLinesNode( group.get() );
}

//=============================================================================
// Compute intersection - adds unOrthoMatrix to the computation
bool CSceneWindowDrawEH::GetIntersection( CMousePoint & intersection, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa )
{
	if( m_scene == NULL )
		return false;

	if( CScreenDrawHandler::GetIntersection( intersection, ea, aa ) )
	{
		// Get un ortho matrix
		osg::Matrix unOrthoMatrix(osg::Matrix::inverse( m_scene->getOrthoTransformMatrix() ) );

		intersection.m_point = intersection.m_point * unOrthoMatrix;

		return true;
	}

	return false;
}

/******************************************************************************
	CLASS CSceneGeometryDrawEH
******************************************************************************/

//=============================================================================
// Constructor
CSceneGeometryDrawEH::CSceneGeometryDrawEH( OSGCanvas * canvas, scene::CSceneBase * scene ) 
    : CGeometryDrawHandler( canvas )
	, m_scene( scene )
    , m_bClearLinesAutomatically(true)
{
	osg::ref_ptr< osg::CDrawingsGroup > group = new osg::CDrawingsGroup;
	scene->addChild( group.get() );
	SetLinesNode( group.get() );
}

//=============================================================================
// Compute intersection - adds unOrthoMatrix to the computation
bool CSceneGeometryDrawEH::GetIntersection( CMousePoint & intersection, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa )
{
	if( m_scene == NULL )
		return false;

	if( CGeometryDrawHandler::GetIntersection( intersection, ea, aa ) )
	{
		// perform additional coordinates check
		const int width = aa.asView()->getCamera()->getViewport()->width();
		const int height = aa.asView()->getCamera()->getViewport()->height();

		if (intersection.m_pointWindow.x()<0 || intersection.m_pointWindow.x()>=width || 
			intersection.m_pointWindow.y()<0 || intersection.m_pointWindow.y()>=height)
			return false;

		// Get un ortho matrix
		osg::Matrix unOrthoMatrix(osg::Matrix::inverse( m_scene->getOrthoTransformMatrix() ) );

		intersection.m_point = intersection.m_point * unOrthoMatrix;

		data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();
		osg::Vec3 p;
		p.x() = osg::Vec3::value_type( CoordConv.fromSceneX( intersection.m_point.x() ) );
		p.y() = osg::Vec3::value_type( CoordConv.fromSceneY( intersection.m_point.y() ) );
		p.z() = osg::Vec3::value_type( CoordConv.fromSceneZ( intersection.m_point.z() ) );

		intersection.m_pointVolume = p;

		return true;
	}

	return false;
}
