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

#include <osg/CShaderTester.h>
#include <osg/CPseudoMaterial.h>
#include <osg/Geode>

osg::CPseudoMaterial::CPseudoMaterial()
{
    m_uniDiffuse = new osg::Uniform("Diffuse", osg::Vec3(1.0f, 1.0f, 1.0f));
    m_uniEmission = new osg::Uniform("Emission", osg::Vec3(0.0f, 0.0f, 0.0f));
    m_uniShininess = new osg::Uniform("Shininess", 0.0f);
    m_uniSpecularity = new osg::Uniform("Specularity", 1.0f);

    m_uniforms.push_back(m_uniDiffuse);
    m_uniforms.push_back(m_uniEmission);
    m_uniforms.push_back(m_uniShininess);
    m_uniforms.push_back(m_uniSpecularity);
}

osg::CPseudoMaterial::~CPseudoMaterial()
{ }

osg::Uniform *osg::CPseudoMaterial::uniform(std::string name)
{
    for (int i = 0; i < m_uniforms.size(); ++i)
    {
        if (m_uniforms[i]->getName() == name)
        {
            return m_uniforms[i];
        }
    }

    return NULL;
}

std::string osg::CPseudoMaterial::compatibility()
{
    return std::string("#version 150 compatibility \n");
}

std::string osg::CPseudoMaterial::uniforms()
{
    std::string str = "// UNIFORMS \n";
    for (int i = 0; i < m_uniforms.size(); ++i)
    {
        str += "uniform " + std::string(osg::Uniform::getTypename(m_uniforms[i]->getType())) + " " + m_uniforms[i]->getName() + "; \n";
    }
    return str;
}

std::vector<std::pair<std::string, std::string> > osg::CPseudoMaterial::vert2fragMembers()
{
    std::vector<std::pair<std::string, std::string> > members;
    members.push_back(std::pair<std::string, std::string>("vec3", "worldPosition"));
    members.push_back(std::pair<std::string, std::string>("vec3", "viewPosition"));
    members.push_back(std::pair<std::string, std::string>("vec3", "viewNormal"));
    members.push_back(std::pair<std::string, std::string>("vec4", "texCoord0"));
    members.push_back(std::pair<std::string, std::string>("vec4", "texCoord1"));
    members.push_back(std::pair<std::string, std::string>("vec4", "color"));
    return members;
}

std::string osg::CPseudoMaterial::vert2fragStruct()
{
    std::string str = "// HELPER STRUCTURE TO COLLECT DATA FROM VERTEX SHADER \n";
    str += "struct tridim_v2f \n";
    str += "{ \n";
    std::vector<std::pair<std::string, std::string> > members = vert2fragMembers();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "    " + members[i].first + " " + members[i].second + "; \n";
    }
    str += "}; \n";
    return str;
}

std::string osg::CPseudoMaterial::vert2fragOuts()
{
    std::string str = "// OUTs \n";
    std::vector<std::pair<std::string, std::string> > members = vert2fragMembers();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "out " + members[i].first + " tridim_" + members[i].second + "; \n";
    }
    return str;
}

std::string osg::CPseudoMaterial::vert2fragIns()
{
    std::string str = "// INs \n";
    std::vector<std::pair<std::string, std::string> > members = vert2fragMembers();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "in " + members[i].first + " tridim_" + members[i].second + "; \n";
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
    return "// VERTEX \n" + compatibility() + " \n" + uniforms() + " \n" + vert2fragOuts() + " \n" + std::string(
        "void main() \n"
        "{ \n"
        "    // Transforming The Vertex \n"
        "    gl_Position = ftransform(); \n"
        "    tridim_worldPosition = gl_Vertex.xyz; \n"
        "    tridim_viewPosition = (gl_ModelViewMatrix * gl_Vertex).xyz; \n"
        "    tridim_viewNormal = normalize((gl_ModelViewMatrix * vec4(gl_Normal.xyz, 1.0) - gl_ModelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz); \n"
        "    tridim_texCoord0 = gl_TexCoord[0] = gl_MultiTexCoord0; \n"
        "    tridim_texCoord1 = gl_TexCoord[1] = gl_MultiTexCoord1; \n"
        "    tridim_color = gl_Color; \n"
        "} \n");
}

std::string osg::CPseudoMaterial::insToStruct()
{
    std::string str = "// CONVERT ALL INs TO STRUCTURE \n";
    str += "tridim_v2f insToStruct() \n";
    str += "{ \n";
    str += "    tridim_v2f O; \n";
    std::vector<std::pair<std::string, std::string> > members = vert2fragMembers();
    for (int i = 0; i < members.size(); ++i)
    {
        str += "    O." + members[i].second + " = tridim_" + members[i].second + "; \n";
    }
    str += "    return O; \n";
    str += "} \n";
    return str;
}

std::string osg::CPseudoMaterial::fragShaderSrc()
{
    return "// FRAGMENT \n" + compatibility() + " \n" + uniforms() + " \n" + vert2fragIns() + " \n" + vert2fragStruct() + " \n" + surfStruct() + " \n" + surfaceShaderSrc() + " \n" + insToStruct() + " \n" + std::string(
        "void main() \n"
        "{ \n"
        "    tridim_v2f I = insToStruct(); \n"
        "    I.viewNormal = normalize(I.viewNormal); \n"
        " \n"
        "    vec3 cameraVec = normalize(I.viewPosition.xyz); \n"
        " \n"
        "    vec3 lightVec0 = normalize(-I.viewPosition.xyz); \n"
        "    vec3 reflVec0 = reflect(lightVec0, I.viewNormal); \n"
        "    vec3 lightColor0 = vec3(1.0, 1.0, 1.0); \n"
        " \n"
        "    float diffuseIntensity = max(0.0, dot(I.viewNormal, lightVec0)); \n"
        "    float specIntensity = max(0.0, dot(reflVec0, cameraVec)); \n"
        " \n"
        "    tridim_surf surface = surf(I); \n"
        " \n"
        "    vec3 color = lightColor0 * max(0.0, diffuseIntensity + pow(specIntensity, 1.0 + surface.shininess) * surface.specularity) * surface.albedo; \n"
        " \n"
        "    gl_FragData[0] = vec4(color.rgb + surface.emission, surface.alpha); \n"
        "} \n");
}

std::string osg::CPseudoMaterial::surfaceShaderSrc()
{
    return std::string(
        "// SURFACE PROPERTIES CALCULATION \n"
        "tridim_surf surf(tridim_v2f I) \n"
        "{ \n"
        "    tridim_surf O; \n"
        "    O.albedo = I.color.rgb * Diffuse; \n"
        "    O.emission = Emission; \n"
        "    O.alpha = 1.0; \n"
        "    O.shininess = Shininess; \n"
        "    O.specularity = Specularity; \n"
        "    return O; \n"
        "} \n");
}

void osg::CPseudoMaterial::apply(osg::Node *node)
{
    m_vertShader = new osg::Shader(osg::Shader::VERTEX, vertShaderSrc());
    m_fragShader = new osg::Shader(osg::Shader::FRAGMENT, fragShaderSrc());
    m_program = new osg::Program;
    m_program->addShader(m_vertShader);
    m_program->addShader(m_fragShader);

    osg::StateSet *stateSet = node->getOrCreateStateSet();
    CPseudoMaterial *prev = dynamic_cast<CPseudoMaterial *>(stateSet->getUpdateCallback());
    if (prev != NULL)
    {
        prev->revert(stateSet);
    }
    stateSet->setUpdateCallback(this);
    stateSet->setAttributeAndModes(m_program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    for (int i = 0; i < m_uniforms.size(); ++i)
    {
        stateSet->addUniform(m_uniforms[i]);
    }

    osg::Group *group = dynamic_cast<osg::Group *>(node);
    if (group != NULL)
    {
        CShaderTester *shaderTester = new CShaderTester("pseudomaterial", vertShaderSrc(), "", fragShaderSrc());
        osg::Geode *geode = new osg::Geode;
        geode->addDrawable(shaderTester);
        group->addChild(geode);
    }
}

void osg::CPseudoMaterial::revert(osg::StateSet *stateSet)
{
    stateSet->removeAttribute(m_program);
    for (int i = 0; i < m_uniforms.size(); ++i)
    {
        stateSet->removeUniform(m_uniforms[i]);
    }
}

void osg::CPseudoMaterial::operator()(StateSet *stateSet, NodeVisitor *nodeVisitor)
{ }

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
        "tridim_surf surf(tridim_v2f I) \n"
        "{ \n"
        "    vec3 cameraVec = normalize(I.viewPosition.xyz); \n"
        "    float rimIntensity = 1.0 - abs(dot(I.viewNormal, cameraVec)); \n"
        "    rimIntensity = RimIntensity * pow(rimIntensity, RimPower); \n"
        " \n"
        "    tridim_surf O; \n"
        "    O.albedo = I.color.rgb * Diffuse; \n"
        "    O.emission = Emission + rimIntensity * RimColor; \n"
        "    O.alpha = 1.0; \n"
        "    O.shininess = Shininess; \n"
        "    O.specularity = Specularity; \n"
        "    return O; \n"
        "} \n");
}
