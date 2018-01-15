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

#ifndef CARMATURE_H
#define CARMATURE_H

////////////////////////////////////////////////////////////
// include
#include <geometry/base/CBone.h>
#include <geometry/base/CAction.h>

namespace geometry
{
    class CArmature : public CBone
    {
    protected:
        std::map<std::string, CAction> m_actions;

    public:
        CArmature(const geometry::Matrix &matrix = geometry::Matrix::identity());
        ~CArmature();

        CArmature *clone(std::map<CBone *, CBone *> &boneMapping) const;
        CArmature *clone() const;

        std::map<std::string, CAction> &getActions();
        const std::map<std::string, CAction> &getActions() const;

    protected:
        static void deepCopy(const CArmature *src, CArmature *dst, std::map<CBone *, CBone *> &boneMapping);

    };
} // namespace data

#endif
