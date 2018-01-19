#ifndef CBONESNAPSHOT_H
#define CBONESNAPSHOT_H

#include <geometry/base/types.h>
#include <data/CSnapshot.h>


namespace data
{
    class CBoneSnapshot : public data::CSnapshot
    {
    public:
        geometry::Matrix m_defaultBoneMatrix;
        geometry::Matrix m_boneTransformationMatrix;

    public:
        CBoneSnapshot(data::CUndoProvider* provider = nullptr);
        virtual long getDataSize();
    };
}

#endif