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


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
CCircularBuffer<T,Size>::CCircularBuffer()
    : m_Data()
    , m_Size(0)
    , m_Last(0)
    , m_DefaultValue()
{
    VPL_ASSERT(Size > 0);
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
CCircularBuffer<T,Size>::CCircularBuffer(const T& DefaultValue)
	: m_Data()
	, m_Size(0)
	, m_Last(0)
	, m_DefaultValue(DefaultValue)
{
    VPL_ASSERT(Size > 0);

    m_Data.fill(DefaultValue);
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
CCircularBuffer<T,Size>::~CCircularBuffer()
{
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
void CCircularBuffer<T,Size>::push(const T& Item)
{
	m_Data.at(m_Last) = Item;

    m_Last = (m_Last + 1) % MAX_SIZE;
    m_Size = (m_Size + 1) % (MAX_SIZE + 1);
}


///////////////////////////////////////////////////////////////////////////////
template <typename T, int Size> inline
T& CCircularBuffer<T,Size>::push()
{
	tData& Item = m_Data.at(m_Last);

    m_Last = (m_Last + 1) % MAX_SIZE;
    m_Size = (m_Size + 1) % (MAX_SIZE + 1);

	return Item;
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
void CCircularBuffer<T,Size>::pop()
{
	if( m_Size > 0 )
	{
		--m_Size;
	}
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
T& CCircularBuffer<T,Size>::at(int i)
{
    if( i >= m_Size )
    {
        return m_DefaultValue;
    }

    int idx = m_Last - i - 1;
    if ( idx < 0 )
    {
        idx += MAX_SIZE;
    }
    return m_Data.at(idx);
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
const T& CCircularBuffer<T,Size>::at(int i) const
{
    if( i >= m_Size )
    {
        return m_DefaultValue;
    }

    int idx = m_Last - i - 1;
    if ( idx < 0 )
    {
        idx += MAX_SIZE;
    }
    return m_Data.at(idx);
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
void CCircularBuffer<T,Size>::clear()
{
	m_Last = m_Size = 0;
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
void CCircularBuffer<T,Size>::fill()
{
	m_Last = m_Size = 0;

    m_Data.fill(m_DefaultValue);
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size> inline
void CCircularBuffer<T,Size>::fill(const T& Value)
{
	m_Last = m_Size = 0;

    m_Data.fill(Value);
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size>
template <class Function>
inline Function CCircularBuffer<T,Size>::forEach(Function Func) const
{
    int idx = m_Last - m_Size;
    int min = vpl::math::getMax(idx, 0);
    for( int i = m_Last - 1; i >= min; --i )
    {
        Func(m_Data.at(i));
    }
    if( idx < 0 )
    {
        min = idx + MAX_SIZE;
        for( int i = MAX_SIZE - 1; i >= min; --i )
        {
            Func(m_Data.at(i));
        }
    }
    return Func;
}


///////////////////////////////////////////////////////////////////////////////
//

template <typename T, int Size>
template <class Function>
inline Function CCircularBuffer<T,Size>::forRange(int n, Function Func) const
{
    int idx = m_Last - vpl::math::getMin(n, m_Size);
    int min = vpl::math::getMax(idx, 0);
    for( int i = m_Last - 1; i >= min; --i )
    {
        Func(m_Data.at(i));
    }
    if( idx < 0 )
    {
        min = idx + MAX_SIZE;
        for( int i = MAX_SIZE - 1; i >= min; --i )
        {
            Func(m_Data.at(i));
        }
    }
    return Func;
}
