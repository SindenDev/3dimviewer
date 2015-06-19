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

#ifndef CObjectObserver_H
#define CObjectObserver_H

#include "CEntryObserver.h"
#include "CChangedEntries.h"
#include "CObjectHolder.h"

#include <VPL/Base/Logging.h>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Base class for all observers of data entries.
//! - Only for objects encapsulated in CObjectHolder<T> class!

template <class T>
class CObjectObserver : public CEntryObserver
{
public:
    //! Object type.
    typedef T tObject;
    
    //! Storable data type.
    typedef CObjectHolder<T> tObjectHolder;

public:
    //! Default constructor.
    CObjectObserver() : m_curID(0) {}

    //! Virtual destructor.
	virtual ~CObjectObserver() {}

    //! Virtual method called on any change of the entry.
    //! - Gets the version number and pointer to the data.
	virtual void changed(CStorageEntry * pEntry)
    {
        if( !pEntry )
        {
            return;
        }

        try {
            tObject *pObject = pEntry->getDataPtr<tObjectHolder>()->getObjectPtr();
			m_curID = pEntry->getId();
            m_NewVersion[m_curID] = pEntry->getLatestVersion();
            pEntry->getChanges(m_Version[m_curID], m_Changes);
            
            vpl::sys::tScopedLock Lock(pEntry->getDataLock());
            objectChanged(pObject);
        }
        catch( const vpl::base::CException& Exception )
        {
            VPL_LOG_INFO("CObjectObserver::changed(): " << Exception.what());
        }
    }

    //! Virtual method called on any change of the entry.
    virtual void objectChanged(tObject *pObject) = 0;

    //! Returns true if the observed object has changed since the last call
    //! of this method.
    bool hasChangedAll()
    {
		if (0 == m_NewVersion.size()) // no data -> use default implementation for one
			return hasChanged();
		bool bChanged = false;
		for(auto it = m_Version.begin(), ite = m_Version.end(); it!=ite; ++it)
		{			
			int id = it->first;
			if( m_NewVersion[id] != m_Version[id] || m_Version.find(id) == m_Version.end() )
			{
				m_Version[id] = m_NewVersion[id];
				bChanged = true;
			}
		}
        return bChanged;
    }

	//! Returns true if the observed object has changed since the last call
    //! of this method.
    bool hasChanged()
    {
		if( m_Version.find(m_curID) == m_Version.end() || m_NewVersion[m_curID] != m_Version[m_curID])
        {
            m_Version[m_curID] = m_NewVersion[m_curID];
            return true;
        }
        return false;
    }

	//! Returns current entry ID
	int getCurrentID() const { return m_curID; } 

    //! Returns changes made since the previous version.
    void getChanges(CChangedEntries& Changes)
    {
        CEntryObserver::tLock Lock(*this);

        Changes = m_Changes;
    }

    //! Clears the changes.
    void clearChanges()
    {
        CEntryObserver::tLock Lock(*this);

        m_Changes.clear();
    }

protected:
    //! Previous and latest version of the object.
	std::map<int, unsigned> m_Version;
	std::map<int, unsigned> m_NewVersion;

	//! Currently handled ID
	int m_curID;

    //! Changes since the previous version.
    CChangedEntries m_Changes;
};


} // namespace data

#endif // CObserver_H
