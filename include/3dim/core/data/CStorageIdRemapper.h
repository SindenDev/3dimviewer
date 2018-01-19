#pragma once

#include <vector>


namespace data
{
    class CStorageIdRemapper
    {
    public:
        CStorageIdRemapper();
        //virtual ~CStorageIdRemapper() = default;
        virtual ~CStorageIdRemapper() {};

        int getId(int oldId) const;

    protected:
        const int OldMaxImportedModels;
        const int OldMaxModels;

        std::vector<int> m_idMap;
    };
}