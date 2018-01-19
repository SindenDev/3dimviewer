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

osg::CPseudoMaterial::CPseudoMaterial(bool twoSided, bool flatShading)
{
    setName("CPseudoMaterial");
    m_uniDummy = new osg::Uniform("-dummy-", 1.0f);

    m_uniDiffuse = new osg::Uniform("Diffuse", osg::Vec3(1.0f, 1.0f, 1.0f));
    m_uniEmission = new osg::Uniform("Emission", osg::Vec3(0.0f, 0.0f, 0.0f));
    m_uniShininess = new osg::Uniform("Shininess", 0.0f);
    m_uniSpecularity = new osg::Uniform("Specularity", 1.0f);

    m_uniforms.push_back(m_uniDiffuse);
    m_uniforms.push_back(m_uniEmission);
    m_uniforms.push_back(m_uniShininess);
    m_uniforms.push_back(m_uniSpecularity);

    m_twoSided = twoSided;
    m_flatShading = flatShading;

    makeDirty();
}

osg::CPseudoMaterial::~CPseudoMaterial()
{ }

void osg::CPseudoMaterial::setFlatShading(bool flatShading)
{
    m_flatShading = flatShading;
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
    members.push_back(std::pair<std::string, std::string>("vec4", "projectionPosition"));
    members.push_back(std::pair<std::string, std::string>("vec3", "worldPosition"));
    members.push_back(std::pair<std::string, std::string>("vec3", "viewPosition"));
    members.push_back(std::pair<std::string, std::string>("vec3", "viewNormal"));
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
        "    gl_Position = tridim_v_projectionPosition = ftransform(); \n"
        "    tridim_v_worldPosition = gl_Vertex.xyz; \n"
        "    tridim_v_viewPosition = (gl_ModelViewMatrix * gl_Vertex).xyz; \n"
        "    tridim_v_viewNormal = normalize((gl_ModelViewMatrix * vec4(gl_Normal.xyz, 1.0) - gl_ModelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz); \n"
        "    tridim_v_texCoord0 = gl_TexCoord[0] = gl_MultiTexCoord0; \n"
        "    tridim_v_texCoord1 = gl_TexCoord[1] = gl_MultiTexCoord1; \n"
        "    tridim_v_color = gl_Color; \n"
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
        "    vec3 n0 = normalize(tridim_v_viewPosition[1].xyz - tridim_v_viewPosition[0].xyz); \n"
        "    vec3 n1 = normalize(tridim_v_viewPosition[2].xyz - tridim_v_viewPosition[1].xyz); \n"
        "    vec3 normal = normalize(cross(n0, n1)); \n"
        " \n"
        "    for (int i = 0; i < 3; ++i) \n"
        "    { \n"
        "        gl_Position = tridim_f_projectionPosition = tridim_v_projectionPosition[i]; \n"
        "        tridim_f_worldPosition = tridim_v_worldPosition[i]; \n"
        "        tridim_f_viewPosition = tridim_v_viewPosition[i]; \n"
        "        tridim_f_viewNormal = normal; \n"
        "        tridim_f_texCoord0 = tridim_v_texCoord0[i]; \n"
        "        tridim_f_texCoord1 = tridim_v_texCoord1[i]; \n"
        "        tridim_f_color = tridim_v_color[i]; \n"
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

std::string osg::CPseudoMaterial::fragShaderSrc()
{
    return "// FRAGMENT \n" + programName() + " \n" + fragVersion() + " \n" + uniforms() + " \n" + fragIns() + " \n" + fragStruct() + " \n" + surfStruct() + " \n" + surfaceShaderSrc() + " \n" + insToStruct() + " \n" + std::string(
        "void main() \n"
        "{ \n"
        "    tridim_f I = insToStruct(); \n"
        "    I.viewNormal = normalize(I.viewNormal); \n"
        " \n"
        "    vec3 cameraVec = normalize(I.viewPosition.xyz); \n"
        " \n"
        "    vec3 lightVec0 = normalize(-I.viewPosition.xyz); \n"
        "    vec3 reflVec0 = reflect(lightVec0, I.viewNormal); \n"
        "    vec3 lightColor0 = vec3(1.0, 1.0, 1.0); \n"
        " \n") + (m_twoSided ? std::string(
        "    if (dot(cameraVec, I.viewNormal) > 0.0) \n"
        "    { \n"
        "        I.viewNormal *= -1.0; \n"
        "    } \n"
        " \n") : std::string("")) + std::string(
        "    float diffuseIntensity = max(0.0, dot(I.viewNormal, lightVec0)); \n"
        "    float specIntensity = max(0.0, dot(reflVec0, cameraVec)); \n"
        " \n"
        "    tridim_surf surface = surf(I); \n"
        " \n"
        "    float specularHighlight = max(0.0, pow(specIntensity, 1.0 + surface.shininess) * surface.specularity); \n"
        "    vec3 color = lightColor0 * max(0.0, diffuseIntensity + specularHighlight) * (surface.albedo + max(0.0, specularHighlight - 1.0)); \n"
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
        "    O.alpha = I.color.a; \n"
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
        "    vec3 cameraVec = normalize(I.viewPosition.xyz); \n"
        "    float rimIntensity = 1.0 - abs(dot(I.viewNormal, cameraVec)); \n"
        "    rimIntensity = RimIntensity * pow(rimIntensity, RimPower); \n"
        " \n"
        "    tridim_surf O; \n"
        "    O.albedo = I.color.rgb * Diffuse; \n"
        "    O.emission = Emission + rimIntensity * RimColor; \n"
        "    O.alpha = I.color.a; \n"
        "    O.shininess = Shininess; \n"
        "    O.specularity = Specularity; \n"
        "    return O; \n"
        "} \n");
}
