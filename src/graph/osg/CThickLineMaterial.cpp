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

#include <osg/CThickLineMaterial.h>
#include <osg/SimpleShader.h>
#include <osg/Camera>
#include <osg/Version>

const char clipByFrustum[] =
"bool clipByFrustum(mat4 m, inout vec4 vertex1, inout vec4 vertex2) \n\
{ \n\
    vec3 p1 = vertex1.xyz; \n\
    vec3 p2 = vertex2.xyz; \n\
    \n\
    vec4 left; \n\
    vec4 right; \n\
    vec4 top; \n\
    vec4 bottom; \n\
    vec4 near; \n\
    vec4 far; \n\
    \n\
    // extract planes from modelviewprojection matrix \n\
    for (int i = 0; i < 4; i++) \n\
    { \n\
        left[i] = m[i][3] + m[i][0]; \n\
        right[i] = m[i][3] - m[i][0]; \n\
        bottom[i] = m[i][3] + m[i][1]; \n\
        top[i] = m[i][3] - m[i][1]; \n\
        near[i] = m[i][3] + m[i][2]; \n\
        far[i] = m[i][3] - m[i][2]; \n\
    } \n\
\n\
    vec4 planes[6] = vec4[6](left, right, top, bottom, near, far); \n\
\n\
    // normalize the planes \n\
    for (int i = 0; i < 6; i++) \n\
    { \n\
        planes[i] = planes[i] / length(planes[i].xyz); \n\
    } \n\
\n\
    float p1t = 0.0; \n\
    float p2t = 1.0; \n\
\n\
    // test against all planes and cut out pieces outside frustum \n\
    for (int i = 0; i < 6; i++) \n\
    { \n\
        float d1 = dot(p1, planes[i].xyz) + planes[i].w; \n\
        float d2 = dot(p2, planes[i].xyz) + planes[i].w; \n\
    \n\
        if(d1 < 0.0 && d2 < 0.0) \n\
        { \n\
            return true; \n\
        } \n\
    \n\
        float t = abs(d1) / (abs(d1) + abs(d2)); \n\
    \n\
        if(d1 < 0.0) \n\
        { \n\
            if(t > p1t) \n\
            { \n\
                p1t = t; \n\
            } \n\
        } \n\
        else if(d2 < 0.0) \n\
        { \n\
            if (t < p2t) \n\
            { \n\
                p2t = t; \n\
            } \n\
        } \n\
        if(p1t >= p2t) \n\
        { \n\
            return true; \n\
        } \n\
    } \n\
\n\
    vec3 v = p2 - p1; \n\
\n\
    if(p1t > 0.0) \n\
    { \n\
        vec3 p = p1 + p1t * v; \n\
        vertex1 = vec4(p, 1.0); \n\
    } \n\
    if(p2t < 1.0) \n\
    { \n\
        vec3 p = p1 + p2t * v; \n\
        vertex2 = vec4(p, 1.0); \n\
    } \n\
        return false; \n\
} \n\
";

const char geomShaderLineStripAdjacencySrc[] =
"   /*Geometry GLSL shader that demonstrates how to draw basic thick and smooth lines in 3D. \n\
    * This file is a part of shader-3dcurve example (https://github.com/vicrucann/shader-3dcurve). \n\
    * \n\
    * \\author Victoria Rudakova \n\
    * \\date January 2017 \n\
    * \\copyright MIT license \n\
    */ \n\
\n\
    uniform vec2 Viewport; \n\
    uniform mat4 osg_ModelViewProjectionMatrix; \n\
    const float MiterLimit = 0.75; \n\
\n\
    layout(lines_adjacency) in; \n\
    layout(triangle_strip, max_vertices = 7) out; \n\
\n\
    in VertexData \n\
    { \n\
        vec4 vertexColor; \n\
    } VertexIn[4]; \n\
\n\
    out VertexData \n\
    { \n\
        vec4 vertexColor; \n\
    } VertexOut; \n\
\n\
bool clipByFrustum(mat4 m, inout vec4 vertex1, inout vec4 vertex2, inout vec4 vertex3, inout vec4 vertex4) \n\
{ \n\
    vec3 p1 = vertex2.xyz; \n\
    vec3 p2 = vertex3.xyz; \n\
    \n\
    vec4 left; \n\
    vec4 right; \n\
    vec4 top; \n\
    vec4 bottom; \n\
    vec4 near; \n\
    vec4 far; \n\
    \n\
    // extract planes from modelviewprojection matrix \n\
    for (int i = 0; i < 4; i++) \n\
    { \n\
        left[i] = m[i][3] + m[i][0]; \n\
        right[i] = m[i][3] - m[i][0]; \n\
        bottom[i] = m[i][3] + m[i][1]; \n\
        top[i] = m[i][3] - m[i][1]; \n\
        near[i] = m[i][3] + m[i][2]; \n\
        far[i] = m[i][3] - m[i][2]; \n\
    } \n\
\n\
    vec4 planes[6] = vec4[6](left, right, top, bottom, near, far); \n\
\n\
    // normalize the planes \n\
    for (int i = 0; i < 6; i++) \n\
    { \n\
        planes[i] = planes[i] / length(planes[i].xyz); \n\
    } \n\
\n\
    float p1t = 0.0; \n\
    float p2t = 1.0; \n\
\n\
    // test against all planes and cut out pieces outside frustum \n\
    for (int i = 0; i < 6; i++) \n\
    { \n\
        float d1 = dot(p1, planes[i].xyz) + planes[i].w; \n\
        float d2 = dot(p2, planes[i].xyz) + planes[i].w; \n\
    \n\
        if(d1 < 0.0 && d2 < 0.0) \n\
        { \n\
            return true; \n\
        } \n\
    \n\
        float t = abs(d1) / (abs(d1) + abs(d2)); \n\
    \n\
        if(d1 < 0.0) \n\
        { \n\
            if(t > p1t) \n\
            { \n\
                p1t = t; \n\
            } \n\
        } \n\
        else if(d2 < 0.0) \n\
        { \n\
            if (t < p2t) \n\
            { \n\
                p2t = t; \n\
            } \n\
        } \n\
        if(p1t >= p2t) \n\
        { \n\
            return true; \n\
        } \n\
    } \n\
\n\
    vec3 v = p2 - p1; \n\
\n\
    if(p1t > 0.0) \n\
    { \n\
        vec3 p = p1 + p1t * v; \n\
        vertex2 = vec4(p, 1.0); \n\
        vertex1 = vertex2; \n\
    } \n\
    if(p2t < 1.0) \n\
    { \n\
        vec3 p = p1 + p2t * v; \n\
        vertex3 = vec4(p, 1.0); \n\
        vertex4 = vertex3; \n\
    } \n\
        return false; \n\
} \n\
void main(void) \n\
{ \n\
    vec4 vertex1 = gl_in[0].gl_Position; \n\
    vec4 vertex2 = gl_in[1].gl_Position; \n\
    vec4 vertex3 = gl_in[2].gl_Position; \n\
    vec4 vertex4 = gl_in[3].gl_Position; \n\
\n\
    if(clipByFrustum(osg_ModelViewProjectionMatrix, vertex1, vertex2, vertex3, vertex4)) \n\
    { \n\
        return; \n\
    } \n\
\n\
    //vec4 color1 = VertexIn[0].vertexColor; \n\
    vec4 color2 = VertexIn[1].vertexColor; \n\
    vec4 color3 = VertexIn[2].vertexColor; \n\
    //vec4 color4 = VertexIn[3].vertexColor; \n\
\n\
    vec4 p1 = osg_ModelViewProjectionMatrix * vertex1; \n\
    vec4 p2 = osg_ModelViewProjectionMatrix * vertex2; \n\
    vec4 p3 = osg_ModelViewProjectionMatrix * vertex3; \n\
    vec4 p4 = osg_ModelViewProjectionMatrix * vertex4; \n\
\n\
    vec3 ndc1 = p1.xyz / p1.w; \n\
    vec3 ndc2 = p2.xyz / p2.w; \n\
    vec3 ndc3 = p3.xyz / p3.w; \n\
    vec3 ndc4 = p4.xyz / p4.w; \n\
\n\
    vec2 s1 = ndc1.xy * Viewport; \n\
    vec2 s2 = ndc2.xy * Viewport; \n\
    vec2 s3 = ndc3.xy * Viewport; \n\
    vec2 s4 = ndc4.xy * Viewport; \n\
\n\
    /* determine the direction of each of the 3 segments (previous, current, next) */ \n\
    vec2 v1 = normalize(s3 - s2); \n\
    vec2 v0 = all(equal(s1, s2)) ? v1 : normalize(s2 - s1); \n\
    vec2 v2 = all(equal(s3, s4)) ? v1 : normalize(s4 - s3); \n\
\n\
    /* determine the normal of each of the 3 segments (previous, current, next) */ \n\
    vec2 n0 = vec2(-v0.y, v0.x); \n\
    vec2 n1 = vec2(-v1.y, v1.x); \n\
    vec2 n2 = vec2(-v2.y, v2.x); \n\
\n\
    /* determine miter lines by averaging the normals of the 2 segments */ \n\
    vec2 miter_a = normalize(n0 + n1); // miter at start of current segment \n\
    vec2 miter_b = normalize(n1 + n2); // miter at end of current segment \n\
\n\
    /* determine the length of the miter by projecting it onto normal and then inverse it */ \n\
    float an1 = dot(miter_a, n1); \n\
    float bn1 = dot(miter_b, n2); \n\
\n\
    if (an1 == 0) an1 = 1; \n\
    if (bn1 == 0) bn1 = 1; \n\
\n\
    float length_a = Thickness / an1; \n\
    float length_b = Thickness / bn1; \n\
\n\
    /* prevent excessively long miters at sharp corners */ \n\
    if (dot(v0, v1) < -MiterLimit) \n\
    { \n\
        miter_a = n1; \n\
        length_a = Thickness; \n\
    \n\
         vec2 offset1 = (Thickness * n0) / Viewport; \n\
         vec2 offset2 = (Thickness * n1) / Viewport; \n\
    \n\
        /* close the gap */ \n\
        if (dot(v0, n1) > 0) \n\
        { \n\
            VertexOut.vertexColor = color2; \n\
            gl_Position = vec4((ndc2.xy + offset1), ndc2.z, 1.0); \n\
            EmitVertex(); \n\
        \n\
            VertexOut.vertexColor = color2; \n\
            gl_Position = vec4((ndc2.xy + offset2), ndc2.z, 1.0); \n\
            EmitVertex(); \n\
        \n\
            VertexOut.vertexColor = color2; \n\
            gl_Position = vec4(ndc2, 1.0); \n\
            EmitVertex(); \n\
        \n\
            EndPrimitive(); \n\
        } \n\
        else \n\
        { \n\
            VertexOut.vertexColor = color2; \n\
            gl_Position = vec4((ndc2.xy - offset2), ndc2.z, 1.0); \n\
            EmitVertex(); \n\
        \n\
            VertexOut.vertexColor = color2; \n\
            gl_Position = vec4((ndc2.xy - offset1), ndc2.z, 1.0); \n\
            EmitVertex(); \n\
        \n\
            VertexOut.vertexColor = color2; \n\
            gl_Position = vec4(ndc2, 1.0); \n\
            EmitVertex(); \n\
        \n\
            EndPrimitive(); \n\
        } \n\
    } \n\
    if (dot(v1, v2) < -MiterLimit) \n\
    { \n\
        miter_b = n1; \n\
        length_b = Thickness; \n\
    } \n\
\n\
    vec2 offset1 = (length_a * miter_a) / Viewport; \n\
    vec2 offset2 = (length_b * miter_b) / Viewport; \n\
\n\
    // generate the triangle strip \n\
    VertexOut.vertexColor = color2; \n\
    gl_Position = vec4((ndc2.xy + offset1), ndc2.z, 1.0); \n\
    EmitVertex(); \n\
\n\
    VertexOut.vertexColor = color2; \n\
    gl_Position = vec4((ndc2.xy - offset1), ndc2.z, 1.0); \n\
    EmitVertex(); \n\
\n\
    VertexOut.vertexColor = color3; \n\
    gl_Position = vec4((ndc3.xy + offset2), ndc3.z, 1.0); \n\
    EmitVertex(); \n\
\n\
    VertexOut.vertexColor = color3; \n\
    gl_Position = vec4((ndc3.xy - offset2), ndc3.z, 1.0); \n\
    EmitVertex(); \n\
\n\
    EndPrimitive(); \n\
} \n\
";

const char geomShaderLinesSrc[] =
"   uniform vec2 Viewport; \n\
    uniform mat4 osg_ModelViewProjectionMatrix; \n\
\n\
    layout(lines) in; \n\
    layout(triangle_strip, max_vertices = 4) out; \n\
\n\
    in VertexData \n\
    { \n\
        vec4 vertexColor; \n\
    } VertexIn[2]; \n\
\n\
    out VertexData \n\
    { \n\
        vec4 vertexColor; \n\
    } VertexOut; \n\
\n\
    vec2 screenOffset(vec3 ndc1, vec3 ndc2, vec2 viewport, float thickness) \n\
    { \n\
        vec2 s1 = ndc1.xy * viewport; \n\
        vec2 s2 = ndc2.xy * viewport; \n\
    \n\
        vec2 v = normalize(s1 - s2); \n\
        vec2 n = vec2(-v.y, v.x); \n\
        return (thickness * n) / viewport; \n\
    } \n\
\n\
    void main(void) \n\
    { \n\
        vec4 vertex1 = gl_in[0].gl_Position; \n\
        vec4 vertex2 = gl_in[1].gl_Position; \n\
    \n\
        if(clipByFrustum(osg_ModelViewProjectionMatrix, vertex1, vertex2)) \n\
        { \n\
            return; \n\
        } \n\
    \n\
        vec4 color1 = VertexIn[0].vertexColor; \n\
        vec4 color2 = VertexIn[1].vertexColor; \n\
    \n\
        vec4 p1 = osg_ModelViewProjectionMatrix * vertex1; \n\
        vec4 p2 = osg_ModelViewProjectionMatrix * vertex2; \n\
    \n\
        vec3 ndc1 = p1.xyz / p1.w; \n\
        vec3 ndc2 = p2.xyz / p2.w; \n\
    \n\
        vec2 offset = screenOffset(ndc1, ndc2, Viewport, Thickness); \n\
    \n\
        VertexOut.vertexColor = color1; \n\
        gl_Position = vec4((ndc1.xy + offset), ndc1.z, 1.0); \n\
        EmitVertex(); \n\
    \n\
        VertexOut.vertexColor = color1; \n\
        gl_Position = vec4((ndc1.xy - offset), ndc1.z, 1.0); \n\
        EmitVertex(); \n\
    \n\
        VertexOut.vertexColor = color2; \n\
        gl_Position = vec4((ndc2.xy + offset), ndc2.z, 1.0); \n\
        EmitVertex(); \n\
    \n\
        VertexOut.vertexColor = color2; \n\
        gl_Position = vec4((ndc2.xy - offset), ndc2.z, 1.0); \n\
        EmitVertex(); \n\
    \n\
        EndPrimitive(); \n\
    } \n\
";

const char geomShaderLineStippleSrc[] =
"   uniform vec2 Viewport; \n\
    uniform mat4 osg_ModelViewProjectionMatrix; \n\
\n\
    layout(lines) in; \n\
    layout(triangle_strip, max_vertices = 4) out; \n\
\n\
    in VertexData \n\
    { \n\
        vec4 vertexColor; \n\
    } VertexIn[2]; \n\
\n\
    out VertexData \n\
    { \n\
        vec2 texCoords; \n\
        vec4 vertexColor; \n\
    } VertexOut; \n\
\n\
    vec2 screenOffset(vec3 ndc1, vec3 ndc2, vec2 viewport, float thickness) \n\
    { \n\
        vec2 s1 = ndc1.xy * viewport; \n\
        vec2 s2 = ndc2.xy * viewport; \n\
    \n\
        vec2 v = normalize(s1 - s2); \n\
        vec2 n = vec2(-v.y, v.x); \n\
        return (thickness * n) / viewport; \n\
    } \n\
\n\
    void main(void) \n\
    { \n\
        vec4 vertex1 = gl_in[0].gl_Position; \n\
        vec4 vertex2 = gl_in[1].gl_Position; \n\
    \n\
        if(clipByFrustum(osg_ModelViewProjectionMatrix, vertex1, vertex2)) \n\
        { \n\
            return; \n\
        } \n\
    \n\
        vec4 color1 = VertexIn[0].vertexColor; \n\
        vec4 color2 = VertexIn[1].vertexColor; \n\
    \n\
        vec4 p1 = osg_ModelViewProjectionMatrix * vertex1; \n\
        vec4 p2 = osg_ModelViewProjectionMatrix * vertex2; \n\
    \n\
        vec3 ndc1 = p1.xyz / p1.w; \n\
        vec3 ndc2 = p2.xyz / p2.w; \n\
    \n\
        vec2 offset = screenOffset(ndc1, ndc2, Viewport, Thickness); \n\
        float t = distance(ndc1.xy * Viewport, ndc2.xy * Viewport) / (Thickness * 16); \n\
    \n\
        VertexOut.texCoords = vec2(0, 0); \n\
        VertexOut.vertexColor = color1; \n\
        gl_Position = vec4((ndc1.xy + offset), ndc1.z, 1.0); \n\
        EmitVertex(); \n\
    \n\
        VertexOut.texCoords = vec2(0, 1); \n\
        VertexOut.vertexColor = color1; \n\
        gl_Position = vec4((ndc1.xy - offset), ndc1.z, 1.0); \n\
        EmitVertex(); \n\
    \n\
        VertexOut.texCoords = vec2(t, 0); \n\
        VertexOut.vertexColor = color2; \n\
        gl_Position = vec4((ndc2.xy + offset), ndc2.z, 1.0); \n\
        EmitVertex(); \n\
    \n\
        VertexOut.texCoords = vec2(t, 1); \n\
        VertexOut.vertexColor = color2; \n\
        gl_Position = vec4((ndc2.xy - offset), ndc2.z, 1.0); \n\
        EmitVertex(); \n\
    \n\
        EndPrimitive(); \n\
    } \n\
";

class ViewportCallback : public osg::UniformCallback
{
public:
    ViewportCallback(osg::Camera* camera) : m_camera(camera)
    {
    }

#if OSG_MIN_VERSION_REQUIRED(3, 7, 0)
    virtual void operator()(osg::UniformBase* uniform, osg::NodeVisitor* /*nv*/) override
    {
        static_cast<osg::Uniform*>(uniform)->set(osg::Vec2f(m_camera->getViewport()->width(), m_camera->getViewport()->height()));
    }
#else
    virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* /*nv*/) override
    {
        uniform->set(osg::Vec2f(m_camera->getViewport()->width(), m_camera->getViewport()->height()));
    }
#endif

private:
    osg::Camera* m_camera;
};

osg::CThickLineMaterial::CThickLineMaterial(osg::Camera* camera, float thickness, ShaderMode mode)
    : m_thickness(thickness)
    , m_mode(mode)
{
    m_viewportVector = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "Viewport");
    m_viewportVector->setUpdateCallback(new ViewportCallback(camera));

    m_program = new osg::Program();

    m_vertShader = new osg::Shader(osg::Shader::VERTEX, colorShaderVertObjectSpace);
    m_geomShader = new osg::Shader(osg::Shader::GEOMETRY, composeShaderSource(mode));

    if (mode == ShaderMode::LineStipple)
    {
        m_fragShader = new osg::Shader(osg::Shader::FRAGMENT, (osg::DisplaySettings::instance()->getNumMultiSamples() > 0) ? texturedShaderAlphaToCoverageFrag : texturedShaderAlphaTestFrag);
    }
    else
    {
        m_fragShader = new osg::Shader(osg::Shader::FRAGMENT, colorShaderFrag);
    }

    m_program->addShader(m_vertShader);
    m_program->addShader(m_geomShader);
    m_program->addShader(m_fragShader);

    if (mode == ShaderMode::LineStipple)
    {
        m_textureStipple = new osg::Texture2D();
        m_textureStipple->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        m_textureStipple->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        m_textureStipple->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
        m_textureStipple->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
        m_textureStipple->setSwizzle(osg::Vec4i(GL_ONE, GL_ONE, GL_ONE, GL_RED));
    }
}

void osg::CThickLineMaterial::apply(osg::Node* node, StateAttribute::GLModeValue value/* = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE*/)
{
    node->getOrCreateStateSet()->addUniform(m_viewportVector);
    node->getOrCreateStateSet()->setAttributeAndModes(m_program, value);

    if (m_mode == ShaderMode::LineStipple)
    {
        if(osg::DisplaySettings::instance()->getNumMultiSamples() > 0)
        {
            node->getOrCreateStateSet()->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

        }
        node->getOrCreateStateSet()->setTextureAttributeAndModes(0, m_textureStipple, osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
    }
}

std::string osg::CThickLineMaterial::composeShaderSource(ShaderMode mode) const
{
    auto shaderSource = [](ShaderMode mode)
    {
        switch (mode)
        {
        case ShaderMode::Lines:
            return std::string(clipByFrustum) + geomShaderLinesSrc;
        case ShaderMode::LineStripAdjacency:
            return std::string(geomShaderLineStripAdjacencySrc);
        case ShaderMode::LineStipple:
            return std::string(clipByFrustum) + geomShaderLineStippleSrc;
        }

        return std::string();
    };

    return std::string("#version 330 core\n") + "const float Thickness = " + std::to_string(m_thickness) + ";\n" + shaderSource(mode);
}

osg::CMaterialLines::CMaterialLines(osg::Camera * camera, float thickness) : CThickLineMaterial(camera, thickness, ShaderMode::Lines)
{
}

osg::CMaterialLines::CMaterialLines(osg::Camera * camera, float thickness, LineMode mode) : CThickLineMaterial(camera, thickness, ShaderMode::LineStipple)
{
    const int patternSize = 8;
    osg::Image* image = new osg::Image();

    unsigned char* pattern = new unsigned char[patternSize];

    if (mode == LineMode::Dotted)
    {
        unsigned char data[patternSize] = { 0xFF, 0x0, 0x0, 0x0, 0xFF, 0x0, 0x0, 0x0 };

        memcpy(pattern, data, patternSize);
    }
    else
    {
        unsigned char data[patternSize] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0, 0x0, 0x0 };

        memcpy(pattern, data, patternSize);
    }

    image->setImage(patternSize, 1, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE, pattern, osg::Image::USE_NEW_DELETE);
    m_textureStipple->setImage(image);
}

osg::CMaterialLineStrip::CMaterialLineStrip(osg::Camera * camera, float thickness) : CThickLineMaterial(camera, thickness, ShaderMode::LineStripAdjacency)
{
}

osg::CMaterialLinesDotted::CMaterialLinesDotted(osg::Camera * camera, float thickness) : CMaterialLines(camera, thickness, LineMode::Dotted)
{
}

osg::CMaterialLinesDashed::CMaterialLinesDashed(osg::Camera * camera, float thickness) : CMaterialLines(camera, thickness, LineMode::Dashed)
{
}
