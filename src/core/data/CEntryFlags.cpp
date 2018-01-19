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

#include <data/CEntryFlags.h>

#include <VPL/Base/Logging.h>
#include <VPL/Base/Warning.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//

CEntryFlags& CEntryFlags::insert(int Flags)
{
    if( int(m_List.size()) > MAX_SIZE )
    {
        VPL_LOG_WARN("CEntryFlags::insert(): Too many items in the history");
        return *this;
    }

    m_List.insert(Flags);
    return *this;
}


///////////////////////////////////////////////////////////////////////////////
//

CEntryFlags& CEntryFlags::insert(const CEntryFlags& List)
{
    if( int(m_List.size() + List.m_List.size()) > MAX_SIZE )
    {
        VPL_LOG_WARN("CEntryFlags::insert(): Too many items in the history");
        return *this;
    }

    m_List.insert(List.m_List.begin(), List.m_List.end());
    return *this;
}


///////////////////////////////////////////////////////////////////////////////
//

struct CCalcAnd
{
    CCalcAnd(int Value) : m_Value(Value) {}

    void operator() (int Flags)
    {
        m_Value &= Flags;
    }

    int getResult() const { return m_Value; }

    int m_Value;
};


bool CEntryFlags::checkFlagAll(int Value) const
{
    static const int INIT_VALUE = 0x7fffffff;

    if( m_List.empty() )
    {
        return false;
    }

    int Flags = forEach(CCalcAnd(INIT_VALUE)).getResult();

    return ((Value & Flags) == Value);
}


///////////////////////////////////////////////////////////////////////////////
//

struct CCalcOr
{
    CCalcOr(int Value) : m_Value(Value) {}

    void operator() (int Flags)
    {
        m_Value |= Flags;
    }

    int getResult() const { return m_Value; }

    int m_Value;
};


bool CEntryFlags::checkFlagAny(int Value) const
{
    static const int INIT_VALUE = 0;

    if( m_List.empty() )
    {
        return false;
    }

    int Flags = forEach(CCalcOr(INIT_VALUE)).getResult();

    return ((Value & Flags) == Value);
}


} // namespace data
