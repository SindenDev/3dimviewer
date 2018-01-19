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

#include <base/Defs.h>
#include <osg/CPlaneUpdateSelection.h>
#include <osg/Version>
#include <VPL/Base/Logging.h>


namespace scene
{

static const int DEFAULT_VOXEL_DEPTH = 64;

//====================================================================================================================
CPlaneUpdateSelection::CPlaneUpdateSelection() : 
	i_VoxelDepth( DEFAULT_VOXEL_DEPTH ),
    f_VoxelSize(1.0),
	f_LastPosition( 0.0 ),
	f_Position( 0.0 ),
    i_Current(0),
    m_pSignal(NULL)
{
	m_dc = new CPlaneUpdateDC( this );
}

//====================================================================================================================
void CPlaneUpdateSelection::setVoxelDepth( int iDepth, float fSize )
{
    i_VoxelDepth = iDepth;
    f_VoxelSize = fSize;
}

//====================================================================================================================
void CPlaneUpdateSelection::setConstraintLink( manipul::CSliceConstraint * constraint )
{
	p_ConstraintLink = constraint;
}

//====================================================================================================================
void CPlaneUpdateSelection::setSignal(tSignal *pSignal)
{
    m_pSignal = pSignal;
}


//====================================================================================================================
void CPlaneUpdateSelection::fixLastPosition( float fPosition )
{
    f_LastPosition = fPosition;
    if( p_ConstraintLink.get() )
    {
        p_ConstraintLink->fixPosition( fPosition );
    }
}

//====================================================================================================================
void CPlaneUpdateSelection::manualTranslation( const osg::Matrix & m )
{
    i_Current = translateByMatrix(m, true);

    fixLastPosition( f_Position );
}

//====================================================================================================================
bool CPlaneUpdateSelection::translate( const osgManipulator::TranslateInLineCommand & command)
{
    switch( command.getStage() )
    {
        case osgManipulator::MotionCommand::MOVE:
        {
	        osg::Matrix	m;
    	    m.makeTranslate( command.getTranslation() );
		    int	iPosition = translateByMatrix( m );
            if( iPosition != i_Current )
            {
                i_Current = iPosition;
                if( m_pSignal )
                {
		            m_pSignal->invoke(i_Current);
                }
            }
        } break;

        case osgManipulator::MotionCommand::START:
        case osgManipulator::MotionCommand::FINISH:
		    fixLastPosition( f_Position );
            break;

        default:
            break;
    }

	return true;
}


//====================================================================================================================
//====================================================================================================================
CPlaneXYUpdateSelection::CPlaneXYUpdateSelection()
{
//    setSignal(&(VPL_SIGNAL(SigSetSliceXY)));
}

//====================================================================================================================
const osg::Matrix& CPlaneXYUpdateSelection::getTranslationToPosition( int iPosition )
{
    vpl::math::limit(iPosition, 0, i_VoxelDepth - 1);

    if( i_VoxelDepth <= 1 )
    {
        f_Position = 0.0f - f_LastPosition;
    }
    else
    {
        f_Position = 1/float(2*i_VoxelDepth) +                // offset to middle of voxel
                     iPosition / float(i_VoxelDepth) -      // position as float (1/i_VoxelDepth is taken by half voxels)
                     f_LastPosition;                        // remove effect of last position
    }

	static osg::Matrix m;
	m.makeTranslate(osg::Vec3(0.0, 0.0, f_Position));
    return m;
}

//====================================================================================================================
int	CPlaneXYUpdateSelection::translateByMatrix( const osg::Matrix & m, bool bModifyMatrix )
{
	osg::Vec3 t = m.getTrans();
    
    f_Position = f_LastPosition + t[2];
    vpl::math::limit(f_Position, 0.0f, 1.0f);
    t[2] = f_Position;
    
    if( bModifyMatrix )
    {
        osg::Matrix m2;
        m2.makeTranslate(t);
        this->setMatrix( m2 );
    }
    
    return int(f_Position * float(i_VoxelDepth));
}


//====================================================================================================================
//====================================================================================================================
CPlaneXZUpdateSelection::CPlaneXZUpdateSelection()
{
//    setSignal(&(VPL_SIGNAL(SigSetSliceXZ)));
}

//====================================================================================================================
const osg::Matrix& CPlaneXZUpdateSelection::getTranslationToPosition( int iPosition )
{
    vpl::math::limit(iPosition, 0, i_VoxelDepth - 1);

    if( i_VoxelDepth <= 1 )
    {
        f_Position = 0.0f - f_LastPosition;
    }
    else
    {
        f_Position = 1/float(2*i_VoxelDepth) +              // offset to middle of voxel
                     iPosition / float(i_VoxelDepth) -      // position as float
                     f_LastPosition;                        // remove effect of last position
    }

	static osg::Matrix m;
	m.makeTranslate(osg::Vec3(0.0, f_Position, 0.0f));
    return m;
}

//====================================================================================================================
int	CPlaneXZUpdateSelection::translateByMatrix( const osg::Matrix & m, bool bModifyMatrix  )
{
	osg::Vec3 t = m.getTrans();

    f_Position = f_LastPosition + t[1];
    vpl::math::limit(f_Position, 0.0f, 1.0f);
    t[1] = f_Position;
    
    if( bModifyMatrix )
    {
        osg::Matrix m2;
        m2.makeTranslate(t);
        this->setMatrix( m2 );
    }

    return int(f_Position * float(i_VoxelDepth));
}

//====================================================================================================================
//====================================================================================================================
CPlaneYZUpdateSelection::CPlaneYZUpdateSelection()
{
//    setSignal(&(VPL_SIGNAL(SigSetSliceYZ)));
}

//====================================================================================================================
const osg::Matrix& CPlaneYZUpdateSelection::getTranslationToPosition( int iPosition )
{
    vpl::math::limit(iPosition, 0, i_VoxelDepth - 1);

    if( i_VoxelDepth <= 1 )
    {
        f_Position = 0.0f - f_LastPosition;
    }
    else
    {
        f_Position = 1/float(2*i_VoxelDepth) +                // offset to middle of voxel
                     iPosition / float(i_VoxelDepth) -      // position as float
                     f_LastPosition;                        // remove effect of last position
    }

	static osg::Matrix m;
	m.makeTranslate(osg::Vec3(f_Position, 0.0f, 0.0f));
    return m;
}

//====================================================================================================================
int	CPlaneYZUpdateSelection::translateByMatrix( const osg::Matrix & m, bool bModifyMatrix  )
{
	osg::Vec3 t = m.getTrans();
    
    f_Position = f_LastPosition + t[0];
    vpl::math::limit(f_Position, 0.0f, 1.0f);
    t[0] = f_Position;
    
    if( bModifyMatrix )
    {
        osg::Matrix m2;
        m2.makeTranslate(t);
        this->setMatrix( m2 );
    }
    
    return int(f_Position * float(i_VoxelDepth));
}



bool CPlaneUpdateSelection::CPlaneUpdateDC::receive( const osgManipulator::TranslateInLineCommand &command )
{
	osgManipulator::DraggerTransformCallback::receive( command );
	m_selection->translate( command );
	return true;
}

bool CPlaneUpdateSelection::CPlaneUpdateDC::receive( const osgManipulator::MotionCommand & command )
{
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,0)
    return osgManipulator::DraggerTransformCallback::receive( command );
#else
	try
	{
		const osgManipulator::TranslateInLineCommand & c( dynamic_cast< const osgManipulator::TranslateInLineCommand &>(command ));
		return receive( c );
	}
	catch (std::bad_cast &)
	{
		return false;
	}
    return false;
#endif
}

} // namespace scene
