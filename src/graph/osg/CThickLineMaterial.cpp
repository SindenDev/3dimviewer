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


const char geomShaderLineStripAdjacencySrc[] =
"/* \brief Geometry GLSL shader that demonstrates how to draw basic thick and smooth lines in 3D.\n\
* This file is a part of shader-3dcurve example (https://github.com/vicrucann/shader-3dcurve).\n\
*\n\
* \\author Victoria Rudakova\n\
* \\date January 2017\n\
* \\copyright MIT license\n\
*/\n\
uniform vec2 Viewport;\n\
\n\
const float MiterLimit = 0.75;\n\
\n\
layout(lines_adjacency) in;\n\
layout(triangle_strip, max_vertices = 7) out;\n\
\n\
in VertexData\n\
{\n\
    vec4 vertexColor;\n\
} VertexIn[4];\n\
\n\
out VertexData\n\
{\n\
    vec4 vertexColor;\n\
} VertexOut;\n\
\n\
vec2 toScreenSpace(vec4 vertex)\n\
{\n\
    return vec2(vertex.xy / vertex.w) * Viewport;\n\
}\n\
\n\
float toZValue(vec4 vertex)\n\
{\n\
    return (vertex.z / vertex.w);\n\
}\n\
\n\
void drawSegment(vec2 points[4], vec4 colors[4], float zValues[4])\n\
{\n\
    vec2 p0 = points[0];\n\
    vec2 p1 = points[1];\n\
    vec2 p2 = points[2];\n\
    vec2 p3 = points[3];\n\
\n\
    /* determine the direction of each of the 3 segments (previous, current, next) */\n\
    vec2 v1 = normalize(p2 - p1); \n\
    vec2 v0 = all(equal(p0, p1)) ? v1 : normalize(p1 - p0); \n\
    vec2 v2 = all(equal(p2, p3)) ? v1 : normalize(p3 - p2); \n\
\n\
    /* determine the normal of each of the 3 segments (previous, current, next) */\n\
    vec2 n0 = vec2(-v0.y, v0.x);\n\
    vec2 n1 = vec2(-v1.y, v1.x);\n\
    vec2 n2 = vec2(-v2.y, v2.x);\n\
\n\
    /* determine miter lines by averaging the normals of the 2 segments */\n\
    vec2 miter_a = normalize(n0 + n1); // miter at start of current segment\n\
    vec2 miter_b = normalize(n1 + n2); // miter at end of current segment\n\
\n\
    /* determine the length of the miter by projecting it onto normal and then inverse it */\n\
    float an1 = dot(miter_a, n1);\n\
    float bn1 = dot(miter_b, n2);\n\
    if (an1 == 0) an1 = 1;\n\
    if (bn1 == 0) bn1 = 1;\n\
    float length_a = Thickness / an1;\n\
    float length_b = Thickness / bn1;\n\
\n\
    /* prevent excessively long miters at sharp corners */\n\
    if (dot(v0, v1) < -MiterLimit)\n\
    {\n\
        miter_a = n1;\n\
        length_a = Thickness;\n\
\n\
        /* close the gap */\n\
        if (dot(v0, n1) > 0)\n\
        {\n\
            VertexOut.vertexColor = colors[1];\n\
            gl_Position = vec4((p1 + Thickness * n0) / Viewport, zValues[1], 1.0);\n\
            EmitVertex();\n\
\n\
            VertexOut.vertexColor = colors[1];\n\
            gl_Position = vec4((p1 + Thickness * n1) / Viewport, zValues[1], 1.0);\n\
            EmitVertex();\n\
\n\
            VertexOut.vertexColor = colors[1];\n\
            gl_Position = vec4(p1 / Viewport, 0.0, 1.0);\n\
            EmitVertex();\n\
\n\
            EndPrimitive();\n\
        }\n\
        else\n\
        {\n\
            VertexOut.vertexColor = colors[1];\n\
            gl_Position = vec4((p1 - Thickness * n1) / Viewport, zValues[1], 1.0);\n\
            EmitVertex();\n\
\n\
            VertexOut.vertexColor = colors[1];\n\
            gl_Position = vec4((p1 - Thickness * n0) / Viewport, zValues[1], 1.0);\n\
            EmitVertex();\n\
\n\
            VertexOut.vertexColor = colors[1];\n\
            gl_Position = vec4(p1 / Viewport, zValues[1], 1.0);\n\
            EmitVertex();\n\
\n\
            EndPrimitive();\n\
        }\n\
    }\n\
    if (dot(v1, v2) < -MiterLimit)\n\
    {\n\
        miter_b = n1;\n\
        length_b = Thickness;\n\
    }\n\
    // generate the triangle strip\n\
    VertexOut.vertexColor = colors[1];\n\
    gl_Position = vec4((p1 + length_a * miter_a) / Viewport, zValues[1], 1.0);\n\
    EmitVertex();\n\
\n\
    VertexOut.vertexColor = colors[1];\n\
    gl_Position = vec4((p1 - length_a * miter_a) / Viewport, zValues[1], 1.0);\n\
    EmitVertex();\n\
\n\
    VertexOut.vertexColor = colors[2];\n\
    gl_Position = vec4((p2 + length_b * miter_b) / Viewport, zValues[2], 1.0);\n\
    EmitVertex();\n\
\n\
    VertexOut.vertexColor = colors[2];\n\
    gl_Position = vec4((p2 - length_b * miter_b) / Viewport, zValues[2], 1.0);\n\
    EmitVertex();\n\
\n\
    EndPrimitive();\n\
}\n\
\n\
void main(void)\n\
{\n\
    /* 4 points*/\n\
    vec4 Points[4];\n\
    Points[0] = gl_in[0].gl_Position;\n\
    Points[1] = gl_in[1].gl_Position;\n\
    Points[2] = gl_in[2].gl_Position;\n\
    Points[3] = gl_in[3].gl_Position;\n\
\n\
    /* 4 attached colors*/\n\
    vec4 colors[4];\n\
    colors[0] = VertexIn[0].vertexColor;\n\
    colors[1] = VertexIn[1].vertexColor;\n\
    colors[2] = VertexIn[2].vertexColor;\n\
    colors[3] = VertexIn[3].vertexColor;\n\
\n\
    /* screen coords*/\n\
    vec2 points[4];\n\
    points[0] = toScreenSpace(Points[0]);\n\
    points[1] = toScreenSpace(Points[1]);\n\
    points[2] = toScreenSpace(Points[2]);\n\
    points[3] = toScreenSpace(Points[3]);\n\
\n\
    /* deepness values*/\n\
    float zValues[4];\n\
    zValues[0] = toZValue(Points[0]);\n\
    zValues[1] = toZValue(Points[1]);\n\
    zValues[2] = toZValue(Points[2]);\n\
    zValues[3] = toZValue(Points[3]);\n\
\n\
    drawSegment(points, colors, zValues);\n\
}";

const char geomShaderLinesSrc[] =
"uniform vec2 Viewport;\n\
\n\
layout(lines) in;\n\
layout(triangle_strip, max_vertices = 4) out;\n\
\n\
in VertexData\n\
{\n\
    vec4 vertexColor;\n\
} VertexIn[2];\n\
\n\
out VertexData\n\
{\n\
    vec4 vertexColor;\n\
} VertexOut;\n\
\n\
vec2 toScreenSpace(vec4 vertex)\n\
{\n\
    return vec2(vertex.xy / vertex.w) * Viewport;\n\
}\n\
\n\
float toZValue(vec4 vertex)\n\
{\n\
    return (vertex.z / vertex.w);\n\
}\n\
\n\
void drawSegment(vec2 points[2], vec4 colors[2], float zValues[2])\n\
{\n\
    vec2 p0 = points[0];\n\
    vec2 p1 = points[1];\n\
\n\
    /* determine the direction of each of the 3 segments (previous, current, next) */\n\
    vec2 v0 = normalize(p1 - p0);\n\
\n\
    /* determine the normal of each of the 3 segments (previous, current, next) */\n\
    vec2 n0 = vec2(-v0.y, v0.x);\n\
\n\
    /* generate the triangle strip*/\n\
    VertexOut.vertexColor = colors[0];\n\
    gl_Position = vec4((p0 + Thickness * n0) / Viewport, zValues[0], 1.0); \n\
    EmitVertex(); \n\
\n\
    VertexOut.vertexColor = colors[0];\n\
    gl_Position = vec4((p0 - Thickness * n0) / Viewport, zValues[0], 1.0); \n\
    EmitVertex(); \n\
\n\
    VertexOut.vertexColor = colors[1];\n\
    gl_Position = vec4((p1 + Thickness * n0) / Viewport, zValues[1], 1.0); \n\
    EmitVertex(); \n\
    \n\
    VertexOut.vertexColor = colors[1];\n\
    gl_Position = vec4((p1 - Thickness * n0) / Viewport, zValues[1], 1.0); \n\
    EmitVertex(); \n\
    \n\
    EndPrimitive(); \n\
}\n\
\n\
void main(void)\n\
{\n\
    /* 2 points*/\n\
    vec4 Points[2];\n\
    Points[0] = gl_in[0].gl_Position;\n\
    Points[1] = gl_in[1].gl_Position;\n\
\n\
    /* 2 attached colors*/\n\
    vec4 colors[2];\n\
    colors[0] = VertexIn[0].vertexColor;\n\
    colors[1] = VertexIn[1].vertexColor;\n\
\n\
    /* screen coords*/\n\
    vec2 points[2];\n\
    points[0] = toScreenSpace(Points[0]);\n\
    points[1] = toScreenSpace(Points[1]);\n\
\n\
    /* deepness values*/\n\
    float zValues[2];\n\
    zValues[0] = toZValue(Points[0]);\n\
    zValues[1] = toZValue(Points[1]);\n\
\n\
    drawSegment(points, colors, zValues);\n\
}";

const char geomShaderLineStippleSrc[] =
"uniform vec2 Viewport;\n\
\n\
layout(lines) in;\n\
layout(triangle_strip, max_vertices = 4) out;\n\
\n\
in VertexData\n\
{\n\
    vec4 vertexColor;\n\
} VertexIn[2];\n\
\n\
out VertexData\n\
{\n\
    vec2 texCoords;\n\
    vec4 vertexColor;\n\
} VertexOut;\n\
\n\
vec2 toScreenSpace(vec4 vertex)\n\
{\n\
    return vec2(vertex.xy / vertex.w) * Viewport;\n\
}\n\
\n\
float toZValue(vec4 vertex)\n\
{\n\
    return (vertex.z / vertex.w);\n\
}\n\
\n\
void drawSegment(vec2 points[2], vec4 colors[2], float zValues[2])\n\
{\n\
    vec2 p0 = points[0];\n\
    vec2 p1 = points[1];\n\
\n\
    /* determine the direction of each of the 3 segments (previous, current, next) */\n\
    vec2 v0 = normalize(p1 - p0);\n\
    float t0 = distance(p0, p1) / (Thickness*16);\n\
\n\
    /* determine the normal of each of the 3 segments (previous, current, next) */\n\
    vec2 n0 = vec2(-v0.y, v0.x);\n\
\n\
    /* generate the triangle strip*/\n\
    VertexOut.texCoords = vec2(0, 0);\n\
    VertexOut.vertexColor = colors[0];\n\
    gl_Position = vec4((p0 + Thickness * n0) / Viewport, zValues[0], 1.0); \n\
    EmitVertex(); \n\
\n\
    VertexOut.texCoords = vec2(0, 1);\n\
    VertexOut.vertexColor = colors[0];\n\
    gl_Position = vec4((p0 - Thickness * n0) / Viewport, zValues[0], 1.0); \n\
    EmitVertex(); \n\
\n\
    VertexOut.texCoords = vec2(t0, 0);\n\
    VertexOut.vertexColor = colors[1];\n\
    gl_Position = vec4((p1 + Thickness * n0) / Viewport, zValues[1], 1.0); \n\
    EmitVertex(); \n\
    \n\
    VertexOut.texCoords = vec2(t0, 1);\n\
    VertexOut.vertexColor = colors[1];\n\
    gl_Position = vec4((p1 - Thickness * n0) / Viewport, zValues[1], 1.0); \n\
    EmitVertex(); \n\
    \n\
    EndPrimitive(); \n\
}\n\
\n\
void main(void)\n\
{\n\
    /* 2 points*/\n\
    vec4 Points[2];\n\
    Points[0] = gl_in[0].gl_Position;\n\
    Points[1] = gl_in[1].gl_Position;\n\
\n\
    /* 2 attached colors*/\n\
    vec4 colors[2];\n\
    colors[0] = VertexIn[0].vertexColor;\n\
    colors[1] = VertexIn[1].vertexColor;\n\
\n\
    /* screen coords*/\n\
    vec2 points[2];\n\
    points[0] = toScreenSpace(Points[0]);\n\
    points[1] = toScreenSpace(Points[1]);\n\
\n\
    /* deepness values*/\n\
    float zValues[2];\n\
    zValues[0] = toZValue(Points[0]);\n\
    zValues[1] = toZValue(Points[1]);\n\
\n\
    drawSegment(points, colors, zValues);\n\
}";

class ViewportCallback : public osg::Uniform::Callback
{
public:
    ViewportCallback(osg::Camera* camera) : m_camera(camera)
    {
    }

    virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* /*nv*/)
    {
        uniform->set(osg::Vec2f(m_camera->getViewport()->width(), m_camera->getViewport()->height()));
    }

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

    m_vertShader = new osg::Shader(osg::Shader::VERTEX, colorShaderVert);
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
            return geomShaderLinesSrc;
        case ShaderMode::LineStripAdjacency:
            return geomShaderLineStripAdjacencySrc;
        case ShaderMode::LineStipple:
            return geomShaderLineStippleSrc;
        }

        return "";
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
