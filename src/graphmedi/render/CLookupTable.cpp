///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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


#include "render/CLookupTable.h"
#include <VPL/Math/Base.h>
#include <algorithm>


///////////////////////////////////////////////////////////////////////////////
// 
bool pointSort(const CLookupTablePoint pointA, const CLookupTablePoint pointB)
{
    return pointA.position() < pointB.position();
}


///////////////////////////////////////////////////////////////////////////////
// CLookupTablePoint
CLookupTablePoint::CLookupTablePoint(double position, osg::Vec4 color)
{
    vpl::math::limit<double>(position, 0.0, 1.0);
    m_position = position;
    m_color = color;
}

CLookupTablePoint::~CLookupTablePoint()
{ }

double CLookupTablePoint::position() const
{
    return m_position;
}

void CLookupTablePoint::setPosition(double position)
{
    vpl::math::limit<double>(position, 0.0, 1.0);

    m_position = position;
}

osg::Vec4 CLookupTablePoint::color() const
{
    return m_color;
}

void CLookupTablePoint::setColor(osg::Vec4 color)
{
    m_color = color;
}


///////////////////////////////////////////////////////////////////////////////
// CLookupTableComponent
CLookupTableComponent::CLookupTableComponent()
{
    m_name = "";
    m_alphaFactor = 1.0;
}

CLookupTableComponent::~CLookupTableComponent()
{ }

osg::Vec4 CLookupTableComponent::color(double position, bool withAlphaFactor) const
{
    vpl::math::limit<double>(position, 0.0, 1.0);
    
    osg::Vec4 color;
    
    if (m_points.size() == 0)
    {
        color = osg::Vec4(0.0, 0.0, 0.0, 0.0);
    }
    else if (m_points.size() == 1)
    {
        return m_points[0].color();
    }
    else
    {
        int index = 0;
        for (int i = 0; i < int(m_points.size()) - 1; ++i)
        {
            if (position > m_points[i].position())
            {
                index = i;
            }
        }

        double amount = (position - m_points[index].position()) / (m_points[index + 1].position() - m_points[index].position());
        vpl::math::limit<double>(amount, 0.0, 1.0);
        osg::Vec4 colorA = m_points[index].color();
        osg::Vec4 colorB = m_points[index + 1].color();
    
        double r = colorA.r() + amount * (colorB.r() - colorA.r());
        double g = colorA.g() + amount * (colorB.g() - colorA.g());
        double b = colorA.b() + amount * (colorB.b() - colorA.b());
        double a = colorA.a() + amount * (colorB.a() - colorA.a());
        if (withAlphaFactor)
        {
            a = a * m_alphaFactor;
        }

        color = osg::Vec4(r, g, b, a);
    }

    return color;
}

double CLookupTableComponent::alphaFactor() const
{
    return m_alphaFactor;
}

void CLookupTableComponent::setAlphaFactor(double alphaFactor)
{
    m_alphaFactor = alphaFactor;
}

int CLookupTableComponent::pointCount() const
{
    return m_points.size();
}

std::string CLookupTableComponent::name() const
{
    return m_name;
}

void CLookupTableComponent::setName(std::string name)
{
    m_name = name;
}

double CLookupTableComponent::pointPosition(int pointIndex) const
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return -1.0;
    }

    return m_points[pointIndex].position();
}

void CLookupTableComponent::setPointPosition(int pointIndex, double position)
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return;
    }

    m_points[pointIndex].setPosition(position);
}

osg::Vec4 CLookupTableComponent::pointColor(int pointIndex) const
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return osg::Vec4(0.0, 0.0, 0.0, 0.0);
    }

    return m_points[pointIndex].color();
}

void CLookupTableComponent::setPointColor(int pointIndex, osg::Vec4 color)
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return;
    }

    m_points[pointIndex].setColor(color);
}

void CLookupTableComponent::addPoint(double position, osg::Vec4 color)
{
    m_points.push_back(CLookupTablePoint(position, color));
    std::sort(m_points.begin(), m_points.end(), pointSort);
}

bool CLookupTableComponent::removePoint(int pointIndex)
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return false;
    }

    int i = 0;
    for (std::vector<CLookupTablePoint>::iterator it = m_points.begin(); it != m_points.end(); ++it)
    {
        if (i == pointIndex)
        {
            m_points.erase(it);
            break;
        }
        i++;
    }

    return true;
}

void CLookupTableComponent::setPoint(int pointIndex, double position, osg::Vec4 color)
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return;
    }

    m_points[pointIndex].setPosition(position);
    m_points[pointIndex].setColor(color);
    std::sort(m_points.begin(), m_points.end(), pointSort);
}

void CLookupTableComponent::clear()
{
    m_points.clear();    
}


///////////////////////////////////////////////////////////////////////////////
// CLookupTable
CLookupTable::CLookupTable()
{ }

CLookupTable::~CLookupTable()
{ }

osg::Vec4 CLookupTable::color(double position) const
{
    osg::Vec4 result;
    
    if (m_components.size() == 0)
    {
        return result;
    }
    else
    {
        result = m_components[0].color(position);

        for (std::size_t i = 1; i < m_components.size(); ++i)
        {
            result = blendColor(m_components[i].color(position), result);
        }
    }
    
    return result;
}

double CLookupTable::alphaFactor(int componentIndex) const
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return 1.0;
    }

    return m_components[componentIndex].alphaFactor();
}

void CLookupTable::setAlphaFactor(int componentIndex, double alphaFactor)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].setAlphaFactor(alphaFactor);
}

int CLookupTable::componentCount() const
{
    return m_components.size();
}

int CLookupTable::pointCount(int componentIndex) const
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return 0;
    }

    return m_components[componentIndex].pointCount();
}

std::string CLookupTable::name() const
{
    return m_name;
}

void CLookupTable::setName(std::string name)
{
    m_name = name;
}

std::string CLookupTable::name(int componentIndex) const
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return "";
    }

    return m_components[componentIndex].name();
}

void CLookupTable::setName(int componentIndex, std::string name)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].setName(name);
}

double CLookupTable::pointPosition(int componentIndex, int pointIndex) const
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return -1.0;
    }

    return m_components[componentIndex].pointPosition(pointIndex);
}

void CLookupTable::setPointPosition(int componentIndex, int pointIndex, double position)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].setPointPosition(pointIndex, position);
}

osg::Vec4 CLookupTable::pointColor(int componentIndex, int pointIndex) const
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return osg::Vec4(0.0, 0.0, 0.0, 0.0);
    }

    return m_components[componentIndex].pointColor(pointIndex);
}

void CLookupTable::setPointColor(int componentIndex, int pointIndex, osg::Vec4 color)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].setPointColor(pointIndex, color);
}

int CLookupTable::addComponent()
{
    m_components.resize(m_components.size() + 1);
    return m_components.size() - 1;
}

bool CLookupTable::removeComponent(int componentIndex)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return false;
    }

    int i = 0;
    for (std::vector<CLookupTableComponent>::iterator it = m_components.begin(); it != m_components.end(); ++it)
    {
        if (i == componentIndex)
        {
            m_components.erase(it);
            break;
        }
        i++;
    }

    return true;
}

void CLookupTable::addPoint(int componentIndex, double position, osg::Vec4 color)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].addPoint(position, color);
}

bool CLookupTable::removePoint(int componentIndex, int pointIndex)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return false;
    }

    return m_components[componentIndex].removePoint(pointIndex);
}

void CLookupTable::setPoint(int componentIndex, int pointIndex, double position, osg::Vec4 color)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].setPoint(pointIndex, position, color);
}

void CLookupTable::clear()
{
    m_components.clear();
}

void CLookupTable::clear(int componentIndex)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].clear();
}

osg::Vec4 CLookupTable::blendColor(osg::Vec4 colorA, osg::Vec4 colorB) const
{
    osg::Vec4 blendColor;

    blendColor.a() = blendAlpha(colorA.a(), colorB.a());
    blendColor.r() = 1.0 / blendColor.a() * blendColorComponent(colorA.r(), colorA.a(), colorB.r(), colorB.a());
    blendColor.g() = 1.0 / blendColor.a() * blendColorComponent(colorA.g(), colorA.a(), colorB.g(), colorB.a());
    blendColor.b() = 1.0 / blendColor.a() * blendColorComponent(colorA.b(), colorA.a(), colorB.b(), colorB.a());

    return blendColor;
}

float CLookupTable::blendColorComponent(float colorAComponent, float colorAAlpha, float colorBComponent, float colorBAlpha) const
{
    return colorAComponent * colorAAlpha + colorBComponent * colorBAlpha * (1.0 - colorAAlpha);
}

float CLookupTable::blendAlpha(float colorAAlpha, float colorBAlpha) const
{
    return colorAAlpha + colorBAlpha * (1.0 - colorAAlpha);
}
