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

#ifndef CBitVolume_H
#define CBitVolume_H

///////////////////////////////////////////////////////////////////////////////
// include files
#include <VPL/Image/Volume.h>
#include <data/CBitOperations.h>

namespace data
{
//! Functor gets the value of bit at given position and set this value to bit at target position. Clears the source bit.
template <typename T>
class CBitReplace
{
public:
    //! Default constructor.
    CBitReplace(const T& bitToClear, const T& bitToSet) : m_bitToClear(bitToClear), m_bitToSet(bitToSet) {}

    //! Checks and eventually replaces value of a given parameter.
    void operator ()(T& value)
    {
        // check if the bit is set
        int bit = data::getBitFromValue<T>(value, m_bitToClear);

        if (bit > 0)
        {
            // clear bit
            data::clearBitInValue<T>(value, m_bitToClear);

            // set bit
            if (m_bitToSet > 0)
            {
                data::setBitInValue<T>(value, m_bitToSet);
            }
        }
    }

protected:
    //! bit indexes
    T m_bitToClear, m_bitToSet;
};

//! Functor moves bit to the target position with shifting the rest of the bits.
template <typename T>
class CBitMove
{
public:
    //! Default constructor.
    CBitMove(const T& sourceBitIndex, const T& destinationBitIndex) : m_sourceBitIndex(sourceBitIndex), m_destinationBitIndex(destinationBitIndex) {}

    //! Moves bit.
    void operator ()(T& value)
    {
        if (value == 0)
        {
            return;
        }

        // save the bit value
        int sourceBit = data::getBitFromValue<T>(value, m_sourceBitIndex);

        if (m_sourceBitIndex < m_destinationBitIndex)
        {
            for (T i = m_sourceBitIndex; i < m_destinationBitIndex; ++i)
            {
                // get the bit on the left
                int leftBit = data::getBitFromValue<T>(value, i + 1);

                if (leftBit > 0)
                {
                    // set bit
                    data::setBitInValue<T>(value, i);
                }
                else
                {
                    // clear bit
                    data::clearBitInValue<T>(value, i);
                }
            }
        }
        else
        {
            for (T i = m_sourceBitIndex; i > m_destinationBitIndex; --i)
            {
                // get the bit on the right
                int rightBit = data::getBitFromValue<T>(value, i - 1);

                if (rightBit > 0)
                {
                    // set bit
                    data::setBitInValue<T>(value, i);
                }
                else
                {
                    // clear bit
                    data::clearBitInValue<T>(value, i);
                }
            }
        }

        if (sourceBit > 0)
        {
            // set bit
            data::setBitInValue<T>(value, m_destinationBitIndex);
        }
        else
        {
            // clear bit
            data::clearBitInValue<T>(value, m_destinationBitIndex);
        }
    }

protected:
    //! bit indexes
    T m_sourceBitIndex, m_destinationBitIndex;
};

//! Functor clears bit at given position and shifts higher bits to the left.
template <typename T>
class CBitClearAndShift
{
public:
    //! Default constructor.
    CBitClearAndShift(const T& clearBitIndex) : m_clearBitIndex(clearBitIndex) {}

    //! Clears given bit and shifts all higher bits.
    void operator ()(T& value)
    {
        if (value == 0)
        {
            return;
        }

        T valueCopy = value;
        T leftMostSetBitIndex = 0;
        while (valueCopy > 1)
        {
            ++leftMostSetBitIndex;
            valueCopy = valueCopy >> 1;
        }

        if (leftMostSetBitIndex >= m_clearBitIndex)
        {
            for (T i = m_clearBitIndex; i < leftMostSetBitIndex; ++i)
            {
                // get the bit on the left
                int leftBit = data::getBitFromValue<T>(value, i + 1);

                if (leftBit > 0)
                {
                    // set bit
                    data::setBitInValue<T>(value, i);
                }
                else
                {
                    // clear bit
                    data::clearBitInValue<T>(value, i);
                }
            }

            data::clearBitInValue<T>(value, leftMostSetBitIndex);
        }
    }

protected:
    //! bit indexes
    T m_clearBitIndex;
};

//! Functor copies bit value at given position to bit at the target position.
template <typename T>
class CBitCopy
{
public:
    //! Default constructor.
    CBitCopy(const T& sourceBitIndex, const T& destinationBitIndex) : m_sourceBitIndex(sourceBitIndex), m_destinationBitIndex(destinationBitIndex) {}

    //! Copies bit.
    void operator ()(T& value)
    {
        if (value == 0)
        {
            return;
        }

        // get the bit value
        int sourceBit = data::getBitFromValue<T>(value, m_sourceBitIndex);

        if (sourceBit > 0)
        {
            // set bit
            data::setBitInValue<T>(value, m_destinationBitIndex);
        }
        else
        {
            // clear bit
            data::clearBitInValue<T>(value, m_destinationBitIndex);
        }
    }

protected:
    //! bit indexes
    T m_sourceBitIndex, m_destinationBitIndex;
};

//! Volume, which works with bits in each voxel.
//! Every bit in voxel represent one region, so there can be maximum of sizeof(tVoxel) overlapping regions.
template <typename tVoxel>
class CBitVolume : public vpl::img::CVolume<tVoxel>
{
public:

    //! Default constructor creates volume of zero size.
    CBitVolume() : vpl::img::CVolume<tVoxel>() {}

    //! Constructor that allocates volume data.
    CBitVolume(vpl::tSize XSize, vpl::tSize YSize, vpl::tSize ZSize, vpl::tSize Margin = 0) : vpl::img::CVolume<tVoxel>(XSize, YSize, ZSize, Margin) {}

    ~CBitVolume() {}

    using vpl::img::CVolume<tVoxel>::at;

    //! Is bit set?
    //! \param i Index to volume to get voxel.
    //! \param bitIndex Index of bit in voxel.
    bool at(vpl::tSize i, vpl::tSize bitIndex)
    {
        vpl::tSize bit = data::getBitFromValue<tVoxel>(at(i), bitIndex);
        return bit > 0;
    }

    //! Is bit set?
    //! \param x, y, z Coordinates of voxel.
    //! \param bitIndex Index of bit in voxel.
    bool at(vpl::tSize x, vpl::tSize y, vpl::tSize z, vpl::tSize bitIndex)
    {
        vpl::tSize bit = data::getBitFromValue<tVoxel>(at(x, y, z), bitIndex);
        return bit > 0;
    }

    //! Sets the subscripted bit in voxel (to 1).
    //! \param i Index to volume to get voxel.
    //! \param bitIndex Index of bit in voxel.
    CBitVolume& setBit(vpl::tSize i, vpl::tSize bitIndex)
    {
        tVoxel& value = at(i);
        data::setBitInValue<tVoxel>(value, bitIndex);
        vpl::img::CVolume<tVoxel>::set(i, value);
        return *this;
    }

    //! Sets the subscripted bit in voxel (to 1). 
    //! \param x, y, z Coordinates of voxel.
    //! \param bitIndex Index of bit in voxel.
    CBitVolume& setBit(vpl::tSize x, vpl::tSize y, vpl::tSize z, vpl::tSize bitIndex)
    {
        tVoxel& value = at(x, y, z);
        data::setBitInValue<tVoxel>(value, bitIndex);
        return *this;
    }

    //! Clears the subscripted bit in voxel. (to 0)
    //! \param i Index to volume to get voxel.
    //! \param bitIndex Index of bit in voxel.
    CBitVolume& clearBit(vpl::tSize i, vpl::tSize bitIndex)
    {
        tVoxel& value = at(i);
        data::clearBitInValue<tVoxel>(value, bitIndex);
        vpl::img::CVolume<tVoxel>::set(i, value);
        return *this;
    }

    //! Clears the subscripted bit in voxel. (to 0)
    //! \param x, y, z Coordinates of voxel.
    //! \param bitIndex Index of bit in voxel.
    CBitVolume& clearBit(vpl::tSize x, vpl::tSize y, vpl::tSize z, vpl::tSize bitIndex)
    {
        tVoxel& value = at(x, y, z);
        data::clearBitInValue<tVoxel>(value, bitIndex);
        return *this;
    }

    //! Moves bit on sourceBitIndex to destinationBitIndex with shifting all bits in between.
    //! \param sourceBitIndex Index of bit, which will be moved.
    //! \param destinationBitIndex Index of bit, where the value of source bit will be moved.
    void moveBitInVolume(tVoxel sourceBitIndex, tVoxel destinationBitIndex)
    {
        vpl::img::CVolume<tVoxel>::pforEach(CBitMove<tVoxel>(sourceBitIndex, destinationBitIndex));
    }

    //! Extract one bit from volume and return it in new volume.
    //! Creates new volume and returns it.
    //! \param bitIndex Index of bit, which values will be copied to new volume. All the other bits will be 0.
    CBitVolume getMaskedVolume(vpl::tSize bitIndex)
    {
        CBitVolume volume;
        volume.resize(vpl::img::CVolume<tVoxel>::m_Size, vpl::img::CVolume<tVoxel>::m_Margin);
        volume.fillEntire(0);
        
        const vpl::tSize sx = vpl::img::CVolume<tVoxel>::m_Size.x();
        const vpl::tSize sy = vpl::img::CVolume<tVoxel>::m_Size.y();
        const vpl::tSize sz = vpl::img::CVolume<tVoxel>::m_Size.z();
        const vpl::tSize Offset = vpl::img::CVolume<tVoxel>::getXOffset();

#pragma omp parallel for
        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                vpl::tSize idx = vpl::img::CVolume<tVoxel>::getIdx(0, j, k);
                for (vpl::tSize i = 0; i < sx; ++i, idx += Offset)
                {
                    if (at(idx, bitIndex))
                    {
                        volume.setBit(idx, bitIndex);
                    }
                }
            }
        }

        return volume;
    }

    //! Replace value of one bit with values from given volume.
    //! Volumes must be of the same size!
    //! \param volume Volume from which the bit value will be read.
    //! \param dstBitIndex Index of bit, which will be set.
    //! \param inputVolumeBitIndex Index of bit in given volume, which value will be read.
    CBitVolume& setBitFromVolume(CBitVolume& volume, vpl::tSize dstBitIndex, vpl::tSize inputVolumeBitIndex)
    {
        const vpl::tSize sx = vpl::img::CVolume<tVoxel>::m_Size.x();
        const vpl::tSize sy = vpl::img::CVolume<tVoxel>::m_Size.y();
        const vpl::tSize sz = vpl::img::CVolume<tVoxel>::m_Size.z();
        const vpl::tSize Offset = vpl::img::CVolume<tVoxel>::getXOffset();

#pragma omp parallel for
        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                vpl::tSize idx = vpl::img::CVolume<tVoxel>::getIdx(0, j, k);
                for (vpl::tSize i = 0; i < sx; ++i, idx += Offset)
                {
                    if (volume.at(idx, inputVolumeBitIndex))
                    {
                        setBit(idx, dstBitIndex);
                    }
                    else
                    {
                        clearBit(idx, dstBitIndex);
                    }
                }
            }
        }

        return *this;
    }

    //! Appends given amount of bits from given volume from given bit index.
    //! Volumes must be of the same size!
    //! \param volume Volume from which the bits value will be read.
    //! \param fromBitIndex Index of first bit, which will be appended.
    //! \param appendBitCnt Number of bits, which will be appended. fromBitIndex + appendBitIndex must be smaller than voxel bits count (sizeof(tVoxel))!
    CBitVolume& appendVolume(CBitVolume& volume, vpl::tSize fromBitIndex, vpl::tSize appendBitCnt)
    {
        const vpl::tSize sx = vpl::img::CVolume<tVoxel>::m_Size.x();
        const vpl::tSize sy = vpl::img::CVolume<tVoxel>::m_Size.y();
        const vpl::tSize sz = vpl::img::CVolume<tVoxel>::m_Size.z();
        const vpl::tSize Offset = vpl::img::CVolume<tVoxel>::getXOffset();

#pragma omp parallel for
        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                vpl::tSize idx = vpl::img::CVolume<tVoxel>::getIdx(0, j, k);
                for (vpl::tSize i = 0; i < sx; ++i, idx += Offset)
                {
                    for (int b = 0; b < appendBitCnt; ++b)
                    {
                        if (volume.at(idx, b))
                        {
                            setBit(idx, fromBitIndex + b);
                        }
                        else
                        {
                            clearBit(idx, fromBitIndex + b);
                        }
                    }
                }
            }
        }

        return *this;
    }

    //! Performs bits union. Takes bit value from volume and bit value from given volume, performs union and set the value to output bit.
    //! Volumes must be of the same size!
    //! \param maskVolume Volume, with which the union will be performed.
    //! \param sourceBitIndex Index of bit in current volume.
    //! \param maskSourceBitIndex Index of bit in given volume.
    //! \param outputBitIndex Index of bit in current volume, in which the result of the union will be written.
    CBitVolume& performUnion(CBitVolume& maskVolume, vpl::tSize sourceBitIndex, vpl::tSize maskSourceBitIndex, vpl::tSize outputBitIndex)
    {
        const vpl::tSize sx = vpl::img::CVolume<tVoxel>::m_Size.x();
        const vpl::tSize sy = vpl::img::CVolume<tVoxel>::m_Size.y();
        const vpl::tSize sz = vpl::img::CVolume<tVoxel>::m_Size.z();
        const vpl::tSize Offset = vpl::img::CVolume<tVoxel>::getXOffset();

#pragma omp parallel for
        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                vpl::tSize idx = vpl::img::CVolume<tVoxel>::getIdx(0, j, k);
                for (vpl::tSize i = 0; i < sx; ++i, idx += Offset)
                {
                    if (at(idx, sourceBitIndex) || maskVolume.at(idx, maskSourceBitIndex))
                    {
                        setBit(idx, outputBitIndex);
                    }
                    else
                    {
                        clearBit(idx, outputBitIndex);
                    }
                }
            }
        }

        return *this;
    }

    //! Performs bits intersection. Takes bit value from volume and bit value from given volume, performs intersection and set the value to output bit.
    //! Volumes must be of the same size!
    //! \param maskVolume Volume, with which the intersection will be performed.
    //! \param sourceBitIndex Index of bit in current volume.
    //! \param maskSourceBitIndex Index of bit in given volume.
    //! \param outputBitIndex Index of bit in current volume, in which the result of the intersection will be written.
    CBitVolume& performIntersection(CBitVolume& maskVolume, vpl::tSize sourceBitIndex, vpl::tSize maskSourceBitIndex, vpl::tSize outputBitIndex)
    {
        const vpl::tSize sx = vpl::img::CVolume<tVoxel>::m_Size.x();
        const vpl::tSize sy = vpl::img::CVolume<tVoxel>::m_Size.y();
        const vpl::tSize sz = vpl::img::CVolume<tVoxel>::m_Size.z();
        const vpl::tSize Offset = vpl::img::CVolume<tVoxel>::getXOffset();

#pragma omp parallel for
        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                vpl::tSize idx = vpl::img::CVolume<tVoxel>::getIdx(0, j, k);
                for (vpl::tSize i = 0; i < sx; ++i, idx += Offset)
                {
                    if (at(idx, sourceBitIndex))
                    {
                        if (maskVolume.at(idx, maskSourceBitIndex))
                        {
                            setBit(idx, outputBitIndex);
                        }
                        else
                        {
                            clearBit(idx, outputBitIndex);
                        }
                    }
                }
            }
        }

        return *this;
    }

    //! Performs bits difference. Takes bit value from volume and bit value from given volume, performs difference and set the value to output bit.
    //! Volumes must be of the same size!
    //! \param maskVolume Volume, with which the difference will be performed.
    //! \param sourceBitIndex Index of bit in current volume.
    //! \param maskSourceBitIndex Index of bit in given volume.
    //! \param outputBitIndex Index of bit in current volume, in which the result of the difference will be written.
    CBitVolume& performDifference(CBitVolume& maskVolume, vpl::tSize sourceBitIndex, vpl::tSize maskSourceBitIndex, vpl::tSize outputBitIndex)
    {
        const vpl::tSize sx = vpl::img::CVolume<tVoxel>::m_Size.x();
        const vpl::tSize sy = vpl::img::CVolume<tVoxel>::m_Size.y();
        const vpl::tSize sz = vpl::img::CVolume<tVoxel>::m_Size.z();
        const vpl::tSize Offset = vpl::img::CVolume<tVoxel>::getXOffset();

#pragma omp parallel for
        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                vpl::tSize idx = vpl::img::CVolume<tVoxel>::getIdx(0, j, k);
                for (vpl::tSize i = 0; i < sx; ++i, idx += Offset)
                {
                    if (at(idx, sourceBitIndex))
                    {
                        if (maskVolume.at(idx, maskSourceBitIndex))
                        {
                            clearBit(idx, outputBitIndex);
                        }
                        else
                        {
                            setBit(idx, outputBitIndex);
                        }
                    }
                }
            }
        }

        return *this;
    }
};
} // namespace data

#endif // CBitVolume_H

  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
