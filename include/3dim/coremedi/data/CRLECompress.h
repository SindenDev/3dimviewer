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

#ifndef CRLECompress_H
#define CRLECompress_H

#include <assert.h>
#include <vector>
#include <limits>

#ifdef _OPENMP
#   include <omp.h>
#endif // _OPENMP

namespace data
{
    template< typename tpElement >
    class CRLECompressedData 
    {
    public:
        //! Constructor
        CRLECompressedData(unsigned reserved_size = 10 );

        //! Destructor
        ~CRLECompressedData(){ clear(); }

        //! Compress 
        template< typename tpInputIterator >
        void compress( tpInputIterator & it );

        //! Decompress
        template< typename tpOutputIterator >
        unsigned long decompress( tpOutputIterator & it );

        //! Reserve given size
        void reserve( unsigned reserved_size )
        {
            m_elements.reserve( reserved_size );
            m_lengths.reserve( reserved_size );
        }

        //! Finalize data - re-reserve to the current size
        void finalize()
        {
            m_elements.reserve( m_elements.size() );
            m_lengths.reserve( m_lengths.size() );
        }

        unsigned long getDataSize() 
        {
            return (unsigned long)(m_elements.size() * ( sizeof(tElement) + sizeof(tLength) ));
        }

        //! Clear all
        void clear()
        {
            m_elements.clear();
            m_lengths.clear();
        }

    protected:
        //! Chunk length counter type
        typedef unsigned char tLength;

        //! Data type
        typedef tpElement tElement;

        //! Lengths vector type
        typedef std::vector< tLength > tLVec;

        //! Elements vector type
        typedef std::vector< tElement > tEVec;

    public:
        //! Lengths
        tLVec m_lengths;

        //! Elements
        tEVec m_elements;

        //! Maximal length
        const tElement m_maxLength;

    }; // class CRLECompressedData


    template< class tpVolume > 
    class CRLECompressVolume
    {
    public:
        //! Constructor
        CRLECompressVolume();

        //! Destructor
        virtual ~CRLECompressVolume(){ clear(); }

        //! Volume type
        typedef typename tpVolume::tVolume tVolume;

        //! Voxel type
        typedef typename tVolume::tVoxel tVoxel;
    
        //! Size type
        typedef int tSize;

    public:
        //! Compress volume
        void compress( tVolume & volume );

        //! Decompress to volume, if possible (same size and margin)
        bool decompress( tVolume & volume );

        //! Get compressed data size
        unsigned long getDataSize();

    protected:
        //! Clear all data
        void clear();

        //! Is the destination volume the same size?
        bool isSameSize( const tVolume & volume );

        //! Resize data array (clear old or allocate new).
        void resize( unsigned int size );

    protected:
        //! Volume sizes
        tSize m_sx, m_sy, m_sz, m_margin;

        //! Compressed data
        std::vector< CRLECompressedData< tVoxel > * > m_data;

        //! Buffer planes
        std::vector< typename vpl::img::CImage< tVoxel >::tSmartPtr > m_buffer;

        //! Number of planes in the buffer
        int m_numPlanes;

    };
} // namespace data

#include <data/CRLECompress.hpp>




#endif // CRLECompress_H
