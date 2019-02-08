#include "data/CStorageIdRemapper.h"

#include <data/CModelManager.h>
#include "data/CModelCut.h"

data::CStorageIdRemapper::CStorageIdRemapper() : CStorageIdRemapperBase(), oldMaxImportedModels(20), oldMaxModels(24)
{
    const int oldBonesModel = 801;
    const int oldImportedModelCutSliceXY = 1520;
    const int oldImportedModelCutSliceXZ = 1540;
    const int oldImportedModelCutSliceYZ = 1560;

    for (int i = 0; i < oldMaxModels; i++)
    {
        m_idMap[oldBonesModel + i] = Storage::BonesModel::Id + i;
    }

    for (int i = 0; i < oldMaxImportedModels; i++)
    {
        m_idMap[oldImportedModelCutSliceXY + i] = Storage::ImportedModelCutSliceXY::Id + i;
        m_idMap[oldImportedModelCutSliceXZ + i] = Storage::ImportedModelCutSliceXZ::Id + i;
        m_idMap[oldImportedModelCutSliceYZ + i] = Storage::ImportedModelCutSliceYZ::Id + i;
    }
}