///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2015 3Dim Laboratory s.r.o.
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

#ifndef CBONE_H
#define CBONE_H

////////////////////////////////////////////////////////////
// include
#include <osg/Geometry>
#include <geometry/base/types.h>

namespace geometry
{
    class CBone
    {
    protected:
        CBone *m_parent;
        std::vector<CBone *> m_children;
        geometry::Matrix m_defaultBoneMatrix;
        geometry::Matrix m_boneTransformationMatrix;
        geometry::Matrix m_objectToBoneMatrix;
        geometry::Matrix m_boneToObjectMatrix;
        geometry::Matrix m_objectToDefaultBoneMatrix;
        geometry::Matrix m_defaultBoneToObjectMatrix;
        bool m_dirty;

    public:
        CBone(const geometry::Matrix &matrix = geometry::Matrix::identity());
        ~CBone();

        CBone *clone(std::map<CBone *, CBone *> &boneMapping) const;
        CBone *clone() const;

    protected:
        static void deepCopy(const CBone *src, CBone *dst, std::map<CBone *, CBone *> &boneMapping);
        static void gatherBones(std::vector<geometry::CBone*> &bones, geometry::CBone *bone);

    public:
        static void gatherMatrices(std::vector<geometry::Matrix> &boneMatrices, geometry::CBone *bone);
        static CBone* getBone(int boneIndex, geometry::CBone *bone);
        static int getBoneIndex(geometry::CBone *bone, geometry::CBone *armature);

        void removeFromParent();
        bool addChild(CBone *child);
        bool removeChild(CBone *child);
        void removeChildren();
        bool isAscendantOf(CBone *descendant);
        bool isDescendantOf(CBone *ascendant);

        bool isLeaf() const;

        bool isDirty() const;
        void makeDirty();

        const geometry::Matrix &getDefaultBoneMatrix() const;
        void setDefaultBoneMatrix(const geometry::Matrix &matrix);
        const geometry::Matrix &getBoneTransformationMatrix() const;
        void setBoneTransformationMatrix(const geometry::Matrix &matrix);
        const geometry::Matrix &getObjectToBoneMatrix();
        const geometry::Matrix &getBoneToObjectMatrix();
        const geometry::Matrix &getObjectToDefaultBoneMatrix();
        const geometry::Matrix &getDefaultBoneToObjectMatrix();
        const std::vector<CBone *> &getChildren() const;
        CBone* getParent();

    private:
        void deleteChildren();
        void recalculateMatrices();

        friend class CModel;
    };
} // namespace data

#endif
