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

#ifndef CLookupTable_H
#define CLookupTable_H


///////////////////////////////////////////////////////////////////////////////
// 
#include <osg/Array>
#include <vector>


///////////////////////////////////////////////////////////////////////////////
// 
class CLookupTablePoint;
class CLookupTableComponent;
class CLookupTable;


///////////////////////////////////////////////////////////////////////////////
//! Class holding info about single control point in LUT component
class CLookupTablePoint
{
private:
    //! identifier of point within component
    int m_id;

    //! relative position of control point
    osg::Vec2d m_position;

    //! color of control point
    osg::Vec4 m_color;

    //! flag if control point takes density into account
    bool m_density;

    //! flag if control point takes gradient into account
    bool m_gradient;

    //! radius used when both density and gradient flags are on
    double m_radius;

public:
    //! Ctor
    CLookupTablePoint(int id, osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius);

    //! Dtor
    ~CLookupTablePoint();

public:
    int id() const;

    //! returns current position of control point
    osg::Vec2d position() const;

    //! sets new position of control point
    void setPosition(osg::Vec2d position);

    //! returns current color of control point
    osg::Vec4 color() const;

    //! sets new color of control point
    void setColor(osg::Vec4 color);

    //! returns density flag
    bool density() const;

    //! sets density flag
    void setDensity(bool value);

    //! returns gradient flag
    bool gradient() const;

    //! sets gradient flag
    void setGradient(bool value);

    //! returns current position of control point
    double radius() const;

    //! sets new position of control point
    void setRadius(double radius);
};


///////////////////////////////////////////////////////////////////////////////
//! Class holding info about single component in LUT
class CLookupTableComponent
{
private:
    //! identifier of next point
    int m_id;

    //! name of component
    std::string m_name;

    //! list of control points
    std::vector<CLookupTablePoint> m_points;

    //! cached point lists
    std::vector<CLookupTablePoint> m_cachedDensityPoints;
    std::vector<CLookupTablePoint> m_cachedGradientPoints;
    std::vector<CLookupTablePoint> m_cached2DPoints;

    //! alpha multiplier for entire component
    double m_alphaFactor;

public:
    //! Ctor
    CLookupTableComponent();

    //! Dtor
    ~CLookupTableComponent();

protected:
    //! returns color considering only density control points
    osg::Vec4 colorDensity(const osg::Vec2d& position, bool withAlphaFactor = true) const;

    //! returns color considering only gradient control points
    osg::Vec4 colorGradient(const osg::Vec2d& position, bool withAlphaFactor = true) const;

    //! returns color considering only full 2d control points
    osg::Vec4 color2d(const osg::Vec2d& position, bool withAlphaFactor = true) const;

    //! rebuilds cached lists of points
    void rebuildCache();

public:
    //! returns color at specified position (does not take alpha factor into account optionally)
    osg::Vec4 color(osg::Vec2d position, bool withAlphaFactor = true) const;

    //! returns alpha factor
    double alphaFactor() const;

    //! sets apha factor (alpha multiplier for entire component)
    void setAlphaFactor(double alphaFactor);

    //! returns current count of control points in component
    int pointCount() const;

    //! returns name of component
    std::string name() const;

    //! sets name of component
    void setName(std::string name);

    //! returns id of control point at specified index
    int pointId(int pointIndex) const;

    //! returns relative position of control point at specified index
    osg::Vec2d pointPosition(int pointIndex) const;

    //! sets relative position of control point at specified index
    void setPointPosition(int pointIndex, osg::Vec2d position);

    //! returns color of control point at specified index
    osg::Vec4 pointColor(int pointIndex) const;

    //! sets color of control point at specified index
    void setPointColor(int pointIndex, osg::Vec4 color);

    //! returns density flag
    bool pointDensity(int pointIndex) const;

    //! sets density flag
    void setPointDensity(int pointIndex, bool value);

    //! returns gradient flag
    bool pointGradient(int pointIndex) const;

    //! sets gradient flag
    void setPointGradient(int pointIndex, bool value);

    //! returns radius
    double pointRadius(int pointIndex) const;

    //! sets radius
    void setPointRadius(int pointIndex, double value);

    //! adds new control point at specified position with specified color (this may affect order of points => indices outside of this class may not be valid)
    void addPoint(osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius);

    //! removes control point at specified index (this may affect order of points => indices outside of this class may not be valid)
    bool removePoint(int pointIndex);

    //! sets position and color of point at specified index (this may affect order of points => indices outside of this class may not be valid)
    void setPoint(int pointIndex, osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius);

    //! removes all points
    void clear();

    //! finds point by its id
    const int findPointById(int id) const;
};


///////////////////////////////////////////////////////////////////////////////
//! class holding info about whole lookup table
class CLookupTable
{
private:
    //! name of LUT
    std::string m_name;

    //! list of components
    std::vector<CLookupTableComponent> m_components;

public:
    //! Ctor
    CLookupTable();

    //! Dtor
    ~CLookupTable();

public:
    //! return color at specified position (components uses alpha-blending - components with higher index cover previous components)
    osg::Vec4 color(const osg::Vec2d& position) const;

    //! returns alpha factor of specified component
    double alphaFactor(int componentIndex) const;

    //! sets alpha factor of specified component
    void setAlphaFactor(int componentIndex, double alphaFactor);

    //! returns count of components in LUT
    int componentCount() const;

    //! returns count of control points in specified component
    int pointCount(int componentIndex) const;

    //! returns name of LUT
    std::string name() const;

    //! sets name of LUT
    void setName(std::string name);

    //! returns name of component at specified index
    std::string name(int componentIndex) const;

    //! sets name of component at specified index
    void setName(int componentIndex, std::string name);

    //! returns relative position of point at specified index in component at specified index
    osg::Vec2d pointPosition(int componentIndex, int pointIndex) const;

    //! sets relative position of point at specified index in component at specified index
    void setPointPosition(int componentIndex, int pointIndex, osg::Vec2d position);

    //! returns color of point at specified index in component at specified index
    osg::Vec4 pointColor(int componentIndex, int pointIndex) const;

    //! sets color of point at specified index in component at specified index
    void setPointColor(int componentIndex, int pointIndex, osg::Vec4 color);

    //! returns density flag
    bool pointDensity(int componentIndex, int pointIndex) const;

    //! sets density flag
    void setPointDensity(int componentIndex, int pointIndex, bool value);

    //! returns gradient flag
    bool pointGradient(int componentIndex, int pointIndex) const;

    //! sets gradient flag
    void setPointGradient(int componentIndex, int pointIndex, bool value);

    //! returns radius
    double pointRadius(int componentIndex, int pointIndex) const;

    //! sets radius
    void setPointRadius(int componentIndex, int pointIndex, double value);

    //! adds new component to the end of list
    int addComponent();

    //! gets component
    const CLookupTableComponent &component(int componentIndex) const;

    //! sets component
    void setComponent(int componentIndex, const CLookupTableComponent &component);

    //! removes component at specified index (this affects indices of components behind removed component)
    bool removeComponent(int componentIndex);

    //! adds new control point at specified position with specified color to component at specified index (this may affect order of points => indices outside of this class may not be valid)
    void addPoint(int componentIndex, osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius);

    //! removes control point at specified index from component at specified index (this may affect order of points => indices outside of this class may not be valid)
    bool removePoint(int componentIndex, int pointIndex);

    //! sets position and color of point at specified index in component at specified index (this may affect order of points => indices outside of this class may not be valid)
    void setPoint(int componentIndex, int pointIndex, osg::Vec2d position, osg::Vec4 color, bool density, bool gradient, double radius);

    //! removes all components
    void clear();

    //! removes all points from component at specified index
    void clear(int componentIndex);

public:
    //! calculates alpha-blending of two colors (A over B)
    static osg::Vec4 blendColor(const osg::Vec4& colorA, const osg::Vec4& colorB);

private:
    //! calculates alpha-blending of color components
    static inline float blendColorComponent(float colorAComponent, float colorAAlpha, float colorBComponent, float colorBAlpha);

    //! calclates alpha-blending of alpha components
    static inline float blendAlpha(float colorAAlpha, float colorBAlpha);
};


//! helper function for sorting control points
static bool pointSort(const CLookupTablePoint pointA, const CLookupTablePoint pointB);


#endif // CLookupTable_H
