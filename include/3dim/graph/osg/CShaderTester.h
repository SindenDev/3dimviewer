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

#ifndef CShaderTester_H_included
#define CShaderTester_H_included
#ifdef __APPLE__
    #include <glew.h>
#else
    #include <GL/glew.h>
#endif
#include <string>
#include <VPL/Base/Logging.h>
#include <osg/Geometry>
#include <osg/CForceCullCallback.h>

#define SHADERLOG

namespace osg
{
    class CShaderTester : public osg::Geometry
    {
    protected:
        bool m_initialized;
        bool m_compiled;
        GLenum m_shaderType;
        std::string m_vertSource;
        std::string m_geomSource;
        std::string m_fragSource;
        std::string m_programName;

    public:
        CShaderTester(std::string programName, std::string vertSource, std::string geomSource, std::string fragSource);
        ~CShaderTester();
        GLuint createAndCompileShader(GLenum shaderType, const char *shaderSource) const;
        virtual void drawImplementation(osg::RenderInfo &renderInfo) const;
    };
}

#endif // CShaderTester_H_included
