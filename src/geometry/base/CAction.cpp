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

#include <geometry/base/CAction.h>

namespace geometry
{

CAction::CAction()
{ }

CAction::CAction(const CAction &other)
    : m_keyframes(other.m_keyframes)
{ }

CAction::~CAction()
{ }

Matrix CAction::matrixLerp(const Matrix &from, const Matrix &to, double weight)
{
    vpl::math::limit(weight, 0.0, 1.0);
    Vec3 tA, tB, t, sA, sB, s;
    Quat rA, rB, r;
    Matrix mA = from;
    Matrix mB = to;
    mA.decompose(tA, rA, sA);
    mB.decompose(tB, rB, sB);
    t = tA + (tB - tA) * geometry::Scalar(weight);
    s = sA + (sB - sA) * geometry::Scalar(weight);
    r = Quat::slerp(weight, rA, rB);
    return Matrix::translate(t) * Matrix::rotate(r) * Matrix::scale(s);
}

CKeyframe CAction::sample(int frame)
{
    CKeyframe keyframe;

    std::map<CBone *, std::vector<std::pair<int, Matrix> > > items;
    for (std::map<int, CKeyframe>::iterator it = m_keyframes.begin(); it != m_keyframes.end(); ++it)
    {
        for (std::map<CBone *, Matrix>::iterator bit = it->second.m_matrices.begin(); bit != it->second.m_matrices.end(); ++bit)
        {
            items[bit->first].push_back(std::pair<int, Matrix>(it->first, bit->second));
        }
    }

    for (std::map<CBone *, std::vector<std::pair<int, Matrix> > >::iterator it = items.begin(); it != items.end(); ++it)
    {
        int index0 = -1;
        int index1 = -1;
        for (int i = 0; i < it->second.size(); ++i)
        {
            if (it->second[i].first <= frame)
            {
                index0 = i;
            }
        }
        if (index0 == -1)
        {
            continue;
        }
        int maxIndex = it->second.size() - 1;
        index1 = std::min(index0 + 1, maxIndex);

        int frameA = it->second[index0].first;
        int frameB = it->second[index1].first;
        Matrix mA = it->second[index0].second;
        Matrix mB = it->second[index1].second;

        Matrix m = matrixLerp(mA, mB, frameB - frameA == 0 ? 0.0 : static_cast<double>(frame - frameA) / (frameB - frameA));
        keyframe.getMatrices()[it->first] = m;
    }

    return keyframe;
}

std::map<int, CKeyframe> &CAction::getKeyframes()
{
    return m_keyframes;
}

const std::map<int, CKeyframe> &CAction::getKeyframes() const
{
    return m_keyframes;
}

} // namespace geometry
