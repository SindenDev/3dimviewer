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

/**
 * \fn  bool CChangedEntries::hasChanged(const CChangedEntries::tFilter &Filter) const
 *
 * \brief   Check if any of given identifiers has changed
 *
 * \param   Filter  Specifies the set of tested ids.
 *
 * \return  true if changed, false if not.
**/

bool CChangedEntries::hasChanged(const CChangedEntries::tFilter &Filter) const
{
    // Empty filter? Do nothing
    if (Filter.size() == 0)
        return false;

    // For all changes
    for (CChangedEntries::tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
        //Test if filter contains this id
        if (Filter.find(it->first) != Filter.end())
            return true;

    return false;
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
    if ((Value == Mask) && (Value == 0))
    {
        if (Filter.size() == 0)
        {
            for (tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
            {
                if (0 != it->second)
                {
                    return false;
                }
            }
            return true;
        }
        else
        {
            tChanges filteredChanges;
            for (tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
            {
                if (Filter.find(it->first) != Filter.end())
                {
                    filteredChanges.insert(*it);
                }
            }

            if (filteredChanges.size() == 0)
            {
                return false;
            }
            else
            {
                for (tChanges::const_iterator it = filteredChanges.begin(); it != filteredChanges.end(); ++it)
                {
                    if (0 != it->second)
                    {
                        return false;
                    }
                }
                return true;
            }
        }
    }

    // normal test
    int Flags = 0x7fffffff;
    if (Filter.size() == 0)
    {
        for (tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
        {
            Flags &= it->second;
        }
    }
    else
    {
        for (tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
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
        if (Filter.size() == 0)
        {
            for (tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
            {
                if (0 == it->second)
                {
                    return true;
                }
            }
        }
        else
        {
            tChanges filteredChanges;
            for (tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
            {
                if (Filter.find(it->first) != Filter.end())
                {
                    filteredChanges.insert(*it);
                }
            }

            if (filteredChanges.size() == 0)
            {
                return false;
            }
            else
            {
                for (tChanges::const_iterator it = filteredChanges.begin(); it != filteredChanges.end(); ++it)
                {
                    if (0 == it->second)
                    {
                        return true;
                    }
                }
                return false;
            }
        }
    }

	// normal test
    int Flags = 0;
    if (Filter.size() == 0)
    {
        for (tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
        {
            Flags |= it->second;
        }
    }
    else
    {
        for (tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
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

bool CChangedEntries::checkFlagsAnyNonEq(int Value, int Mask, const CChangedEntries::tFilter &Filter) const
{
    if( m_Changes.empty() )
        return false;

    for (CChangedEntries::tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
    {
        if (Filter.empty() || Filter.find(it->first) != Filter.end())
        {
            int Flags = it->second;
			if (Value!=(Mask&Flags))
				return true;
        }
    }
	return false;
}

bool CChangedEntries::checkFlagsAnyEq(int Value, int Mask, const CChangedEntries::tFilter &Filter) const
{
    if( m_Changes.empty() )
        return false;

    for (CChangedEntries::tChanges::const_iterator it = m_Changes.begin(); it != m_Changes.end(); ++it)
    {
        if (Filter.empty() || Filter.find(it->first) != Filter.end())
        {
            int Flags = it->second;
			if (Value==(Mask&Flags))
				return true;
        }
    }
	return false;
}

//! Checks for presence of a change that has one or more flags from the Mask set
bool CChangedEntries::checkFlagsAnySet(int Mask, const CChangedEntries::tFilter &Filter) const
{
	return checkFlagsAnyNonEq(0,Mask,Filter);
}

//! Checks for presence of a change that has one or more flags from the Mask zero
bool CChangedEntries::checkFlagsAnyNotSet(int Mask, const CChangedEntries::tFilter &Filter) const
{
	return checkFlagsAnyNonEq(Mask,Mask,Filter);
}

//! Checks for presence of a change that has zero flags
bool CChangedEntries::checkFlagsAllZero(const CChangedEntries::tFilter &Filter) const
{
	return checkFlagsAnyEq(0,-1,Filter);
}

bool CChangedEntries::checkFlagsAllClassSpecificZero(const CChangedEntries::tFilter &Filter) const
{
	return checkFlagsAnyEq(0,(1 << 16) - 1,Filter);
}

//! Checks for presence of a change that has flags that mach the mask
bool CChangedEntries::checkFlagsAllSet(int Mask, const CChangedEntries::tFilter &Filter) const
{
	return checkFlagsAnyEq(Mask,Mask,Filter);
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
