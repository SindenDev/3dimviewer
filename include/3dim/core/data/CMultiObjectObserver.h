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

#ifndef CMultiObjectObserver_H
#define CMultiObjectObserver_H

#include "CObjectObserver.h"

// STL
#include <vector>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Global definitions

namespace MultiObjectObserver
{
    //! Flags representing changed objects.
    enum
    {
        NO_CHANGES  = 0,
        FIRST       = 1 << 0,
        SECOND      = 1 << 1
    };
}


///////////////////////////////////////////////////////////////////////////////
//! Base class for observers of data entries.
//! - Only for objects encapsulated in CObjectHolder<T> class!

template <class T1, class T2>
class CMultiObjectObserver : public CEntryObserver
{
public:
    //! Object type.
    typedef T1 tObject1;
    typedef T2 tObject2;
    
    //! Storable data type.
    typedef CObjectHolder<T1> tObjectHolder1;
    typedef CObjectHolder<T2> tObjectHolder2;

public:
    //! Default constructor.
    CMultiObjectObserver()
		: m_curID1(0)
		, m_curID2(0)
    {}

    //! Virtual destructor.
	virtual ~CMultiObjectObserver() {}

    //! Virtual method called on any change of the entry.
    //! - Gets the version number and pointer to the data.
	virtual void changed(CStorageEntry * pEntry)
    {
        if( !pEntry )
        {
            return;
        }

        try {
            if( pEntry->checkType<tObjectHolder1>() )
            {
                tObject1 *pObject1 = pEntry->getDataPtr<tObjectHolder1>()->getObjectPtr();
				m_curID1 = pEntry->getId();
                m_NewVersion1[m_curID1] = pEntry->getLatestVersion();
                pEntry->getChanges(m_Version1[m_curID1], m_Changes1);
                
                vpl::sys::tScopedLock Lock(pEntry->getDataLock());
                objectChanged(pObject1);
            }
            else if( pEntry->checkType<tObjectHolder2>() )
            {
                tObject2 *pObject2 = pEntry->getDataPtr<tObjectHolder2>()->getObjectPtr();
				m_curID2 = pEntry->getId();
                m_NewVersion2[m_curID2] = pEntry->getLatestVersion();
                pEntry->getChanges(m_Version2[m_curID2], m_Changes2);
                
                vpl::sys::tScopedLock Lock(pEntry->getDataLock());
                objectChanged(pObject2);
            }
        }
        catch( const vpl::base::CException& Exception )
        {
            VPL_LOG_INFO("CMultiObjectObserver::changed(): " << Exception.what());
        }
    }

    //! Virtual method called on any change of the first object.
    virtual void objectChanged(tObject1 *pObject) = 0;

    //! Virtual method called on any change of the second object.
    virtual void objectChanged(tObject2 *pObject) = 0;

    //! Returns a non-zero value if one of the observed objects have changed
    //! since the last call of this method.
    //! - Returned value represents changed objects (see predefined flags).
    int hasChangedAll()
    {
        int Aux = 0;
		if (0 == m_Version1.size())
		{
			Aux |= MultiObjectObserver::FIRST;
			m_Version1[m_curID1] = m_NewVersion1[m_curID1];
		}
		for(auto it = m_Version1.begin(), ite = m_Version1.end(); it!=ite; ++it)
		{			
			int id = it->first;
			if( m_Version1.find(id) == m_Version1.end() || m_NewVersion1[id] != m_Version1[id] )
			{
				Aux |= MultiObjectObserver::FIRST;
				m_Version1[id] = m_NewVersion1[id];
			}
		}
		if (0 == m_Version2.size())
		{
			Aux |= MultiObjectObserver::SECOND;
			m_Version2[m_curID2] = m_NewVersion2[m_curID2];
		}
		for(auto it = m_Version2.begin(), ite = m_Version2.end(); it!=ite; ++it)
		{
			int id = it->first;
			if( m_Version2.find(id) == m_Version2.end() || m_NewVersion2[id] != m_Version2[id] )
			{
				Aux |= MultiObjectObserver::SECOND;
				m_Version2[id] = m_NewVersion2[id];
			}
		}
        return Aux;
    }

    //! Returns a non-zero value if one of the observed objects have changed
    //! since the last call of this method.
    //! - Returned value represents changed objects (see predefined flags).
    int hasChanged()
    {
        int Aux = 0;
		if( m_Version1.find(m_curID1) == m_Version1.end() || m_NewVersion1[m_curID1] != m_Version1[m_curID1] )
		{
			Aux |= MultiObjectObserver::FIRST;
			m_Version1[m_curID1] = m_NewVersion1[m_curID1];
		}
		if( m_Version2.find(m_curID2) == m_Version2.end() || m_NewVersion2[m_curID2] != m_Version2[m_curID2] )
		{
			Aux |= MultiObjectObserver::SECOND;
			m_Version2[m_curID2] = m_NewVersion2[m_curID2];
		}
        return Aux;
    }    

    //! Returns changes since the previous version.
    //! - Only the first or the second object can be specified!
    void getChanges(int ObjectNum, CChangedEntries& Changes)
    {
        CEntryObserver::tLock Lock(*this);

        Changes = (ObjectNum & MultiObjectObserver::FIRST) ? m_Changes1 : m_Changes2;
    }

    //! Clears the changes made since the previous version.
    //! - Both objects can be specified at once!
    void clearChanges(int ObjectNum)
    {
    	CEntryObserver::tLock Lock(*this);

        if( ObjectNum & MultiObjectObserver::FIRST )
        {
            m_Changes1.clear();
        }        
        if( ObjectNum & MultiObjectObserver::SECOND )
        {
            m_Changes2.clear();
        }
    }

protected:
    //! Previous and latest version of the object.
    std::map<int,unsigned> m_Version1;
	std::map<int,unsigned> m_NewVersion1;

    //! Changes since the previous version.
    CChangedEntries m_Changes1;

    //! Previous and latest version of the second object.
    std::map<int,unsigned> m_Version2;
	std::map<int,unsigned> m_NewVersion2;

    //! Changes since the previous version.
    CChangedEntries m_Changes2;

	//! Current IDs
	int m_curID1, m_curID2;
};


} // namespace data

#endif // CObserver_H
