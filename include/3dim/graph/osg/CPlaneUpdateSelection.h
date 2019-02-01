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

#ifndef CPlaneUpdateSelection_H
#define CPlaneUpdateSelection_H

#include <osg/CSignalDraggerCallback.h>
#include <osgManipulator/Command>

#include <VPL/Module/Signal.h>
#include <coremedi/app/Signals.h>
#include <iostream>

#include "CPlaneConstraint.h"

namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! OSG selection invokes a specified signal on any slice position change.

class CPlaneUpdateSelection : public osg::MatrixTransform
{
public:
	class CPlaneUpdateDC : public osgManipulator::DraggerTransformCallback
	{
	public:
		//! Constructor
		CPlaneUpdateDC( CPlaneUpdateSelection * selection )
			: DraggerTransformCallback( selection )
			, m_selection( selection )
		{
		}

		//! Receive translate in line motion command
		virtual bool receive (const osgManipulator::TranslateInLineCommand &command);

		//! Receive any motion command
		virtual bool receive( const osgManipulator::MotionCommand & command );

	protected:
		//! Selection
		osg::ref_ptr< CPlaneUpdateSelection > m_selection;
	};
public:
    //! Signal invoked on plane position change.
    typedef vpl::mod::CSignal<void, int> tSignal;

protected:
    //! Number of voxels in motion direction.
    int	i_VoxelDepth;

    //! Voxel size.
    float f_VoxelSize;

    //! Last relative position.
    float f_LastPosition;

    //! Relative position of the slice in the range <0,1>.
    float f_Position;

    //! Position of the slice.
    int i_Current;

    //! Pointer to constraining object
    osg::ref_ptr<manipul::CSliceConstraint> p_ConstraintLink;

    //! Signal invoked on plane position change.
    tSignal *m_pSignal;

	//! Dragger command
	osg::ref_ptr< CPlaneUpdateDC > m_dc;
	
public:
    //! Constructor
    CPlaneUpdateSelection();

    //! Sets signal invoked on plane position change.
    void setSignal(tSignal *pSignal);

    //! Sets pointer to line constraint
    void setConstraintLink( manipul::CSliceConstraint * constraint );

    //! Sets number of voxels in motion direction
    void setVoxelDepth( int iDepth, float fSize );

    //! Returns number of voxels in motion direction
    int	getVoxelDepth() const { return i_VoxelDepth; }

    //! Returns voxel size in motion direction
    float getVoxelSize() const { return f_VoxelSize; }

    //! Returns the last slice position normalized to the range <0,1>.
    float getPosition() const { return f_Position; }

    //! Returns the last slice position.
    int getIntPosition() const { return i_Current; }

    //! Translates selection subtree by directly changing it's matrix.
    void manualTranslation( const osg::Matrix & m );

    //! When motion is over, store relative last position.
    virtual void fixLastPosition( float fPosition) = 0;

    //! Returns matrix that will move slice to specified position.
    virtual const osg::Matrix& getTranslationToPosition( int iPosition ) = 0;

    //! Moves slice to a new position specified by translation matrix.
    virtual int translateByMatrix( const osg::Matrix & m, bool bModifyMatrix = false ) = 0;

	//! Get dragger command
	CPlaneUpdateDC * getDraggerCommand( ) { return m_dc; }

protected:
    //! Translate in line 
    virtual bool translate( const osgManipulator::TranslateInLineCommand & command );
};


///////////////////////////////////////////////////////////////////////////////
//! OSG selection designed for orthogonal slices stored in the data storage.

class CPlaneXYUpdateSelection : public CPlaneUpdateSelection
{
public:
    //! Constructor
    CPlaneXYUpdateSelection();

    //! Returns matrix that will move slice to specified position.
    virtual const osg::Matrix& getTranslationToPosition( int iPosition ) override;

    //! Moves slice to a new position specified by translation matrix.
    virtual int translateByMatrix( const osg::Matrix & m, bool bModifyMatrix = false ) override;

    //! When motion is over, store relative last position.
    virtual void fixLastPosition(float fPosition) override;
};


///////////////////////////////////////////////////////////////////////////////
//! OSG selection designed for orthogonal slices stored in the data storage.

class CPlaneXZUpdateSelection : public CPlaneUpdateSelection
{
public:
    //! Constructor
    CPlaneXZUpdateSelection();

    //! Returns matrix that will move slice to specified position.
    virtual const osg::Matrix& getTranslationToPosition( int iPosition ) override;

    //! Moves slice to a new position specified by translation matrix.
    virtual int translateByMatrix( const osg::Matrix & m, bool bModifyMatrix = false ) override;

    //! When motion is over, store relative last position.
    virtual void fixLastPosition(float fPosition) override;
};


///////////////////////////////////////////////////////////////////////////////
//! OSG selection designed for orthogonal slices stored in the data storage.

class CPlaneYZUpdateSelection : public CPlaneUpdateSelection
{
public:
    //! Constructor
    CPlaneYZUpdateSelection();

    //! Returns matrix that will move slice to specified position.
    virtual const osg::Matrix& getTranslationToPosition( int iPosition ) override;

    //! Moves slice to a new position specified by translation matrix.
    virtual int translateByMatrix( const osg::Matrix & m, bool bModifyMatrix = false ) override;

    //! When motion is over, store relative last position.
    virtual void fixLastPosition(float fPosition) override;
};


///////////////////////////////////////////////////////////////////////////////
//! OSG selection designed for orthogonal slices stored in the data storage.

class CPlaneARBUpdateSelection : public CPlaneUpdateSelection
{
public:
    //! Constructor
    CPlaneARBUpdateSelection();

    //! Returns matrix that will move slice to specified position.
    virtual const osg::Matrix& getTranslationToPosition(int iPosition) override;

    //! Moves slice to a new position specified by translation matrix.
    virtual int translateByMatrix(const osg::Matrix & m, bool bModifyMatrix = false) override;

    //! When motion is over, store relative last position.
    virtual void fixLastPosition(float fPosition) override;

    void setPlaneNormal(const osg::Vec3& normal);

protected:

    osg::Vec3 m_planeNormal;
};


} // namespace scene

#endif // CPlaneUpdateSelection_H
