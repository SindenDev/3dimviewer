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

#include <osg/CShaderTester.h>
#include <osg/CPseudoMaterial.h>
#include <osg/Geode>

osg::CDirectionalLightSource::CDirectionalLightSource()
    : m_color(1.0, 1.0, 1.0)
    , m_direction(0.0, 0.0, 1.0)
{ }

osg::CDirectionalLightSource::CDirectionalLightSource(osg::Vec3 color, osg::Vec3 direction)
    : m_color(color)
    , m_direction(direction)
{ }

osg::CDirectionalLightSource::CDirectionalLightSource(const osg::CDirectionalLightSource &other)
    : m_color(other.m_color)
    , m_direction(other.m_direction)
{ }

osg::CDirectionalLightSource::~CDirectionalLightSource()
{ }

osg::CDirectionalLightSource &osg::CDirectionalLightSource::operator=(const osg::CDirectionalLightSource &other)
{
    if (this != &other)
    {
        m_color = other.m_color;
        m_direction = other.m_direction;
    }
    return *this;
}

osg::Vec3 &osg::CDirectionalLightSource::color()
{
    return m_color;
}

osg::Vec3 &osg::CDirectionalLightSource::direction()
{
    return m_direction;
}

osg::CPointLightSource::CPointLightSource()
    : m_color(1.0, 1.0, 1.0)
    , m_position(0.0, 0.0, 0.0)
    , m_attenuation(1.0)
{ }

osg::CPointLightSource::CPointLightSource(osg::Vec3 color, osg::Vec3 position, double attenuation)
    : m_color(color)
    , m_position(position)
    , m_attenuation(attenuation)
{ }

osg::CPointLightSource::CPointLightSource(const CPointLightSource &other)
    : m_color(other.m_color)
    , m_position(other.m_position)
    , m_attenuation(other.m_attenuation)
{ }

osg::CPointLightSource::~CPointLightSource()
{ }

osg::CPointLightSource &osg::CPointLightSource::operator=(const osg::CPointLightSource &other)
{
    if (this != &other)
    {
        m_color = other.m_color;
        m_position = other.m_position;
        m_attenuation = other.m_attenuation;
    }
    return *this;
}

osg::Vec3 &osg::CPointLightSource::color()
{
    return m_color;
}

osg::Vec3 &osg::CPointLightSource::position()
{
    return m_position;
}

double &osg::CPointLightSource::attenuation()
{
    return m_attenuation;
}

osg::CSpotLightSource::CSpotLightSource()
    : m_color(1.0, 1.0, 1.0)
    , m_position(0.0, 0.0, 0.0)
    , m_direction(0.0, 0.0, 1.0)
    , m_attenuation(0.0)
    , m_focus(1.0)
    , m_angle(30.0)
{ }

osg::CSpotLightSource::CSpotLightSource(osg::Vec3 color, osg::Vec3 position, osg::Vec3 direction, double attenuation, double focus, double angle)
    : m_color(color)
    , m_position(position)
    , m_direction(direction)
    , m_attenuation(attenuation)
    , m_focus(focus)
    , m_angle(angle)
{ }

osg::CSpotLightSource::CSpotLightSource(const osg::CSpotLightSource &other)
    : m_color(other.m_color)
    , m_position(other.m_position)
    , m_direction(other.m_direction)
    , m_attenuation(other.m_attenuation)
    , m_focus(other.m_focus)
    , m_angle(other.m_angle)
{ }

osg::CSpotLightSource::~CSpotLightSource()
{ }

osg::CSpotLightSource &osg::CSpotLightSource::operator=(const osg::CSpotLightSource &other)
{
    if (this != &other)
    {
        m_color = other.m_color;
        m_position = other.m_position;
        m_direction = other.m_direction;
        m_attenuation = other.m_attenuation;
        m_focus = other.m_focus;
        m_angle = other.m_angle;
    }
    return *this;
}

osg::Vec3 &osg::CSpotLightSource::color()
{
    return m_color;
}

osg::Vec3 &osg::CSpotLightSource::position()
{
    return m_position;
}

osg::Vec3 &osg::CSpotLightSource::direction()
{
    return m_direction;
}

double &osg::CSpotLightSource::attenuation()
{
    return m_attenuation;
}

double &osg::CSpotLightSource::focus()
{
    return m_focus;
}

double &osg::CSpotLightSource::angle()
{
    return m_angle;
}

osg::CAttribute::CAttribute(int location, std::string name, EAttributeType type)
    : location(location)
    , name(name)
    , type(type)
{ }

osg::CAttribute::~CAttribute()
{ }

std::string osg::CAttribute::getTypename(EAttributeType type)
{
    switch (type)
    {
    case EAT_VEC4:
        return "vec4";
        break;

    case EAT_VEC3:
        return "vec3";
        break;

    case EAT_VEC2:
        return "vec2";
        break;

    case EAT_FLOAT:
        return "float";
        break;

    default:
        return "";
        break;
    }
}

osg::CPseudoMaterial::CPseudoMaterial(bool twoSided, bool flatShading, bool useVertexColors)
{
    setName("CPseudoMaterial");
    m_uniDummy = new osg::Uniform("-dummy-", 1.0f);

    m_uniAlpha = new osg::Uniform("Alpha", 1.0f);
    m_uniDiffuse = new osg::Uniform("Diffuse", osg::Vec3(1.0f, 1.0f, 1.0f));
    m_uniEmission = new osg::Uniform("Emission", osg::Vec3(0.0f, 0.0f, 0.0f));
    m_uniShininess = new osg::Uniform("Shininess", 0.0f);
    m_uniSpecularity = new osg::Uniform("Specularity", 1.0f);

    m_uniforms.push_back(m_uniAlpha);
    m_uniforms.push_back(m_uniDiffuse);
    m_uniforms.push_back(m_uniEmission);
    m_uniforms.push_back(m_uniShininess);
    m_uniforms.push_back(m_uniSpecularity);

    m_twoSided = twoSided;
    m_flatShading = flatShading;
    m_useVertexColors = useVertexColors;

    m_directionalLightSourceCount = 0;
    m_pointLightSourceCount = 0;
    m_spotLightSourceCount = 0;
    updateLightUniforms();

    singleLightSetup();
    //fourLightSetup();

    makeDirty();
}

osg::CPseudoMaterial::~CPseudoMaterial()
{ }

void osg::CPseudoMaterial::setFlatShading(bool flatShading)
{
    m_flatShading = flatShading;
    makeDirty();
}

void osg::CPseudoMaterial::setUseVertexColors(bool useVertexColors)
{
    m_useVertexColors = useVertexColors;
    makeDirty();
}

std::vector<osg::CDirectionalLightSource> &osg::CPseudoMaterial::directionalLightSources()
{
    return m_directionalLightSources;
}

std::vector<osg::CPointLightSource> &osg::CPseudoMaterial::pointLightSources()
{
    return m_pointLightSources;
}

std::vector<osg::CSpotLightSource> &osg::CPseudoMaterial::spotLightSources()
{
    return m_spotLightSources;
}

void osg::CPseudoMaterial::fourLightSetup()
{
    osg::Vec3 lightColor[] = {
        osg::Vec3(1.0f, 1.0f, 1.0f) * 0.9f,
        osg::Vec3(1.0f, 0.7f, 0.4f) * 0.6f,
        osg::Vec3(0.7f, 0.8f, 1.0f) * 0.4f,
        osg::Vec3(1.0f, 1.0f, 1.0f) * 0.3f };

    osg::Vec3 lightDirection[] = {
        osg::Vec3( 0.5f, -0.1f,  0.5f),
        osg::Vec3(-0.3f, -0.2f, -0.3f),
        osg::Vec3(-0.1f,  0.5f, -0.2f),
        osg::Vec3(-0.7f,  0.1f, -0.2f) };
    lightDirection[0].normalize();
    lightDirection[1].normalize();
    lightDirection[2].normalize();
    lightDirection[3].normalize();

    m_directionalLightSources.clear();
    m_pointLightSources.clear();
    m_spotLightSources.clear();

    m_directionalLightSources.push_back(CDirectionalLightSource(lightColor[0], lightDirection[0]));
    m_directionalLightSources.push_back(CDirectionalLightSource(lightColor[1], lightDirection[1]));
    m_directionalLightSources.push_back(CDirectionalLightSource(lightColor[2], lightDirection[2]));
    m_directionalLightSources.push_back(CDirectionalLightSource(lightColor[3], lightDirection[3]));

    updateLightUniforms();

    makeDirty();
}

void osg::CPseudoMaterial::singleLightSetup()
{
    m_directionalLightSources.clear();
    m_pointLightSources.clear();
    m_spotLightSources.clear();

    m_directionalLightSources.push_back(CDirectionalLightSource(osg::Vec3(1.0, 1.0, 1.0), osg::Vec3(0.0, 0.0, 1.0)));

    updateLightUniforms();

    makeDirty();
}

void osg::CPseudoMaterial::removeUniform(osg::Uniform *uniform)
{
    for (std::set<osg::ref_ptr<osg::Object> >::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
    {
        osg::Node *node = dynamic_cast<osg::Node *>((*it).get());
        osg::Drawable *drawable = dynamic_cast<osg::Drawable *>((*it).get());

        osg::StateSet *stateSet = (node != NULL ? node->getOrCreateStateSet() : drawable->getOrCreateStateSet());
        stateSet->removeUniform(uniform);
    }

    for (std::vector<osg::ref_ptr<osg::Uniform> >::iterator it = m_uniforms.begin(); it != m_uniforms.end(); ++it)
    {
        if (*it == uniform)
        {
            m_uniforms.erase(it);
            break;
        }
    }
}

void osg::CPseudoMaterial::updateLightUniforms()
{
    // directional lights
    if (m_directionalLightSourceCount != m_directionalLightSources.size())
    {
        removeUniform(m_uniDirectionalLightSourceColor);
        removeUniform(m_uniDirectionalLightSourceDirection);

        m_uniDirectionalLightSourceColor = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "DirectionalLightSourceColor", std::max(1, static_cast<int>(m_directionalLightSources.size())));
        m_uniDirectionalLightSourceDirection = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "DirectionalLightSourceDirection", std::max(1, static_cast<int>(m_directionalLightSources.size())));

        m_uniforms.push_back(m_uniDirectionalLightSourceColor);
        m_uniforms.push_back(m_uniDirectionalLightSourceDirection);

        m_directionalLightSourceCount = m_directionalLightSources.size();
    }

    for (int i = 0; i < m_directionalLightSources.size(); ++i)
    {
        m_uniDirectionalLightSourceColor->setElement(i, m_directionalLightSources[i].color());
        m_uniDirectionalLightSourceDirection->setElement(i, m_directionalLightSources[i].direction());
    }

    // point lights
    if (m_pointLightSourceCount != m_pointLightSources.size())
    {
        removeUniform(m_uniPointLightSourceColor);
        removeUniform(m_uniPointLightSourcePosition);
        removeUniform(m_uniPointLightSourceAttenuation);

        m_uniPointLightSourceColor = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "PointLightSourceColor", std::max(1, static_cast<int>(m_pointLightSources.size())));
        m_uniPointLightSourcePosition = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "PointLightSourcePosition", std::max(1, static_cast<int>(m_pointLightSources.size())));
        m_uniPointLightSourceAttenuation = new osg::Uniform(osg::Uniform::FLOAT, "PointLightSourceAttenuation", std::max(1, static_cast<int>(m_pointLightSources.size())));

        m_uniforms.push_back(m_uniPointLightSourceColor);
        m_uniforms.push_back(m_uniPointLightSourcePosition);
        m_uniforms.push_back(m_uniPointLightSourceAttenuation);

        m_pointLightSourceCount = m_pointLightSources.size();
    }

    for (int i = 0; i < m_pointLightSources.size(); ++i)
    {
        m_uniPointLightSourceColor->setElement(i, m_pointLightSources[i].color());
        m_uniPointLightSourcePosition->setElement(i, m_pointLightSources[i].position());
        m_uniPointLightSourceAttenuation->setElement(i, m_pointLightSources[i].attenuation());
    }

    // spot lights
    if (m_spotLightSourceCount != m_spotLightSources.size())
    {
        removeUniform(m_uniSpotLightSourceColor);
        removeUniform(m_uniSpotLightSourcePosition);
        removeUniform(m_uniSpotLightSourceDirection);
        removeUniform(m_uniSpotLightSourceAttenuation);
        removeUniform(m_uniSpotLightSourceFocus);
        removeUniform(m_uniSpotLightSourceAngle);

        m_uniSpotLightSourceColor = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "SpotLightSourceColor", std::max(1, static_cast<int>(m_spotLightSources.size())));
        m_uniSpotLightSourcePosition = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "SpotLightSourcePosition", std::max(1, static_cast<int>(m_spotLightSources.size())));
        m_uniSpotLightSourceDirection = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "SpotLightSourceDirection", std::max(1, static_cast<int>(m_spotLightSources.size())));
        m_uniSpotLightSourceAttenuation = new osg::Uniform(osg::Uniform::FLOAT, "SpotLightSourceAttenuation", std::max(1, static_cast<int>(m_spotLightSources.size())));
        m_uniSpotLightSourceFocus = new osg::Uniform(osg::Uniform::FLOAT, "SpotLightSourceFocus", std::max(1, static_cast<int>(m_spotLightSources.size())));
        m_uniSpotLightSourceAngle = new osg::Uniform(osg::Uniform::FLOAT, "SpotLightSourceAngle", std::max(1, static_cast<int>(m_spotLightSources.size())));

        m_uniforms.push_back(m_uniSpotLightSourceColor);
        m_uniforms.push_back(m_uniSpotLightSourcePosition);
        m_uniforms.push_back(m_uniSpotLightSourceDirection);
        m_uniforms.push_back(m_uniSpotLightSourceAttenuation);
        m_uniforms.push_back(m_uniSpotLightSourceFocus);
        m_uniforms.push_back(m_uniSpotLightSourceAngle);

        m_spotLightSourceCount = m_spotLightSources.size();
    }

    for (int i = 0; i < m_spotLightSources.size(); ++i)
    {
        m_uniSpotLightSourceColor->setElement(i, m_spotLightSources[i].color());
        m_uniSpotLightSourcePosition->setElement(i, m_spotLightSources[i].position());
        m_uniSpotLightSourceDirection->setElement(i, m_spotLightSources[i].direction());
        m_uniSpotLightSourceAttenuation->setElement(i, m_spotLightSources[i].attenuation());
        m_uniSpotLightSourceFocus->setElement(i, m_spotLightSources[i].focus());
        m_uniSpotLightSourceAngle->setElement(i, m_spotLightSources[i].angle());
    }

    makeDirty();
}

void osg::CPseudoMaterial::makeDirty()
{
    m_dirty = true;

    std::set<osg::ref_ptr<osg::Object> > objects = m_objects;
    for (std::set<osg::ref_ptr<osg::Object> >::iterator it = objects.begin(); it != objects.end(); ++it)
    {
        apply(*it);
    }
}

void osg::CPseudoMaterial::copyInternals(CPseudoMaterial *other)
{
    if ((other == this) || (other == NULL))
    {
        return;
    }

    for (int i = 0; i < other->m_internalUniforms.size(); ++i)
    {
        osg::Uniform *srcUniform = other->m_internalUniforms[i];
        osg::Uniform *dstUniform = uniform(srcUniform->getName());
        dstUniform->setType(srcUniform->getType());
        dstUniform->copyData(*srcUniform);
    }
}

osg::Uniform *osg::CPseudoMaterial::uniform(std::string name)
{
    for (int i = 0; i < m_uniforms.size(); ++i)
    {
        if (m_uniforms[i]->getName() == name)
        {
            return m_uniforms[i];
        }
    }

    return m_uniDummy;
}

std::string osg::CPseudoMaterial::vertVersion()
{
#ifdef __APPLE__
    return std::string("#version 110");
#else
    return std::string("#version 150 compatibility \n");
#endif
}

std::string osg::CPseudoMaterial::geomVersion()
{
#ifdef __APPLE__
    return std::string("#version 120 \n"
                       "#extension GL_EXT_geometry_shader4 : enable \n");
#else
    return std::string("#version 150 compatibility \n");
#endif
}

std::string osg::CPseudoMaterial::fragVersion()
{
#ifdef __APPLE__
    return std::string("#version 110");
#else
    return std::string("#version 150 compatibility \n");
#endif
}

std::string osg::CPseudoMaterial::uniforms()
{
    std::string str = "// UNIFORMS \n";
    for (int i = 0; i < m_uniforms.size(); ++i)
    {
        int numElements = m_uniforms[i]->getNumElements();
        if (numElements == 1)
        {
            str += "uniform " + std::string(osg::Uniform::getTypename(m_uniforms[i]->getType())) + " " + m_uniforms[i]->getName() + "; \n";
        }
        else
        {
            std::stringstream stream;
            stream << numElements;
            str += "uniform " + std::string(osg::Uniform::getTypename(m_uniforms[i]->getType())) + " " + m_uniforms[i]->getName() + "[" + stream.str() + "]; \n";
        }
    }
    return str;
}

std::string osg::CPseudoMaterial::attributes()
{
    std::string str = "// ATTRIBUTES \n";
    for (int i = 0; i < m_attributes.size(); ++i)
    {
        str += "attribute " + std::string(osg::CAttribute::getTypename(m_attributes[i].type)) + " " + m_attributes[i].name + "; \n";
    }
    return str;
}

std::vector<std::pair<std::string, std::string> > osg::CPseudoMaterial::shaderInOuts()
{
    std::vector<std::pair<std::string, std::string> > members;
    members.push_back(std::pair<std::string, std::string>("vec3", "objectSpacePosition"));
    members.push_back(std::pair<std::string, std::string>("vec3", "objectSpaceNormal"));
    members.push_back(std::pair<std::string, std::string>("vec3", "viewSpacePosition"));
    members.push_back(std::pair<std::string, std::string>("vec3", "viewSpaceNormal"));
    members.push_back(std::pair<std::string, std::string>("vec4", "projectionSpacePosition"));
    members.push_back(std::pair<std::string, std::string>("vec4", "texCoord0"));
    members.push_back(std::pair<std::string, std::string>("vec4", "texCoord1"));
    members.push_back(std::pair<std::string, std::string>("vec4", "color"));
    return members;
}

std::string osg::CPseudoMaterial::fragStruct()
{
    std::string str = "// HELPER STRUCTURE TO COLLECT DATA \n";
    str += "struct tridim_f \n";
    str += "{ \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "    " + members[i].first + " " + members[i].second + "; \n";
    }
    str += "}; \n";
    return str;
}

std::string osg::CPseudoMaterial::vertOuts()
{
#ifdef __APPLE__
    std::string str = "// VARYINGs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "varying " + members[i].first + " tridim_v_" + members[i].second + "; \n";
    }
    return str;
#else
    std::string str = "// OUTs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "out " + members[i].first + " tridim_v_" + members[i].second + "; \n";
    }
    return str;
#endif
}

std::string osg::CPseudoMaterial::geomIns()
{
#ifdef __APPLE__
    std::string str = "// VARYING INs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "varying in " + members[i].first + " tridim_v_" + members[i].second + "[3]; \n";
    }
    return str;
#else
    std::string str = "// INs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "in " + members[i].first + " tridim_v_" + members[i].second + "[3]; \n";
    }
    return str;
#endif
}

std::string osg::CPseudoMaterial::geomOuts()
{
#ifdef __APPLE__
    std::string str = "// VARYING OUTs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "varying out " + members[i].first + " tridim_f_" + members[i].second + "; \n";
    }
    return str;
#else
    std::string str = "// OUTs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "out " + members[i].first + " tridim_f_" + members[i].second + "; \n";
    }
    return str;
#endif
}

std::string osg::CPseudoMaterial::fragIns()
{
    std::string prefix = m_flatShading ? "tridim_f_" : "tridim_v_";

#ifdef __APPLE__
    std::string str = "// VARYINGs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "varying " + members[i].first + " " + prefix + members[i].second + "; \n";
    }
    return str;
#else
    std::string str = "// INs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "in " + members[i].first + " " + prefix + members[i].second + "; \n";
    }
    return str;
#endif
}

std::string osg::CPseudoMaterial::surfStruct()
{
    return std::string(
        "// SURFACE STRUCTURE \n"
        "struct tridim_surf \n"
        "{ \n"
        "    vec3 albedo; \n"
        "    vec3 emission; \n"
        "    float alpha; \n"
        "    float shininess; \n"
        "    float specularity; \n"
        "}; \n");
}

std::string osg::CPseudoMaterial::vertShaderSrc()
{
    return "// VERTEX \n" + programName() + " \n" + vertVersion() + " \n" + uniforms() + " \n" + attributes() + " \n" + vertOuts() + " \n" + std::string(
        "void main() \n"
        "{ \n"
        "    // Transforming The Vertex \n"
        "    gl_Position = ftransform(); \n"
        "    tridim_v_projectionSpacePosition = ftransform(); \n"
        " \n"
        "    tridim_v_objectSpacePosition = gl_Vertex.xyz; \n"
        "    tridim_v_objectSpaceNormal = gl_Normal.xyz; \n"
        " \n"
        "    tridim_v_viewSpacePosition = (gl_ModelViewMatrix * gl_Vertex).xyz; \n"
        "    tridim_v_viewSpaceNormal = normalize((gl_ModelViewMatrix * vec4(gl_Normal.xyz, 1.0) - gl_ModelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz); \n"
        " \n"
        "    tridim_v_texCoord0 = gl_TexCoord[0] = gl_MultiTexCoord0; \n"
        "    tridim_v_texCoord1 = gl_TexCoord[1] = gl_MultiTexCoord1; \n"
        " \n") + (m_useVertexColors ? std::string(
        "    tridim_v_color = gl_Color; \n") : std::string(
        "    tridim_v_color = vec4(1.0, 1.0, 1.0, 1.0); \n")) + std::string(
        "} \n");
}

std::string osg::CPseudoMaterial::insToStruct()
{
    std::string prefix = m_flatShading ? "tridim_f_" : "tridim_v_";

    std::string str = "// CONVERT ALL INs TO STRUCTURE \n";
    str += "tridim_f insToStruct() \n";
    str += "{ \n";
    str += "    tridim_f O; \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "    O." + members[i].second + " = " + prefix + members[i].second + "; \n";
    }
    str += "    return O; \n";
    str += "} \n";
    return str;
}

std::string osg::CPseudoMaterial::geomShaderSrc()
{
    if (m_flatShading)
    {
    return "// GEOMETRY \n" + programName() + " \n" + geomVersion() + " \n" + uniforms() + " \n" + geomIns() + " \n" + geomOuts() + " \n" + std::string(
#ifndef __APPLE__
        "layout (triangles) in; \n"
        "layout (triangle_strip, max_vertices = 3) out; \n"
        " \n"
#endif
        "void main() \n"
        "{ \n"
        "    vec3 n0 = normalize(tridim_v_viewSpacePosition[1].xyz - tridim_v_viewSpacePosition[0].xyz); \n"
        "    vec3 n1 = normalize(tridim_v_viewSpacePosition[2].xyz - tridim_v_viewSpacePosition[1].xyz); \n"
        "    vec3 viewSpaceNormal = normalize(cross(n0, n1)); \n"
        " \n"
        "    for (int i = 0; i < 3; ++i) \n"
        "    { \n"
        "        gl_Position = tridim_v_projectionSpacePosition[i]; \n"
        "        tridim_f_projectionSpacePosition = tridim_v_projectionSpacePosition[i]; \n"
        " \n"
        "        tridim_f_objectSpacePosition = tridim_v_objectSpacePosition[i]; \n"
        "        tridim_f_objectSpaceNormal = tridim_v_objectSpaceNormal[i]; \n"
        " \n"
        "        tridim_f_viewSpacePosition = tridim_v_viewSpacePosition[i]; \n"
        "        tridim_f_viewSpaceNormal = viewSpaceNormal; \n"
        " \n"
        "        tridim_f_texCoord0 = tridim_v_texCoord0[i]; \n"
        "        tridim_f_texCoord1 = tridim_v_texCoord1[i]; \n"
        " \n"
        "        tridim_f_color = tridim_v_color[i]; \n"
        " \n"
        "        EmitVertex(); \n"
        "    } \n"
        "    EndPrimitive(); \n"
        "} \n");
    }
    else
    {
        return "";
    }
}

std::string osg::CPseudoMaterial::directionalLightsSrc()
{
    if (m_directionalLightSources.empty())
    {
        return std::string(
            "// DIRECTIONAL LIGHTS CALCULATION \n"
            "vec3 directionalLights(tridim_f I, tridim_surf S) \n"
            "{ \n"
            "    return vec3(0.0, 0.0, 0.0); \n"
            "} \n");
    }
    else
    {
        if (m_directionalLightSources.size() == 1)
        {
            return std::string(
                "// DIRECTIONAL LIGHTS CALCULATION \n"
                "vec3 directionalLights(tridim_f I, tridim_surf S) \n"
                "{ \n"
                "    vec3 cameraVec = normalize(I.viewSpacePosition.xyz); \n") + (m_twoSided ? std::string(
                "    if (dot(cameraVec, I.viewSpaceNormal) > 0.0) \n"
                "    { \n"
                "        I.viewSpaceNormal *= -1.0; \n"
                "    } \n"
                " \n") : std::string("")) + std::string(
                "    vec3 reflVec = reflect(DirectionalLightSourceDirection, I.viewSpaceNormal); \n"
                "    float diffuseIntensity = max(0.0, dot(I.viewSpaceNormal, DirectionalLightSourceDirection)); \n"
                "    float specularIntensity = max(0.0, dot(reflVec, cameraVec)); \n"
                "    float specularHighlight = max(0.0, pow(specularIntensity, 1.0 + S.shininess) * S.specularity); \n"
                "    vec3 color = DirectionalLightSourceColor * max(0.0, diffuseIntensity + specularHighlight) * (S.albedo + max(0.0, specularHighlight - 1.0)); \n"
                " \n"
                "    return color; \n"
                "} \n");
        }
        else
        {
            std::stringstream stream;
            stream << m_directionalLightSources.size();
            std::string lightCount = stream.str();

            return std::string(
                "// DIRECTIONAL LIGHTS CALCULATION \n"
                "vec3 directionalLights(tridim_f I, tridim_surf S) \n"
                "{ \n"
                "    vec3 cameraVec = normalize(I.viewSpacePosition.xyz); \n") + (m_twoSided ? std::string(
                "    if (dot(cameraVec, I.viewSpaceNormal) > 0.0) \n"
                "    { \n"
                "        I.viewSpaceNormal *= -1.0; \n"
                "    } \n"
                " \n") : std::string("")) + std::string(
                "    vec3 color = vec3(0.0, 0.0, 0.0); \n"
                "    for (int i = 0; i < " + lightCount + "; ++i)"
                "    { \n"
                "        vec3 reflVec = reflect(DirectionalLightSourceDirection[i], I.viewSpaceNormal); \n"
                "        float diffuseIntensity = max(0.0, dot(I.viewSpaceNormal, DirectionalLightSourceDirection[i])); \n"
                "        float specularIntensity = max(0.0, dot(reflVec, cameraVec)); \n"
                "        float specularHighlight = max(0.0, pow(specularIntensity, 1.0 + S.shininess) * S.specularity); \n"
                "        color += DirectionalLightSourceColor[i] * max(0.0, diffuseIntensity + specularHighlight) * (S.albedo + max(0.0, specularHighlight - 1.0)); \n"
                "    } \n"
                " \n"
                "    return color; \n"
                "} \n");
        }
    }
}

std::string osg::CPseudoMaterial::pointLightsSrc()
{
    // PointLightSourceColor
    // PointLightSourcePosition
    // PointLightSourceAttenuation
    return std::string(
        "// POINT LIGHTS CALCULATION \n"
        "vec3 pointLights(tridim_f I, tridim_surf S) \n"
        "{ \n"
        "    return vec3(0.0, 0.0, 0.0); \n"
        "} \n");
}

std::string osg::CPseudoMaterial::spotLightsSrc()
{
    // SpotLightSourceColor
    // SpotLightSourcePosition
    // SpotLightSourceDirection
    // SpotLightSourceAttenuation
    // SpotLightSourceFocus
    // SpotLightSourceAngle
    return std::string(
        "// SPOT LIGHTS CALCULATION \n"
        "vec3 spotLights(tridim_f I, tridim_surf S) \n"
        "{ \n"
        "    return vec3(0.0, 0.0, 0.0); \n"
        "} \n");
}

std::string osg::CPseudoMaterial::lightingSrc()
{
    return directionalLightsSrc() + " \n" + pointLightsSrc() + " \n" + spotLightsSrc() + " \n" + std::string(
        "// LIGHTING CALCULATION \n"
        "vec3 lighting(tridim_f I, tridim_surf S) \n"
        "{ \n"
        "    vec3 color = vec3(0.0, 0.0, 0.0); \n"
        " \n"
        "    color += directionalLights(I, S); \n"
        "    color += pointLights(I, S); \n"
        "    color += spotLights(I, S); \n"
        " \n"
        "    return color; \n"
        "} \n");
}

std::string osg::CPseudoMaterial::fragShaderSrc()
{
    return "// FRAGMENT \n" + programName() + " \n" + fragVersion() + " \n" + uniforms() + " \n" + fragIns() + " \n" + fragStruct() + " \n" + surfStruct() + " \n" + lightingSrc() + " \n" + surfaceShaderSrc() + " \n" + insToStruct() + " \n" + std::string(
        "void main() \n"
        "{ \n"
        "    tridim_f I = insToStruct(); \n"
        "    I.viewSpaceNormal = normalize(I.viewSpaceNormal); \n"
        " \n"
        "    tridim_surf surface = surf(I); \n"
        " \n"
        "    vec3 color = lighting(I, surface); \n"
        " \n"
        "    gl_FragData[0] = vec4(color.rgb + surface.emission, surface.alpha); \n"
        "} \n");
}

std::string osg::CPseudoMaterial::surfaceShaderSrc()
{
    return std::string(
        "// SURFACE PROPERTIES CALCULATION \n"
        "tridim_surf surf(tridim_f I) \n"
        "{ \n"
        "    tridim_surf O; \n"
        "    O.albedo = I.color.rgb * Diffuse; \n"
        "    O.emission = Emission; \n"
        "    O.alpha = I.color.a * Alpha; \n"
        "    O.shininess = Shininess; \n"
        "    O.specularity = Specularity; \n"
        "    return O; \n"
        "} \n");
}

void osg::CPseudoMaterial::apply(osg::Object *object, osg::Node *shaderTesterNode)
{
    osg::Node *node = dynamic_cast<osg::Node *>(object);
    osg::Drawable *drawable = dynamic_cast<osg::Drawable *>(object);

    if (node == NULL && drawable == NULL)
    {
        return;
    }

    if (m_dirty)
    {
        m_program = new osg::Program;

        m_vertShader = new osg::Shader(osg::Shader::VERTEX, vertShaderSrc());
        m_program->addShader(m_vertShader);

        if (m_flatShading)
        {
            m_geomShader = new osg::Shader(osg::Shader::GEOMETRY, geomShaderSrc());
            m_program->addShader(m_geomShader);

            m_program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
            m_program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
            m_program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
        }

        m_fragShader = new osg::Shader(osg::Shader::FRAGMENT, fragShaderSrc());
        m_program->addShader(m_fragShader);

        for (int i = 0; i < m_attributes.size(); ++i)
        {
            m_program->addBindAttribLocation(m_attributes[i].name, m_attributes[i].location);
        }
    }

    osg::StateSet *stateSet = (node != NULL ? node->getOrCreateStateSet() : drawable->getOrCreateStateSet());
    CPseudoMaterial *prev = dynamic_cast<CPseudoMaterial *>(stateSet->getUpdateCallback());
    if (prev != NULL)
    {
        prev->revert(node);
    }
    stateSet->setUpdateCallback(this);
    stateSet->setAttributeAndModes(m_program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    for (int i = 0; i < m_uniforms.size(); ++i)
    {
        stateSet->addUniform(m_uniforms[i]);
    }
    m_objects.insert(object);

    if (m_dirty)
    {
        // commented: this probably causes crashes because OSG tree can contain Geode containing Geode and that is not checked in some OSG functions
        osg::Group *group = dynamic_cast<osg::Group *>(shaderTesterNode == NULL ? node : shaderTesterNode);
        if (group != NULL)
        {
            /*
            CShaderTester *shaderTester = new CShaderTester(programName(), vertShaderSrc(), geomShaderSrc(), fragShaderSrc());
            osg::Geode *geode = new osg::Geode;
            geode->addDrawable(shaderTester);
            group->addChild(geode);
            */
        }

        m_dirty = false;
    }
}

void osg::CPseudoMaterial::revert(osg::Object *object)
{
    osg::Node *node = dynamic_cast<osg::Node *>(object);
    osg::Drawable *drawable = dynamic_cast<osg::Drawable *>(object);

    if (node == NULL && drawable == NULL)
    {
        return;
    }

    m_objects.erase(object);

    osg::StateSet *stateSet = (node != NULL ? node->getOrCreateStateSet() : drawable->getOrCreateStateSet());
    stateSet->removeAttribute(m_program);
    for (int i = 0; i < m_uniforms.size(); ++i)
    {
        stateSet->removeUniform(m_uniforms[i]);
    }
}

void osg::CPseudoMaterial::operator()(StateSet *stateSet, NodeVisitor *nodeVisitor)
{
    if (m_dirty)
    {
        if (m_vertShader == NULL)
        {
            m_vertShader = new osg::Shader(osg::Shader::VERTEX, vertShaderSrc());
        }
        else
        {
            m_vertShader->setShaderSource(vertShaderSrc());
            m_vertShader->dirtyShader();
        }

        if (m_geomShader == NULL)
        {
            m_geomShader = new osg::Shader(osg::Shader::GEOMETRY, geomShaderSrc());
        }
        else
        {
            m_geomShader->setShaderSource(geomShaderSrc());
            m_geomShader->dirtyShader();
        }

        if (m_fragShader == NULL)
        {
            m_fragShader = new osg::Shader(osg::Shader::FRAGMENT, fragShaderSrc());
        }
        else
        {
            m_fragShader->setShaderSource(fragShaderSrc());
            m_fragShader->dirtyShader();
        }

        if (m_program == NULL)
        {
            m_program = new osg::Program;
            m_program->addShader(m_vertShader);
            m_program->addShader(m_fragShader);

            if (m_flatShading)
            {
                m_program->addShader(m_geomShader);
                m_program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
                m_program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
                m_program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
            }

            for (int i = 0; i < m_attributes.size(); ++i)
            {
                m_program->addBindAttribLocation(m_attributes[i].name, m_attributes[i].location);
            }
        }
        else
        {
            if (m_program != NULL)
            {
                while (m_program->getNumShaders() > 0)
                {
                    m_program->removeShader(m_program->getShader(0));
                }
            }

            m_program->addShader(m_vertShader);
            m_program->addShader(m_fragShader);

            if (m_flatShading)
            {
                m_program->addShader(m_geomShader);
                m_program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
                m_program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
                m_program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
            }

            m_program->dirtyProgram();
        }

        m_dirty = false;
    }
}

osg::CPseudoMaterial_Rim::CPseudoMaterial_Rim()
{
    m_uniRimColor = new osg::Uniform("RimColor", osg::Vec3(1.0, 1.0, 1.0));
    m_uniRimIntensity = new osg::Uniform("RimIntensity", 1.0f);
    m_uniRimPower = new osg::Uniform("RimPower", 2.0f);

    m_uniforms.push_back(m_uniRimColor);
    m_uniforms.push_back(m_uniRimIntensity);
    m_uniforms.push_back(m_uniRimPower);
}

osg::CPseudoMaterial_Rim::~CPseudoMaterial_Rim()
{ }

std::string osg::CPseudoMaterial_Rim::surfaceShaderSrc()
{
    return std::string(
        "// SURFACE PROPERTIES CALCULATION \n"
        "tridim_surf surf(tridim_f I) \n"
        "{ \n"
        "    vec3 cameraVec = normalize(I.viewSpacePosition.xyz); \n"
        "    float rimIntensity = 1.0 - abs(dot(I.viewSpaceNormal, cameraVec)); \n"
        "    rimIntensity = RimIntensity * pow(rimIntensity, RimPower); \n"
        " \n"
        "    tridim_surf O; \n"
        "    O.albedo = I.color.rgb * Diffuse; \n"
        "    O.emission = Emission + rimIntensity * RimColor; \n"
        "    O.alpha = I.color.a * Alpha; \n"
        "    O.shininess = Shininess; \n"
        "    O.specularity = Specularity; \n"
        "    return O; \n"
        "} \n");
}

osg::CPseudoMaterial_Skinned::CPseudoMaterial_Skinned(int maxBones, bool twoSided)
    : osg::CPseudoMaterial(twoSided)
{
    m_attributes.push_back(osg::CAttribute(6, "vertexGroupIndices", osg::CAttribute::EAT_VEC4));
    m_attributes.push_back(osg::CAttribute(7, "vertexGroupWeights", osg::CAttribute::EAT_VEC4));

    m_uniBoneMatrices = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "boneMatrices", maxBones);
    for (int i = 0; i < maxBones; ++i)
    {
        m_uniBoneMatrices->setElement(i, osg::Matrix::identity());
    }
    m_uniforms.push_back(m_uniBoneMatrices);
    m_internalUniforms.push_back(m_uniBoneMatrices);
}

osg::CPseudoMaterial_Skinned::~CPseudoMaterial_Skinned()
{ }

std::string osg::CPseudoMaterial_Skinned::vertShaderSrc()
{
    return "// VERTEX \n" + programName() + " \n" + vertVersion() + " \n" + uniforms() + " \n" + attributes() + " \n" + vertOuts() + " \n" + std::string(
        "void main() \n"
        "{ \n"
        "    // Transforming The Vertex \n"
        "    ivec4 indices = ivec4(vertexGroupIndices.x + 0.5, vertexGroupIndices.y + 0.5, vertexGroupIndices.z + 0.5, vertexGroupIndices.w + 0.5); \n"
        " \n"
        "    vec4 skinnedVertex = vec4(0.0, 0.0, 0.0, 0.0); \n"
        "    skinnedVertex += vertexGroupWeights.x * (boneMatrices[indices.x] * gl_Vertex); \n"
        "    skinnedVertex += vertexGroupWeights.y * (boneMatrices[indices.y] * gl_Vertex); \n"
        "    skinnedVertex += vertexGroupWeights.z * (boneMatrices[indices.z] * gl_Vertex); \n"
        "    skinnedVertex += vertexGroupWeights.w * (boneMatrices[indices.w] * gl_Vertex); \n"
        "    skinnedVertex.w = 1.0; \n"
        " \n"
        "    vec4 skinnedOrigin = vec4(0.0, 0.0, 0.0, 0.0); \n"
        "    skinnedOrigin += vertexGroupWeights.x * (boneMatrices[indices.x] * vec4(0.0, 0.0, 0.0, 1.0)); \n"
        "    skinnedOrigin += vertexGroupWeights.y * (boneMatrices[indices.y] * vec4(0.0, 0.0, 0.0, 1.0)); \n"
        "    skinnedOrigin += vertexGroupWeights.z * (boneMatrices[indices.z] * vec4(0.0, 0.0, 0.0, 1.0)); \n"
        "    skinnedOrigin += vertexGroupWeights.w * (boneMatrices[indices.w] * vec4(0.0, 0.0, 0.0, 1.0)); \n"
        "    skinnedOrigin.w = 1.0; \n"
        " \n"
        "    vec4 skinnedNormal = vec4(0.0, 0.0, 0.0, 0.0); \n"
        "    skinnedNormal += vertexGroupWeights.x * (boneMatrices[indices.x] * vec4(gl_Normal.xyz, 1.0)); \n"
        "    skinnedNormal += vertexGroupWeights.y * (boneMatrices[indices.y] * vec4(gl_Normal.xyz, 1.0)); \n"
        "    skinnedNormal += vertexGroupWeights.z * (boneMatrices[indices.z] * vec4(gl_Normal.xyz, 1.0)); \n"
        "    skinnedNormal += vertexGroupWeights.w * (boneMatrices[indices.w] * vec4(gl_Normal.xyz, 1.0)); \n"
        "    skinnedNormal.w = 1.0; \n"
        " \n"
        "    gl_Position = gl_ModelViewProjectionMatrix * skinnedVertex; \n"
        "    tridim_v_projectionSpacePosition = gl_ModelViewProjectionMatrix * skinnedVertex; \n"
        " \n"
        "    tridim_v_objectSpacePosition = skinnedVertex.xyz; \n"
        "    tridim_v_objectSpaceNormal = skinnedNormal.xyz; \n"
        " \n"
        "    tridim_v_viewSpacePosition = (gl_ModelViewMatrix * skinnedVertex).xyz; \n"
        "    tridim_v_viewSpaceNormal = normalize((gl_ModelViewMatrix * skinnedNormal - gl_ModelViewMatrix * skinnedOrigin).xyz); \n"
        " \n"
        "    tridim_v_texCoord0 = gl_TexCoord[0] = gl_MultiTexCoord0; \n"
        "    tridim_v_texCoord1 = gl_TexCoord[1] = gl_MultiTexCoord1; \n") + (m_useVertexColors ? std::string(
        " \n"
        "    tridim_v_color = gl_Color; \n") : std::string(
        "    tridim_v_color = vec4(1.0, 1.0, 1.0, 1.0); \n")) + std::string(
        "} \n");
}

osg::CPseudoMaterial_Skinned_Rim::CPseudoMaterial_Skinned_Rim(int maxBones, bool twoSided)
    : osg::CPseudoMaterial_Skinned(maxBones, twoSided)
{
    m_uniRimColor = new osg::Uniform("RimColor", osg::Vec3(1.0, 1.0, 1.0));
    m_uniRimIntensity = new osg::Uniform("RimIntensity", 1.0f);
    m_uniRimPower = new osg::Uniform("RimPower", 2.0f);

    m_uniforms.push_back(m_uniRimColor);
    m_uniforms.push_back(m_uniRimIntensity);
    m_uniforms.push_back(m_uniRimPower);
}

osg::CPseudoMaterial_Skinned_Rim::~CPseudoMaterial_Skinned_Rim()
{ }

std::string osg::CPseudoMaterial_Skinned_Rim::surfaceShaderSrc()
{
    return std::string(
        "// SURFACE PROPERTIES CALCULATION \n"
        "tridim_surf surf(tridim_f I) \n"
        "{ \n"
        "    vec3 cameraVec = normalize(I.viewSpacePosition.xyz); \n"
        "    float rimIntensity = 1.0 - abs(dot(I.viewSpaceNormal, cameraVec)); \n"
        "    rimIntensity = RimIntensity * pow(rimIntensity, RimPower); \n"
        " \n"
        "    tridim_surf O; \n"
        "    O.albedo = I.color.rgb * Diffuse; \n"
        "    O.emission = Emission + rimIntensity * RimColor; \n"
        "    O.alpha = I.color.a * Alpha; \n"
        "    O.shininess = Shininess; \n"
        "    O.specularity = Specularity; \n"
        "    return O; \n"
        "} \n");
}
