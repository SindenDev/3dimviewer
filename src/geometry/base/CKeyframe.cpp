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

#include <geometry/base/CKeyframe.h>

namespace geometry
{

CKeyframe::CKeyframe()
{ }

CKeyframe::CKeyframe(const CKeyframe &other)
    : m_matrices(other.m_matrices)
{ }

CKeyframe::CKeyframe(CBone *bone)
{
    addMatricesFromBone(bone);
}

CKeyframe::~CKeyframe()
{ }

void CKeyframe::addMatricesFromBone(CBone *bone)
{
    if (bone == NULL)
    {
        return;
    }

    m_matrices[bone] = bone->getBoneTransformationMatrix();
    for (int i = 0; i < bone->getChildren().size(); ++i)
    {
        addMatricesFromBone(bone->getChildren()[i]);
    }
}

std::map<CBone *, Matrix> &CKeyframe::getMatrices()
{
    return m_matrices;
}

const std::map<CBone *, Matrix> &CKeyframe::getMatrices() const
{
    return m_matrices;
}

CKeyframe CKeyframe::operator+(const CKeyframe &other)
{
    CKeyframe result(*this);
    for (std::map<CBone *, Matrix>::const_iterator it = other.m_matrices.begin(); it != other.m_matrices.end(); ++it)
    {
        result.m_matrices[it->first] = it->second;
    }
    return result;
}

CKeyframe &CKeyframe::operator=(const CKeyframe &other)
{
    if (this != &other)
    {
        m_matrices = other.m_matrices;
    }
    return *this;
}

void CKeyframe::apply()
{
    for (std::map<CBone *, Matrix>::iterator it = m_matrices.begin(); it != m_matrices.end(); ++it)
    {
        it->first->setBoneTransformationMatrix(it->second);
    }
}

} // namespace geometry
