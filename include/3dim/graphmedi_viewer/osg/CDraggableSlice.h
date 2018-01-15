///////////////////////////////////////////////////////////////////////////////
// $Id: CDraggableSlice.h 1933 2012-05-25 09:16:49Z tryhuk $
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

#ifndef CDraggableSlice_H
#define CDraggableSlice_H

#include <osg/CSliceDragger.h>
#include <osg/CPlaneConstraint.h>
#include <osg/CPlaneUpdateSelection.h>
#include <osg/CDraggerEventHandler.h>	// nothin' more needed
#include <osg/CGeneralObjectObserverOSG.h>

#include <data/COrthoSlice.h>

#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osg/CoordinateSystemNode>
#include <osgText/Text>
//#include <osg/CTools.h>

class OSGCanvas;

namespace scene
{

////////////////////////////////////////////////////////////
//! Simple colour frame around a slice.
//
class CFrameGeode : public osg::Geode
{
protected:
	osg::ref_ptr< osg::Geometry > m_FrameGeometry;

public:
	//! Set color method
	void setColor( float r, float g, float b );

protected:
    //! Constructor
    CFrameGeode();
};


class CFrameXYGeode : public CFrameGeode
{
public:
	//! Constructor
	CFrameXYGeode();
};


class CFrameXZGeode : public CFrameGeode
{
public:
	//! Constructor
	CFrameXZGeode();
};


class CFrameYZGeode : public CFrameGeode
{
public:
	//! Constructor
	CFrameYZGeode();
};


////////////////////////////////////////////////////////////
//! Textured slice.
//
class CSliceGeode : public osg::Geode
{
protected:
	//! slice rectangular geometry
	osg::ref_ptr< osg::Geometry > m_SliceGeometry;
	osg::ref_ptr< osg::StateSet > p_StateSet;
	
public:
	//! Set texture and texture coordinates
	void setTextureAndCoordinates( osg::Texture2D * texture, float x_min, float x_max, float y_min, float y_max );

protected:
	//! L'Constructeur :)
	CSliceGeode();

    void setupScene();
};


class CSliceXYGeode : public CSliceGeode
{
public:
	//! L'Constructeur :)
	CSliceXYGeode();
};


class CSliceXZGeode : public CSliceGeode
{
public:
	//! L'Constructeur :)
	CSliceXZGeode();
};


class CSliceYZGeode : public CSliceGeode
{
public:
	//! L'Constructeur :)
	CSliceYZGeode();
};


///////////////////////////////////////////////////////////////////////////////
//! Draggable textured slice.

class CDraggableSlice : public osg::MatrixTransform
{
public:
    //! Signal invoked on slice position change.
    typedef CPlaneUpdateSelection::tSignal tSignal;

protected:
    //! Slice geode pointer
    osg::ref_ptr< CSliceGeode >	p_Slice;

    //! Main dragger pointer
    osg::ref_ptr< osgManipulator::Dragger >	p_Dragger;	
//    osg::ref_ptr< osgManipulator::CSliceDragger > p_Dragger;

    //! Dummy geometry
    osg::ref_ptr< CDummyDraggableGeode > p_Dummy;

    //! Frame
    osg::ref_ptr< CFrameGeode > p_Frame;

    //! Plane identifier
    data::COrthoSlice::EPlane m_Plane;

    //! Line constraint
    osg::ref_ptr< manipul::CSliceConstraint > p_Constraint;

    //! Selection that signals
    osg::ref_ptr< scene::CPlaneUpdateSelection > p_Selection;

    // ???
    osg::ref_ptr< osg::Group >	p_Anchor;

    //! Flag if the slice is used in an orthogonal view.
    bool m_Ortho;

    //! Dragger handle
    osg::ref_ptr< osg::Node > p_DraggerHandle;

public:
	//! Makes dummy geometry thin or thick
	virtual void dummyThin(bool thin) = 0;

    //! Makes dummy geometry visible 
    void dummyVisible() { p_Dummy->visible( true ); }

    //! Makes dummy geometry invisible
    void dummyInvisible() { p_Dummy->visible( false ); }

    //! Set texture
    void setTextureAndCoordinates( osg::Texture2D * texture, float x_min, float x_max, float y_min, float y_max );

    //! Sets scene proportions
    virtual void scaleScene( float dx, float dy, float dz, int sx, int sy, int sz );

    //! Returns number of voxels in motion direction
    int	getVoxelDepth();

    //! Returns size of voxel in motion direction
    float getVoxelSize();

    //! Changes the signal invoked on slice position change.
    void setSignal(tSignal *pSignal);

    //! Moves slice in depth to specified position
    void moveInDepth( int position );

    //! Sets the frame color.
    void setFrameColor( float r, float g, float b ) { p_Frame->setColor(r, g, b); }

    //! Attach subtree to draggable slice
    void anchorToSlice( osg::Node * node );

    //! Get slice geometry node
    osg::Geode * getSliceGeode()
    {
	    return p_Slice.get();
    }

    //! Get slice dragger
    osgManipulator::Dragger * getSliceDragger()
    {
        return p_Dragger.get();
    }

    //! Get absolute position
    osg::Vec3 getWorldPosition()
    {
        osg::MatrixList l = p_Selection->getWorldMatrices();
        osg::Vec3 position(0.0, 0.0, 0.0);

        if( !l.empty() )
        {
            osg::Matrix m(*l.begin());
            position = m.getTrans() * osg::Matrix::rotate(m.getRotate());
            return position;
        }
        else
        {
            return osg::Vec3(0.0, 0.0, 0.0);
        }
    }

    //! Accept node visitors - used to compute current view matrix
    virtual void accept(osg::NodeVisitor& nv);

protected:
    //! Constructor
    CDraggableSlice(bool isOrtho = false);

    //! Destructor
    virtual ~CDraggableSlice();

    //! Initializes the scene.
    void setupScene();

    //! Helper method that creates dragger handle geometry
    osg::Node* createDraggerHandle(osg::Vec3 center, osg::Vec3 color) const;

    //! Current view matrix
    osg::Matrix m_viewMatrix;

    //! Scale factor
    float m_scaleFactor;
};


///////////////////////////////////////////////////////////////////////////////
//! Draggable textured slice designed for orthogonal slices stored
//! in the storage.

class CDraggableSliceXY : public CDraggableSlice, public scene::CGeneralObjectObserverOSG<CDraggableSliceXY>
{
public:
	//! Constructor.
    CDraggableSliceXY( OSGCanvas * pCanvas, bool isOrtho = false, int SliceId = data::Storage::UNKNOWN, bool draggerHandle = false );

    virtual ~CDraggableSliceXY();

    //! Sets scene proportions
	virtual void scaleScene( float dx, float dy, float dz, int sx, int sy, int sz );

    //! Method called on OSG update callback.
    virtual void updateFromStorage();

    //! Virtual method called on any change of the entry
    virtual void objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);
	
	//! Makes dummy geometry thin or thick
	virtual void dummyThin(bool thin) { p_Dummy->setUpSquarePlaneXY(thin); }

protected:
    //! Identifier of the storage entry.
    int m_SliceId;
};


///////////////////////////////////////////////////////////////////////////////
//! Draggable textured slice designed for orthogonal slices stored
//! in the storage.

class CDraggableSliceXZ : public CDraggableSlice, public scene::CGeneralObjectObserverOSG<CDraggableSliceXZ>
{
public:
	//! Constructor.
    CDraggableSliceXZ( OSGCanvas * pCanvas, bool isOrtho = false, int SliceId = data::Storage::UNKNOWN, bool draggerHandle = false );

    virtual ~CDraggableSliceXZ();

    //! Sets scene proportions
	virtual void scaleScene( float dx, float dy, float dz, int sx, int sy, int sz );

    //! Method called on OSG update callback.
    virtual void updateFromStorage();

    //! Virtual method called on any change of the entry
    virtual void objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);

	//! Makes dummy geometry thin or thick
	virtual void dummyThin(bool thin) { p_Dummy->setUpSquarePlaneXZ(thin); }

protected:
    //! Identifier of the storage entry.
    int m_SliceId;
};


///////////////////////////////////////////////////////////////////////////////
//! Draggable textured slice designed for orthogonal slices stored
//! in the storage.

class CDraggableSliceYZ : public CDraggableSlice, public scene::CGeneralObjectObserverOSG<CDraggableSliceYZ>
{
public:
	//! Constructor.
    CDraggableSliceYZ( OSGCanvas * pCanvas, bool isOrtho = false, int SliceId = data::Storage::UNKNOWN, bool draggerHandle = false );

    virtual ~CDraggableSliceYZ();

    //! Sets scene proportions
	virtual void scaleScene( float dx, float dy, float dz, int sx, int sy, int sz );

    //! Method called on OSG update callback.
    virtual void updateFromStorage();

    //! Virtual method called on any change of the entry
    virtual void objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes);

	//! Makes dummy geometry thin or thick
	virtual void dummyThin(bool thin) { p_Dummy->setUpSquarePlaneYZ(thin); }
protected:
    //! Identifier of the storage entry.
    int m_SliceId;
};


} // namespace scene

#endif // CDraggableSlice_H
