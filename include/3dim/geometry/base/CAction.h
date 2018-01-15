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

#ifndef CACTION_H
#define CACTION_H

////////////////////////////////////////////////////////////
// include
#include <geometry/base/CKeyframe.h>
#include <geometry/base/types.h>

namespace geometry
{
    class CAction
    {
    protected:
        std::map<int, CKeyframe> m_keyframes;

    public:
        CAction();
        CAction(const CAction &other);
        ~CAction();

    public:
        CKeyframe sample(int frame);

        std::map<int, CKeyframe> &getKeyframes();
        const std::map<int, CKeyframe> &getKeyframes() const;

    private:
        Matrix matrixLerp(const Matrix &from, const Matrix &to, double weight);

    public:
        friend class CModel;
    };
} // namespace geometry

#endif
