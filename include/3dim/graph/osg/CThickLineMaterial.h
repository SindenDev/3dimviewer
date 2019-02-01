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

#ifndef CThickLineMaterial_H
#define CThickLineMaterial_H

#include <osg/StateSet>
#include <osg/Texture2D>

namespace osg
{
    // Line width via geometry shader, non-public base class
    class CThickLineMaterial : public osg::Referenced
    {
    protected:
        enum class ShaderMode
        {
            Lines,
            LineStripAdjacency,
            LineStipple
        };

        enum class LineMode
        {
            Dotted,
            Dashed
        };

        CThickLineMaterial(osg::Camera* camera, float thickness, ShaderMode inputPrimitive);

    public:
        virtual void apply(osg::Node* node, StateAttribute::GLModeValue value = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    protected:
        std::string composeShaderSource(ShaderMode primitive) const;

    protected:
        osg::ref_ptr<osg::Uniform> m_viewportVector;

        osg::ref_ptr<osg::Shader> m_vertShader;
        osg::ref_ptr<osg::Shader> m_geomShader;
        osg::ref_ptr<osg::Shader> m_fragShader;
        osg::ref_ptr<osg::Program> m_program;

        osg::ref_ptr<osg::Texture2D> m_textureStipple;

        float m_thickness;
        ShaderMode m_mode;
    };

    // Apply to GL_LINES
    class CMaterialLines : public CThickLineMaterial
    {
    public:
        CMaterialLines(osg::Camera* camera, float thickness);

    protected:
        CMaterialLines(osg::Camera* camera, float thickness, LineMode mode);
    };

    // Apply to GL_LINE_STRIP_ADJACENCY
    class CMaterialLineStrip : public CThickLineMaterial
    {
    public:
        CMaterialLineStrip(osg::Camera* camera, float thickness);
    };

    // Apply to GL_LINES, dotted pattern
    class CMaterialLinesDotted : public CMaterialLines
    {
    public:
        CMaterialLinesDotted(osg::Camera* camera, float thickness);
    };

    // Apply to GL_LINES, dashed pattern
    class CMaterialLinesDashed : public CMaterialLines
    {
    public:
        CMaterialLinesDashed(osg::Camera* camera, float thickness);
    };

} // namespace osg

#endif // CThickLineMaterial_H
