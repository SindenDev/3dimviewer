////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file   segmentation\CISMorphology.h
///
/// \brief  Declares the cis morphology class. 
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CISMorphology_H_included
#define CISMorphology_H_included

#include <VPL/Math/Base.h>
#include <VPL/Module/Progress.h>

namespace seg
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class  CMorphology
///
/// \brief  CMorphology is a base class for morphological operations. It gathers data and tests 
///         validity of pointers.
////////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    class tpVolume,
    class tpKernel
>
class CMorphology
{
protected:
    //! Volume type
    typedef tpVolume tVolume;

    //! Voxel type
    typedef typename tpVolume::tVoxel tVoxel;

    //! Kernel (structuring element) type
    typedef tpKernel tKernel;

    //! Kernel voxel type
    typedef typename tpKernel::tVoxel tKVoxel;

public:
    //! Constructor
    CMorphology( tVolume * src = NULL, tVolume *dst = NULL, tKernel * kernel = NULL );
    
    //! Set source volume
    void setSource( tVolume * volume ){ m_src = volume; }

    //! Set destination volume
    void setDestination( tVolume * volume ) { m_dst = volume; }

    //! Set kernel
    void setKernel( tKernel * kernel ){ m_kernel = kernel; }

    //! Set volume mask - if voxel has this value, is used as a 1, else it is 0
    void setVolumeMask( tVoxel mask ){ m_vmask = mask; }

    //! Set kernel mask - if voxel has this value, is used as a 1, else it is 0
    void setKernelMask( tKVoxel mask ){ m_kmask = mask; }

    //! Set kernel center
    void setKernelCenter( unsigned int x, unsigned int y, unsigned int z ) { m_kx = x; m_ky = y; m_kz = z; }

    //! Set coloring value
    void setColor( tVoxel color ) { m_color = color; }

protected:
    //! Test if data pointers are correctly set
    bool testData() { return m_src != NULL && m_dst != NULL && m_kernel != NULL
                            && m_src->getXSize() <= m_dst->getXSize()
                            && m_src->getYSize() <= m_dst->getYSize()
                            && m_src->getZSize() <= m_dst->getZSize(); }

protected:
    //! Volume data
    tVolume * m_src, *m_dst;

    //! Kernel data
    tKernel * m_kernel;

    //! Voxel mask value
    tVoxel m_vmask;

    //! Kernel mask value
    tKVoxel m_kmask;

    //! Coloring value
    tVoxel m_color;

    //! Kernel center position
    unsigned int m_kx, m_ky, m_kz;

}; // template class CMorphology

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class  CDilatation
///
/// \brief  Dilatation implementation. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    class tpVolume,
    class tpKernel
>
class CDilatation : public CMorphology< tpVolume, tpKernel >, public vpl::mod::CProgress
{
public:
    // Volume type
	typedef CMorphology< tpVolume, tpKernel > base;
    typedef typename base::tVolume tVolume;
    typedef typename base::tVoxel tVoxel;
    typedef typename base::tKernel tKernel;
    typedef typename base::tKVoxel tKVoxel;

public:
    //! Constructor
    CDilatation( tVolume * src = NULL, tVolume *dst = NULL, tKernel * kernel = NULL )
        : base( src, dst, kernel )
    {}

    //! Apply operation
    bool apply()
    {
        if( !this->testData() )
            return false;

        CProgress::tProgressInitializer StartProgress( *this );

        // Get data sizes
        unsigned int  vsx( this->m_src->getXSize() ) // Volume size
                    , vsy( this->m_src->getYSize() )
                    , vsz( this->m_src->getZSize() )
                    , ksx( this->m_kernel->getXSize() ) // kernel size
                    , ksy( this->m_kernel->getYSize() )
                    , ksz( this->m_kernel->getZSize() )
                    , sx, sy, sz, ex, ey, ez; // Kernel window

        // Data pointer
        vpl::tSize idxS, idxD, idxDK;
        const vpl::tSize offSrcX=this->m_src->getXOffset(),
                         offDstX=this->m_dst->getXOffset();
        
        CProgress::setProgressMax( vsz );

        // For each data voxel
        for( unsigned int vz = 0; vz < vsz; ++vz )
        {
            ez = vpl::math::getMin( ksz, vsz - vz + this->m_kz );
            sz = vpl::math::getMax( 0, (int)this->m_kz - vpl::math::getMin( (int)vz, (int)this->m_kz ) );
    
            for( unsigned int vy = 0; vy < vsy; ++vy )
            {
                idxS = this->m_src->getIdx( 0, vy, vz );
                idxD = this->m_dst->getIdx( 0, vy, vz );
                idxDK = this->m_dst->getIdx( 0 - this->m_kx, vy - this->m_ky, vz - this->m_kz );
                ey = vpl::math::getMin( ksy, vsy - vy + this->m_ky );
                sy = vpl::math::getMax( 0, (int)this->m_ky - vpl::math::getMin( (int)vy, (int)this->m_ky ) );

                for( unsigned int vx = 0; vx < vsx; ++vx, idxS+=offSrcX, idxD+=offDstX, idxDK+=offDstX )
                {
                    ex = vpl::math::getMin( ksx, vsx - vx + this->m_kx );
                    sx = vpl::math::getMax( 0, (int)this->m_kx - vpl::math::getMin( (int)vx, (int)this->m_kx ) );
                    
                    if( this->m_src->at(idxS) == this->m_vmask )
                    {
                        // Apply kernel
                        applyKernel( idxDK, sx, sy, sz, ex, ey, ez );
                    }

                }
            }

            // Notify progress observers...
            if( !CProgress::progress() )
            {
                return false;
            }
        }

        return true;
    } // Apply dilatation

protected:
    //! Apply kernel on image (or a part of the kernel).
    void applyKernel(  vpl::tSize idxV, unsigned int ksx, unsigned int ksy, unsigned int ksz, unsigned int kex, unsigned int key, unsigned int kez )
    {
        unsigned int xoff(this->m_dst->getXOffset() ), yoff( this->m_dst->getYOffset() ), zoff( this->m_dst->getZOffset() );
        const vpl::tSize offKernelX=this->m_kernel->getXOffset();

        for( unsigned int kz = ksz; kz < kez; ++kz )
        {
            for( unsigned int ky = ksy; ky < key; ++ky )
            {
                vpl::tSize idxK = this->m_kernel->getIdx( ksx , ky, kz );
                vpl::tSize idxVV = idxV + ksx * xoff + kz * zoff + ky * yoff;

                for( unsigned int kx = ksx; kx < kex; ++kx, idxK+=offKernelX, idxVV+=xoff )
                {
                    if(  this->m_kernel->at(idxK) == this->m_kmask )
                        this->m_dst->set(idxVV, this->m_color);
                }
            }
        }
    }

}; // template class CDilatation


////////////////////////////////////////////////////////////////////////////////////////////////////
/// \class  CErosion
///
/// \brief  Erosion implementation. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    class tpVolume,
    class tpKernel
>
class CErosion : public CMorphology< tpVolume, tpKernel >, public vpl::mod::CProgress
{
public:
    // Volume type
	typedef CMorphology< tpVolume, tpKernel > base;
    typedef typename base::tVolume tVolume;
    typedef typename base::tVoxel tVoxel;
    typedef typename base::tKernel tKernel;
    typedef typename base::tKVoxel tKVoxel;

public:
    //! Constructor
    CErosion( tVolume * src = NULL, tVolume *dst = NULL, tKernel * kernel = NULL )
        : base( src, dst, kernel )
    {}

    //! Set erase color
    void setEraseColor( tVoxel color ) { m_ecolor = color; }

    //! Apply operation
    bool apply()
    {
        if( ! this->testData() )
            return false;

        CProgress::tProgressInitializer StartProgress( *this );

        // Get data sizes
        unsigned int  vsx( this->m_src->getXSize() ) // Volume size
                    , vsy( this->m_src->getYSize() )
                    , vsz( this->m_src->getZSize() )
                    , ksx( this->m_kernel->getXSize() ) // kernel size
                    , ksy( this->m_kernel->getYSize() )
                    , ksz( this->m_kernel->getZSize() );

        CProgress::setProgressMax( vsz );

        const vpl::tSize xoffSrc=this->m_src->getXOffset();
        const vpl::tSize xoff(this->m_dst->getXOffset() ), zoff( this->m_dst->getZOffset() );

        // For each data voxel
        for( unsigned int vz = this->m_kz; vz < vsz - ksz + this->m_kz; ++vz )
        {
    
            for( unsigned int vy = this->m_ky; vy < vsy - ksy + this->m_ky; ++vy )
            {
                vpl::tSize idxS = this->m_src->getIdx( this->m_kx, vy, vz );
                vpl::tSize idxD = this->m_dst->getIdx( this->m_kx, vy, vz );

                for( unsigned int vx = this->m_kx; vx < vsx - ksx + this->m_kx; ++vx, idxS+=xoffSrc, idxD+=xoff )
                {
                    // Apply kernel
                    applyKernel( idxS, idxD, ksx, ksy, ksz );
                }
            }

            // Notify progress observers...
            if( !CProgress::progress() )
            {
                return false;
            }
        }

        unsigned int x, y, z;        

        // Erase borders
        // XY planes
        for( z = 0; z <= this->m_kz; ++z )
            for( y = 0; y < vsy; ++y )
            {
                vpl::tSize ptr1 = this->m_dst->getIdx( 0, y, z );
                vpl::tSize ptr2 = this->m_dst->getIdx( 0, y, vsz - 1 - z );
                for( x = 0; x < vsx; ++x, ptr1+=xoff, ptr2+=xoff )
                {
                    this->m_dst->set(ptr1, m_ecolor);
                    this->m_dst->set(ptr2, m_ecolor);
                }
            }

        // XZ planes
        for( y = 0; y <= this->m_ky; ++y )
            for( z = 0; z < vsz; ++z )
            {
                vpl::tSize ptr1 = this->m_dst->getIdx( 0, y, z );
                vpl::tSize ptr2 = this->m_dst->getIdx( 0, vsy - y - 1, z );
                for( x = 0; x < vsx; ++x, ptr1+=xoff, ptr2+=xoff )
                {
                    this->m_dst->set(ptr1, m_ecolor);
                    this->m_dst->set(ptr2, m_ecolor);
                }
            }

        // YZ planes
        for( x = 0; x <= this->m_kx; ++x )
            for( y = 0; y < vsy; ++y )
            {
                vpl::tSize ptr1 = this->m_dst->getIdx( x, y, 0 );
                vpl::tSize ptr2 = this->m_dst->getIdx( vsx - x - 1, y, 0 );
                for( z = 0; z < vsz; ++z, ptr1 += zoff, ptr2 += zoff )
                {
                    this->m_dst->set(ptr1, m_ecolor);
                    this->m_dst->set(ptr2, m_ecolor);
                }
            }

        return true;


    }

protected:
    //! Apply kernel on image.
    void applyKernel( vpl::tSize idxS, vpl::tSize idxD, unsigned int kex, unsigned int key, unsigned int kez )
    {
        if( this->m_src->at(idxS) != this->m_vmask )
        {
            this->m_dst->set(idxD,this->m_src->at(idxS));
            return;
        }

        const vpl::tSize offKernelX=this->m_kernel->getXOffset();
        const vpl::tSize offSrcX=this->m_src->getXOffset();

        int yoff( this->m_src->getYOffset() ), zoff( this->m_src->getZOffset() );

        for( unsigned int kz = 0; kz < kez; ++kz )
            for( unsigned int ky = 0; ky < key; ++ky )
            {
                vpl::tSize idxK = this->m_kernel->getIdx( 0 , ky, kz );
                vpl::tSize idxV = idxS - (int)this->m_kx + ((int)kz - (int)this->m_kz)  * zoff + ((int)ky-(int)this->m_ky) * yoff;

                for( unsigned int kx = 0; kx < kex; ++kx, idxK+=offKernelX, idxV+=offSrcX )
                {
                    if( this->m_kernel->at(idxK) == this->m_kmask && this->m_src->at(idxV) != this->m_vmask)
                    {
                        this->m_dst->set(idxD, m_ecolor);
                        return;
                    }
                }
            }
 
         // All is OK, draw voxel
        this->m_dst->set(idxD, this->m_color);
    }

protected:
    //! Erasing color - used on eroded voxel that was colored before
    tVoxel m_ecolor;

}; // template class CErosion

/******************************************************************************
    CMorphology - implementation
******************************************************************************/

template
<
    class tpVolume,
    class tpKernel
>
CMorphology< tpVolume, tpKernel>::CMorphology( tVolume * src, tVolume *dst, tKernel * kernel )
: m_src( src )
, m_dst( dst )
, m_kernel( kernel )
, m_kx( 0 ), m_ky( 0 ), m_kz( 0 )
{
}

} // namespace seg

//CISMorphology_H_included
#endif

