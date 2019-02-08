///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
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

#ifndef CColorTable_H
#define CColorTable_H

#include <osg/Array>

#include <QColor>
#include <random>
#include <limits>

namespace data
{
    class CColorTable
    {
    public:
        CColorTable();

        osg::Vec3 randomNext();

        static const osg::Vec3& get(int index);
        static int size();

    private:
        double m_lastHue;

        std::mt19937 m_mt;
        std::uniform_real_distribution<double> m_saturationRange;
        std::uniform_real_distribution<double> m_valueRange;

        static std::array<osg::Vec3, 16> m_colorTable;
    };
}
#endif
