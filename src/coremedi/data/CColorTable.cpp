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

#include <data/CColorTable.h>
#include <array>
#include <random>

const double goldenRatio = 0.618033988749895;

std::array<osg::Vec3, 16> data::CColorTable::m_colorTable =
{
    osg::Vec3(0.30, 0.85, 0.30),
    osg::Vec3(0.20, 0.80, 0.95),
    osg::Vec3(0.95, 0.95, 0.30),
    osg::Vec3(0.90, 0.30, 0.30),
    osg::Vec3(0.25, 0.45, 0.95),
    osg::Vec3(0.95, 0.60, 0.20),
    osg::Vec3(0.45, 0.90, 0.75),
    osg::Vec3(0.65, 0.50, 0.95),

    osg::Vec3(0.90, 0.45, 0.35),
    osg::Vec3(0.95, 0.80, 0.30),
    osg::Vec3(0.65, 0.90, 0.55),
    osg::Vec3(0.90, 0.30, 0.60),
    osg::Vec3(0.85, 0.70, 0.60),
    osg::Vec3(0.60, 0.95, 0.20),
    osg::Vec3(0.70, 0.75, 0.85),
    osg::Vec3(0.95, 0.75, 0.80),
};

data::CColorTable::CColorTable()
    : m_lastHue(0.0)
    , m_saturationRange(0.2, 0.8)
    , m_valueRange(0.85, 0.95)
{
}

osg::Vec3 data::CColorTable::randomNext()
{
    m_lastHue += goldenRatio;

    QColor color = QColor::fromHsvF(fmod(m_lastHue, 1.0), m_saturationRange(m_mt), m_valueRange(m_mt));

    return osg::Vec3(color.redF(), color.greenF(), color.blueF());
}

const osg::Vec3 & data::CColorTable::get(int index)
{
    return m_colorTable[index % 16];
}

int data::CColorTable::size()
{
    return m_colorTable.size();
}
