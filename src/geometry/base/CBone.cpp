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

#include <geometry/base/CBone.h>

namespace geometry
{

CBone::CBone(const geometry::Matrix &defaultMatrix)
    : m_parent(NULL)
    , m_defaultBoneMatrix(defaultMatrix)
    , m_boneTransformationMatrix(geometry::Matrix::identity())
    , m_objectToBoneMatrix(geometry::Matrix::identity())
    , m_boneToObjectMatrix(geometry::Matrix::identity())
    , m_objectToDefaultBoneMatrix(geometry::Matrix::identity())
    , m_defaultBoneToObjectMatrix(geometry::Matrix::identity())
    , m_dirty(true)
{
    recalculateMatrices();
}

CBone::~CBone()
{
    removeFromParent();
    deleteChildren();
}

CBone *CBone::clone() const
{
    std::map<CBone *, CBone *> boneMapping;

    return clone(boneMapping);
}

CBone *CBone::clone(std::map<CBone *, CBone *>& boneMapping) const
{
    CBone *copy = new CBone;
    deepCopy(this, copy, boneMapping);
    return copy;
}

void CBone::deepCopy(const CBone *src, CBone *dst, std::map<CBone *, CBone *> &boneMapping)
{
    boneMapping[const_cast<CBone *>(src)] = dst;
    dst->m_defaultBoneMatrix = src->m_defaultBoneMatrix;
    dst->m_boneTransformationMatrix = src->m_boneTransformationMatrix;
    dst->m_objectToBoneMatrix = src->m_objectToBoneMatrix;
    dst->m_boneToObjectMatrix = src->m_boneToObjectMatrix;
    dst->m_objectToDefaultBoneMatrix = src->m_objectToDefaultBoneMatrix;
    dst->m_defaultBoneToObjectMatrix = src->m_defaultBoneToObjectMatrix;
    dst->m_dirty = src->m_dirty;
    for (int i = 0; i < src->m_children.size(); ++i)
    {
        dst->addChild(src->m_children[i]->clone(boneMapping));
    }
}

void CBone::setDefaultBoneMatrix(const geometry::Matrix &matrix)
{
    m_defaultBoneMatrix = matrix;
    makeDirty();
}

const geometry::Matrix &CBone::getDefaultBoneMatrix() const
{
    return m_defaultBoneMatrix;
}

const geometry::Matrix &CBone::getBoneTransformationMatrix() const
{
    return m_boneTransformationMatrix;
}

void CBone::setBoneTransformationMatrix(const geometry::Matrix &matrix)
{
    m_boneTransformationMatrix = matrix;
    makeDirty();
}

const geometry::Matrix &CBone::getObjectToDefaultBoneMatrix()
{
    recalculateMatrices();
    return m_objectToDefaultBoneMatrix;
}

const geometry::Matrix &CBone::getDefaultBoneToObjectMatrix()
{
    recalculateMatrices();
    return m_defaultBoneToObjectMatrix;
}

const geometry::Matrix &CBone::getObjectToBoneMatrix()
{
    recalculateMatrices();
    return m_objectToBoneMatrix;
}

const geometry::Matrix &CBone::getBoneToObjectMatrix()
{
    recalculateMatrices();
    return m_boneToObjectMatrix;
}

const std::vector<CBone *> &CBone::getChildren() const
{
    return m_children;
}

void CBone::makeDirty()
{
    m_dirty = true;
    for (int i = 0; i < m_children.size(); ++i)
    {
        m_children[i]->makeDirty();
    }
}

bool CBone::addChild(CBone *child)
{
    if (NULL == child)
    {
        return false;
    }

    if (child->isAscendantOf(this))
    {
        return false;
    }

    child->removeFromParent();
    child->m_parent = this;
    m_children.push_back(child);
    child->makeDirty();

    return true;
}

bool CBone::removeChild(CBone *child)
{
    for (std::vector<CBone *>::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        if (*it == child)
        {
            child->m_parent = NULL;
            m_children.erase(it);
            child->makeDirty();
            return true;
        }
    }

    return false;
}

bool CBone::isAscendantOf(CBone *descendant)
{
    CBone *curr = descendant;
    while (curr != NULL)
    {
        if (curr == this)
        {
            return true;
        }

        curr = curr->m_parent;
    }

    return false;
}

bool CBone::isDescendantOf(CBone *ascendant)
{
    return ascendant->isAscendantOf(this);
}

void CBone::removeFromParent()
{
    if (NULL == m_parent)
    {
        return;
    }

    m_parent->removeChild(this);
}

void CBone::removeChildren()
{
    while (m_children.size() > 0)
    {
        removeChild(m_children.front());
    }
}

void CBone::deleteChildren()
{
    while (m_children.size() > 0)
    {
        CBone *curr = m_children[0];
        removeChild(curr);
        delete curr;
    }
}

bool CBone::isLeaf() const
{
    return !m_children.empty();
}

bool CBone::isDirty() const
{
    return m_dirty;
}

void CBone::recalculateMatrices()
{
    if (m_dirty)
    {
        m_defaultBoneToObjectMatrix = (m_parent != NULL ? m_parent->getDefaultBoneToObjectMatrix() : geometry::Matrix::identity()) * m_defaultBoneMatrix;
        m_objectToDefaultBoneMatrix = geometry::Matrix::inverse(m_defaultBoneToObjectMatrix);

        m_boneToObjectMatrix = (m_parent != NULL ? m_parent->getBoneToObjectMatrix() : geometry::Matrix::identity()) * m_defaultBoneMatrix * m_boneTransformationMatrix;
        m_objectToBoneMatrix = geometry::Matrix::inverse(m_boneToObjectMatrix);

        m_dirty = false;
    }
}

void CBone::gatherMatrices(std::vector<geometry::Matrix> &boneMatrices, geometry::CBone *bone)
{
    boneMatrices.push_back(bone->getBoneToObjectMatrix() * bone->getObjectToDefaultBoneMatrix());

    for (int i = 0; i < bone->getChildren().size(); ++i)
    {
        gatherMatrices(boneMatrices, bone->getChildren()[i]);
    }
}

void CBone::gatherBones(std::vector<geometry::CBone*> &bones, geometry::CBone *bone)
{
    bones.push_back(bone);

    for (int i = 0; i < bone->getChildren().size(); ++i)
    {
        gatherBones(bones, bone->getChildren()[i]);
    }
}

CBone* CBone::getBone(int boneIndex, geometry::CBone *bone)
{
    if (boneIndex < 0)
        return NULL;

    std::vector<geometry::CBone*> bones;

    gatherBones(bones, bone);

    if (boneIndex >= bones.size())
        return NULL;

    return bones[boneIndex];
}

CBone* CBone::getParent()
{
    return m_parent;
}

int CBone::getBoneIndex(geometry::CBone *bone, geometry::CBone *armature)
{
    std::vector<geometry::CBone*> bones;
    gatherBones(bones, armature);

    for (int i = 0; i < bones.size(); ++i)
    {
        if (bones[i] == bone)
            return i;
    }

    return -1;
}

} // namespace geometry
