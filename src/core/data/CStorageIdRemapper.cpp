#include "data/CStorageIdRemapper.h"

#include "data/CDataStorage.h"
#include <data/CModelManager.h>
#include "data/CModelCut.h"

data::CStorageIdRemapper::CStorageIdRemapper() : m_idMap(Storage::MAX_ID), OldMaxImportedModels(20), OldMaxModels(24)
{
    const int OldBonesModel = 801;
    const int OldImportedModelCutSliceXY = 1520;
    const int OldImportedModelCutSliceXZ = 1540;
    const int OldImportedModelCutSliceYZ = 1560;

    //c++11
    //std::iota(std::begin(m_idMap), std::end(m_idMap), 0);

    int n = 0;
    std::generate(std::begin(m_idMap), std::end(m_idMap), [&n]() {return n++; });

    for (int i = 0; i < OldMaxModels; i++)
    {
        m_idMap[OldBonesModel + i] = Storage::BonesModel::Id + i;
    }

    for (int i = 0; i < OldMaxImportedModels; i++)
    {
        m_idMap[OldImportedModelCutSliceXY + i] = Storage::ImportedModelCutSliceXY::Id + i;
        m_idMap[OldImportedModelCutSliceXZ + i] = Storage::ImportedModelCutSliceXZ::Id + i;
        m_idMap[OldImportedModelCutSliceYZ + i] = Storage::ImportedModelCutSliceYZ::Id + i;
    }
}

int data::CStorageIdRemapper::getId(int oldId) const
{
    return m_idMap[oldId];
}
