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

#include <geometry/base/CArmature.h>

namespace geometry
{

CArmature::CArmature(const geometry::Matrix &defaultMatrix)
    : CBone(defaultMatrix)
{ }

CArmature::~CArmature()
{ }

CArmature *CArmature::clone() const
{
    std::map<CBone *, CBone *> boneMapping;

    return clone(boneMapping);
}

CArmature *CArmature::clone(std::map<CBone *, CBone *> &boneMapping) const
{
    CArmature *copy = new CArmature;
    deepCopy(this, copy, boneMapping);
    return copy;
}

std::map<std::string, CAction> &CArmature::getActions()
{
    return m_actions;
}

const std::map<std::string, CAction> &CArmature::getActions() const
{
    return m_actions;
}

void CArmature::deepCopy(const CArmature *src, CArmature *dst, std::map<CBone *, CBone *> &boneMapping)
{
    CBone::deepCopy(src, dst, boneMapping);
    dst->m_actions = src->m_actions;
    for (std::map<std::string, CAction>::iterator ait = dst->m_actions.begin(); ait != dst->m_actions.end(); ++ait)
    {
        std::map<int, CKeyframe> &keyframes = ait->second.getKeyframes();
        for (std::map<int, CKeyframe>::iterator kit = keyframes.begin(); kit != keyframes.end(); ++kit)
        {
            std::map<CBone *, geometry::Matrix> oldMatrices = kit->second.getMatrices();
            std::map<CBone *, geometry::Matrix> &newMatrices = kit->second.getMatrices();
            newMatrices.clear();

            for (std::map<CBone *, geometry::Matrix>::iterator mit = oldMatrices.begin(); mit != oldMatrices.end(); ++mit)
            {
                newMatrices[boneMapping[mit->first]] = mit->second;
            }
        }
    }
}

} // namespace geometry
