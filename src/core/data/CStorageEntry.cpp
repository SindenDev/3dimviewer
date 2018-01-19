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

#include <data/CStorageEntry.h>
#include <data/CEntryObserver.h>

//#define DEBUG_OUTPUT_DESTRUCTION

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Virtual destructor.
CStorageEntry::~CStorageEntry()
{
#ifdef DEBUG_OUTPUT_DESTRUCTION
  #ifdef _WIN32
    if (0!=m_Id)
    {
        std::stringstream ss;
        if (NULL != m_spData.get())
            ss << "Deleting storage entry " << m_Id << " " << typeid(*m_spData.get()).name() << "\n";
        else
            ss << "Deleting storage entry " << m_Id << "\n";
        std::string str = ss.str();
        OutputDebugStringA(str.c_str());
    }
  #endif
#endif
}

    ///////////////////////////////////////////////////////////////////////////////
//

void CStorageEntry::connect(CEntryObserver *pObserver)
{
    VPL_CHECK(pObserver, return);

    pObserver->connect(this);
}


///////////////////////////////////////////////////////////////////////////////
//

void CStorageEntry::disconnect(CEntryObserver *pObserver)
{
    VPL_CHECK(pObserver, return);

    pObserver->disconnect(this);
}


///////////////////////////////////////////////////////////////////////////////
//

void CStorageEntry::block(CEntryObserver *pObserver)
{
    VPL_CHECK(pObserver, return);

    pObserver->block(this);
}


///////////////////////////////////////////////////////////////////////////////
//

void CStorageEntry::unblock(CEntryObserver *pObserver)
{
    VPL_CHECK(pObserver, return);

    pObserver->unblock(this);
}


} // namespace data
