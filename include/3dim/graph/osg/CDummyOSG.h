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

#ifndef CDummyOSG_H
#define CDummyOSG_H

#include <cassert>

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Depth>
#include <osg/PolygonOffset>
#include <osg/StateSet>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/AutoTransform>

#include <osgManipulator/CommandManager>
#include <osgManipulator/Translate1DDragger>

#include <3dim/graph/osg/CForceCullCallback.h>
#include <osg/CThickLineMaterial.h>

namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! Class able to constract dummy geometries of different type.
//
class CDummyGeometry : public osg::Geometry
{
public:
	//! Creates 1x1 plane
	static void squarePlaneXY( osg::Geometry * geometry, bool thin );

    //! Creates 1x1 plane
	static void squarePlaneXZ(osg::Geometry * geometry, bool thin);

    //! Creates 1x1 plane
	static void squarePlaneYZ(osg::Geometry * geometry, bool thin);

    //! Creates 1x1x1 cube
	static void cube( osg::Geometry * geometry, bool generateNormals = false );

	//! Set dummy geometry color
	void setColor( const osg::Vec4	& color );						
};


////////////////////////////////////////////////////////////
//! a Class

class CDummyDraggableGeode : public osg::Geode, public osg::CInvisibleObjectInterface
{
protected:
	//! String identificator
	unsigned ID;

	//! Dragger pointer
    osg::ref_ptr<osgManipulator::Dragger> p_Dragger;

	//! Pointer to geometry
    osg::ref_ptr<CDummyGeometry> p_Geometry;


public:
	//! Constructor 
	CDummyDraggableGeode();

	//! Destructor
	virtual ~CDummyDraggableGeode();

	//! Creates dummy square planar geometry
    void setUpSquarePlaneXY(bool thin);

	//! Creates dummy square planar geometry
	void setUpSquarePlaneXZ(bool thin);

    //! Creates dummy square planar geometry
	void setUpSquarePlaneYZ(bool thin);

    //! Creates dummy cube geometry
    void setUpCube();

    //! Creates dummy thick frame geometry (for grabbing on plane perpendicular to viewplane)
	void setUpThickFrame();

	//! Returns identification string 
	inline unsigned	getID() { return ID; }

	//! Sets identification string 
	inline void	setID( unsigned id ) { ID = id; }

	//! Sets dummy geometry
	void setGeometry( osg::Geometry * geometry );

	//! Returns pointer to associated dragger
	osgManipulator::Dragger * getDragger();

	//! Sets internal pointer to dragger
	void setDragger( osgManipulator::Dragger * d );

	//! Sets color of dummy geometry ( visible is vith 10% alpha )
	void setColor( float r, float g, float b );

	//! Sets geometry visible or invisible 
	void visible( bool v );
};


////////////////////////////////////////////////////////////
//! a Class

class CDummyCubeGeode : public osg::Geode, public osg::CInvisibleObjectInterface
{
protected:
	//! Pointer to geometry
    osg::ref_ptr<CDummyGeometry> p_Geometry;

public:
	//! Constructor 
	CDummyCubeGeode();

	//! Destructor
	virtual ~CDummyCubeGeode();

    //! Creates dummy cube geometry
    void setUpCube();

	//! Sets dummy geometry
	void setGeometry( osg::Geometry * geometry );

	//! Sets color of dummy geometry ( visible is vith 10% alpha )
	void setColor( float r, float g, float b );

	//! Sets alpha of dummy geometry
	void setAlpha( float a );

	//! Sets geometry visible or invisible 
	void visible( bool v );
};


///////////////////////////////////////////////////////////////////////////////
//! Gizmo is always rendered and has wireframe geometry.

class CGizmo : public osg::AutoTransform
{
protected:
    //! Gizmo geode
    osg::ref_ptr< osg::Geode > p_Geode;

    //! Gizmo geometry
    osg::ref_ptr< osg::Geometry > p_Geometry;

    //! Gizmo size
    double d_Size;

public:
    //! Constructor
    CGizmo( double size, const osg::Vec3 & offset = osg::Vec3( 0.0, 0.0, 0.0 ) );

    //! Creates gizmo geometry
    virtual void createGeometry( const osg::Vec3 & offset );

    //! Creates gizmo bounding dummy
    virtual void createDummy( const osg::Vec3 & offset );

    //! Set gizmo color
    void setColor( const osg::Vec4 & color );

    //! Returns color of a gizmo
    osg::Vec4 getColor();

    //!	Set gizmo size
    virtual void setSize( double size );

    //! Returns size of gizmo
    double getSize();
};


class CTriangleGizmo : public CGizmo
{
protected:
    osg::Vec3 m_Offset;

    osg::Vec3 m_Orientation;

    osg::ref_ptr<osg::CMaterialLineStrip> m_lineMaterial;

public:
	//! Constructor
	CTriangleGizmo( double size, const osg::Vec3 & offset = osg::Vec3( 0.0, 0.0, 0.0 ) );

	//! Create geometry
	virtual void createGeometry( const osg::Vec3 & offset );

	//! Create invisible dummy around
	virtual void createDummy( const osg::Vec3 & offset );

    virtual void setSizeAndOffset( double size, const osg::Vec3 &offset, const osg::Vec3 & orientation);

    virtual void setOrientation(const osg::Vec3 & orientation);

    void setMaterial(osg::CMaterialLineStrip* material);
};


class CCircleGizmo : public CGizmo
{
protected:
    osg::Vec3 m_Offset;

    osg::ref_ptr<osg::CMaterialLineStrip> m_lineMaterial;

public:
    //! COnstructor
    CCircleGizmo( double size, const osg::Vec3 & offset = osg::Vec3( 0.0, 0.0, 0.0 ) );

    //! Creates geometry
    virtual void createGeometry( const osg::Vec3 & offset );

    //! Create invisible dummy around
    virtual void createDummy( const osg::Vec3 & offset );

    virtual void setSize( double size );

    virtual void setSizeAndOffset( double size, const osg::Vec3 &offset);

    void setMaterial(osg::CMaterialLineStrip* material);
};


class COrientableLineGizmo : public CGizmo
{
protected:
	osg::Vec3 m_Offset;

	osg::Vec3 m_Orientation;

	osg::ref_ptr< osg::Geode > p_DummyGeode;

	osg::ref_ptr< osg::Geometry > p_DummyGeometry;

	osg::ref_ptr< osg::MatrixTransform > p_DummyTransform;

    osg::ref_ptr<osg::CMaterialLines> m_lineMaterial;

public:
	COrientableLineGizmo( double size, const osg::Vec3 & offset, const osg::Vec3 & orientation );

	//! Create geometry
	virtual void createGeometry( const osg::Vec3 & offset, const osg::Vec3 & orientation );

	//! Create invisible dummy around
	virtual void createDummy( const osg::Vec3 & offset, const osg::Vec3 & orientation );

	virtual void setSize( double t );

	virtual void setOffset( const osg::Vec3 & offset );

	virtual void setOrientation( const osg::Vec3 & orientation );

    void setMaterial(osg::CMaterialLines* material);
};


class CLineGizmo : public CGizmo
{
protected:
    osg::Vec3 m_Offset;

    osg::Vec3 m_Orientation;

    osg::ref_ptr< osg::Geode > p_DummyGeode;

    osg::ref_ptr< osg::MatrixTransform > p_DummyTransform;

    osg::ref_ptr<osg::CMaterialLines> m_lineMaterial;

public:
    CLineGizmo(double size,
               const osg::Vec3 & offset = osg::Vec3( 0.0, 0.0, 0.0 ),
               const osg::Vec3 & orientation = osg::Vec3( 0.0, 1.0, 0.0 )
               );

    virtual void createGeometry( const osg::Vec3 & offset, const osg::Vec3 & orientation = osg::Vec3( 0.0, 1.0, 0.0 ) );

    virtual void createDummy( const osg::Vec3 & offset, const osg::Vec3 & orientation );

    virtual void setSize( double size );

    virtual void setOffset( const osg::Vec3 & offset );

    void setMaterial(osg::CMaterialLines* material);
};


class CSquareGizmo : public CGizmo
{
protected :
        osg::Vec3 m_Offset;

        osg::ref_ptr<osg::CMaterialLineStrip> m_lineMaterial;

public:
    CSquareGizmo( double size, const osg::Vec3 & offset = osg::Vec3( 0.0, 0.0, 0.0 ), double dummyScale = 1.2 );

    virtual void createGeometry( const osg::Vec3 & offset );

    virtual void createDummy( const osg::Vec3 & offset, double dummyScale );

    virtual void setSize( double size )
    {
        setSize( size, 1.2 );
    }

    virtual void setSize( double size, double dummyScale );

    void setMaterial(osg::CMaterialLineStrip* material);
};


} // namespace scene

#endif // CDummyOSG_H
