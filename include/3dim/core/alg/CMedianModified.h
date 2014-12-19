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

#include <VPL/Base/Assert.h>
#include <VPL/Base/Types.h>
#include <VPL/Base/Data.h>
#include <VPL/Math/Base.h>
#include <VPL/Module/Progress.h>
#include <VPL/Image/VolumeFilter.h>

#ifndef CMedianModified_H_included
#define CMedianModified_H_included

namespace vpl
{
namespace img
{


template <class V>
class CModifiedMedianFilter : public CVolumeFilter<V>, public vpl::mod::CProgress
{
public:
    //! Volume filter base.
    typedef CVolumeFilter<V> base;
    typedef typename base::tVolume tVolume;
    typedef typename base::tVoxel tVoxel;

public:
    //! Constructor that creates a new median filter.
    //! - Parameter 'Size' is a window size and it must be an odd number.
    CModifiedMedianFilter(tSize Size, const tVoxel & active, const tVoxel & mask = tVoxel() )
        : m_color( active )
        , m_null( mask )
        , m_MedianSize(Size)
        , m_Data(Size * Size * Size * ompGetMaxThreads())
    {
        VPL_ASSERT((Size % 2) == 1);
    }

    //! Destructor
    ~CModifiedMedianFilter() {}


    //! Median volume filtering
    //! - Returns false on failure
    bool operator()(const tVolume& SrcVolume, tVolume& DstVolume);

    //! Returns filter response at specified volume position.
    //! - Value is not normalized!
    tVoxel getResponse(const tVolume& SrcVolume, tSize x, tSize y, tSize z);

    //! Sets the windows size
    void setSize(tSize Size)
    {
        VPL_ASSERT((Size % 2) == 1);

        m_MedianSize = Size;
        m_Data.resize(Size * Size * Size * ompGetMaxThreads());
    }

    virtual tSize getSize() const {return m_MedianSize;}

protected:
    //! Find median 
    tVoxel myFindMedian(tVoxel * pData, tSize Size, tVoxel current);

protected:
    //! Used "color"
    tVoxel m_color;

    //! Used "no color" mask
    tVoxel m_null;

    //! Median filter size
    vpl::tSize m_MedianSize;

    //! Internal data buffer
    vpl::base::CData<tVoxel> m_Data;  
};


//! Median value finding (Z Algorithm)
template <class V>
typename CModifiedMedianFilter<V>::tVoxel CModifiedMedianFilter<V>::myFindMedian(tVoxel * pData, tSize Size, tVoxel current)
{
    vpl::tSize count(0), threshold( Size / 2);

    for( vpl::tSize i = 0; i < Size; ++i )
    {
        if( pData[i] == m_color )
            ++count;
    }

    if( count > threshold )
        return m_color;

    if( current == m_color )
        return m_null;

    return current;

}




//==============================================================================
/*
 * Methods templates.
 */

// Volume filtering method
template <class V>
bool CModifiedMedianFilter<V>::operator()(const tVolume& SrcVolume, tVolume& DstVolume)
{
    CProgress::tProgressInitializer StartProgress(*this);

    // Volume size
    vpl::tSize XCount = vpl::math::getMin(SrcVolume.getXSize(), DstVolume.getXSize());
    vpl::tSize YCount = vpl::math::getMin(SrcVolume.getYSize(), DstVolume.getYSize());
    vpl::tSize ZCount = vpl::math::getMin(SrcVolume.getZSize(), DstVolume.getZSize());

    // Initialize the progress observer
    CProgress::setProgressMax(ZCount);

    vpl::tSize KernelSize = m_MedianSize * m_MedianSize * m_MedianSize;
    vpl::tSize kernelHalf( m_MedianSize / 2 );
    for( tSize z = 0; z < ZCount; ++z )
    {
#pragma omp parallel for schedule(static) default(shared)
        for( tSize y = 0; y < YCount; ++y )
        {
            tSize Start = ompGetThreadNum() * KernelSize;
            for( tSize x = 0; x < XCount; ++x )
            {
                tVoxel current = SrcVolume( x, y, z );

                if( current == m_null || current == m_color )
                {
                    // Copy voxels from the window
                    SrcVolume.rect(CPoint3i(x-kernelHalf, y-kernelHalf, z-kernelHalf), CSize3i(m_MedianSize)).copyTo(m_Data.getPtr(Start));

                    // Median finding
                    tVoxel Median = myFindMedian(m_Data.getPtr(Start), KernelSize, current);

                    // Set pixel value
                    DstVolume.set(x, y, z, Median);
                }else{
                    DstVolume.set(x, y, z, current );
                }
            }
        }

        // Notify progress observers...
        if( !CProgress::progress() )
        {
            return false;
        }
    }

    // O.K.
    return true;
}


// Volume filter response
template <class V>
typename CModifiedMedianFilter<V>::tVoxel CModifiedMedianFilter<V>::getResponse(const tVolume& SrcVolume, tSize x, tSize y, tSize z)
{
    // Copy voxels from the window
    SrcVolume.rect(CPoint3i(x, y, z), CSize3i(m_MedianSize)).copyTo(m_Data.getPtr());

    // Median finding
    return median::findMedian<tVoxel>(m_Data.getPtr(), m_MedianSize * m_MedianSize * m_MedianSize);
}

} // namespace img
} // namespace vpl

/*
////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Modified median filter. 
////////////////////////////////////////////////////////////////////////////////////////////////////
template <class V>
struct CModifiedMedianFilter : public vpl::img::CVolumeFilter< V >, public vpl::mod::CProgress
{
public:
    //! Volume filter base.
    typedef vpl::img::CVolumeFilter<V> base;
    typedef typename base::tVolume tVolume;
    typedef typename base::tVoxel tVoxel;

public:
    //! Default constructor.
    CModifiedMedianFilter( const tVoxel & active, const tVoxel & mask = tVoxel() ) 
		: m_color( active )
		, m_null( mask )
		, m_MedianSize( 1 )
	{ }
    
    //! Virtual destructor.
    virtual ~CModifiedMedianFilter() {} 
    
    //! Filtering of input/source volume.
    //! - Returns false on failure.
    virtual bool operator()(const tVolume& SrcVolume, tVolume& DstVolume);

	//! Sets the windows size
    void setSize(vpl::tSize Size)
    {
        VPL_ASSERT((Size % 2) == 1);

        m_MedianSize = Size / 2;
    }

protected:
	//! Find median modified
	tVoxel findMedian( const tVolume& SrcVolume, vpl::tSize x, vpl::tSize y, vpl::tSize z, tVoxel current );

	//! Median filter size
    vpl::tSize m_MedianSize;
protected:
	//! Used "color"
	tVoxel m_color;

	//! Used "no color" mask
	tVoxel m_null;
};

template <class V>

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	 casting operator. 
//!
//!\typeparam	V	. 
//!
//!\return	The result of the operation. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool CModifiedMedianFilter< V >::operator()(const tVolume& SrcVolume, tVolume& DstVolume)
{
	CProgress::tProgressInitializer StartProgress(*this);

    // Volume size
    vpl::tSize XCount = vpl::math::getMin(SrcVolume.getXSize(), DstVolume.getXSize());
    vpl::tSize YCount = vpl::math::getMin(SrcVolume.getYSize(), DstVolume.getYSize());
    vpl::tSize ZCount = vpl::math::getMin(SrcVolume.getZSize(), DstVolume.getZSize());

    // Initialize the progress observer
    CProgress::setProgressMax(ZCount);

    // Filter the image
    for( vpl::tSize z = 0; z < ZCount; ++z )
    {
        for( vpl::tSize y = 0; y < YCount; ++y )
        {
            for( vpl::tSize x = 0; x < XCount; ++x )
            {
				tVoxel current = SrcVolume( x, y, z );

//				if( current == m_null || current == m_color )
				{
					DstVolume( x, y, z ) = findMedian( SrcVolume, x, y, z, current );
				}
			}
		}

		// Notify progress observers...
        if( !CProgress::progress() )
        {
            return false;
        }
	}

	// O.K.
    return true;
}

template <class V>

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Searches for the first median. 
//!
//!\typeparam	V	. 
//!\param	x	The x coordinate. 
//!\param	y	The y coordinate. 
//!\param	z	The z coordinate. 
//!
//!\return	The found median. 
////////////////////////////////////////////////////////////////////////////////////////////////////
typename CModifiedMedianFilter< V >::tVoxel 
CModifiedMedianFilter< V >::findMedian( const tVolume& SrcVolume, vpl::tSize x, vpl::tSize y, vpl::tSize z, tVoxel current )
{
	vpl::tSize count(0);
	vpl::tSize threshold( m_MedianSize * m_MedianSize * m_MedianSize / 2 );

	for( vpl::tSize k = z - m_MedianSize; k <= z + m_MedianSize; ++k )
	{
		for( vpl::tSize j = y - m_MedianSize; j <= y + m_MedianSize; ++j )
		{ 
			for( vpl::tSize i = x - m_MedianSize; i <= x + m_MedianSize; ++i )
			{
				if( SrcVolume( i, j, k ) == m_color )
					++count;
			}
		}
	}

	if( count > threshold )
		return m_color;

	if( current == m_color )
		return m_null;

	return current;
}

*/
// CMedianModified_H_included
#endif

