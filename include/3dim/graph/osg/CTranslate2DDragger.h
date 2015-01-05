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

#ifndef CTranslate2DDragger_H
#define CTranslate2DDragger_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osgManipulator/Dragger>
#include <osgManipulator/Projector>

#include <osg/PolygonOffset>
#include <osg/CTwoMaterialsNode.h>


namespace osgManipulator
{

///////////////////////////////////////////////////////////////////////////////
//! Dragger for performing 2D translation.

class  CTranslate2DDragger : public osg::CTwoMaterialsNode< Dragger >
{
public:
	CTranslate2DDragger();

	CTranslate2DDragger(const osg::Plane& plane);

    //! Destructor
    virtual ~CTranslate2DDragger();

    //! Set dragging plane
    void setPlane( const osg::Plane & plane ) { if( !m_bTranslating ) _projector = new PlaneProjector(plane); }

	/** Handle pick events on dragger and generate TranslateInLine commands. */
	virtual bool handle(const PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:
	//! Revert plane rotation given by matrix transformations
	virtual void revertTransformsOnPlane();

	//! Modify command before dispatch
	virtual void updateCommand(MotionCommand * command)
	{
	}

protected:
	osg::ref_ptr< PlaneProjector >  _projector;

	osg::Vec3d _startProjectedPoint;

	osg::ref_ptr<osg::PolygonOffset> _polygonOffset;

	osg::Plane m_initialPlane;

	//! Disable dragger geometry rotation transforms
	bool m_bNoGeometryTransform;

    //! Are we translating now?
    bool m_bTranslating;
};


} // namespace osgManipulator

#endif // CTranslate2DDragger_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
