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

#include <osg/CPseudoMaterial.h>
#include <osg/Geode>
#include <sstream>


osg::CDirectionalLightSource::CDirectionalLightSource(osg::Vec3 color, osg::Vec3 direction)
    : m_color(color)
    , m_direction(direction)
{ }

osg::CDirectionalLightSource::CDirectionalLightSource() : CDirectionalLightSource(osg::Vec3(1.0, 1.0, 1.0), osg::Vec3(0.0, 0.0, 1.0)) {}


osg::Vec3 &osg::CDirectionalLightSource::color()
{
    return m_color;
}

osg::Vec3 &osg::CDirectionalLightSource::direction()
{
    return m_direction;
}

osg::CPointLightSource::CPointLightSource(osg::Vec3 color, osg::Vec3 position, double attenuation)
    : m_color(color)
    , m_position(position)
    , m_attenuation(attenuation)
{ }

osg::CPointLightSource::CPointLightSource() : CPointLightSource(osg::Vec3(1.0, 1.0, 1.0), osg::Vec3(0.0, 0.0, 0.0), 1.0) {}

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

osg::CSpotLightSource::CSpotLightSource(osg::Vec3 color, osg::Vec3 position, osg::Vec3 direction, double attenuation, double focus, double angle)
    : m_color(color)
    , m_position(position)
    , m_direction(direction)
    , m_attenuation(attenuation)
    , m_focus(focus)
    , m_angle(angle)
{ }

osg::CSpotLightSource::CSpotLightSource() : CSpotLightSource(osg::Vec3(1.0, 1.0, 1.0), osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0), 0.0, 1.0, 30.0) {}

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

osg::CPseudoMaterial::CPseudoMaterial(bool twoSided/* = false*/, bool flatShading/* = false*/, bool useVertexColors/* = true*/)
    : m_twoSided(twoSided)
    , m_flatShading(flatShading)
    , m_useVertexColors(useVertexColors)
    , m_dirty(true)
{
    //setName("CPseudoMaterial");

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

    //singleLightSetup();
    fourLightSetup();

    updateLightUniforms();

    setupShaders();
}

void osg::CPseudoMaterial::setupShaders()
{
    m_program = new osg::Program();

    m_geomShader = new osg::Shader(osg::Shader::GEOMETRY);
    m_vertShader = new osg::Shader(osg::Shader::VERTEX);
    m_fragShader = new osg::Shader(osg::Shader::FRAGMENT);

    m_program->addShader(m_vertShader);
    m_program->addShader(m_fragShader);
}

void osg::CPseudoMaterial::setTextures(const std::map<std::string, osg::ref_ptr<osg::Texture2D>>& textures)
{
    for (auto uni : m_uniTextures)
    {
        m_uniforms.erase(std::find(begin(m_uniforms), end(m_uniforms), uni));
    }

    m_uniTextures.clear();

    int unit = 1;
    for (auto& texture : textures)
    {
        m_uniTextures.push_back(new osg::Uniform(osg::Uniform::SAMPLER_2D, "Sampler" + texture.first, 1));
        m_uniTextures.back()->setElement(0, unit++);

        m_uniforms.push_back(m_uniTextures.back());
    }

    m_textures = textures;

    m_dirty = true;
}

void osg::CPseudoMaterial::setFlatShading(bool value)
{
    if (m_flatShading != value)
    {
        m_flatShading = value;
        m_dirty = true;
    }
}

void osg::CPseudoMaterial::setUseVertexColors(bool value)
{
    if (m_useVertexColors != value)
    {
        m_useVertexColors = value;
        m_dirty = true;
    }
}

void osg::CPseudoMaterial::applySingleLightSetup()
{
    m_directionalLightSources.clear();
    singleLightSetup();

    m_uniforms.erase(std::find(begin(m_uniforms), end(m_uniforms), m_uniDirectionalLightSourceColor));
    m_uniforms.erase(std::find(begin(m_uniforms), end(m_uniforms), m_uniDirectionalLightSourceDirection));

    updateLightUniforms();
}

void osg::CPseudoMaterial::applyFourLightSetup()
{
    m_directionalLightSources.clear();
    fourLightSetup();

    m_uniforms.erase(std::find(begin(m_uniforms), end(m_uniforms), m_uniDirectionalLightSourceColor));
    m_uniforms.erase(std::find(begin(m_uniforms), end(m_uniforms), m_uniDirectionalLightSourceDirection));

    updateLightUniforms();
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

    m_directionalLightSources.push_back(CDirectionalLightSource(lightColor[0], lightDirection[0]));
    m_directionalLightSources.push_back(CDirectionalLightSource(lightColor[1], lightDirection[1]));
    m_directionalLightSources.push_back(CDirectionalLightSource(lightColor[2], lightDirection[2]));
    m_directionalLightSources.push_back(CDirectionalLightSource(lightColor[3], lightDirection[3]));
}

void osg::CPseudoMaterial::singleLightSetup()
{
    m_directionalLightSources.push_back(CDirectionalLightSource(osg::Vec3(1.0, 1.0, 1.0), osg::Vec3(0.0, 0.0, 1.0)));
}

void osg::CPseudoMaterial::updateLightUniforms()
{
    // directional lights
    if (!m_directionalLightSources.empty())
    {
        m_uniDirectionalLightSourceColor = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "DirectionalLightSourceColor",static_cast<int>(m_directionalLightSources.size()));
        m_uniDirectionalLightSourceDirection = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "DirectionalLightSourceDirection", static_cast<int>(m_directionalLightSources.size()));

        m_uniforms.push_back(m_uniDirectionalLightSourceColor);
        m_uniforms.push_back(m_uniDirectionalLightSourceDirection);

        for (int i = 0; i < m_directionalLightSources.size(); ++i)
        {
            m_uniDirectionalLightSourceColor->setElement(i, m_directionalLightSources[i].color());
            m_uniDirectionalLightSourceDirection->setElement(i, m_directionalLightSources[i].direction());
        }
    }

    // point lights
    if (!m_pointLightSources.empty())
    {
        m_uniPointLightSourceColor = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "PointLightSourceColor",static_cast<int>(m_pointLightSources.size()));
        m_uniPointLightSourcePosition = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "PointLightSourcePosition", static_cast<int>(m_pointLightSources.size()));
        m_uniPointLightSourceAttenuation = new osg::Uniform(osg::Uniform::FLOAT, "PointLightSourceAttenuation", static_cast<int>(m_pointLightSources.size()));

        m_uniforms.push_back(m_uniPointLightSourceColor);
        m_uniforms.push_back(m_uniPointLightSourcePosition);
        m_uniforms.push_back(m_uniPointLightSourceAttenuation);

        for (int i = 0; i < m_pointLightSources.size(); ++i)
        {
            m_uniPointLightSourceColor->setElement(i, m_pointLightSources[i].color());
            m_uniPointLightSourcePosition->setElement(i, m_pointLightSources[i].position());
            m_uniPointLightSourceAttenuation->setElement(i, m_pointLightSources[i].attenuation());
        }
    }

    // spot lights
    if (!m_spotLightSources.empty())
    {
        m_uniSpotLightSourceColor = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "SpotLightSourceColor", static_cast<int>(m_spotLightSources.size()));
        m_uniSpotLightSourcePosition = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "SpotLightSourcePosition", static_cast<int>(m_spotLightSources.size()));
        m_uniSpotLightSourceDirection = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "SpotLightSourceDirection", static_cast<int>(m_spotLightSources.size()));
        m_uniSpotLightSourceAttenuation = new osg::Uniform(osg::Uniform::FLOAT, "SpotLightSourceAttenuation", static_cast<int>(m_spotLightSources.size()));
        m_uniSpotLightSourceFocus = new osg::Uniform(osg::Uniform::FLOAT, "SpotLightSourceFocus", static_cast<int>(m_spotLightSources.size()));
        m_uniSpotLightSourceAngle = new osg::Uniform(osg::Uniform::FLOAT, "SpotLightSourceAngle", static_cast<int>(m_spotLightSources.size()));

        m_uniforms.push_back(m_uniSpotLightSourceColor);
        m_uniforms.push_back(m_uniSpotLightSourcePosition);
        m_uniforms.push_back(m_uniSpotLightSourceDirection);
        m_uniforms.push_back(m_uniSpotLightSourceAttenuation);
        m_uniforms.push_back(m_uniSpotLightSourceFocus);
        m_uniforms.push_back(m_uniSpotLightSourceAngle);

        for (int i = 0; i < m_spotLightSources.size(); ++i)
        {
            m_uniSpotLightSourceColor->setElement(i, m_spotLightSources[i].color());
            m_uniSpotLightSourcePosition->setElement(i, m_spotLightSources[i].position());
            m_uniSpotLightSourceDirection->setElement(i, m_spotLightSources[i].direction());
            m_uniSpotLightSourceAttenuation->setElement(i, m_spotLightSources[i].attenuation());
            m_uniSpotLightSourceFocus->setElement(i, m_spotLightSources[i].focus());
            m_uniSpotLightSourceAngle->setElement(i, m_spotLightSources[i].angle());
        }
    }
}

void osg::CPseudoMaterial::copyInternals(CPseudoMaterial* other)
{
    if (other == this)
    {
        return;
    }

    for (int i = 0; i < other->m_internalUniforms.size(); ++i)
    {
        osg::Uniform *srcUniform = other->m_internalUniforms[i];
        osg::Uniform *dstUniform = uniform(srcUniform->getName());

        if (nullptr != dstUniform)
        {
            dstUniform->setType(srcUniform->getType());
            dstUniform->copyData(*srcUniform);
        }
    }
}

osg::Uniform* osg::CPseudoMaterial::uniform(const std::string& name)
{
    auto it = std::find_if(m_uniforms.begin(), m_uniforms.end(), [&name](osg::Uniform* uni) {return uni->getName() == name; });

    return it != m_uniforms.end() ? *it : nullptr;
}

std::string osg::CPseudoMaterial::vertVersion()
{
    return std::string("#version 330 core \n");
}

std::string osg::CPseudoMaterial::geomVersion()
{
    return std::string("#version 330 core \n");
}

std::string osg::CPseudoMaterial::fragVersion()
{
    return std::string("#version 330 core \n");
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
    std::string str = "// IN PER VERTEX DATA \n";
    for (int i = 0; i < m_attributes.size(); ++i)
    {
        str += "in " + std::string(osg::CAttribute::getTypename(m_attributes[i].type)) + " " + m_attributes[i].name + "; \n";
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
    std::string str = "// OUTs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "out " + members[i].first + " tridim_v_" + members[i].second + "; \n";
    }
    return str;
}

std::string osg::CPseudoMaterial::geomIns()
{
    std::string str = "// INs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "in " + members[i].first + " tridim_v_" + members[i].second + "[3]; \n";
    }
    return str;
}

std::string osg::CPseudoMaterial::geomOuts()
{
    std::string str = "// OUTs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "out " + members[i].first + " tridim_f_" + members[i].second + "; \n";
    }
    return str;
}

std::string osg::CPseudoMaterial::fragIns()
{
    std::string prefix = "tridim_v_";

    std::string str = "// INs \n";
    std::vector<std::pair<std::string, std::string> > members = shaderInOuts();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "in " + members[i].first + " " + prefix + members[i].second + "; \n";
    }
    return str;
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
        "uniform mat4 osg_ModelViewProjectionMatrix; \n"
        "uniform mat4 osg_ModelViewMatrix; \n"
        "in vec4 osg_Vertex; \n"
        "in vec4 osg_Color; \n"
        "in vec3 osg_Normal; \n"
        "in vec4 osg_MultiTexCoord0; \n"
        "in vec4 osg_MultiTexCoord1; \n"
        "void main() \n"
        "{ \n"
        "    // Transforming The Vertex \n"
        "    gl_Position = tridim_v_projectionSpacePosition = osg_ModelViewProjectionMatrix * osg_Vertex; \n"
        "    tridim_v_objectSpacePosition = osg_Vertex.xyz; \n"
        "    tridim_v_objectSpaceNormal = osg_Normal.xyz; \n"
        "    tridim_v_viewSpacePosition = (osg_ModelViewMatrix * osg_Vertex).xyz; \n"
        "    tridim_v_viewSpaceNormal = normalize((osg_ModelViewMatrix * vec4(osg_Normal.xyz, 1.0) - osg_ModelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz); \n"
        "    tridim_v_texCoord0 = osg_MultiTexCoord0; \n"
        "    tridim_v_texCoord1 = osg_MultiTexCoord1; \n") + (m_useVertexColors ? std::string(
        "    tridim_v_color = osg_Color; \n") : std::string(
        "    tridim_v_color = vec4(1.0, 1.0, 1.0, 1.0); \n")) + std::string(
        "} \n");
}

std::string osg::CPseudoMaterial::insToStruct()
{
    std::string prefix = "tridim_v_";

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
        "layout (triangles) in; \n"
        "layout (triangle_strip, max_vertices = 3) out; \n"
        " \n"
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

void osg::CPseudoMaterial::recompileShaders()
{
    m_vertShader->setShaderSource(vertShaderSrc());
    m_vertShader->dirtyShader();

    m_geomShader->setShaderSource(geomShaderSrc());
    m_geomShader->dirtyShader();

    m_fragShader->setShaderSource(fragShaderSrc());
    m_fragShader->dirtyShader();

    for (int i = 0; i < m_attributes.size(); ++i)
    {
        m_program->addBindAttribLocation(m_attributes[i].name, m_attributes[i].location);
    }
}

std::string osg::CPseudoMaterial::fragShaderSrc()
{
    return "// FRAGMENT \n" + programName() + " \n" + fragVersion() + " \n" + uniforms() + " \n" + fragIns() + " \n" + fragStruct() + " \n" + surfStruct() + " \n" + lightingSrc() + " \n" + surfaceShaderSrc() + " \n" + insToStruct() + " \n" + std::string(
        "out vec4 outColor; \n"
        "void main() \n"
        "{ \n"
        "    tridim_f I = insToStruct(); \n"
        "    I.viewSpaceNormal = normalize(") + std::string(m_flatShading ? "cross(dFdx(I.viewSpacePosition), dFdy(I.viewSpacePosition))" : "I.viewSpaceNormal") + std::string("); \n"
        " \n"
        "    tridim_surf surface = surf(I); \n"
        " \n"
        "    vec3 color = lighting(I, surface); \n"
        " \n"
        "    outColor = vec4(color.rgb + surface.emission, surface.alpha); \n"
        "} \n");
}

std::string osg::CPseudoMaterial::surfaceShaderSrc()
{
    return std::string(
        "// SURFACE PROPERTIES CALCULATION \n"
        "tridim_surf surf(tridim_f I) \n"
        "{ \n"
        "    tridim_surf O; \n") + (m_textures.find("Diffuse") != m_textures.end() ? std::string(
        "    vec4 sampleDiffuse = texture(SamplerDiffuse, I.texCoord0.xy); \n"
        "    O.albedo = I.color.rgb * Diffuse * sampleDiffuse.rgb; \n"
        "    O.alpha = I.color.a * Alpha * sampleDiffuse.a; \n") : std::string(
        "    O.albedo = I.color.rgb * Diffuse; \n"
        "    O.alpha = I.color.a * Alpha; \n")) + (m_textures.find("Emission") != m_textures.end() ? std::string(
        "    vec4 sampleEmission = texture(SamplerEmission, I.texCoord0.xy); \n"
        "    O.emission = Emission * sampleEmission.rgb; \n") : std::string(
        "    O.emission = Emission; \n")) + (m_textures.find("Specularity") != m_textures.end() ? std::string(
        "    vec4 sampleSpecularity = texture(SamplerSpecularity, I.texCoord0.xy); \n"
        "    O.shininess = Shininess * sampleSpecularity.r; \n"
        "    O.specularity = Specularity * sampleSpecularity.g; \n") : std::string(
        "    O.shininess = Shininess; \n"
        "    O.specularity = Specularity; \n")) + std::string(
        "    return O; \n"
        "} \n");
}

class CPseudoMaterialData : public osg::Referenced
{
public:
    CPseudoMaterialData(const osg::ref_ptr<osg::Program> program, const std::vector<osg::ref_ptr<osg::Uniform>>& uniformsBackup, const std::map<std::string, osg::ref_ptr<osg::Texture2D>>& texturesBackup)
        : m_program(program), m_uniformsBackup(uniformsBackup), m_texturesBackup(texturesBackup)
    {
    }

    const osg::ref_ptr<osg::Program> m_program;
    const std::vector<osg::ref_ptr<osg::Uniform> > m_uniformsBackup;
    const std::map<std::string, osg::ref_ptr<osg::Texture2D>> m_texturesBackup;
};

void osg::CPseudoMaterial::apply(osg::Node* node, StateAttribute::GLModeValue value/* = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE*/)
{
    revert(node);

    if (m_dirty)
    {
        recompileShaders();

        m_dirty = false;
    }

    auto stateSet = node->getOrCreateStateSet();

    for (int i = 0; i < m_uniforms.size(); ++i)
    {
        stateSet->addUniform(m_uniforms[i]);
    }

    int unit = 1;
    for (auto& texture : m_textures)
    {
        stateSet->setTextureAttributeAndModes(unit++, texture.second);
    }

    stateSet->setAttributeAndModes(m_program, value);

    stateSet->setUserData(new CPseudoMaterialData(m_program, m_uniforms, m_textures));
}

void osg::CPseudoMaterial::revert(osg::Node* node)
{
    auto data = dynamic_cast<CPseudoMaterialData*>(node->getOrCreateStateSet()->getUserData());

    if (data != nullptr)
    {
        auto stateSet = node->getOrCreateStateSet();

        for (auto uniform : data->m_uniformsBackup)
        {
            stateSet->removeUniform(uniform);
        }

        int unit = 1;
        for (auto& texture : data->m_texturesBackup)
        {
            stateSet->removeTextureAttribute(unit++, texture.second);
        }

        stateSet->removeAttribute(data->m_program);

        stateSet->setUserData(nullptr);
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

std::string osg::CPseudoMaterial_Rim::surfaceShaderSrc()
{
    return std::string(
        "// SURFACE PROPERTIES CALCULATION \n"
        "tridim_surf surf(tridim_f I) \n"
        "{ \n"
        "    tridim_surf O; \n"
        "    vec3 cameraVec = normalize(I.viewSpacePosition.xyz); \n"
        "    float rimIntensity = 1.0 - abs(dot(I.viewSpaceNormal, cameraVec)); \n"
        "    rimIntensity = RimIntensity * pow(rimIntensity, RimPower); \n"
        " \n") + (m_textures.find("Diffuse") != m_textures.end() ? std::string(
        "    vec4 sampleDiffuse = texture(SamplerDiffuse, I.texCoord0.xy); \n"
        "    O.albedo = I.color.rgb * Diffuse * sampleDiffuse.rgb; \n"
        "    O.alpha = I.color.a * Alpha * sampleDiffuse.a; \n") : std::string(
        "    O.albedo = I.color.rgb * Diffuse; \n"
        "    O.alpha = I.color.a * Alpha; \n")) + (m_textures.find("Emission") != m_textures.end() ? std::string(
        "    vec4 sampleEmission = texture(SamplerEmission, I.texCoord0.xy); \n"
        "    O.emission = Emission * sampleEmission.rgb + rimIntensity * RimColor; \n") : std::string(
        "    O.emission = Emission + rimIntensity * RimColor; \n")) + (m_textures.find("Specularity") != m_textures.end() ? std::string(
        "    vec4 sampleSpecularity = texture(SamplerSpecularity, I.texCoord0.xy); \n"
        "    O.shininess = Shininess * sampleSpecularity.r; \n"
        "    O.specularity = Specularity * sampleSpecularity.g; \n") : std::string(
        "    O.shininess = Shininess; \n"
        "    O.specularity = Specularity; \n")) + std::string(
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

std::string osg::CPseudoMaterial_Skinned::vertShaderSrc()
{
    return "// VERTEX \n" + programName() + " \n" + vertVersion() + " \n" + uniforms() + " \n" + attributes() + " \n" + vertOuts() + " \n" + std::string(
        "uniform mat4 osg_ModelViewProjectionMatrix; \n"
        "uniform mat4 osg_ModelViewMatrix; \n"
        "in vec4 osg_Vertex; \n"
        "in vec4 osg_Color; \n"
        "in vec3 osg_Normal; \n"
        "in vec4 osg_MultiTexCoord0; \n"
        "in vec4 osg_MultiTexCoord1; \n"
        "void main() \n"
        "{ \n"
        "    // Transforming The Vertex \n"
        "    ivec4 indices = ivec4(vertexGroupIndices.x + 0.5, vertexGroupIndices.y + 0.5, vertexGroupIndices.z + 0.5, vertexGroupIndices.w + 0.5); \n"
        " \n"
        "    vec4 skinnedVertex = vec4(0.0, 0.0, 0.0, 0.0); \n"
        "    skinnedVertex += vertexGroupWeights.x * (boneMatrices[indices.x] * osg_Vertex); \n"
        "    skinnedVertex += vertexGroupWeights.y * (boneMatrices[indices.y] * osg_Vertex); \n"
        "    skinnedVertex += vertexGroupWeights.z * (boneMatrices[indices.z] * osg_Vertex); \n"
        "    skinnedVertex += vertexGroupWeights.w * (boneMatrices[indices.w] * osg_Vertex); \n"
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
        "    skinnedNormal += vertexGroupWeights.x * (boneMatrices[indices.x] * vec4(osg_Normal.xyz, 1.0)); \n"
        "    skinnedNormal += vertexGroupWeights.y * (boneMatrices[indices.y] * vec4(osg_Normal.xyz, 1.0)); \n"
        "    skinnedNormal += vertexGroupWeights.z * (boneMatrices[indices.z] * vec4(osg_Normal.xyz, 1.0)); \n"
        "    skinnedNormal += vertexGroupWeights.w * (boneMatrices[indices.w] * vec4(osg_Normal.xyz, 1.0)); \n"
        "    skinnedNormal.w = 1.0; \n"
        " \n"
        "    gl_Position = tridim_v_projectionSpacePosition = osg_ModelViewProjectionMatrix * skinnedVertex; \n"
        "    tridim_v_objectSpacePosition = skinnedVertex.xyz; \n"
        "    tridim_v_objectSpaceNormal = skinnedNormal.xyz; \n"
        "    tridim_v_viewSpacePosition = (osg_ModelViewMatrix * skinnedVertex).xyz; \n"
        "    tridim_v_viewSpaceNormal = normalize((osg_ModelViewMatrix * skinnedNormal - osg_ModelViewMatrix * skinnedOrigin).xyz); \n"
        "    tridim_v_texCoord0 = osg_MultiTexCoord0; \n"
        "    tridim_v_texCoord1 = osg_MultiTexCoord1; \n") + (m_useVertexColors ? std::string(
        "    tridim_v_color = osg_Color; \n") : std::string(
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

std::string osg::CPseudoMaterial_Skinned_Rim::surfaceShaderSrc()
{
    return std::string(
        "// SURFACE PROPERTIES CALCULATION \n"
        "tridim_surf surf(tridim_f I) \n"
        "{ \n"
        "    tridim_surf O; \n"
        "    vec3 cameraVec = normalize(I.viewSpacePosition.xyz); \n"
        "    float rimIntensity = 1.0 - abs(dot(I.viewSpaceNormal, cameraVec)); \n"
        "    rimIntensity = RimIntensity * pow(rimIntensity, RimPower); \n"
        " \n") + (m_textures.find("Diffuse") != m_textures.end() ? std::string(
        "    vec4 sampleDiffuse = texture(SamplerDiffuse, I.texCoord0.xy); \n"
        "    O.albedo = I.color.rgb * Diffuse * sampleDiffuse.rgb; \n"
        "    O.alpha = I.color.a * Alpha * sampleDiffuse.a; \n") : std::string(
        "    O.albedo = I.color.rgb * Diffuse; \n"
        "    O.alpha = I.color.a * Alpha; \n")) + (m_textures.find("Emission") != m_textures.end() ? std::string(
        "    vec4 sampleEmission = texture(SamplerEmission, I.texCoord0.xy); \n"
        "    O.emission = Emission * sampleEmission.rgb + rimIntensity * RimColor; \n") : std::string(
        "    O.emission = Emission + rimIntensity * RimColor; \n")) + (m_textures.find("Specularity") != m_textures.end() ? std::string(
        "    vec4 sampleSpecularity = texture(SamplerSpecularity, I.texCoord0.xy); \n"
        "    O.shininess = Shininess * sampleSpecularity.r; \n"
        "    O.specularity = Specularity * sampleSpecularity.g; \n") : std::string(
        "    O.shininess = Shininess; \n"
        "    O.specularity = Specularity; \n")) + std::string(
        "    return O; \n"
        "} \n");
}
