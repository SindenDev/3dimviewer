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
#include "sys/stat.h"
#include <osg/Version>

osg::CShaderTester::CShaderTester(std::string programName, std::string vertSource, std::string geomSource, std::string fragSource)
    : osg::Geometry()
    , m_initialized(false)
    , m_compiled(false)
    , m_vertSource(vertSource)
    , m_geomSource(geomSource)
    , m_fragSource(fragSource)
    , m_programName(programName)
{
    std::stringstream ss;
    ss << "CShaderTester " << programName;
    setName(ss.str());

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    setCullingActive(false);
#else
    setCullCallback(new osg::CForceCullCallback(true));
#endif
#ifdef SHADERLOG
	struct stat attrib;
	stat("shaderlog.txt", &attrib);
	tm modified = *localtime(&(attrib.st_mtime));
	time_t t = time(0);   // get time now
    tm now = *localtime( & t );
	if (now.tm_year != modified.tm_year ||
		now.tm_yday != modified.tm_yday)
	{
		std::ofstream out("shaderlog.txt", std::ios_base::trunc); // reset file contents
		out.close();
	}
#endif
}

osg::CShaderTester::~CShaderTester()
{ }

GLuint osg::CShaderTester::createAndCompileShader(GLenum shaderType, const char *shaderSource) const
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

#ifdef SHADERLOG
    std::ofstream out("shaderlog.txt", std::ios_base::app);
    out << shaderSource << std::endl;
#endif

    GLint result = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result != GL_TRUE)
    {
        GLchar buffer[2048 + 1] = { };
        GLsizei length;
        GLsizei size = 2048;
        glGetShaderInfoLog(shader, size, &length, buffer);

#ifdef SHADERLOG
        out << std::endl << buffer;
#endif
    }
    else
    {
#ifdef SHADERLOG
        out << std::endl << "SHADER OK" << std::endl << "////////////////////////////////////////" << std::endl << std::endl;
#endif
    }

#ifdef SHADERLOG
    out.close();
#endif

    return shader;
}

void osg::CShaderTester::drawImplementation(osg::RenderInfo &renderInfo) const
{
    if ((!m_initialized) /* && (glewInit() == GLEW_OK)*/) // can't call glewInit here!
    {
        *const_cast<bool *>(&m_initialized) = true;
    }

    if (!m_initialized || m_compiled)
    {
        return;
    }

    GLuint vertShader = 0;
    GLuint geomShader = 0;
    GLuint fragShader = 0;
    GLuint program = glCreateProgram();

    VPL_LOG_INFO("Testing shader program: " << m_programName << " ...");
    if (m_vertSource.length() > 0) { vertShader = createAndCompileShader(GL_VERTEX_SHADER, m_vertSource.c_str()); }
    if (m_geomSource.length() > 0) { geomShader = createAndCompileShader(GL_GEOMETRY_SHADER_EXT, m_geomSource.c_str()); }
    if (m_fragSource.length() > 0) { fragShader = createAndCompileShader(GL_FRAGMENT_SHADER, m_fragSource.c_str()); }

    if (vertShader > 0) { glAttachShader(program, vertShader); }
    if (geomShader > 0) { glAttachShader(program, geomShader); }
    if (fragShader > 0) { glAttachShader(program, fragShader); }

    glLinkProgram(program);
    GLint result = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result != GL_TRUE)
    {
        GLchar buffer[2048 + 1];
        GLsizei length;
        GLsizei size = 2048;
        glGetProgramInfoLog(program, size, &length, buffer);
        bool damn = true;
#ifdef SHADERLOG
        std::ofstream out("shaderlog.txt", std::ios_base::app);
        out << buffer;
        out.close();
#endif
    }
    else
    {
#ifdef SHADERLOG
        std::ofstream out("shaderlog.txt", std::ios_base::app);
        out << std::endl << "PROGRAM \"" << m_programName << "\" OK" << std::endl << "////////////////////////////////////////" << std::endl << std::endl;
        out.close();
#endif
    }

    glDeleteProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(geomShader);
    glDeleteShader(fragShader);

    *const_cast<bool *>(&m_compiled) = true;

    VPL_LOG_INFO("    ...OK");
}
