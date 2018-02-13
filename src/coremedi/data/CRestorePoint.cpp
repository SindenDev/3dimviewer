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

#include <data/CRestorePoint.h>
#include <app/Signals.h>

data::CRestorePoint::CRestorePoint()
    : m_snapshot(NULL)
    , m_stored(false)
{ }

data::CRestorePoint::CRestorePoint(const CRestorePoint &other)
    : m_snapshot(other.m_snapshot)
    , m_storageIds(other.m_storageIds)
    , m_stored(other.m_stored)
{ }

data::CRestorePoint::~CRestorePoint()
{ }

data::CRestorePoint &data::CRestorePoint::operator=(const CRestorePoint &other)
{
    if (this != &other)
    {
        m_snapshot = other.m_snapshot;
        m_storageIds = other.m_storageIds;
        m_stored = other.m_stored;
    }
    return *this;
}

bool data::CRestorePoint::addSnapshot(data::CSnapshot *snapshot, int storageId)
{
    if (m_stored)
    {
        return false;
    }

    if (m_snapshot == NULL)
    {
        m_snapshot = snapshot;
        m_storageIds.insert(storageId);
        return true;
    }
    else
    {
        if (m_storageIds.find(storageId) == m_storageIds.end())
        {
            m_snapshot->addSnapshot(snapshot);
            m_storageIds.insert(storageId);
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool data::CRestorePoint::store()
{
    if (m_stored)
    {
        return false;
    }
    else
    {
        m_stored = true;
        VPL_SIGNAL(SigUndoSnapshot).invoke(m_snapshot);
        return true;
    }
}
