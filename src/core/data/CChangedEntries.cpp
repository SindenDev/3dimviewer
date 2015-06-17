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

#include <data/CChangedEntries.h>
#include <data/CStorageEntry.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//

CChangedEntries::CChangedEntries(CStorageEntry* pEntry)
{
    if( pEntry )
    {
        pEntry->getChanges(*this);
    }
}

///////////////////////////////////////////////////////////////////////////////
//

CChangedEntries::CChangedEntries(const CChangedEntries& Changes)
    : m_Changes(Changes.m_Changes)
    , m_EntryId(Changes.m_EntryId)
{
}

///////////////////////////////////////////////////////////////////////////////
//

CChangedEntries& CChangedEntries::operator= (const CChangedEntries& Changes)
{
    m_Changes = Changes.m_Changes;
    m_EntryId = Changes.m_EntryId;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
//

bool CChangedEntries::checkExactFlagsAll(int Value, int Mask, tFilter Filter) const
{
    if( m_Changes.empty() )
    {
        return false;
    }
	// special case when testing for invalidation without flags
	if (Value == Mask && Value == 0) 
	{
		tChanges::const_iterator itEnd = m_Changes.end();
		if (Filter.size() == 0)
		{
			for( tChanges::const_iterator it = m_Changes.begin(); it != itEnd; ++it )
			{
				if (0!=it->second)
					return false;
			}
		}
		else
		{
			for( tChanges::const_iterator it = m_Changes.begin(); it != itEnd; ++it )
			{
				if (Filter.find(it->first) != Filter.end())
				{
					if (0!=it->second)
						return false;
				}
			}
		}
		return true;
	}

	// normal test
    int Flags = 0x7fffffff;
    tChanges::const_iterator itEnd = m_Changes.end();
    if (Filter.size() == 0)
    {
        for( tChanges::const_iterator it = m_Changes.begin(); it != itEnd; ++it )
        {
            Flags &= it->second;
        }
    }
    else
    {
        for( tChanges::const_iterator it = m_Changes.begin(); it != itEnd; ++it )
        {
            if (Filter.find(it->first) != Filter.end())
            {
                Flags &= it->second;
            }
        }
    }

	// NOTE: original implementation doesn't work always correctly -> fixed by additional masking by Value
#if(0)
    return ((Flags & Mask) == Value);	
#else
	if (0==Value)
		return ((Flags & Mask) == Value);
	else
		return ((Flags & Mask & Value) == Value); // mask by Value because default Mask is 0x7fffffff
#endif

}

///////////////////////////////////////////////////////////////////////////////
//

bool CChangedEntries::checkExactFlagsAny(int Value, int Mask, tFilter Filter) const
{
    if( m_Changes.empty() )
    {
        return false;
    }
	// special case when testing for invalidation without flags
	if (Value == Mask && Value == 0) 
	{
		tChanges::const_iterator itEnd = m_Changes.end();
		if (Filter.size() == 0)
		{
			for( tChanges::const_iterator it = m_Changes.begin(); it != itEnd; ++it )
			{
				if (0==it->second)
					return true;
			}
		}
		else
		{
			for( tChanges::const_iterator it = m_Changes.begin(); it != itEnd; ++it )
			{
				if (Filter.find(it->first) != Filter.end())
				{
					if (0==it->second)
						return true;
				}
			}
		}
		return false;
	}

	// normal test
    int Flags = 0;
    tChanges::const_iterator itEnd = m_Changes.end();
    if (Filter.size() == 0)
    {
        for( tChanges::const_iterator it = m_Changes.begin(); it != itEnd; ++it )
        {
            Flags |= it->second;
        }
    }
    else
    {
        for( tChanges::const_iterator it = m_Changes.begin(); it != itEnd; ++it )
        {
            if (Filter.find(it->first) != Filter.end())
            {
                Flags |= it->second;
            }
        }
    }

	// NOTE: original implementation doesn't work always correctly -> fixed by additional masking by Value
#if(0)
    return ((Flags & Mask) == Value);	
#else
	if (0==Value)
		return ((Flags & Mask) == Value);	
	else
		return ((Flags & Mask & Value) == Value); // mask by Value because default Mask is 0x7fffffff
#endif
}

///////////////////////////////////////////////////////////////////////////////
//

bool CChangedEntries::checkFlagAll(int Value) const
{
    return checkExactFlagsAll(Value, Value);
}

bool CChangedEntries::checkFlagAll(int Value, tFilter Filter) const
{
    return checkExactFlagsAll(Value, Value, Filter);
}

///////////////////////////////////////////////////////////////////////////////
//

bool CChangedEntries::checkFlagAny(int Value) const
{
    return checkExactFlagsAny(Value, Value);
}

bool CChangedEntries::checkFlagAny(int Value, tFilter Filter) const
{
    return checkExactFlagsAny(Value, Value, Filter);
}

///////////////////////////////////////////////////////////////////////////////
//

bool CChangedEntries::isParentValid() const
{
    if( m_Changes.size() == 1 )
    {
        return (m_Changes.begin()->first != m_EntryId);
    }
    else
    {
        return (m_Changes.size() != 0);
    }
}


} // namespace data
