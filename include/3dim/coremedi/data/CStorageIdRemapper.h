#ifndef CStorageIdRemapper_H
#define CStorageIdRemapper_H

#include "data/CStorageIdRemapperBase.h"

namespace data
{
    class CStorageIdRemapper : public CStorageIdRemapperBase
    {
    public:
        CStorageIdRemapper();

    protected:
        const int oldMaxImportedModels;
        const int oldMaxModels;
    };
}

#endif