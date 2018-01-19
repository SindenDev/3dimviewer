///////////////////////////////////////////////////////////////////////////////
// $Id:$
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

#include <data/CEntryChangeLog.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//

CEntryChangeLog::CEntryChangeLog() : m_Version(2)
{
    m_Log.push( CEntryRecord(Storage::UNKNOWN, 0) );
    m_Log.push( CEntryRecord(Storage::UNKNOWN, 0) );
}

///////////////////////////////////////////////////////////////////////////////
//

bool CEntryChangeLog::getChanges(unsigned VersionNum, CChangedEntries& Changes) const
{
    Changes.clear();

    if( VersionNum >= m_Version )
    {
        return true;
    }

    int Count = int(m_Version - VersionNum);
    m_Log.forRange( Count, SGetChanges(Changes) );

    return (Count <= m_Log.getSize()) ? true : false;
}


///////////////////////////////////////////////////////////////////////////////
//

CEntryChangeLog& CEntryChangeLog::insert(const CChangedEntries & Changes)
{
    CChangedEntries::tChanges::const_iterator itEnd = Changes.getImpl().end();
    CChangedEntries::tChanges::const_iterator it = Changes.getImpl().begin();
    for( ; it != itEnd; ++it )
    {
        m_Log.push( CEntryRecord(it->first, it->second) );
        ++m_Version;
    }

    return *this;
}


} // namespace data
