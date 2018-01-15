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

#ifndef CGeneralObjectObserver_H
#define CGeneralObjectObserver_H

#include "CEntryObserver.h"
#include "CChangedEntries.h"
#include "CObjectHolder.h"
#include <VPL/Base/Logging.h>
#include <set>

namespace data
{
    // Helper find method for id range
    template<class InputIt, class T>
    InputIt find(InputIt first, InputIt last, const T& valueMin, int cnt)
    {
        for (; first != last; ++first) 
        {
            if (*first >= valueMin && *first <= valueMin + cnt)
            {
                return first;
            }
        }
        return last;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //! Class for general observers of data entries - this observer can handle
    //! multiple entries of multiple types, but user has to check type in
    //! objectChanged(...) method
    //! - Only for objects encapsulated in CObjectHolder<T> class!
    template<class T>
    class CGeneralObjectObserver : public CEntryObserver
    {
    public:
        typedef void (T::*tObserverMemFuncPtr)(data::CStorageEntry *, const CChangedEntries &);
        typedef vpl::base::CFunctor<void, data::CStorageEntry *, const CChangedEntries &> tObserverHandler;
        typedef CGeneralObjectObserver<T> tObserverType;

        //! Default constructor.
        CGeneralObjectObserver()
            : m_changesHandledAutoInvoke(true)
        { }

        //! Virtual destructor.
        virtual ~CGeneralObjectObserver()
        { }

        //! Connects the observer to a given storage entry.
        void connect(CStorageEntry *pEntry)
        {
            CEntryObserver::connect(pEntry);
            m_changedEntries.insert(pEntry->getId());
            m_newVersion[pEntry->getId()] = pEntry->getLatestVersion();
        }

        //! Connects the observer to a given storage entry.
        void connect(CStorageEntry *pEntry, tObserverHandler handler)
        {
            CEntryObserver::connect(pEntry);
            m_handlers[pEntry->getId()] = handler;
            m_changedEntries.insert(pEntry->getId());
            m_newVersion[pEntry->getId()] = pEntry->getLatestVersion();
        }

        //! Disconnects the observer from a given storage entry.
        void disconnect(CStorageEntry *pEntry)
        {
            CEntryObserver::disconnect(pEntry);
            m_handlers.erase(pEntry->getId());
        }

        //! Virtual method called on any change of the entry.
        //! - Gets the version number and pointer to the data.
        virtual void changed(CStorageEntry *pEntry)
        {
            if (!pEntry)
            {
                return;
            }

            try
            {
                m_changedEntries.insert(pEntry->getId());
                m_newVersion[pEntry->getId()] = pEntry->getLatestVersion();

                vpl::sys::tScopedLock Lock(pEntry->getDataLock());

                CChangedEntries changes;
                getChanges(pEntry, changes);
                if (m_changesHandledAutoInvoke)
                {
                    changesHandled(pEntry->getId());
                }

                std::map<int, tObserverHandler>::iterator handlerIt = m_handlers.find(pEntry->getId());
                if (handlerIt != m_handlers.end())
                {
                    handlerIt->second.operator()(pEntry, changes);
                }
                else
                {
                    objectChanged(pEntry, changes);
                }
            }
            catch (const vpl::base::CException& Exception)
            {
                VPL_LOG_INFO("CGeneralObjectObserver::changed(): " << Exception.what());
            }
        }

        //! Virtual method called on any change of the entry.
        virtual void objectChanged(CStorageEntry *pEntry, const CChangedEntries &changes) = 0;

        //! Returns true if the observed object has changed since the last call of this method.
        bool hasChanged(int id = -1)
        {
            CEntryObserver::tLock Lock(*this);
            if (id == -1)
            {
                return !m_changedEntries.empty();
            }
            else
            {
                return m_changedEntries.find(id) != m_changedEntries.end();
            }
        }

        //! Gets set of changed entries
        void getChangedEntries(std::set<int> &changedEntries)
        {
            CEntryObserver::tLock Lock(*this);
            changedEntries = m_changedEntries;
        }

        //! Returns true if the observed object has changed since the last call of this method.
        void changesHandled(int id = -1)
        {
            CEntryObserver::tLock Lock(*this);

            if (id == -1)
            {
                for (std::set<int>::iterator it = m_changedEntries.begin(); it != m_changedEntries.end(); ++it)
                {
                    id = *it;
                    if (m_version.find(id) == m_version.end() || m_newVersion[id] != m_version[id])
                    {
                        m_version[id] = m_newVersion[id];
                    }
                }
                m_changedEntries.clear();
            }
            else
            {
                if (m_version.find(id) == m_version.end() || m_newVersion[id] != m_version[id])
                {
                    m_version[id] = m_newVersion[id];
                }
                m_changedEntries.erase(id);
            }
        }

        //! Returns changes made since the previous version.
        void getChanges(CStorageEntry *pEntry, CChangedEntries &changes)
        {
            CEntryObserver::tLock Lock(*this);
            pEntry->getChanges(m_version[pEntry->getId()], changes);
        }

    protected:
        //! Previous and latest version of the object.
        std::set<int> m_changedEntries;
        std::map<int, unsigned> m_version;
        std::map<int, unsigned> m_newVersion;
        std::map<int, tObserverHandler> m_handlers;

        //! Flag if changesHandled() method should be invoked automatically
        //! - True - every change should be handled immediately by the observer,
        //!          m_changedEntries will be always empty or with one item only
        //! - False - changes cumulate and will be handled later by the observer,
        //!           m_changedEntries can have multiple items at once, this is
        //!           typically used for OSG stuff reacting on storage changes
        bool m_changesHandledAutoInvoke;
    };
} // namespace data

#endif // CGeneralObserver_H
