///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2008-2014 3Dim Laboratory s.r.o. 
// All rights reserved 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CISVolumeStepper_H_included
#define CISVolumeStepper_H_included

#include <VPL/Image/DensityVolume.h>
#include <osg/Vec3>

namespace seg{

//#define SIZETEST 1

#ifdef SIZETEST

#define STEST { for(int t = 0; t < 3; ++t){assert( m_position[m_assignment[t]] >= 0 && m_position[m_assignment[t]] < m_size[m_assignment[t]]);}};

#else

#define STEST

#endif


//! Operational mode
enum ESteppingMode
{
    VOLUME,
    PLANE_XY,
    PLANE_XZ,
    PLANE_YZ,
    PLANE_FREE
};

template
<
    class tpVolume
>
class CVolumeStepper
{
public:
    //! Volume type
    typedef tpVolume tVolume;

    //! Voxel type
    typedef typename tpVolume::tVoxel tVoxel;

public:
    //! Constructor
    CVolumeStepper();

    //! Initialize stepper
    void init( tVolume * volume, ESteppingMode mode = VOLUME );

    //! Set pointer position
    void setPosition( vpl::tSize x, vpl::tSize y, vpl::tSize z )
    {
        m_position[ m_assignment[ 0 ] ] = x;
        m_position[ m_assignment[ 1 ] ] = y;
        m_position[ m_assignment[ 2 ] ] = z;
        m_pos.set(x,y,z);
     
        STEST;
    }

    //! Set pointer position
    void setPositionF( double x, double y, double z )
    {
#ifdef _DEBUG
        assert(PLANE_FREE==m_mode);
#endif        
        m_pos.set(x,y,z);
        updatePositionFromFloat();        
     
        STEST;
    }

    //! Set pointer position - 2D version
    void setPosition( vpl::tSize x, vpl::tSize y )
    {
        if (PLANE_FREE==m_mode)
        {
            m_pos = osg::Vec3(x,y,0) * m_mx2Dto3D;
            updatePositionFromFloat();
        }
        else
        {
            m_position[ m_assignment[ 0 ] ] = x;
            m_position[ m_assignment[ 1 ] ] = y;
        }        
        STEST;
    }

    //! Seek next voxel in given direction
    void getNextVoxelInDirection(int &x, int &y)
    {
        if (PLANE_FREE!=m_mode) 
            return; // no check needed
        osg::Vec3 p(m_pos);
        int f = 0;
        vpl::tSize posi[3];
        do 
        {
            f++;
            p = (p * m_mx3Dto2D + osg::Vec3(x,y,0)) * m_mx2Dto3D;            
            posi[0] = (int)(p[0]);
            posi[1] = (int)(p[1]);
            posi[2] = (int)(p[2]);            
        } while (posi[0]==m_position[0] && posi[1]==m_position[1] && posi[2]==m_position[2]);
        x = f*x;
        y = f*y;
    }

    //! Get pointer position
    void getPosition( vpl::tSize & x, vpl::tSize & y, vpl::tSize & z )
    {
        x = m_position[ m_assignment[ 0 ] ];
        y = m_position[ m_assignment[ 1 ] ];
        z = m_position[ m_assignment[ 2 ] ];

        STEST;
    }

    //! Get value
    inline tVoxel & get()
    {
        STEST;
        return m_volume->at( m_position[0], m_position[1], m_position[2]);
    }

    //! Set travelling mode
    void setMode( ESteppingMode mode );

    //! Get mode
    ESteppingMode getMode() { return m_mode; }

    //! Set plane vectors for PLANE_FREE mode
    void setPlaneMatrix(const osg::Vec3 &zeroPos, const osg::Matrix &mx2Dto3D)
    {
#ifdef _DEBUG
        assert(PLANE_FREE==m_mode);
#endif
        m_zeroPos = zeroPos;
        m_mx2Dto3D = mx2Dto3D;
        m_mx3Dto2D = osg::Matrix::inverse(m_mx2Dto3D);
    }

    //! Set position and get reference
    tVoxel & setAndGet( vpl::tSize x, vpl::tSize y, vpl::tSize z )
    {
        setPosition( x, y, z );
        STEST;
        return m_volume->at( x, y, z);
    }

    //! Update int position 
    inline void updatePositionFromFloat()
    {
        m_position[0] = (int)(m_pos[0]);
        m_position[1] = (int)(m_pos[1]);
        m_position[2] = (int)(m_pos[2]);
    }

    //! Increment position - x axis
    void incX()
    {
        if (PLANE_FREE==m_mode)
        {
            m_pos = (m_pos * m_mx3Dto2D + osg::Vec3(1,0,0)) * m_mx2Dto3D;
            updatePositionFromFloat();
        }
        else
        {
            ++m_position[ m_assignment[ 0 ] ];
        }
        STEST;
    }

    //! Increment position - y axis
    void incY()
    {
        if (PLANE_FREE==m_mode)
        {
            m_pos = (m_pos * m_mx3Dto2D + osg::Vec3(0,1,0)) * m_mx2Dto3D;
            updatePositionFromFloat();
        }
        else
        {
            ++m_position[ m_assignment[ 1 ] ];
        }
        STEST;
    }

    //! Increment position - z axis
    void incZ()
    {
        if (PLANE_FREE==m_mode)
        {
            m_pos = (m_pos * m_mx3Dto2D + osg::Vec3(0,0,1)) * m_mx2Dto3D;
            updatePositionFromFloat();
        }
        else
        {
            ++m_position[ m_assignment[ 2 ] ];            
        }
        STEST;
    }

    //! Decrement position - x axis
    void decX()
    {
        if (PLANE_FREE==m_mode)
        {
            m_pos = (m_pos * m_mx3Dto2D + osg::Vec3(-1,0,0)) * m_mx2Dto3D;
            updatePositionFromFloat();
        }
        else
        {
            --m_position[ m_assignment[ 0 ] ];            
        }
        STEST;
    }

    //! Decrement position - y axis
    void decY()
    {
        if (PLANE_FREE==m_mode)
        {
            m_pos = (m_pos * m_mx3Dto2D + osg::Vec3(0,-1,0)) * m_mx2Dto3D;
            updatePositionFromFloat();
        }
        else
        {
            --m_position[ m_assignment[ 1 ] ];
        }
        STEST;
    }
    
    //! Decrement position - z axis
    void decZ()
    {
        if (PLANE_FREE==m_mode)
        {
            m_pos = (m_pos * m_mx3Dto2D + osg::Vec3(0,0,-1)) * m_mx2Dto3D;
            updatePositionFromFloat();
        }
        else
        {
            --m_position[ m_assignment[ 2 ] ];
        }
        STEST;
    }

    //! Is this coordinate inside the volume?
    bool isIn( )
    {
        for( int i = 0; i < 3; ++i )
        {
            if( m_position[ i ] < 0 || m_position[ i ] >= m_size[ i ] )
                return false;
        }

        return true;
    }

    
    //! Get voxel on position x, y, z
    tVoxel & getVoxel( vpl::tSize x, vpl::tSize y, vpl::tSize z ) { return m_volume->at( x, y , z ); }

    //! Set voxel on position
    void set( const tVoxel & v ){ (*m_volume)( m_position[0], m_position[1], m_position[2]) = v; }
  
protected:
    //! Volume pointer
    tVolume * m_volume;


    //! Position
    vpl::tSize m_position[3];

    //! Volume sizes
    vpl::tSize m_size[ 3 ];

    //! Current mode
    ESteppingMode m_mode;

    //! Axis assignment
    unsigned char m_assignment[3];

    //! Plane vectors for PLANE_FREE mode
    osg::Vec3 m_pos, m_zeroPos;
    osg::Matrix m_mx2Dto3D;
    osg::Matrix m_mx3Dto2D;
}; // class CVolumeStepper

template
<
    typename tpVolume1,
    typename tpVolume2
>
class CJoinedStepper
{
public:
    //! First stepper type
    typedef CVolumeStepper< tpVolume1 > tStepper1;

    //! Second stepper type
    typedef CVolumeStepper< tpVolume2 > tStepper2;

    //! Volume type
    typedef tpVolume1 tVolume1;

    //! Voxel type
    typedef typename tpVolume1::tVoxel tVoxel1;

    //! Voxel type
    typedef typename tpVolume1::tVoxel tVoxel;

    //! Volume type
    typedef tpVolume2 tVolume2;

    //! Voxel type
    typedef typename tpVolume2::tVoxel tVoxel2;

public:
    //! Initialize stepper
    void init( tVolume1 * volume1, tVolume2 * volume2,  ESteppingMode mode = VOLUME )
    {
        // Test volumes
        assert( volume1 != NULL && volume2 != NULL && testVolumeSizes( volume1, volume2 ) );

        m_volume1.init( volume1, mode );
        m_volume2.init( volume2, mode );
    }

    tVoxel1 get(){ return m_volume1.get(); }
    void set( const tVoxel1 & v){ m_volume1.set(v); }

    //! Set plane vectors for PLANE_FREE mode
    void setPlaneMatrix(const osg::Vec3 &zeroPos, const osg::Matrix &mx2Dto3D)
    {
        m_volume1.setPlaneMatrix(zeroPos,mx2Dto3D);
        m_volume2.setPlaneMatrix(zeroPos,mx2Dto3D);
    }

    //! Set pointer position
    void setPosition( vpl::tSize x, vpl::tSize y, vpl::tSize z )
    {
        m_volume1.setPosition( x, y, z );
        m_volume2.setPosition( x, y, z );
    }

    //! Set pointer position
    void setPositionF( double x, double y, double z )
    {
        m_volume1.setPositionF( x, y, z );
        m_volume2.setPositionF( x, y, z );
    }

    //! Set pointer position - 2D version
    void setPosition( vpl::tSize x, vpl::tSize y )
    {
        m_volume1.setPosition( x, y );
        m_volume2.setPosition( x, y );
    }

    //! Get pointer position
    void getPosition( vpl::tSize & x, vpl::tSize & y, vpl::tSize & z )
    {
        m_volume1.getPosition( x, y, z );
    }

    //! Get next voxel in given direction
    void getNextVoxelInDirection(int &x, int &y)
    {
        m_volume1.getNextVoxelInDirection(x,y);
    }

    //! Get first stepper
    tStepper1 * getVolume1() { return & m_volume1; }

    //! Get second stepper
    tStepper2 * getVolume2() { return & m_volume2; }

    //! Set traveling mode
    void setMode( ESteppingMode mode )
    {
        m_volume1.setMode( mode );
        m_volume2.setMode( mode );
    }

    //! Get mode
    ESteppingMode getMode() { return m_volume1.getMode(); }

    //! Set position and get reference
    tVoxel1 & setAndGet1( vpl::tSize x, vpl::tSize y, vpl::tSize z )
    {
        setPosition( x, y, z );
        return m_volume1.get();
    }

    //! Set position and get reference
    tVoxel2 & setAndGet2( vpl::tSize x, vpl::tSize y, vpl::tSize z )
    {
        setPosition( x, y, z );
        return m_volume2.get();
    }

    //! Increment position - x axis
    void incX()
    {
        m_volume1.incX();
        m_volume2.incX();
    }

    //! Increment position - y axis
    void incY()
    {
        m_volume1.incY();
        m_volume2.incY();
    }

    //! Increment position - z axis
    void incZ()
    {
        m_volume1.incZ();
        m_volume2.incZ();
    }

    //! Decrement position - x axis
    void decX()
    {
        m_volume1.decX();
        m_volume2.decX();
    }

    //! Decrement position - y axis
    void decY()
    {
        m_volume1.decY();
        m_volume2.decY();
    }
    
    //! Decrement position - z axis
    void decZ()
    {
        m_volume1.decZ();
        m_volume2.decZ();
    }

    //! Is this coordinate inside the volume?
    bool isIn( )
    {
        return m_volume1.isIn();
    }

protected:
    //! Test if both volumes has the same size
    bool testVolumeSizes( tVolume1 * v1, tVolume2 * v2 )
    {
        return v1->getXSize() == v2->getXSize() && v1->getYSize() == v2->getYSize() && v1->getZSize() == v2->getZSize();
    }

   

protected:
    //! Stepper 1
    tStepper1 m_volume1;

    //! Stepper 2
    tStepper2 m_volume2;



}; // class CJoinedStepper

} // namespace seg

///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Constructor. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template < class tpVolume >
seg::CVolumeStepper< tpVolume >::CVolumeStepper(void)
: m_volume( NULL )
, m_mode( VOLUME )
{
    for( int i = 0; i < 3; ++i )
    {
        m_assignment[ i ] = i;
        m_position[ i ] = 0;
        m_size[ i ] = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Initialize stepper. 
//!
//!\param [in,out]  volume  If non-null, the volume. 
//!\param   mode            The mode. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template < class tpVolume >
void seg::CVolumeStepper< tpVolume >::init(tVolume *volume, ESteppingMode mode)
{
    assert( volume != NULL );

    m_volume = volume;
     
    // Get size
    m_size[ 0 ] = volume->getXSize();
    m_size[ 1 ] = volume->getYSize();
    m_size[ 2 ] = volume->getZSize();

    setMode( mode );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Set travelling mode. 
//!
//!\param   mode    The mode. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template < class tpVolume >
void seg::CVolumeStepper< tpVolume >::setMode(ESteppingMode mode)
{
    m_mode = mode;

    switch( mode )
    {
    case VOLUME:
    case PLANE_FREE:
    case PLANE_XY:
        for( int i = 0; i < 3; ++i )
            m_assignment[ i ] = i;
        break;

    case PLANE_XZ:
        m_assignment[ 0 ] = 0;
        m_assignment[ 1 ] = 2;
        m_assignment[ 2 ] = 1;
        break;

    case PLANE_YZ:
        m_assignment[ 0 ] = 1;
        m_assignment[ 1 ] = 2;
        m_assignment[ 2 ] = 0;
        break;

    default:
        // Something is wrong
        assert( false );

    };
}


// CISVolumeStepper_H_included
#endif

