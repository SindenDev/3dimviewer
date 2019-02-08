///////////////////////////////////////////////////////////////////////////////
// $Id$
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
CLookupTablePoint::CLookupTablePoint(int id, osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius)
{
    m_id = id;
    vpl::math::limit<double>(position[0], 0.0, 1.0);
    vpl::math::limit<double>(position[1], 0.0, 1.0);
    m_position = position;
    m_color = color;
    m_density = density;
    m_gradient = gradient;
    m_radius = radius;
}

CLookupTablePoint::~CLookupTablePoint()
{ }

int CLookupTablePoint::id() const
{
    return m_id;
}

osg::Vec2d CLookupTablePoint::position() const
{
    return m_position;
}

void CLookupTablePoint::setPosition(osg::Vec2d position)
{
    vpl::math::limit<double>(position[0], 0.0, 1.0);
    vpl::math::limit<double>(position[1], 0.0, 1.0);

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

bool CLookupTablePoint::density() const
{
    return m_density;
}

void CLookupTablePoint::setDensity(bool value)
{
    m_density = value;
}

bool CLookupTablePoint::gradient() const
{
    return m_gradient;
}

void CLookupTablePoint::setGradient(bool value)
{
    m_gradient = value;
}

double CLookupTablePoint::radius() const
{
    return m_radius;
}

void CLookupTablePoint::setRadius(double value)
{
    m_radius= value;
}


///////////////////////////////////////////////////////////////////////////////
// CLookupTableComponent
CLookupTableComponent::CLookupTableComponent()
{
    m_name = "";
    m_alphaFactor = 1.0;
    m_id = 0;
}

CLookupTableComponent::~CLookupTableComponent()
{ }

osg::Vec4 CLookupTableComponent::colorDensity(const osg::Vec2d& position, bool withAlphaFactor) const
{
    osg::Vec4 color;
    const int sizePoints = m_cachedDensityPoints.size();

    if (sizePoints == 0)
    {
        color = osg::Vec4(0.0, 0.0, 0.0, 0.0);
    }
    else if (sizePoints == 1)
    {
        return m_cachedDensityPoints[0].color();
    }
    else
    {
        int index = 0;
        for (int i = 0; i < sizePoints - 1; ++i)
        {
            if (position.x() > m_cachedDensityPoints[i].position().x())
            {
                index = i;
            }
        }

        double amount = (position.x() - m_cachedDensityPoints[index].position().x()) / (m_cachedDensityPoints[index + 1].position().x() - m_cachedDensityPoints[index].position().x());
        vpl::math::limit<double>(amount, 0.0, 1.0);
        osg::Vec4 colorA = m_cachedDensityPoints[index].color();
        osg::Vec4 colorB = m_cachedDensityPoints[index + 1].color();

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

osg::Vec4 CLookupTableComponent::colorGradient(const osg::Vec2d& position, bool withAlphaFactor) const
{
    osg::Vec4 color;
    const int gpSize = (int)m_cachedGradientPoints.size();

    if (gpSize == 0)
    {
        color = osg::Vec4(0.0, 0.0, 0.0, 0.0);
    }
    else if (gpSize == 1)
    {
        return m_cachedGradientPoints[0].color();
    }
    else
    {
        int index = 0;
        for (int i = 0; i < gpSize - 1; ++i)
        {
            if (position.y() > m_cachedGradientPoints[i].position().y())
            {
                index = i;
            }
        }

        double amount = (position.y() - m_cachedGradientPoints[index].position().y()) / (m_cachedGradientPoints[index + 1].position().y() - m_cachedGradientPoints[index].position().y());
        vpl::math::limit<double>(amount, 0.0, 1.0);
        osg::Vec4 colorA = m_cachedGradientPoints[index].color();
        osg::Vec4 colorB = m_cachedGradientPoints[index + 1].color();

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

#pragma optimize("", off) // VS2013 sometimes over-optimizes this code and crashes afterwards (Release build only)
osg::Vec4 CLookupTableComponent::color2d(const osg::Vec2d& position, bool withAlphaFactor) const
{
    osg::Vec4 color = osg::Vec4(0.0, 0.0, 0.0, 0.0);

    for (int i = 0; i < m_cached2DPoints.size(); ++i)
    {
        double amount = 1.0 - std::min(1.0, (position - m_cached2DPoints[i].position()).length() / m_cached2DPoints[i].radius());
        osg::Vec4 pointColor = m_cached2DPoints[i].color();
        pointColor.a() *= amount;

        color = CLookupTable::blendColor(pointColor, color);
    }

    if (withAlphaFactor)
    {
        color.a() *= m_alphaFactor;
    }

    return color;
}

osg::Vec4 CLookupTableComponent::color(osg::Vec2d position, bool withAlphaFactor) const
{
    vpl::math::limit<double>(position[0], 0.0, 1.0);
    vpl::math::limit<double>(position[1], 0.0, 1.0);

    osg::Vec4 color = osg::Vec4(0.0, 0.0, 0.0, 0.0);

    color = CLookupTable::blendColor(colorDensity(position, false), color);
    color = CLookupTable::blendColor(colorGradient(position, false), color);
    color = CLookupTable::blendColor(color2d(position, false), color);

    if (withAlphaFactor)
    {
        color.a() *= m_alphaFactor;
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

int CLookupTableComponent::pointId(int pointIndex) const
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return -1;
    }

    return m_points[pointIndex].id();
}

osg::Vec2d CLookupTableComponent::pointPosition(int pointIndex) const
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return osg::Vec2d(-1.0, -1.0);
    }

    return m_points[pointIndex].position();
}

void CLookupTableComponent::setPointPosition(int pointIndex, osg::Vec2d position)
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return;
    }

    m_points[pointIndex].setPosition(position);
    std::sort(m_points.begin(), m_points.end(), pointSort);
    rebuildCache();
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
    rebuildCache();
}

bool CLookupTableComponent::pointDensity(int pointIndex) const
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return false;
    }

    return m_points[pointIndex].density();
}

void CLookupTableComponent::setPointDensity(int pointIndex, bool value)
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return;
    }

    m_points[pointIndex].setDensity(value);
    rebuildCache();
}

bool CLookupTableComponent::pointGradient(int pointIndex) const
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return false;
    }

    return m_points[pointIndex].gradient();
}

void CLookupTableComponent::setPointGradient(int pointIndex, bool value)
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return;
    }

    m_points[pointIndex].setGradient(value);
    rebuildCache();
}

double CLookupTableComponent::pointRadius(int pointIndex) const
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return false;
    }

    return m_points[pointIndex].radius();
}

void CLookupTableComponent::setPointRadius(int pointIndex, double value)
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return;
    }

    m_points[pointIndex].setRadius(value);
    rebuildCache();
}

void CLookupTableComponent::rebuildCache()
{
    m_cachedDensityPoints.clear();
    m_cachedGradientPoints.clear();
    m_cached2DPoints.clear();

    for (int i = 0; i < m_points.size(); ++i)
    {
        if (m_points[i].density() && !m_points[i].gradient())
        {
            m_cachedDensityPoints.push_back(m_points[i]);
        }
        else if (!m_points[i].density() && m_points[i].gradient())
        {
            m_cachedGradientPoints.push_back(m_points[i]);
        }
        else if (m_points[i].density() && m_points[i].gradient())
        {
            m_cached2DPoints.push_back(m_points[i]);
        }
    }
}

void CLookupTableComponent::addPoint(osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius)
{
    m_points.push_back(CLookupTablePoint(m_id++, position, color, density, gradient, radius));
    std::sort(m_points.begin(), m_points.end(), pointSort);
    rebuildCache();
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

    rebuildCache();

    return true;
}

void CLookupTableComponent::setPoint(int pointIndex, osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius)
{
    if ((pointIndex < 0) || (pointIndex >= int(m_points.size())))
    {
        return;
    }

    m_points[pointIndex].setPosition(position);
    m_points[pointIndex].setColor(color);
    m_points[pointIndex].setDensity(density);
    m_points[pointIndex].setGradient(gradient);
    m_points[pointIndex].setRadius(radius);
    std::sort(m_points.begin(), m_points.end(), pointSort);
    rebuildCache();
}

void CLookupTableComponent::clear()
{
    m_points.clear();
    rebuildCache();
}

const int CLookupTableComponent::findPointById(int id) const
{
    for (int i = 0; i < m_points.size(); ++i)
    {
        if (m_points[i].id() == id)
        {
            return i;
        }
    }
    return -1;
}


///////////////////////////////////////////////////////////////////////////////
// CLookupTable
CLookupTable::CLookupTable()
{ }

CLookupTable::~CLookupTable()
{ }

osg::Vec4 CLookupTable::color(const osg::Vec2d& position) const
{
    osg::Vec4 result;
    const size_t componentsSize = m_components.size();

    if (componentsSize == 0)
    {
        return result;
    }
    else
    {
        result = m_components[0].color(position);

        for (std::size_t i = 1; i < componentsSize; ++i)
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

osg::Vec2d CLookupTable::pointPosition(int componentIndex, int pointIndex) const
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return osg::Vec2d(-1.0, -1.0);
    }

    return m_components[componentIndex].pointPosition(pointIndex);
}

void CLookupTable::setPointPosition(int componentIndex, int pointIndex, osg::Vec2d position)
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

bool CLookupTable::pointDensity(int componentIndex, int pointIndex) const
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return false;
    }

    return m_components[componentIndex].pointDensity(pointIndex);
}

void CLookupTable::setPointDensity(int componentIndex, int pointIndex, bool value)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].setPointDensity(pointIndex, value);
}

bool CLookupTable::pointGradient(int componentIndex, int pointIndex) const
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return false;
    }

    return m_components[componentIndex].pointGradient(pointIndex);
}

void CLookupTable::setPointGradient(int componentIndex, int pointIndex, bool value)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].setPointGradient(pointIndex, value);
}

double CLookupTable::pointRadius(int componentIndex, int pointIndex) const
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return false;
    }

    return m_components[componentIndex].pointRadius(pointIndex);
}

void CLookupTable::setPointRadius(int componentIndex, int pointIndex, double value)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].setPointRadius(pointIndex, value);
}

int CLookupTable::addComponent()
{
    m_components.resize(m_components.size() + 1);
    return m_components.size() - 1;
}

const CLookupTableComponent &CLookupTable::component(int componentIndex) const
{
    return m_components[componentIndex];
}

void CLookupTable::setComponent(int componentIndex, const CLookupTableComponent &component)
{
    m_components[componentIndex] = component;
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

void CLookupTable::addPoint(int componentIndex, osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].addPoint(position, color, density, gradient, radius);
}

bool CLookupTable::removePoint(int componentIndex, int pointIndex)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return false;
    }

    return m_components[componentIndex].removePoint(pointIndex);
}

void CLookupTable::setPoint(int componentIndex, int pointIndex, osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius)
{
    if ((componentIndex < 0) || (componentIndex >= int(m_components.size())))
    {
        return;
    }

    m_components[componentIndex].setPoint(pointIndex, position, color, density, gradient, radius);
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

osg::Vec4 CLookupTable::blendColor(const osg::Vec4 &colorA, const osg::Vec4 &colorB)
{
    const float a = blendAlpha(colorA.a(), colorB.a());
    float r = 0, g = 0, b = 0;
    if (a > 0.0)
    {
        const float invA = 1.0f / a;
        r = invA * blendColorComponent(colorA.r(), colorA.a(), colorB.r(), colorB.a());
        g = invA * blendColorComponent(colorA.g(), colorA.a(), colorB.g(), colorB.a());
        b = invA * blendColorComponent(colorA.b(), colorA.a(), colorB.b(), colorB.a());
    }
    return osg::Vec4(r, g, b, a);
}

float CLookupTable::blendColorComponent(float colorAComponent, float colorAAlpha, float colorBComponent, float colorBAlpha)
{
    return colorAComponent * colorAAlpha + colorBComponent * colorBAlpha * (1.0 - colorAAlpha);
}

float CLookupTable::blendAlpha(float colorAAlpha, float colorBAlpha)
{
    return colorAAlpha + colorBAlpha * (1.0 - colorAAlpha);
}
