///////////////////////////////////////////////////////////////////////////////
// $Id: CRLECompress.hpp 1289 2011-05-15 00:08:39Z spanel $
//
// ***** BEGIN LICENSE BLOCK *****
// Version: MPL 1.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is 3DimViewer code.
//
// The Initial Developer of the Original Code is
// 3Dim Laboratory s.r.o.
// Portions created by the Initial Developer are Copyright (C) 2008-2011
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//
// ***** END LICENSE BLOCK *****
///////////////////////////////////////////////////////////////////////////////

#ifdef _OPENMP
#   include <omp.h>
#endif


template< typename tpElement >
data::CRLECompressedData< tpElement >::CRLECompressedData(unsigned reserved_size )
    : m_maxLength(std::numeric_limits<tLength>::max())
{
    if( reserved_size > 0 )
        reserve( reserved_size );

}

template< typename tpElement >
template< typename tpInputIterator >
void data::CRLECompressedData< tpElement >::compress ( tpInputIterator & it )
{
    if( ! it )
        return;

    tLength counter(1);
    tElement e, le(*it);

    // First element is stored, go to next
    ++it;

    for( ;it; ++it )
    {
        if( counter == m_maxLength )
        {
            // Store pair
            m_lengths.push_back(counter);
            m_elements.push_back(le);

            // Reset counter and last element value
            counter = 1;
            le = *it;

            continue;
        }

        // Get next
        e = *it;

        if( e == le )
        {
            ++counter;
        }else{

            // store pair
            m_lengths.push_back(counter);
            m_elements.push_back(le);

            // Reset counter and last element
            counter = 1;
            le = e;
        }
        
    }

    // Store last elements
    m_lengths.push_back(counter);
    m_elements.push_back(le);
}

template< typename tpElement >
template< typename tpOutputIterator >
unsigned long data::CRLECompressedData< tpElement >::decompress( tpOutputIterator & it )
{
    if( ! it || m_elements.size() == 0 )
        return 0;
    
    // Get iterators
    typename tEVec::iterator eIt( m_elements.begin() ), eItEnd( m_elements.end() );
    typename tLVec::iterator lIt( m_lengths.begin() );

    // Written data counter
    unsigned long counter( 0 );

    // For all pairs
    for( ;eIt != eItEnd; ++eIt, ++lIt )
    {
        // Get pair information
        tElement e( *eIt );
        tLength length( *lIt );

        for( tLength l = 0; l < length; ++l )
        {
            *it = e;
            ++it;
            ++counter;

            if( ! it )
                return counter; // Output is full;
        }
    }

    return counter;
}

template< class tpVolume > 
data::CRLECompressVolume< tpVolume >::CRLECompressVolume()
    : m_sx(0)
    , m_sy(0)
    , m_sz(0)
    , m_margin(0)
{

#ifdef _OPENMP
    m_numPlanes = vpl::ompGetMaxThreads();
#else
    m_numPlanes = 1;
#endif

    m_buffer.resize( m_numPlanes );

}

template< class tpVolume > 
void data::CRLECompressVolume< tpVolume >::compress( tVolume & volume )
{
    // Clear old data
    clear();

    // Get volume information
    m_sx = volume.getXSize();
    m_sy = volume.getYSize();
    m_sz = volume.getZSize();
    m_margin = volume.getMargin();

    // Anyone of sizes is zero.
    if( m_sx * m_sy * m_sy == 0 )
        return;

    // Allocate output data array
    resize( m_sz );

    // Allocate buffers
    for( int i = 0; i < m_numPlanes; ++i )
    {
        m_buffer[i] = new vpl::img::CImage< tVoxel >( m_sx, m_sy );
    }
    
    // Compress volume
#pragma omp parallel for schedule(static) default(shared)
    for( int i = 0; i < int( m_sz); ++i )
    {
#ifdef _OPENMP
        int plane_id( vpl::ompGetThreadNum() );
#else
        int plane_id(0);
#endif
        // Copy plane data
        volume.getPlaneXY( i, *m_buffer[plane_id] );

        // Create iterator
        typename vpl::img::CImage<tVoxel>::tIterator It(*m_buffer[plane_id]);

        // Call compression
        m_data[ i ]->compress( It );
    }
}

template< class tpVolume > 
bool data::CRLECompressVolume< tpVolume >::decompress( tVolume & volume )
{
    if( ! isSameSize( volume ) )
        return false;

    // Crate plane buffer
    vpl::img::CImage< tVoxel > * plane( new vpl::img::CImage< tVoxel >( m_sx, m_sy ) );

    // Compress volume
    for( int i = 0; i < int( m_sz); ++i )
    {
        // Create iterator
        typename vpl::img::CImage<tVoxel>::tIterator It(*plane);

        // Call compression
        m_data[ i ]->decompress( It );

        // Copy plane data
        volume.setPlaneXY( i, *plane );
    }

    // Delete buffer
    delete plane;

    return true;
}

/** 
 * Clear all data
 */
template< class tpVolume > 
void data::CRLECompressVolume< tpVolume >::clear()
{
    resize(0);
}

/** 
 *  Is the destination volume the same size?
 */
template< class tpVolume > 
bool data::CRLECompressVolume< tpVolume >::isSameSize( const tVolume & volume )
{
    return m_sx == volume.getXSize() && m_sy == volume.getYSize() && m_sz == volume.getZSize();
}

/** 
 *  Resize data array (clear old or allocate new).
 */
template< class tpVolume > 
void data::CRLECompressVolume< tpVolume >::resize( unsigned int size )
{
    unsigned int oldSize( (unsigned int)m_data.size() );

    if( size > oldSize )
    {
        m_data.resize(size);

        for( unsigned int i = oldSize; i < size; ++i )
            m_data[i] = new CRLECompressedData< tVoxel >;
    }

    if( size < oldSize )
    {
        for( unsigned int i = size; i < oldSize; ++i )
            delete m_data[i];

        m_data.resize( size );
    }

    typename std::vector< CRLECompressedData< tVoxel > *>::iterator i;
    for( i = m_data.begin(); i < m_data.end(); ++i )
    {
        (*i)->clear();
        (*i)->reserve( 10 );
    }
}

/**
 * Get compressed data size
 */
template< class tpVolume > 
unsigned long data::CRLECompressVolume< tpVolume >::getDataSize()
{
    unsigned long counter(0);

    typename std::vector< CRLECompressedData< tVoxel > *>::iterator i;
    for( i = m_data.begin(); i < m_data.end(); ++i )
    {
        counter += (*i)->getDataSize();
    }

    return counter;
}
