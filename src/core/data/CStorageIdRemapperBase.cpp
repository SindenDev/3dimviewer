#include "data/CStorageIdRemapperBase.h"
#include "data/CDataStorage.h"
#include <numeric>

data::CStorageIdRemapperBase::CStorageIdRemapperBase() : m_idMap(Storage::MAX_ID) 
{
    std::iota(std::begin(m_idMap), std::end(m_idMap), 0);
}

int data::CStorageIdRemapperBase::getId(int oldId) const
{
    if (oldId < 0 || oldId >= m_idMap.size())
    {
        VPL_LOG_INFO("Trying to remap id out of range: " << oldId);        
#ifdef _WIN32
        if (IsDebuggerPresent())
        {
            __debugbreak();
        }
        else
        {
            assert(false);
        }
#else
        assert(false);
#endif
    }
    return m_idMap[oldId];
}
