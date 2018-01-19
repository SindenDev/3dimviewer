
#include <geometry/base/CBoneSnapshot.h>

data::CBoneSnapshot::CBoneSnapshot(data::CUndoProvider* provider)
    : data::CSnapshot(data::UNDO_ALL, provider)
    , m_defaultBoneMatrix(geometry::Matrix::identity())
    , m_boneTransformationMatrix(geometry::Matrix::identity())
{
}

long data::CBoneSnapshot::getDataSize()
{
    return sizeof(CBoneSnapshot);
}
