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

#ifndef CEntryObserver_H
#define CEntryObserver_H

#include <VPL/Base/Lock.h>
#include <VPL/Module/Signal.h>

#include "CStorageEntry.h"

// STL
#include <vector>


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Base class for all observers of storage entries.

class CEntryObserver : public vpl::base::CLockableObject<CEntryObserver>
{
public:
    //! Storage signal invoked on any change of an entry.
    typedef CStorageEntry::tSignal tSignal;

    //! Lock type.
    typedef vpl::base::CLockableObject<CEntryObserver>::CLock tLock;

public:
    //! Default constructor.
	CEntryObserver() {}

    //! Virtual destructor.
    //! - Disconnects the observer.
	virtual ~CEntryObserver()
    {
        tLock Lock(*this);

        tConnections::iterator itEnd = m_Connections.end();
        for( tConnections::iterator it = m_Connections.begin(); it != itEnd; ++it )
        {
            tSignal *pSignal = reinterpret_cast<tSignal *>(it->getSignalPtr());
            if( pSignal )
            {
                pSignal->disconnect(*it);
            }
        }
    }

    //! Virtual method called on any change of the entry.
	virtual void changed(CStorageEntry * pSubject) = 0;

    //! Connects the observer to a given storage entry.
    void connect(CStorageEntry * pEntry)
    {
        VPL_CHECK(pEntry, return);

        tLock Lock(*this);

        vpl::mod::CSignalConnection sc = pEntry->getSignal().connect(this, &CEntryObserver::changed);
        m_Connections.push_back(sc);
    }

    //! Disconnects the observer from a given storage entry.
    void disconnect(CStorageEntry * pEntry)
    {
        VPL_CHECK(pEntry, return);

        tLock Lock(*this);

        tConnections::iterator itEnd = m_Connections.end();
        for( tConnections::iterator it = m_Connections.begin(); it != itEnd; ++it )
        {
            // Try to disconnect...
            pEntry->getSignal().disconnect(*it);
        }
    }

    //! Blocks connection to a given entry.
    void block(CStorageEntry * pEntry)
    {
        VPL_CHECK(pEntry, return);

        tLock Lock(*this);

        tConnections::iterator itEnd = m_Connections.end();
        for( tConnections::iterator it = m_Connections.begin(); it != itEnd; ++it )
        {
            pEntry->getSignal().block(*it);
        }
    }

    //! Unblocks connection to a given entry.
    void unblock(CStorageEntry * pEntry)
    {
        VPL_CHECK(pEntry, return);

        tLock Lock(*this);

        tConnections::iterator itEnd = m_Connections.end();
        for( tConnections::iterator it = m_Connections.begin(); it != itEnd; ++it )
        {
            pEntry->getSignal().unblock(*it);
        }
    }

protected:
    //! Vector of all connections.
    typedef std::vector<vpl::mod::CSignalConnection> tConnections;

    //! Stored signal connections.
    tConnections m_Connections;

    // Friend class
    friend class CStorageEntry;
};


} // namespace data

#endif // CObserver_H

