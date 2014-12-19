///////////////////////////////////////////////////////////////////////////////
// $Id:$
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

#ifndef CCircularBuffer_H
#define CCircularBuffer_H

#include <VPL/Base/StaticData.h>
#include <VPL/Math/Base.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Circular buffer of a fixed length. If the buffer is full, stored data
//! are overwritten.

template <typename T, int Size>
class CCircularBuffer
{
public:
    //! Fixed buffer size.
    enum { MAX_SIZE = Size };
    
    //! Data storage type.
    typedef vpl::base::CStaticData<T, Size> tDataStorage;
    
    //! Item type.
    typedef typename tDataStorage::tData tData;
    typedef typename tDataStorage::tDataPtr tDataPtr;
    typedef typename tDataStorage::tConstData tConstData;
    typedef typename tDataStorage::tConstDataPtr tConstDataPtr;

public:
    //! Default constructor.
    inline CCircularBuffer();
    
    //! Constructor that initializes all items of the buffer.
    inline CCircularBuffer(const T& DefaultValue);
    
    //! Default destructor.
    inline ~CCircularBuffer();
    
    
    //! Returns current size of the buffer.
    int getSize() const { return m_Size; }
    int size() const { return m_Size; }
    
    //! Returns maximal allowed size of the buffer.
    int getCapacity() const { return MAX_SIZE; }
    int capacity() const { return MAX_SIZE; }
    
    //! Returns i-th latest item.
    //! - The oldest item in the buffer has the index getSize() - 1.
    inline T& at(int i);
    inline const T& at(int i) const;
    
    //! Returns i-th latest item.
    //! - The oldest item in the buffer has the index getSize() - 1.
    inline T& operator() (int i)
    {
        return at(i);
    }
    inline const T& operator() (int i) const
    {
        return at(i);
    }
    
    
    //! Inserts a new item to the buffer.
    inline void push(const T& Item);
    
    //! Adds a new item to the buffer and returns reference to it.
    inline T& push();
    
    //! Removes the oldest item in the buffer.
    inline void pop();
    
    
    //! Removes all items from the buffer.
    inline void clear();
    
    //! Sets values of all items.
    inline void fill();
    
    //! Sets values of all items.
    inline void fill(const T& Value);
    
    //! Calls a given function object for every item in the buffer.
    template <class Function>
    inline Function forEach(Function Func) const;

    //! Calls a given function object for n-th latest items in the buffer.
    template <class Function>
    inline Function forRange(int n, Function Func) const;

protected:
    //! Buffer data.
    tDataStorage m_Data;
    
    //! Current size of the buffer.
    int m_Size;
    
    //! Index of the last item.
    int m_Last;
    
    //! Default value returned when an item out of range is subscripted.
    tData m_DefaultValue;
};


///////////////////////////////////////////////////////////////////////////////
//

#include "CCircularBuffer.hxx"


} // namespace data

#endif // CCircularBuffer_H
