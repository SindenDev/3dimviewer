
#ifndef CStorageIdRemapperBase_H
#define CStorageIdRemapperBase_H

#include <vector>

namespace data
{
    class CStorageIdRemapperBase
    {
    public:
        CStorageIdRemapperBase();
        virtual ~CStorageIdRemapperBase() = default;

        int getId(int oldId) const;

    protected:
        std::vector<int> m_idMap;
    };
}

#endif