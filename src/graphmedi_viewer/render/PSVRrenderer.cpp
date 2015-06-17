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

#include <configure.h>

#define USE_VAORECT // there's a bug in that code - find it if you want to use it

// Check predefined type of volume rendering algorithm
#ifdef USE_PSVR

///////////////////////////////////////////////////////////////////////////////
// include files

// GLEW must be included first!
#ifdef __APPLE__
#    include <glew.h>
#else
#    include <GL/glew.h>
#endif
//#include <GL/gl.h>

#include <osg/OSGCanvas.h>

#include <render/PSVRrenderer.h>
#include <render/PSVRosg.h>
#include <render/PSVRshaders.h>
#include <render/CGraficCardDesc.h>
#include <osg/CSceneOSG.h>
#include "cvolumerendererwindow.h"

#include <VPL/Math/Base.h>
#include <VPL/Math/Random.h>
#include <VPL/System/Sleep.h>
//#include <VPL/Base/Logging.h>
#include <VPL/Image/VolumeFilters/Sobel.h>
#include <VPL/Image/VolumeFilters/Gaussian.h>

#include <app/Signals.h>

//#define ROUNDING_MASK 0xFFFFFFFE
#define ROUNDING_MASK 0xFFFFFFFC


#ifdef __APPLE__
    #define glGenVertexArraysX    glGenVertexArraysAPPLE
    #define glBindVertexArrayX    glBindVertexArrayAPPLE
    #define glDeleteVertexArraysX glDeleteVertexArraysAPPLE
    #define glIsVertexArrayX      glIsVertexArrayAPPLE
#else
    #define glGenVertexArraysX    glGenVertexArrays
    #define glBindVertexArrayX    glBindVertexArray
    #define glDeleteVertexArraysX glDeleteVertexArrays
    #define glIsVertexArrayX      glIsVertexArray
#endif

namespace PSVR
{

///////////////////////////////////////////////////////////////////////////////
//! Creates shader program from a given string.
//! - Returns false on failure.
bool createShaderProgram(const char * frag, GLuint * shader, GLuint * program)
{
    if (!frag || !shader || !program)
    {
        return false;
    }

    *shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*shader, 1, &frag, NULL);
    glCompileShader(*shader);

    if (glGetShaderiv)
    {
        GLint Result = GL_FALSE;
        glGetShaderiv(*shader, GL_COMPILE_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot compile fragment shader!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetShaderInfoLog(*shader, size, &length, buffer);

            VPL_LOG_INFO("glGetShaderInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    *program = glCreateProgram();
    glAttachShader(*program, *shader);
    glLinkProgram(*program);

    if (glGetProgramiv)
    {
        GLint Result = GL_FALSE;
        glGetProgramiv(*program, GL_LINK_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot link shader program!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetProgramInfoLog(*program, size, &length, buffer);

            VPL_LOG_INFO("glGetProgramInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Creates shader program from a given string.
bool createShaderProgram(const char * frag, const char * vert, GLuint * FragShader, GLuint * VertShader, GLuint * program)
{
    if (!frag || !vert || !FragShader || !VertShader || !program)
    {
        return false;
    }

    *FragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*FragShader, 1, &frag, NULL);
    glCompileShader(*FragShader);

    if (glGetShaderiv)
    {
        GLint Result = GL_FALSE;
        glGetShaderiv(*FragShader, GL_COMPILE_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot compile fragment shader!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetShaderInfoLog(*FragShader, size, &length, buffer);

            VPL_LOG_INFO("glGetShaderInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    *VertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*VertShader, 1, &vert, NULL);
    glCompileShader(*VertShader);

    if (glGetShaderiv)
    {
        GLint Result = GL_FALSE;
        glGetShaderiv(*VertShader, GL_COMPILE_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot compile vertex shader!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetShaderInfoLog(*VertShader, size, &length, buffer);

            VPL_LOG_INFO("glGetShaderInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    *program = glCreateProgram();
    glAttachShader(*program, *FragShader);
    glAttachShader(*program, *VertShader);
    glLinkProgram(*program);

    if (glGetProgramiv)
    {
        GLint Result = GL_FALSE;
        glGetProgramiv(*program, GL_LINK_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot link shader program!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetProgramInfoLog(*program, size, &length, buffer);

            VPL_LOG_INFO("glGetProgramInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
char * readShader(const char * Filename)
{
    FILE * pFile = fopen(Filename, "rb");
    if (!pFile)
    {
        return NULL;
    }
    fseek(pFile, 0, SEEK_END);
    long lSize = ftell(pFile);
    rewind(pFile);
    char * buffer = (char *)malloc(sizeof(char) * lSize + 1);
    size_t result = fread(buffer, 1, lSize, pFile);
    buffer[lSize] = '\0';
    fclose(pFile);
    return buffer;
}

///////////////////////////////////////////////////////////////////////////////
//
bool loadShaderProgram(const char * filename, unsigned * shader, unsigned * program)
{
    if (!filename || !shader || !program)
    {
        return false;
    }

    *shader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar * shaderfile = readShader(filename);
    if (!shaderfile)
    {
        VPL_LOG_INFO("Error: Cannot find shader program (" << filename << ")!");
        return false;
    }
    glShaderSource(*shader, 1, &shaderfile, NULL);
    free((void*)shaderfile);
    glCompileShader(*shader);

    if (glGetShaderiv)
    {
        GLint Result = GL_FALSE;
        glGetShaderiv(*shader, GL_COMPILE_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot compile fragment shader (" << filename << ")!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetShaderInfoLog(*shader, size, &length, buffer);

            VPL_LOG_INFO("glGetShaderInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    *program = glCreateProgram();
    glAttachShader(*program, *shader);
    glLinkProgram(*program);

    if (glGetProgramiv)
    {
        GLint Result = GL_FALSE;
        glGetProgramiv(*program, GL_LINK_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot link shader program!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetProgramInfoLog(*program, size, &length, buffer);

            VPL_LOG_INFO("glGetProgramInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool loadShaderProgram(const char * fragfilename, const char * vertfilename, unsigned * fragshader, unsigned * vertshader, unsigned * program)
{
    if (!fragfilename || !vertfilename || !fragshader || !vertshader || !program)
    {
        return false;
    }

    if (glIsShader(*fragshader) == GL_FALSE)
    {
        *fragshader = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const GLchar * shaderfile = readShader(fragfilename);
    if (!shaderfile)
    {
        VPL_LOG_INFO("Error: Cannot find shader program (" << fragfilename << ")!");
        return false;
    }
    glShaderSource(*fragshader, 1, &shaderfile, NULL);
    free((void*)shaderfile);
    glCompileShader(*fragshader);

    if (glGetShaderiv)
    {
        GLint Result = GL_FALSE;
        glGetShaderiv(*fragshader, GL_COMPILE_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot compile fragment shader (" << fragfilename << ")!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetShaderInfoLog(*fragshader, size, &length, buffer);

            VPL_LOG_INFO("glGetShaderInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    if (glIsShader(*vertshader) == GL_FALSE)
    {
        *vertshader = glCreateShader(GL_VERTEX_SHADER);
    }
    shaderfile = readShader(vertfilename);
    if (!shaderfile)
    {
        VPL_LOG_INFO("Error: Cannot find shader program (" << vertfilename << ")!");
        return false;
    }
    glShaderSource(*vertshader, 1, &shaderfile, NULL);
    free((void*)shaderfile);
    glCompileShader(*vertshader);

    if (glGetShaderiv)
    {
        GLint Result = GL_FALSE;
        glGetShaderiv(*vertshader, GL_COMPILE_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot compile vertex shader (" << vertfilename << ")!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetShaderInfoLog(*vertshader, size, &length, buffer);

            VPL_LOG_INFO("glGetShaderInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    if (glIsProgram(*program) == GL_FALSE)
    {
        *program = glCreateProgram();
    }
    glAttachShader(*program, *fragshader);
    glAttachShader(*program, *vertshader);
    glLinkProgram(*program);

    if (glGetProgramiv)
    {
        GLint Result = GL_FALSE;
        glGetProgramiv(*program, GL_LINK_STATUS, &Result);
        if (Result != GL_TRUE)
        {
            VPL_LOG_INFO("Error: Cannot link shader program!");

            GLchar buffer[2048 + 1];
            GLsizei length;
            GLsizei size = 2048;
            glGetProgramInfoLog(*program, size, &length, buffer);

            VPL_LOG_INFO("glGetProgramInfoLog():" << std::endl << buffer);

            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//! sqrt(2)/2
const double Sqrt3Div2 = 0.866025404;

namespace conf
{
    //! Raycasting shaders
    const char *shaderFilenames[] =
        { "shaders2/suma.frag"
        , "shaders2/max.frag"
        , "shaders2/shade.frag"
        , "shaders2/add.frag"
        , "shaders2/surface.frag"
    };

    //! Standard deviation of the random noise generator.
    const double NoiseSigma = 35.0;

    //! Number of quads that forms side of the rendered box.
    const int NumOfQuads = 15;

    //! Number of quads that forms side of the rendered box.
    const int NumOfBatches  = 10;

    // Rendering parameters for different quality levels.
    /////////////////////////////////////////////////////

    //! Rendering resolution.
    const int RenderingSize[PSVolumeRendering::QUALITY_LEVELS]     = { 128, 256, 512, 1024 };

    //! 3D texture sampling step.
    const float TextureSampling[PSVolumeRendering::QUALITY_LEVELS] = { 0.7f, 0.6f, 0.5f, 0.4f };
    //const float TextureSampling[PSVolumeRendering::QUALITY_LEVELS] = { 0.5f, 0.5f, 0.5f, 0.5f };

    //! Input volume data sub-sampling coefficient.
    const float DataSampling[PSVolumeRendering::QUALITY_LEVELS]    = { 0.25f, 0.25f, 0.5f, 1.0f };

    // Rendering parameters while using mouse.
    /////////////////////////////////////////////////////

    //! Rendering resolution (mouse mode).
    const int MouseRenderingSize[PSVolumeRendering::QUALITY_LEVELS]     = { 128, 128, 256, 512 };

    //! 3D texture sampling (mouse mode).
    //const float MouseTextureSampling[PSVolumeRendering::QUALITY_LEVELS] = { 0.8f, 0.8f, 0.8f, 0.8f };
    const float MouseTextureSampling[PSVolumeRendering::QUALITY_LEVELS] = { 0.75f, 0.75f, 0.75f, 0.75f };

    // Rectangular polygon
    /////////////////////////////////////////////////////

    static const float rectVertices[] = { 
         1.0f,  1.0f, 0.0f,   -1.0f,  1.0f, 0.0f,   -1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,    1.0f, -1.0f, 0.0f,    1.0f,  1.0f, 0.0f
    };
}

///////////////////////////////////////////////////////////////////////////////
//! OpenGL variables used by the renderer.
struct PSVolumeRendering::PSVolumeRenderingData
{
    GLuint VolumeTexture, AuxVolumeTexture, CustomAuxVolumeTexture, LookUpTexture;

    GLuint BicKerTexture, NoiseTexture, RTTexture, DEPTexture, RaysStartEnd;
    GLuint OffScreenFramebuffer0, OffScreenFramebuffer1, ResizeFramebuffer;
    
    GLuint GeometryDepthTexture;

    GLuint shaderFBO, PshaderFBO, shaderResize, PshaderResize;
    GLuint shaderRayCast[SHADERS_COUNT - 1], PshaderRayCast[SHADERS_COUNT - 1];
    GLuint vertexShader, fsQuadVS;

    GLuint VAOBox[conf::NumOfBatches], VAORect;
    GLuint VBOBox[conf::NumOfBatches], VBORect;
};

//! Parameters of the rendering.
struct PSVolumeRendering::PSVolumeRenderingParams
{
    ////////////////////////////////////////////////////////////////////////////
    // 3D volume information

    //! Volume data size in voxels.
    vpl::tSize XSize, YSize, ZSize;
    vpl::tSize CustomXSize, CustomYSize, CustomZSize;

    //! Real voxel size.
    float dX, dY, dZ;

    //! Real volume size (num. of voxels * real voxel size).
    float RealXSize, RealYSize, RealZSize;

    //! Aspect ratio.
    float aspectRatio_YtoX, aspectRatio_ZtoX;

    ////////////////////////////////////////////////////////////////////////////
    // Skipping volume information

    //! Skipping volume size
    vpl::tSize AuxXSize, AuxYSize, AuxZSize;
    vpl::tSize CustomAuxXSize, CustomAuxYSize, CustomAuxZSize;

    //! Skipping volume texture size
    float AuxTexXSize, AuxTexYSize, AuxTexZSize;
    float CustomAuxTexXSize, CustomAuxTexYSize, CustomAuxTexZSize;

    ////////////////////////////////////////////////////////////////////////////
    // Volume rendering parameters

    //! Intensity shifting and scaling.
    float dataPreMultiplication, dataOffset;

    //! Output image contrast and brightness.
    float imageContrast, imageBrightness;

    //! Surface detection parameters.
    float surfaceNormalMult, surfaceNormalExp;

    //! 3D texture sampling step along the ray...
    float volumeSamplingDistance;

    //! Cutting plane...
    float planeA, planeB, planeC, planeD, planeDeltaNear, planeDeltaFar;

    ////////////////////////////////////////////////////////////////////////////
    // Volume rendering state variables

    //! Current rendering quality.
    int currentQuality;

    //! Rendering resolution.
    int renderingSize;

    //! Currently selected shader.
    int selectedShader;

    //! Currently selected color LUT.
    int selectedLut;

    //! Inverted matrices for object-space position reconstruction of VR box
    float invProjectionMatrix[16];
    float invModelViewMatrix[16];
};

///////////////////////////////////////////////////////////////////////////////
//! constructor - initialize main variables
PSVolumeRendering::PSVolumeRendering()
    : m_GlewInit(0)
    , m_maximumVolumeSize(-1)
    , m_pCanvas(NULL)
    , m_Enabled(false)
    , m_Error(DATA_NOT_SPECIFIED)
    , m_Flags(0)
    , m_FailureCounter(0)
    , m_Mutex(false)
    , m_Thread(setupLoop, this, false)
    , m_spGLData(new PSVolumeRenderingData)
    , m_spParams(new PSVolumeRenderingParams)
    , m_spVolumeData(NULL)
    , m_customShaderId(0)
{
    memset(m_spGLData, 0, sizeof(PSVolumeRenderingData));

    m_Thread.resume();

    // Initialize volume and surface data
    m_VolumeData.resize(INIT_SIZE, INIT_SIZE, INIT_SIZE, 0);
    m_VolumeData.fillEntire(0);

    m_spParams->AuxXSize = m_spParams->AuxYSize = m_spParams->AuxZSize = INIT_SIZE / 8;
    m_AuxVolumeData.resize(m_spParams->AuxXSize, m_spParams->AuxYSize, m_spParams->AuxZSize, 0);
    m_AuxVolumeData.fillEntire(vpl::img::tRGBPixel(0));
    m_spParams->AuxTexXSize = m_spParams->AuxTexYSize = m_spParams->AuxTexZSize = 1.0f;

    // Initialize rendering parameters
    m_spParams->XSize = m_spParams->YSize = m_spParams->ZSize = INIT_SIZE;
    m_spParams->CustomXSize = m_spParams->CustomYSize = m_spParams->CustomZSize = 0;
    m_spParams->RealXSize = m_spParams->RealYSize = m_spParams->RealZSize = INIT_SIZE;
    m_spParams->dX = m_spParams->dY = m_spParams->dZ = 1.0f;
    m_spParams->aspectRatio_YtoX = m_spParams->aspectRatio_ZtoX = 1.0f;

    m_spParams->AuxXSize = m_spParams->AuxYSize = m_spParams->AuxZSize = 0;
    m_spParams->CustomAuxXSize = m_spParams->CustomAuxYSize = m_spParams->CustomAuxZSize = 0;
    m_spParams->AuxTexXSize = m_spParams->AuxTexYSize = m_spParams->AuxTexZSize = 0;
    m_spParams->CustomAuxTexXSize = m_spParams->CustomAuxTexYSize = m_spParams->CustomAuxTexZSize = 0;

    m_spParams->dataPreMultiplication = 1.0f;
    m_spParams->dataOffset = 0.0f;

    m_spParams->imageContrast = 1.0f;
    m_spParams->imageBrightness = 0.0f;

    m_spParams->surfaceNormalMult = 15.0f;
    m_spParams->surfaceNormalExp = 2.0f;

    m_spParams->planeA = 1.0;
    m_spParams->planeB = 0.0;
    m_spParams->planeC = 0.0;
    m_spParams->planeD = -0.5;
    m_spParams->planeDeltaNear = 1.0;
    m_spParams->planeDeltaFar = -1.0;

    m_spParams->currentQuality = LOW_QUALITY;
    m_spParams->volumeSamplingDistance = conf::TextureSampling[LOW_QUALITY];
    m_spParams->renderingSize = conf::RenderingSize[LOW_QUALITY];

    m_spParams->selectedShader = SURFACE;
    m_spParams->selectedLut = SURFACE_BONE;

    // Prepare the box bounding volume data
    prepareBox(conf::NumOfQuads);
    //prepareBox(1);

    // create default lookup tables
    createLookupTables();
}

///////////////////////////////////////////////////////////////////////////////
//
PSVolumeRendering::~PSVolumeRendering()
{
    // Do not accept any changes
    //m_Thread.terminate(true);

    // Stop the rendering
    release();

    cleanup(); // we need that the destruction callback are called before destruction of this object
}

///////////////////////////////////////////////////////////////////////////////
// renderer plugin interface implementation
bool PSVolumeRendering::isCustomShaderActive()
{
    return (getShader() == CUSTOM);
}

QSize PSVolumeRendering::getWindowSize()
{
    return m_pCanvas->size();
}

osg::Matrix PSVolumeRendering::getWorldMatrix()
{
    osg::Matrix retMatrix = osg::Matrix::identity();

    CVolumeRendererWindow *vrWindow = dynamic_cast<CVolumeRendererWindow *>(m_pCanvas);
    if (vrWindow == NULL)
    {
        return retMatrix;
    }

    scene::CScene3D *vrScene = dynamic_cast<scene::CScene3D *>(vrWindow->getView()->getSceneData());
    if (vrScene == NULL)
    {
        return retMatrix;
    }

    PSVR::osgPSVolumeRenderingGeode *vrGeode = dynamic_cast<PSVR::osgPSVolumeRenderingGeode *>(vrScene->getVRGeode());
    if (vrGeode == NULL)
    {
        return retMatrix;
    }
    
    osg::Vec3 scale = osg::Vec3(getRealXSize(), getRealYSize(), getRealZSize());
    scale *= 0.5f;
    osg::Matrix scaleMatrix = osg::Matrix::scale(scale);
    retMatrix = scaleMatrix * vrGeode->getWorldMatrices()[0];
    
    return retMatrix;
}

osg::Matrix PSVolumeRendering::getViewMatrix()
{
    osg::Matrix retMatrix = osg::Matrix::identity();

    CVolumeRendererWindow *vrWindow = dynamic_cast<CVolumeRendererWindow *>(m_pCanvas);
    if (vrWindow == NULL)
    {
        return retMatrix;
    }
    retMatrix = vrWindow->getView()->getCamera()->getViewMatrix();

    return retMatrix;
}

osg::Matrix PSVolumeRendering::getProjectionMatrix()
{
    osg::Matrix retMatrix = osg::Matrix::identity();

    CVolumeRendererWindow *vrWindow = dynamic_cast<CVolumeRendererWindow *>(m_pCanvas);
    if (vrWindow == NULL)
    {
        return retMatrix;
    }
    retMatrix = vrWindow->getView()->getCamera()->getProjectionMatrix();

    return retMatrix;
}

osg::Matrix PSVolumeRendering::getTransformMatrix()
{
    return getWorldMatrix() * getViewMatrix() * getProjectionMatrix();
}

unsigned int PSVolumeRendering::internalCreateCustomShader(std::string vertexShaderSource, std::string fragmentShaderSource)
{ 
    if (!testFlag(INITIALIZED))
    {
        return 0;
    }

    if (!m_pCanvas->getGraphicWindow()->makeCurrentImplementation())
    {
        return 0;
    }

    GLuint vertexShaderId, fragmentShaderId, programId;
    if (!createShaderProgram(fragmentShaderSource.c_str(), vertexShaderSource.c_str(), &fragmentShaderId, &vertexShaderId, &programId))
    {
        m_pCanvas->getGraphicWindow()->releaseContextImplementation();
        return 0;
    }
    m_pCanvas->getGraphicWindow()->releaseContextImplementation();

    m_programShaders[programId].push_back(fragmentShaderId);
    m_programShaders[programId].push_back(vertexShaderId);

    return programId;
}

void PSVolumeRendering::internalDeleteCustomShader(unsigned int shaderId)
{
    if (!m_pCanvas->getGraphicWindow()->makeCurrentImplementation())
    {
        return;
    }

    if ((getShader() == CUSTOM) && (m_customShaderId == shaderId))
    {
        useCustomShader(0);
    }

    std::vector<unsigned int> shaders = m_programShaders[shaderId];
    for (std::size_t i = 0; i < shaders.size(); ++i)
    {
        glDeleteShader(shaders[i]);
    }
    glDeleteProgram(shaderId);

    m_programShaders.erase(shaderId);

    m_pCanvas->getGraphicWindow()->releaseContextImplementation();
}

void PSVolumeRendering::internalUseCustomShader(unsigned int shaderId)
{
    m_customShaderId = shaderId;
    setShader(CUSTOM);
    enable();
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, int value)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    glUniform1i(pos, value);
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, float value)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    glUniform1f(pos, value);
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, osg::Vec2 value)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    glUniform2f(pos, value.x(), value.y());
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, osg::Vec3 value)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    glUniform3f(pos, value.x(), value.y(), value.z());
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, osg::Vec4 value)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    glUniform4f(pos, value.x(), value.y(), value.z(), value.w());
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, osg::Matrix value)
{
    setParameter(shaderId, name, &value, 1);
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, int *value, int count)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    glUniform1iv(pos, count, value);
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, float *value, int count)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    glUniform1fv(pos, count, value);
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, osg::Vec2 *value, int count)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    osg::Vec2::value_type *temp = new osg::Vec2::value_type[2 * count];
    for (int i = 0; i < count; ++i)
    {
        temp[i * 2 + 0] = value[i][0];
        temp[i * 2 + 1] = value[i][1];
    }
    glUniform2fv(pos, count, temp);
    delete temp;
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, osg::Vec3 *value, int count)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    osg::Vec3::value_type *temp = new osg::Vec3::value_type[3 * count];
    for (int i = 0; i < count; ++i)
    {
        temp[i * 3 + 0] = value[i][0];
        temp[i * 3 + 1] = value[i][1];
        temp[i * 3 + 2] = value[i][2];
    }
    glUniform3fv(pos, count, temp);
    delete temp;
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, osg::Vec4 *value, int count)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    osg::Vec4::value_type *temp = new osg::Vec4::value_type[4 * count];
    for (int i = 0; i < count; ++i)
    {
        temp[i * 4 + 0] = value[i][0];
        temp[i * 4 + 1] = value[i][1];
        temp[i * 4 + 2] = value[i][2];
        temp[i * 4 + 3] = value[i][3];
    }
    glUniform4fv(pos, count, temp);
    delete temp;
}

void PSVolumeRendering::setParameter(unsigned int shaderId, std::string name, osg::Matrix *value, int count)
{
    GLint pos = glGetUniformLocation(shaderId, name.c_str());
    float *temp = new float[16 * count];
    for (int m = 0; m < count; ++m)
    {
        for (int i = 0; i < 16; i++)
        {
            temp[m * 16 + i] = value[m].ptr()[i];
        }
    }
    glUniformMatrix4fv(pos, count, false, temp);
    delete temp;
}

unsigned int PSVolumeRendering::internalCreateCustomVolume()
{
    if (!testFlag(INITIALIZED))
    {
        return 0;
    }

    if (!m_pCanvas->getGraphicWindow()->makeCurrentImplementation())
    {
        m_pCanvas->getGraphicWindow()->releaseContextImplementation();
        return 0;
    }

    unsigned int textureId = 0;
    glGenTextures(1, &textureId);

    int currBinding = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &currBinding);
    glBindTexture(GL_TEXTURE_3D, textureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_3D, currBinding);

    m_pCanvas->getGraphicWindow()->releaseContextImplementation();

    return textureId;
}

void PSVolumeRendering::internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<bool> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume)
{
    tLock(*this);
    m_Error &= (0xffff - CUSTOM_DATA_NOT_SPECIFIED);
    setAndSignalFlag(CUSTOM_DATA_INVALID);
}

void PSVolumeRendering::internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel16> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume)
{
    tLock(*this);
    m_Error &= (0xffff - CUSTOM_DATA_NOT_SPECIFIED);
    setAndSignalFlag(CUSTOM_DATA_INVALID);
}

void PSVolumeRendering::internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel8> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume)
{
    tLock(*this);
    m_Error &= (0xffff - CUSTOM_DATA_NOT_SPECIFIED);
    setAndSignalFlag(CUSTOM_DATA_INVALID);
}

void PSVolumeRendering::internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tRGBPixel> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume)
{
    tLock(*this);
    m_Error &= (0xffff - CUSTOM_DATA_NOT_SPECIFIED);
    setAndSignalFlag(CUSTOM_DATA_INVALID);
}

void PSVolumeRendering::getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<bool> &volume)
{
    tLock(*this);
    bool loadingData = testFlag(CUSTOM_DATA_INVALID);
    bool loadingTexture = testFlag(CUSTOM_TEXTURE_INVALID);

    // custom data is copied from CVolumeRenderer::m_volume_bool to PSVolumeRendering::m_customData_bool
    if (loadingData)
    {
        volume.copy(m_volume_bool);
    }
    // custom data is copied from PSVolumeRendering::m_customData_bool to texture
    else if (loadingTexture)
    {
        vpl::tSize XSize = m_customData_bool.getXSize();
        vpl::tSize YSize = m_customData_bool.getYSize();
        vpl::tSize ZSize = m_customData_bool.getZSize();

        volume.resize(XSize, YSize, ZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
        for (vpl::tSize z = 0; z < ZSize; z++)
        {
            for (vpl::tSize y = 0; y < YSize; y++)
            {
                for (vpl::tSize x = 0; x < XSize; x++)
                {
                    volume(x, y, z) = m_customData_bool(x, y, z);
                }
            }
        }
    }
    // custom data is in the graphics card
    else
    {
        // save state
        GLint boundTexture, boundReadFbo;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &boundReadFbo);
        glGetIntegerv(GL_TEXTURE_3D_BINDING_EXT, &boundTexture);

        // create working framebuffer
        GLuint workingFbo;
        glGenFramebuffersEXT(1, &workingFbo);

        // bind working objects
        glBindTexture(GL_TEXTURE_3D, volumeId);
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, workingFbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        // get volume info
        GLint w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        volume.resize(w, h, d);

        // grab 3D texture slice by slice
        vpl::img::CImage<vpl::img::tPixel8, vpl::base::CRefData> slice_tPixel8(volume.getXSize(), volume.getYSize());
        vpl::img::CImage<bool, vpl::base::CRefData> slice_bool(volume.getXSize(), volume.getYSize());
        for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
        {
            glFramebufferTexture3DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, volumeId, 0, z);
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glReadPixels(0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, slice_tPixel8.getPtr());

            // convert to bool values
            for (vpl::tSize y = 0; y < slice_bool.getYSize(); ++y)
            {
                for (vpl::tSize x = 0; x < slice_bool.getXSize(); ++x)
                {
                    vpl::img::tPixel8 value = slice_tPixel8.at(x, y);
                    slice_bool.at(x, y) = (value != 0);
                }
            }

            volume.setPlaneXY(z, slice_bool);
        }

        // restore state
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, boundReadFbo);
        glBindTexture(GL_TEXTURE_3D, boundTexture);

        // clean
        glDeleteFramebuffersEXT(1, &workingFbo);

        // grab error
        GLenum error = glGetError();
    }

    return;
}

void PSVolumeRendering::getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel16> &volume)
{
    tLock(*this);
    bool loadingData = testFlag(CUSTOM_DATA_INVALID);
    bool loadingTexture = testFlag(CUSTOM_TEXTURE_INVALID);

    // custom data is copied from CVolumeRenderer::m_volume_tPixel16 to PSVolumeRendering::m_customData_tPixel16
    if (loadingData)
    {
        volume.copy(m_volume_tPixel16);
    }
    // custom data is copied from PSVolumeRendering::m_customData_tPixel16 to texture
    else if (loadingTexture)
    {
        vpl::tSize XSize = m_customData_tPixel16.getXSize();
        vpl::tSize YSize = m_customData_tPixel16.getYSize();
        vpl::tSize ZSize = m_customData_tPixel16.getZSize();

        volume.resize(XSize, YSize, ZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
        for (vpl::tSize z = 0; z < ZSize; z++)
        {
            for (vpl::tSize y = 0; y < YSize; y++)
            {
                for (vpl::tSize x = 0; x < XSize; x++)
                {
                    volume(x, y, z) = m_customData_tPixel16(x, y, z);
                }
            }
        }
    }
    // custom data is in the graphics card
    else
    {
        // save state
        GLint boundTexture, boundReadFbo;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &boundReadFbo);
        glGetIntegerv(GL_TEXTURE_3D_BINDING_EXT, &boundTexture);

        // create working framebuffer
        GLuint workingFbo;
        glGenFramebuffersEXT(1, &workingFbo);

        // bind working objects
        glBindTexture(GL_TEXTURE_3D, volumeId);
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, workingFbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        // get volume info
        GLint w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        volume.resize(w, h, d);

        // grab 3D texture slice by slice
        vpl::img::CImage<vpl::img::tPixel16, vpl::base::CRefData> slice(volume.getXSize(), volume.getYSize());
        for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
        {
            glFramebufferTexture3DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, volumeId, 0, z);
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glReadPixels(0, 0, w, h, GL_RED, GL_UNSIGNED_SHORT, slice.getPtr());

            volume.setPlaneXY(z, slice);
        }

        // restore state
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, boundReadFbo);
        glBindTexture(GL_TEXTURE_3D, boundTexture);

        // clean
        glDeleteFramebuffersEXT(1, &workingFbo);

        // grab error
        GLenum error = glGetError();
    }

    return;
}

void PSVolumeRendering::getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel8> &volume)
{
    tLock(*this);
    bool loadingData = testFlag(CUSTOM_DATA_INVALID);
    bool loadingTexture = testFlag(CUSTOM_TEXTURE_INVALID);

    // custom data is copied from CVolumeRenderer::m_volume_tPixel8 to PSVolumeRendering::m_customData_tPixel8
    if (loadingData)
    {
        volume.copy(m_volume_tPixel8);
    }
    // custom data is copied from PSVolumeRendering::m_customData_tPixel8 to texture
    else if (loadingTexture)
    {
        vpl::tSize XSize = m_customData_tPixel8.getXSize();
        vpl::tSize YSize = m_customData_tPixel8.getYSize();
        vpl::tSize ZSize = m_customData_tPixel8.getZSize();

        volume.resize(XSize, YSize, ZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
        for (vpl::tSize z = 0; z < ZSize; z++)
        {
            for (vpl::tSize y = 0; y < YSize; y++)
            {
                for (vpl::tSize x = 0; x < XSize; x++)
                {
                    volume(x, y, z) = m_customData_tPixel8(x, y, z);
                }
            }
        }
    }
    // custom data is in the graphics card
    else
    {
        // save state
        GLint boundTexture, boundReadFbo;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &boundReadFbo);
        glGetIntegerv(GL_TEXTURE_3D_BINDING_EXT, &boundTexture);

        // create working framebuffer
        GLuint workingFbo;
        glGenFramebuffersEXT(1, &workingFbo);

        // bind working objects
        glBindTexture(GL_TEXTURE_3D, volumeId);
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, workingFbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        // get volume info
        GLint w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        volume.resize(w, h, d);

        // grab 3D texture slice by slice
        vpl::img::CImage<vpl::img::tPixel8, vpl::base::CRefData> slice(volume.getXSize(), volume.getYSize());
        for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
        {
            glFramebufferTexture3DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, volumeId, 0, z);
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glReadPixels(0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, slice.getPtr());

            volume.setPlaneXY(z, slice);
        }

        // restore state
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, boundReadFbo);
        glBindTexture(GL_TEXTURE_3D, boundTexture);

        // clean
        glDeleteFramebuffersEXT(1, &workingFbo);

        // grab error
        GLenum error = glGetError();
    }

    return;
}

void PSVolumeRendering::getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tRGBPixel> &volume)
{
    tLock(*this);
    bool loadingData = testFlag(CUSTOM_DATA_INVALID);
    bool loadingTexture = testFlag(CUSTOM_TEXTURE_INVALID);

    // custom data is copied from CVolumeRenderer::m_volume_tRGBPixel to PSVolumeRendering::m_customData_tRGBPixel
    if (loadingData)
    {
        volume.copy(m_volume_tRGBPixel);
    }
    // custom data is copied from PSVolumeRendering::m_customData_tRGBPixel to texture
    else if (loadingTexture)
    {
        vpl::tSize XSize = m_customData_tRGBPixel.getXSize();
        vpl::tSize YSize = m_customData_tRGBPixel.getYSize();
        vpl::tSize ZSize = m_customData_tRGBPixel.getZSize();

        volume.resize(XSize, YSize, ZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
        for (vpl::tSize z = 0; z < ZSize; z++)
        {
            for (vpl::tSize y = 0; y < YSize; y++)
            {
                for (vpl::tSize x = 0; x < XSize; x++)
                {
                    volume(x, y, z) = m_customData_tRGBPixel(x, y, z);
                }
            }
        }
    }
    // custom data is in the graphics card
    else
    {
        // save state
        GLint boundTexture, boundReadFbo;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &boundReadFbo);
        glGetIntegerv(GL_TEXTURE_3D_BINDING_EXT, &boundTexture);

        // create working framebuffer
        GLuint workingFbo;
        glGenFramebuffersEXT(1, &workingFbo);

        // bind working objects
        glBindTexture(GL_TEXTURE_3D, volumeId);
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, workingFbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        // get volume info
        GLint w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        volume.resize(w, h, d);

        // grab 3D texture slice by slice
        vpl::img::CImage<vpl::img::tRGBPixel, vpl::base::CRefData> slice(volume.getXSize(), volume.getYSize());
        for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
        {
            glFramebufferTexture3DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, volumeId, 0, z);
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, slice.getPtr());

            volume.setPlaneXY(z, slice);
        }

        // restore state
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, boundReadFbo);
        glBindTexture(GL_TEXTURE_3D, boundTexture);

        // clean
        glDeleteFramebuffersEXT(1, &workingFbo);

        // grab error
        GLenum error = glGetError();
    }

    return;
}

void PSVolumeRendering::resetLookupTables()
{
    createLookupTables();
    updateLookupTables();
}

void PSVolumeRendering::updateLookupTables()
{
    updateLookupTable(m_lookupTables["MIP_SOFT"],       m_internalLookupTables[MIP_SOFT],       m_skipConditions[MIP_SOFT]);
    updateLookupTable(m_lookupTables["MIP_HARD"],       m_internalLookupTables[MIP_HARD],       m_skipConditions[MIP_HARD]);
    updateLookupTable(m_lookupTables["XRAY_SOFT"],      m_internalLookupTables[XRAY_SOFT],      m_skipConditions[XRAY_SOFT]);
    updateLookupTable(m_lookupTables["XRAY_HARD"],      m_internalLookupTables[XRAY_HARD],      m_skipConditions[XRAY_HARD]);
    updateLookupTable(m_lookupTables["SHA_AIR"],        m_internalLookupTables[SHA_AIR],        m_skipConditions[SHA_AIR]);
    updateLookupTable(m_lookupTables["SHA_TRAN"],       m_internalLookupTables[SHA_TRAN],       m_skipConditions[SHA_TRAN]);
    updateLookupTable(m_lookupTables["SHA_BONE0"],      m_internalLookupTables[SHA_BONE0],      m_skipConditions[SHA_BONE0]);
    updateLookupTable(m_lookupTables["SHA_BONE1"],      m_internalLookupTables[SHA_BONE1],      m_skipConditions[SHA_BONE1]);
    updateLookupTable(m_lookupTables["SHA_BONE2"],      m_internalLookupTables[SHA_BONE2],      m_skipConditions[SHA_BONE2]);
    updateLookupTable(m_lookupTables["SURFACE_SKIN"],   m_internalLookupTables[SURFACE_SKIN],   m_skipConditions[SURFACE_SKIN]);
    updateLookupTable(m_lookupTables["SURFACE_BONE"],   m_internalLookupTables[SURFACE_BONE],   m_skipConditions[SURFACE_BONE]);

    setLut(getLut());
}

void PSVolumeRendering::updateLookupTable(CLookupTable &lookupTable, unsigned short *internalLookupTable, float &skipCondition)
{
    for (int i = LUT_1D_SIZE - 1; i >= 0; --i)
    {
        double position = double(i) / double(LUT_1D_SIZE);
        osg::Vec4 color = lookupTable.color(position);

        internalLookupTable[4 * i + 0] = static_cast<unsigned short>(color.r() * 65535.0);
        internalLookupTable[4 * i + 1] = static_cast<unsigned short>(color.g() * 65535.0);
        internalLookupTable[4 * i + 2] = static_cast<unsigned short>(color.b() * 65535.0);
        internalLookupTable[4 * i + 3] = static_cast<unsigned short>((1.0 - color.a()) * 65535.0);

        if (color.a() > 0.0f)
        {
            skipCondition = position;
        }
    }
}

void PSVolumeRendering::internalDeleteCustomVolume(unsigned int volumeId)
{
    glDeleteTextures(1, &volumeId);
}

///////////////////////////////////////////////////////////////////////////////
//
float PSVolumeRendering::getRealXSize() const
{
    return m_spParams->RealXSize;
}

float PSVolumeRendering::getRealYSize() const
{
    return m_spParams->RealYSize;
}

float PSVolumeRendering::getRealZSize() const
{
    return m_spParams->RealZSize;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::redraw(bool bEraseBackground)
{
    if (m_pCanvas)
    {
        m_pCanvas->Refresh(bEraseBackground);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::enable(bool bEnable)
{
    m_Enabled = bEnable;

    // reload shaders
    if ((m_Enabled) && (m_Flags & INITIALIZED))
    {
        reloadShader();
    }
    // signal others that VR has been enabled
    VPL_SIGNAL(SigVREnabledChange).invoke(bEnable);

    // redraw
    if (m_pCanvas)
    {
        m_pCanvas->Refresh(false);
    }
}

///////////////////////////////////////////////////////////////////////////////
// are we able to render?
bool PSVolumeRendering::canStart()
{
    tLock Lock(*this);

    // Set the current OpenGL rendering context
    if (!m_pCanvas->getGraphicWindow()->makeCurrentImplementation())
    {
        return false;
    }

    bool bResult = internalCanStart();

    // Release the graphic context
    m_pCanvas->getGraphicWindow()->releaseContextImplementation();

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// take CDensityVolume and copy data and information
void PSVolumeRendering::uploadData(vpl::img::CDensityVolume * pData)
{
    if (!pData)
    {
        return;
    }

    tLock Lock(*this);

    // Store reference to the data
    vpl::img::CDensityVolume *workingPtr;

    data::CObjectPtr<data::CDensityData> spVolumeData(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()));
    workingPtr = spVolumeData.get();

    m_spParams->RealXSize = float(workingPtr->getXSize() * workingPtr->getDX());
    m_spParams->RealYSize = float(workingPtr->getYSize() * workingPtr->getDY());
    m_spParams->RealZSize = float(workingPtr->getZSize() * workingPtr->getDZ());
    m_Error &= (0xffff - DATA_NOT_SPECIFIED);

    m_pCanvas->getView()->home();

    setAndSignalFlag(DATA_INVALID | LUT_INVALID | OSR_INVALID);
}

///////////////////////////////////////////////////////////////////////////////
//! - Updates specified surface
void PSVolumeRendering::updateRenderTargets()
{
    setFlag(OSR_INVALID);
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::createLookupTables()
{
    m_internalLookupTables.clear();
    m_skipConditions.clear();
    for (int i = 0; i < LOOKUPS_COUNT; ++i)
    {
        m_internalLookupTables.push_back(new unsigned short[4 * LUT_1D_SIZE]);
        m_skipConditions.push_back(0.0f);
    }

    CLookupTable &mipSoft = m_lookupTables["MIP_SOFT"];
    CLookupTable &mipHard = m_lookupTables["MIP_HARD"];
    CLookupTable &xraySoft = m_lookupTables["XRAY_SOFT"];
    CLookupTable &xrayHard = m_lookupTables["XRAY_HARD"];
    CLookupTable &shadingAir = m_lookupTables["SHA_AIR"];
    CLookupTable &shadingTransparent = m_lookupTables["SHA_TRAN"];
    CLookupTable &shadingBone0 = m_lookupTables["SHA_BONE0"];
    CLookupTable &shadingBone1 = m_lookupTables["SHA_BONE1"];
    CLookupTable &shadingBone2 = m_lookupTables["SHA_BONE2"];
    CLookupTable &surfaceSkin = m_lookupTables["SURFACE_SKIN"];
    CLookupTable &surfaceBone = m_lookupTables["SURFACE_BONE"];

    mipSoft.setName("MIP soft");
    mipSoft.clear();
    mipSoft.addComponent();
    mipSoft.setName(0, "component0");
    mipSoft.addPoint(0, 0.000, osg::Vec4(0.000, 0.000, 0.000, 0.000));
    mipSoft.addPoint(0, 0.123, osg::Vec4(0.870, 0.705, 0.262, 1.000));
    mipSoft.addPoint(0, 0.246, osg::Vec4(0.000, 0.000, 0.000, 0.000));

    mipHard.setName("MIP hard");
    mipHard.clear();
    mipHard.addComponent();
    mipHard.setName(0, "component0");
    mipHard.addPoint(0, 0.174, osg::Vec4(0.000, 0.000, 0.000, 0.000));
    mipHard.addPoint(0, 0.533, osg::Vec4(0.988, 0.988, 0.988, 1.000));
    mipHard.addPoint(0, 1.000, osg::Vec4(1.000, 1.000, 1.000, 1.000));

    xraySoft.setName("X-ray soft");
    xraySoft.clear();
    xraySoft.addComponent();
    xraySoft.setName(0, "component0");
    xraySoft.addPoint(0, 0.038, osg::Vec4(0.000, 0.000, 0.000, 0.000));
    xraySoft.addPoint(0, 0.130, osg::Vec4(0.341, 0.266, 0.101, 0.139));
    xraySoft.addPoint(0, 0.309, osg::Vec4(0.015, 0.039, 0.109, 0.000));
    xraySoft.addPoint(0, 0.310, osg::Vec4(0.000, 0.000, 0.000, 0.000));

    xrayHard.setName("X-ray hard");
    xrayHard.clear();
    xrayHard.addComponent();
    xrayHard.setName(0, "component0");
    xrayHard.addPoint(0, 0.147, osg::Vec4(0.000, 0.000, 0.000, 1.000));
    xrayHard.addPoint(0, 0.148, osg::Vec4(0.043, 0.070, 0.101, 1.000));
    xrayHard.addPoint(0, 1.000, osg::Vec4(1.000, 1.000, 1.000, 1.000));

    shadingAir.setName("Shading - air");
    shadingAir.clear();
    shadingAir.addComponent();
    shadingAir.setName(0, "component0");
    shadingAir.addPoint(0, 0.105, osg::Vec4(0.000, 0.000, 0.000, 0.000));
    shadingAir.addPoint(0, 0.106, osg::Vec4(0.000, 0.000, 0.000, 0.040));
    shadingAir.addPoint(0, 0.139, osg::Vec4(0.407, 0.988, 0.960, 0.204));
    shadingAir.addPoint(0, 0.162, osg::Vec4(0.000, 0.000, 0.000, 0.000));

    shadingTransparent.setName("Shading - transparent");
    shadingTransparent.clear();
    shadingTransparent.addComponent();
    shadingTransparent.setName(0, "component0");
    shadingTransparent.addPoint(0, 0.071, osg::Vec4(0.000, 0.000, 0.000, 0.000));
    shadingTransparent.addPoint(0, 0.129, osg::Vec4(0.988, 0.000, 0.000, 0.034));
    shadingTransparent.addPoint(0, 0.181, osg::Vec4(0.952, 0.968, 0.019, 0.034));
    shadingTransparent.addPoint(0, 0.223, osg::Vec4(0.082, 0.980, 0.000, 0.051));
    shadingTransparent.addPoint(0, 0.273, osg::Vec4(0.529, 1.000, 0.952, 0.170));
    shadingTransparent.addPoint(0, 0.412, osg::Vec4(0.788, 0.843, 1.000, 1.000));
    shadingTransparent.addPoint(0, 1.000, osg::Vec4(0.007, 0.027, 0.980, 1.000));

    shadingBone0.setName("Shading - bone (skull)");
    shadingBone0.clear();
    shadingBone0.addComponent();
    shadingBone0.setName(0, "bone");
    shadingBone0.setAlphaFactor(0, 0.172);
    shadingBone0.addPoint(0, 0.265, osg::Vec4(1.000, 0.992, 0.949, 0.000));
    shadingBone0.addPoint(0, 0.273, osg::Vec4(1.000, 0.988, 0.917, 1.000));
    shadingBone0.addComponent();
    shadingBone0.setName(1, "skin");
    shadingBone0.setAlphaFactor(1, 0.028);
    shadingBone0.addPoint(1, 0.126, osg::Vec4(1.000, 0.498, 0.498, 0.000));
    shadingBone0.addPoint(1, 0.170, osg::Vec4(1.000, 0.498, 0.498, 1.000));
    shadingBone0.addPoint(1, 0.223, osg::Vec4(1.000, 0.498, 0.498, 0.000));

    shadingBone1.setName("Shading - bone (spine)");
    shadingBone1.clear();
    shadingBone1.addComponent();
    shadingBone1.setName(0, "bone");
    shadingBone1.setAlphaFactor(0, 0.217);
    shadingBone1.addPoint(0, 0.190, osg::Vec4(1.000, 0.992, 0.949, 0.000));
    shadingBone1.addPoint(0, 0.201, osg::Vec4(1.000, 0.988, 0.917, 1.000));
    shadingBone1.addComponent();
    shadingBone1.setName(1, "skin");
    shadingBone1.setAlphaFactor(1, 0.028);
    shadingBone1.addPoint(1, 0.126, osg::Vec4(1.000, 0.498, 0.498, 0.000));
    shadingBone1.addPoint(1, 0.170, osg::Vec4(1.000, 0.498, 0.498, 1.000));
    shadingBone1.addPoint(1, 0.223, osg::Vec4(1.000, 0.498, 0.498, 0.000));

    shadingBone2.setName("Shading - bone (pelvis)");
    shadingBone2.clear();
    shadingBone2.addComponent();
    shadingBone2.setName(0, "bone");
    shadingBone2.setAlphaFactor(0, 0.172);
    shadingBone2.addPoint(0, 0.184, osg::Vec4(1.000, 0.992, 0.949, 0.000));
    shadingBone2.addPoint(0, 0.198, osg::Vec4(1.000, 0.988, 0.917, 1.000));
    shadingBone2.addComponent();
    shadingBone2.setName(1, "skin");
    shadingBone2.setAlphaFactor(1, 0.037);
    shadingBone2.addPoint(1, 0.111, osg::Vec4(1.000, 0.498, 0.498, 0.000));
    shadingBone2.addPoint(1, 0.149, osg::Vec4(1.000, 0.498, 0.498, 1.000));
    shadingBone2.addPoint(1, 0.193, osg::Vec4(1.000, 0.498, 0.498, 0.000));

    surfaceSkin.setName("Surface - skin");
    surfaceSkin.clear();
    surfaceSkin.addComponent();
    surfaceSkin.setName(0, "component0");
    surfaceSkin.addPoint(0, 0.000, osg::Vec4(0.000, 0.000, 0.000, 0.000));
    surfaceSkin.addPoint(0, 0.102, osg::Vec4(1.000, 0.996, 0.988, 1.000));
    surfaceSkin.addPoint(0, 1.000, osg::Vec4(0.498, 0.498, 0.498, 1.000));

    surfaceBone.setName("Surface - bones");
    surfaceBone.clear();
    surfaceBone.addComponent();
    surfaceBone.setName(0, "component0");
    surfaceBone.addPoint(0, 0.172, osg::Vec4(0.000, 0.000, 0.000, 0.000));
    surfaceBone.addPoint(0, 0.173, osg::Vec4(0.380, 0.258, 0.066, 0.000));
    surfaceBone.addPoint(0, 0.269, osg::Vec4(1.000, 1.000, 1.000, 1.000));

    updateLookupTables();
}

///////////////////////////////////////////////////////////////////////////////
// deallocate all used resources
void PSVolumeRendering::release()
{
    for (std::size_t i = 0; i < m_internalLookupTables.size(); ++i)
    {
        delete[] m_internalLookupTables[i];
    }
    m_internalLookupTables.clear();

    m_Thread.terminate(true);

    tLock Lock(*this);

    m_Enabled = false;
    m_Error = DATA_NOT_SPECIFIED;

    if (testFlag(INITIALIZED))
    {
        // delete textures
        glDeleteTextures(1, &m_spGLData->VolumeTexture);
        glDeleteTextures(1, &m_spGLData->LookUpTexture);
        glDeleteTextures(1, &m_spGLData->AuxVolumeTexture);
        glDeleteTextures(1, &m_spGLData->CustomAuxVolumeTexture);

        glDeleteTextures(1, &m_spGLData->RaysStartEnd);
        glDeleteTextures(1, &m_spGLData->BicKerTexture);
        glDeleteTextures(1, &m_spGLData->RTTexture);
        glDeleteTextures(1, &m_spGLData->DEPTexture);
        glDeleteTextures(1, &m_spGLData->NoiseTexture);

        glDeleteTextures(1, &m_spGLData->GeometryDepthTexture);

        // delete framebuffers
        glDeleteFramebuffersEXT(1, &m_spGLData->OffScreenFramebuffer0);
        glDeleteFramebuffersEXT(1, &m_spGLData->OffScreenFramebuffer1);
        glDeleteFramebuffersEXT(1, &m_spGLData->ResizeFramebuffer);

        // delete shaders
        glDeleteProgram(m_spGLData->PshaderFBO);
        for (int i = 0; i < SHADERS_COUNT - 1; ++i)
        {
            glDeleteProgram(m_spGLData->PshaderRayCast[i]);
            glDeleteShader(m_spGLData->shaderRayCast[i]);
        }
        glDeleteProgram(m_spGLData->PshaderResize);
        glDeleteShader(m_spGLData->vertexShader);
        glDeleteShader(m_spGLData->fsQuadVS);

        // VAO
        glDeleteVertexArraysX(conf::NumOfBatches, m_spGLData->VAOBox);
#ifdef USE_VAORECT
        glDeleteVertexArraysX(1, &m_spGLData->VAORect);
#endif

        // VBO
        glDeleteBuffersARB(conf::NumOfBatches, m_spGLData->VBOBox);
        glDeleteBuffersARB(1, &m_spGLData->VBORect);
    }

    // Clear all flags
    m_Flags = PSVR_NO_FLAGS;
}

///////////////////////////////////////////////////////////////////////////////
// Faster gauss filter implementation
float getFilteredVal(vpl::img::CDensityVolume * pVolume, int x, int y, int z)
{
    vpl::tSize zSums[3] = { };
    vpl::tSize xOff = pVolume->getXOffset();
    for (int dz = -1; dz <= 1; dz++)
    {
        vpl::tSize ySums[3] = { };
        for (int dy = -1; dy <= 1; dy++)
        {
            vpl::tSize idx = pVolume->getIdx(x, y + dy, z + dz);
            ySums[dy + 1] = pVolume->at(idx - xOff) + 3 * pVolume->at(idx) + pVolume->at(idx + xOff);
        }
        zSums[dz + 1] = ySums[0] + 3 * ySums[1] + ySums[2];
    }
    //float val = pVolume->at(x,y,z);
    float sum = (zSums[0] + 3 * zSums[1] + zSums[2]) / 125.0;
    return sum;
}

bool PSVolumeRendering::internalUploadData()
{
    vpl::img::CDensityVolume *workingPtr;
    data::CObjectPtr<data::CDensityData> spVolumeData(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()));
    workingPtr = spVolumeData.get();

    if (!workingPtr || workingPtr->getZSize() <= 0)
    {
        return false;
    }

    tLock Lock(*this);

    // Estimate volume dimensions
    m_spParams->XSize = workingPtr->getXSize();
    m_spParams->YSize = workingPtr->getYSize();
    m_spParams->ZSize = workingPtr->getZSize();

    // Real voxel size
    m_spParams->dX = workingPtr->getDX();
    m_spParams->dY = workingPtr->getDY();
    m_spParams->dZ = workingPtr->getDZ();

    // Data sub-sampling coeff
    float SubSampling = conf::DataSampling[m_spParams->currentQuality];

    // Sub-sampling...
    //if (SubSampling < 1.0f)
    if (SubSampling != 1.0f)
    {
        m_spParams->XSize = vpl::math::round2Int(float(m_spParams->XSize) * SubSampling);
        m_spParams->YSize = vpl::math::round2Int(float(m_spParams->YSize) * SubSampling);
        m_spParams->ZSize = vpl::math::round2Int(float(m_spParams->ZSize) * SubSampling);

        float InvSubSampling = 1.0f / SubSampling;
        m_spParams->dX *= InvSubSampling;
        m_spParams->dY *= InvSubSampling;
        m_spParams->dZ *= InvSubSampling;
    }

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->XSize &= ROUNDING_MASK;
    m_spParams->YSize &= ROUNDING_MASK;
    m_spParams->ZSize &= ROUNDING_MASK;

    // Voxel size correction for even texture size
    vpl::img::CVector3d realVolumeSize;
    realVolumeSize.x() = workingPtr->getDX() * workingPtr->getXSize();
    realVolumeSize.y() = workingPtr->getDY() * workingPtr->getYSize();
    realVolumeSize.z() = workingPtr->getDZ() * workingPtr->getZSize();
    m_spParams->dX = realVolumeSize.x() / m_spParams->XSize;
    m_spParams->dY = realVolumeSize.y() / m_spParams->YSize;
    m_spParams->dZ = realVolumeSize.z() / m_spParams->ZSize;

    // Allocate the data
    try
    {
        m_VolumeData.resize(m_spParams->XSize, m_spParams->YSize, m_spParams->ZSize, 0);
    }
    catch (std::bad_alloc &e)
    {
        VPL_LOG_INFO("Exception: VR cannot create volume (" << e.what() << ")");

        SubSampling = 1.0;
        m_spParams->XSize = m_spParams->YSize = m_spParams->ZSize = vpl::math::getMin<int>(INIT_SIZE, m_spParams->XSize, m_spParams->YSize, m_spParams->ZSize);
        m_spParams->dX = m_spParams->dY = m_spParams->dZ = 1.0;
        m_VolumeData.resize(m_spParams->XSize, m_spParams->YSize, m_spParams->ZSize, 0);
    }

    // Copy volume shape
    m_spParams->aspectRatio_YtoX = m_spParams->dY / m_spParams->dX;
    m_spParams->aspectRatio_ZtoX = m_spParams->dZ / m_spParams->dX;

    // Skipping volume
    m_spParams->AuxXSize = (m_spParams->XSize + 7) / 8 + 3;
    m_spParams->AuxYSize = (m_spParams->YSize + 7) / 8 + 3;
    m_spParams->AuxZSize = (m_spParams->ZSize + 7) / 8 + 3;

    // Even texture size...
    m_spParams->AuxXSize &= ROUNDING_MASK;
    m_spParams->AuxYSize &= ROUNDING_MASK;
    m_spParams->AuxZSize &= ROUNDING_MASK;
    
    // Allocate the skipping volume
    m_AuxVolumeData.resize(m_spParams->AuxXSize, m_spParams->AuxYSize, m_spParams->AuxZSize, 0);
    m_spParams->AuxTexXSize = float(m_spParams->XSize) / float(m_spParams->AuxXSize * 8);
    m_spParams->AuxTexYSize = float(m_spParams->YSize) / float(m_spParams->AuxYSize * 8);
    m_spParams->AuxTexZSize = float(m_spParams->ZSize) / float(m_spParams->AuxZSize * 8);

    // Clear the volume
    m_AuxVolumeData.fillEntire(vpl::img::tRGBPixel(0));

    // Scaling factor
    static const vpl::img::tDensityPixel voxelMax = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMax();
    static const vpl::img::tDensityPixel voxelMin = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin();
#ifdef FULL_3D_TEXTURE
    static const float dataScale = 65536.0f / float(voxelMax - voxelMin);
    static const float skipScale = 255.0f / 65536.0f;
#else
    static const float dataScale = 255.0f / float(voxelMax - voxelMin);
    static const float skipScale = 1.0f;
#endif // FULL_3D_TEXTURE

    //osg::Timer timer;
    //osg::Timer_t t1 = timer.tick();

    // Check if the data are empty (i.e. reset of the storage, etc.)
    vpl::img::tDensityPixel MaxValue = vpl::img::getMax<vpl::img::tDensityPixel>(*workingPtr);
    bool bEmptyData = (MaxValue == voxelMin) ? true : false;

    // No need to interpolate and copy the data if they are empty
    if (bEmptyData)
    {
        // Just clear the internal volume
        m_VolumeData.fillEntire(tVolumeData::tVoxel(0));
    }
    else
    {
        // Copy (and interpolate) data voxel by voxel
        //if (SubSampling < 1.0f)
        if (SubSampling != 1.0f)
        {
            double XStep = double(workingPtr->getXSize() - 1) / (m_spParams->XSize - 1);
            double YStep = double(workingPtr->getYSize() - 1) / (m_spParams->YSize - 1);
            double ZStep = double(workingPtr->getZSize() - 1) / (m_spParams->ZSize - 1);
#pragma omp parallel for schedule(static) default(shared)
            for (vpl::tSize z = 0; z < m_spParams->ZSize; z++)
            {
                vpl::img::CPoint3D Point(0.0, 0.0, z * ZStep);
                for (vpl::tSize y = 0; y < m_spParams->YSize; y++, Point.y() += YStep)
                {
                    Point.x() = 0.0;
                    for (vpl::tSize x = 0; x < m_spParams->XSize; x++, Point.x() += XStep)
                    {
                        tVolumeData::tVoxel normalizedPixel = tVolumeData::tVoxel(float(workingPtr->interpolate(Point) - voxelMin) * dataScale);
                        m_VolumeData(x, y, z) = normalizedPixel;
                    }
                }
            }
        }

        // No sub-sampling is required...
        else
        {
            //int clk = clock();
//#define MDSTK_GAUSS
#ifdef MDSTK_GAUSS
            // Gaussian filter
            vpl::img::CVolumeGauss3Filter<vpl::img::CDensityVolume> GaussFilter;
            // initialize kernel (before multiple threads kick in)
            float tmpPixel = float(GaussFilter.getResponse(*workingPtr, 0, 0, 0));
            m_VolumeData(0, 0, 0) = tmpPixel;
#endif

#pragma omp parallel for schedule(static) default(shared)
            for (vpl::tSize z = 0; z < m_spParams->ZSize; z++)
            {
                for (vpl::tSize y = 0; y < m_spParams->YSize; y++)
                {
                    for (vpl::tSize x = 0; x < m_spParams->XSize; x++)
                    {
#ifdef MDSTK_GAUSS
                        float Pixel = float(GaussFilter.getResponse(*workingPtr, x, y, z));
#else
                        float Pixel = float(getFilteredVal(workingPtr, x, y, z));
#endif
                        tVolumeData::tVoxel normalizedPixel = tVolumeData::tVoxel((Pixel - voxelMin) * dataScale);
                        //tVolumeData::tVoxel normalizedPixel = tVolumeData::tVoxel(float(workingPtr->at(x, y, z) - voxelMin) * dataScale);
                        m_VolumeData(x, y, z) = normalizedPixel;
                   }
                }
            }
            //clk = clock()-clk;
            //char sss[64]={};
            //sprintf(sss,"%d\n",clk);
            //OutputDebugStringA(sss);
        }

        // Sobel filters
        vpl::img::CVolumeSobelX<tVolumeData> SobelX;
        vpl::img::CVolumeSobelY<tVolumeData> SobelY;
        vpl::img::CVolumeSobelZ<tVolumeData> SobelZ;

        // Gradient normalization
        float gradScale = 0.33f * skipScale;

        // Skipping volume - min/max
        vpl::img::tRGBPixel::tComponent pixelMin = vpl::img::CPixelTraits<vpl::img::tRGBPixel>::getPixelMin().r();
        vpl::img::tRGBPixel::tComponent pixelMax = vpl::img::CPixelTraits<vpl::img::tRGBPixel>::getPixelMax().r();

#pragma omp parallel for schedule(static,8) default(shared)
        for (vpl::tSize z = 0; z < m_spParams->ZSize; z++)
        {
            vpl::tSize sz = z / 8;
            for (vpl::tSize y = 0; y < m_spParams->YSize; y++)
            {
                vpl::tSize sy = y / 8;
                for (vpl::tSize x = 0; x < m_spParams->XSize; x++)
                {
                    vpl::tSize sx = x / 8;
                    vpl::img::tRGBPixel& RGBPixel = m_AuxVolumeData(sx, sy, sz);

                    // Min/Max
                    vpl::img::tRGBPixel::tComponent SkipPixel = vpl::img::tRGBPixel::tComponent(skipScale * float(m_VolumeData(x, y, z)));
                    RGBPixel.r() = vpl::math::getMin<vpl::img::tRGBPixel::tComponent>(RGBPixel.r(), vpl::math::getMax<vpl::img::tRGBPixel::tComponent>(SkipPixel - 1, pixelMin));
                    RGBPixel.g() = vpl::math::getMax<vpl::img::tRGBPixel::tComponent>(RGBPixel.g(), vpl::math::getMin<vpl::img::tRGBPixel::tComponent>(SkipPixel + 1, pixelMax));
                }
            }
        }

        // Skipping volume - gradient magnitude
#pragma omp parallel for schedule(static,8) default(shared)
        for (vpl::tSize z = 1; z < (m_spParams->ZSize - 1); z += 2)
        {
            vpl::tSize sz = z / 8;
            for (vpl::tSize y = 1; y < (m_spParams->YSize - 1); y += 2)
            {
                vpl::tSize sy = y / 8;
                for (vpl::tSize x = 1; x < (m_spParams->XSize - 1); x += 2)
                {
                    vpl::tSize sx = x / 8;
                    vpl::img::tRGBPixel& RGBPixel = m_AuxVolumeData(sx, sy, sz);
                
                    // Edge magnitude
                    float GradX = float(SobelX.getResponse(m_VolumeData, x, y, z));
                    float GradY = float(SobelY.getResponse(m_VolumeData, x, y, z));
                    float GradZ = float(SobelZ.getResponse(m_VolumeData, x, y, z));
                    float GradMag = vpl::math::getAbs(GradX) + vpl::math::getAbs(GradY) + vpl::math::getAbs(GradZ);
                    vpl::img::tRGBPixel::tComponent Grad = vpl::img::tRGBPixel::tComponent(gradScale * GradMag);
                    RGBPixel.b() = vpl::math::getMax(RGBPixel.b(), Grad);
                }
            }
        }
    }

    //osg::Timer_t t2 = timer.tick();
    //double diff1 = timer.delta_m(t1, t2);
    //osg::Timer_t t3 = timer.tick();
    //double diff2 = timer.delta_m(t2, t3);

    // O.K.
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::storeVolumeToTexture(vpl::img::CVolume<vpl::img::tPixel8> volume)
{
    // create texture
    glTexImage3D(GL_TEXTURE_3D, 0,
        GL_LUMINANCE8,
        volume.getXSize(), volume.getYSize(), volume.getZSize(),
        0, GL_LUMINANCE,
        GL_UNSIGNED_BYTE,
        NULL);

    // fill it with data slice by slice
    vpl::img::CImage<vpl::img::tPixel8, vpl::base::CRefData> slice(volume.getXSize(), volume.getYSize());
    for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
    {
        volume.getPlaneXY(z, slice);

        glTexSubImage3D(GL_TEXTURE_3D, 0,
            0, 0, z,
            volume.getXSize(), volume.getYSize(), 1,
            GL_LUMINANCE,
            GL_UNSIGNED_BYTE,
            slice.getPtr());
    }
}

void PSVolumeRendering::storeVolumeToTexture(vpl::img::CVolume<vpl::img::tPixel16> volume)
{
    // create texture
    glTexImage3D(GL_TEXTURE_3D, 0,
        GL_LUMINANCE16,
        volume.getXSize(), volume.getYSize(), volume.getZSize(),
        0, GL_LUMINANCE,
        GL_UNSIGNED_SHORT,
        NULL);

    // fill it with data slice by slice
    vpl::img::CImage<vpl::img::tPixel16, vpl::base::CRefData> slice(volume.getXSize(), volume.getYSize());
    for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
    {
        volume.getPlaneXY(z, slice);

        glTexSubImage3D(GL_TEXTURE_3D, 0,
            0, 0, z,
            volume.getXSize(), volume.getYSize(), 1,
            GL_LUMINANCE,
            GL_UNSIGNED_SHORT,
            slice.getPtr());
    }
}

void PSVolumeRendering::storeVolumeToTexture(vpl::img::CVolume<vpl::img::tRGBPixel> volume)
{
    // create texture
    glTexImage3D(GL_TEXTURE_3D, 0,
        GL_RGBA8,
        volume.getXSize(), volume.getYSize(), volume.getZSize(),
        0, GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL);

    // fill it with data slice by slice
    vpl::img::CImage<vpl::img::tRGBPixel, vpl::base::CRefData> slice(volume.getXSize(), volume.getYSize());
    for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
    {
        volume.getPlaneXY(z, slice);

        glTexSubImage3D(GL_TEXTURE_3D, 0,
            0, 0, z,
            volume.getXSize(), volume.getYSize(), 1,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            slice.getPtr());
    }
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalCanStart()
{
    if (!m_pCanvas)
    {
        m_Error |= UNDEFINED_GL_CANVAS;
        VPL_LOG_INFO("Error: OpengGL canvas was not set!");
        return false;
    }

    // Check size of graphic card memory
    try
    {
        vr::CGraficCardDesc Desc;
        //std::string ssName = Desc.getAdapterName();
        unsigned int uiMem = Desc.getAdapterRAM();
        if (uiMem < 256)
        {
            m_Error |= UNSUPPORTED_GRAPHIC_CARD;
            VPL_LOG_INFO("Error: Not enough graphic memory!");
            return false;
        }
    }
    catch (...)
    {
        //m_Error |= UNSUPPORTED_GRAPHIC_CARD;
        //VPL_LOG_INFO("Error: Cannot inspect your graphic card!");
        //return false;
    }

    // First-time init of the GLEW library
    if (m_GlewInit == 0)
    {
        m_GlewInit = -1;
        glewExperimental = GL_TRUE;
        if (glewInit() == GLEW_OK)
        {
            m_GlewInit = 1;
        }
    }
    if (m_GlewInit < 0)
    {
        m_Error |= GLEW_INIT_FAILED;
        VPL_LOG_INFO("Error: Cannot initialize the GLEW library!");
        return false;
    }

    // check maximum size of volume texture
    if (m_maximumVolumeSize == -1)
    {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_3D, texture);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        long maximumSize = 1;
        int maximumTextureDimensions;
        glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maximumTextureDimensions);
        for (long size = 1; size <= maximumTextureDimensions; size *= 2)
        {
            glTexImage3D(GL_PROXY_TEXTURE_3D, 0, GL_RGB, size, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            GLenum error = glGetError();
            int w, h, d;
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

            if ((error == GL_NO_ERROR) && (w == size) && (h == size) && (d == size))
            {
                maximumSize = size;
            }
            else
            {
                break;
            }
        }
        glDeleteTextures(1, &texture);

        m_maximumVolumeSize = maximumSize;
    }

    // Check the OpenGL version
    if (!glewIsSupported("GL_VERSION_2_1") ||
        !glewIsSupported("GL_ARB_shading_language_100") ||
        !glewIsSupported("GL_ARB_vertex_buffer_object") ||
        !glewIsSupported("GL_ARB_vertex_array_object") ||
        !glewIsSupported("GL_EXT_framebuffer_object") ||
        //!glewIsSupported("GL_EXT_gpu_shader4") ||
        !glTexImage3D || !glCreateShader || !glGetUniformLocation ||
        !glFramebufferTexture2DEXT || !glGenBuffers)
    {
        m_Error |= UNSUPPORTED_SHADER_MODEL;
        VPL_LOG_INFO("Error: Unsupported graphic hardware (OpenGL 2.1 required)!");
        if (!glewIsSupported("GL_VERSION_2_1"))
            VPL_LOG_INFO("GL_VERSION_2_1 not supported");
        if (!glewIsSupported("GL_ARB_shading_language_100"))
            VPL_LOG_INFO("GL_ARB_shading_language_100 not supported");
        if (!glewIsSupported("GL_ARB_vertex_buffer_object"))
            VPL_LOG_INFO("GL_ARB_vertex_buffer_object not supported");
        if (!glewIsSupported("GL_ARB_vertex_array_object"))
            VPL_LOG_INFO("GL_ARB_vertex_array_object not supported");
        if (!glewIsSupported("GL_EXT_framebuffer_object"))
            VPL_LOG_INFO("GL_EXT_framebuffer_object not supported");
        if (!glTexImage3D || !glCreateShader || !glGetUniformLocation || !glFramebufferTexture2DEXT || !glGenBuffers)
            VPL_LOG_INFO("Some gl functions missing");
        return false;
    }
    if (sizeof(tCoords) != (3 * sizeof(float)))
    {
        m_Error |= UNSUPPORTED_SHADER_MODEL;
        VPL_LOG_INFO("Error: Unsupported memory alignment!");
        return false;
    }

    // O.K.
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// first-time initialization of OpenGL state, textures, shaders, lookups, ...
bool PSVolumeRendering::internalInitRendering()
{
    // Save current state
    bool bVertexArray = glIsEnabled(GL_VERTEX_ARRAY);
    bool bTexCoordArray = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
    bool bNormalArray = glIsEnabled(GL_NORMAL_ARRAY);
    bool bColorArray = glIsEnabled(GL_COLOR_ARRAY);
    bool bIndexArray = glIsEnabled(GL_INDEX_ARRAY);
    bool bSecondaryColorArray = glIsEnabled(GL_SECONDARY_COLOR_ARRAY);
    bool bEdgeFlagArray = glIsEnabled(GL_EDGE_FLAG_ARRAY);
    bool bFogCoordArray = glIsEnabled(GL_FOG_COORD_ARRAY);

    GLint arrayBuffer, elementArrayBuffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementArrayBuffer);

    m_ErrorStrings.clear();

    // Already initialized?
    if (testFlag(INITIALIZED))
    {
        return true;
    }

    bool retVal = true;
    int glerr = 0;

    // general OpenGL settings
    //glClearColor(0.2f, 0.2f, 0.4f, 0.0f);
    //glClearDepth(1.0f);
    //glEnable(GL_DEPTH_TEST);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    // generate textures
    glGenTextures(1, &m_spGLData->VolumeTexture);
    glGenTextures(1, &m_spGLData->LookUpTexture);
    glGenTextures(1, &m_spGLData->AuxVolumeTexture);
    glGenTextures(1, &m_spGLData->CustomAuxVolumeTexture);

    glGenTextures(1, &m_spGLData->RaysStartEnd);
    glGenTextures(1, &m_spGLData->BicKerTexture);
    glGenTextures(1, &m_spGLData->RTTexture);
    glGenTextures(1, &m_spGLData->DEPTexture);
    glGenTextures(1, &m_spGLData->NoiseTexture);

    glGenTextures(1, &m_spGLData->GeometryDepthTexture);

    // generate framebuffers
    glGenFramebuffersEXT(1, &m_spGLData->OffScreenFramebuffer0);
    glGenFramebuffersEXT(1, &m_spGLData->OffScreenFramebuffer1);
    glGenFramebuffersEXT(1, &m_spGLData->ResizeFramebuffer);

    // Setup 3D texture
    ///////////////////////////////////////////////////////////////////////////
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, m_spGLData->VolumeTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Create the texture
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    storeVolumeToTexture(m_VolumeData);
#ifndef __APPLE__
    {
        GLenum error = glGetError();

        int w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        if ((error != GL_NO_ERROR) || (w != m_VolumeData.getXSize()) || (h != m_VolumeData.getYSize()) || (d != m_VolumeData.getZSize()))
        {
            m_Error |= INIT_FAILED;
            VPL_LOG_INFO("Error: Cannot initialize 3D texture!");
            return false;
        }
    }
#endif

    // Setup skipping 3D texture
    ///////////////////////////////////////////////////////////////////////////
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, m_spGLData->AuxVolumeTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Create the texture
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    storeVolumeToTexture(m_AuxVolumeData);
#ifndef __APPLE__
    {
        GLenum error = glGetError();

        int w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        if ((error != GL_NO_ERROR) || (w != m_AuxVolumeData.getXSize()) || (h != m_AuxVolumeData.getYSize()) || (d != m_AuxVolumeData.getZSize()))
        {
            m_Error |= INIT_FAILED;
            VPL_LOG_INFO("Error: Cannot initialize skipping 3D texture!");
            return false;
        }
    }
#endif

    // LUT texture
    ///////////////////////////////////////////////////////////////////////////
    // setup 1D lookup texture in OpenGL and pre-load the first one
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, m_spGLData->LookUpTexture);
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16, LUT_1D_SIZE, 0, GL_RGBA, GL_UNSIGNED_SHORT, m_internalLookupTables[0]);

    // load shaders from string constants
    ///////////////////////////////////////////////////////////////////////////
#ifdef LOAD_SHADERS
    if (!loadShaderProgram("shaders2/fbo.frag", "shaders2/simple.vert", &m_spGLData->shaderFBO, &m_spGLData->vertexShader, &m_spGLData->PshaderFBO))
    {
        m_Error |= INIT_FAILED;
        m_ErrorStrings.push_back("Error: Cannot initialize shader (off-screen rendering)!");
        VPL_LOG_INFO(m_ErrorStrings.back().c_str());
        retVal = false;
    }
    for (int i = 0; i < SHADERS_COUNT - 1; ++i)
    {
        if (!loadShaderProgram(conf::shaderFilenames[i], "shaders2/simple.vert", &m_spGLData->shaderRayCast[i], &m_spGLData->vertexShader, &m_spGLData->PshaderRayCast[i]))
        {
            m_Error |= INIT_FAILED;
            m_ErrorStrings.push_back("Error: Cannot initialize shader (volume rendering)!");
            VPL_LOG_INFO(m_ErrorStrings.back().c_str());
            retVal = false;
        }
    }
    if (!loadShaderProgram("shaders2/resize2.frag", "shaders2/fsquad.vert", &m_spGLData->shaderResize, &m_spGLData->fsQuadVS, &m_spGLData->PshaderResize))
    {
        m_Error |= INIT_FAILED;
        m_ErrorStrings.push_back("Error: Cannot initialize shader (image composition)!");
        VPL_LOG_INFO(m_ErrorStrings.back().c_str());
        retVal = false;
    }
#else
    if (!createShaderProgram(shader::OSRT, shader::Vert, &m_spGLData->shaderFBO, &m_spGLData->vertexShader, &m_spGLData->PshaderFBO))
    {
        m_Error |= INIT_FAILED;
        m_ErrorStrings.push_back("Error: Cannot initialize shader (off-screen rendering)!");
        VPL_LOG_INFO(m_ErrorStrings.back().c_str());
        retVal = false;
    }
    if (!createShaderProgram(shader::XRay, shader::Vert, &m_spGLData->shaderRayCast[0], &m_spGLData->vertexShader, &m_spGLData->PshaderRayCast[0]))
    {
        m_Error |= INIT_FAILED;
        m_ErrorStrings.push_back("Error: Cannot initialize shader (x-ray)!");
        VPL_LOG_INFO(m_ErrorStrings.back().c_str());
        retVal = false;
    }
    if (!createShaderProgram(shader::MAX_IP, shader::Vert, &m_spGLData->shaderRayCast[1], &m_spGLData->vertexShader, &m_spGLData->PshaderRayCast[1]))
    {
        m_Error |= INIT_FAILED;
        m_ErrorStrings.push_back("Error: Cannot initialize shader (MIP)!");
        VPL_LOG_INFO(m_ErrorStrings.back().c_str());
        retVal = false;
    }
    if (!createShaderProgram(shader::Shade, shader::Vert, &m_spGLData->shaderRayCast[2], &m_spGLData->vertexShader, &m_spGLData->PshaderRayCast[2]))
    {
        m_Error |= INIT_FAILED;
        m_ErrorStrings.push_back("Error: Cannot initialize shader (shading)!");
        VPL_LOG_INFO(m_ErrorStrings.back().c_str());
        retVal = false;
    }
    if (!createShaderProgram(shader::Add, shader::Vert, &m_spGLData->shaderRayCast[3], &m_spGLData->vertexShader, &m_spGLData->PshaderRayCast[3]))
    {
        m_Error |= INIT_FAILED;
        m_ErrorStrings.push_back("Error: Cannot initialize shader (add)!");
        VPL_LOG_INFO(m_ErrorStrings.back().c_str());
        retVal = false;
    }
    if (!createShaderProgram(shader::Surface, shader::Vert, &m_spGLData->shaderRayCast[4], &m_spGLData->vertexShader, &m_spGLData->PshaderRayCast[4]))
    {
        m_Error |= INIT_FAILED;
        m_ErrorStrings.push_back("Error: Cannot initialize shader (surface)!");
        VPL_LOG_INFO(m_ErrorStrings.back().c_str());
        retVal = false;
    }
    if (!createShaderProgram(shader::Resize, shader::FSQuadVS, &m_spGLData->shaderResize, &m_spGLData->fsQuadVS, &m_spGLData->PshaderResize))
    {
        m_Error |= INIT_FAILED;
        m_ErrorStrings.push_back("Error: Cannot initialize shader (image composition)!");
        VPL_LOG_INFO(m_ErrorStrings.back().c_str());
        retVal = false;
    }
#endif // LOAD_SHADERS

    // noise texture
    ///////////////////////////////////////////////////////////////////////////
    // Prepare a random noise texture
    vpl::math::CNormalPRNG Generator;
    for (int i = 0; i < NOISE_SIZE * NOISE_SIZE; ++i)
    {
        noise[i] = 128 + (unsigned char)Generator.random(0.0, conf::NoiseSigma);
    }

    // setup the noise texture
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_spGLData->NoiseTexture);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, NOISE_SIZE, NOISE_SIZE, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, noise);

    // The current rendering size
    vpl::img::CPoint3D renderingSize = getRenderingSize(m_spParams, m_Flags);

    // setup addition textures used for off-screen rendering
    ///////////////////////////////////////////////////////////////////////////
    // fbo - off screen texture render targer for back culling - texture coords, world coords of rays' starts and texture coords, world coords of rays' ends
    glBindTexture(GL_TEXTURE_3D, m_spGLData->RaysStartEnd);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F_ARB, renderingSize.x(), renderingSize.y(), 2, 0, GL_RGB, GL_FLOAT, NULL);

    // render target texture
    glBindTexture(GL_TEXTURE_2D, m_spGLData->RTTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, renderingSize.x(), renderingSize.y(), 0, GL_RGB, GL_FLOAT, NULL);

    // depth target texture
    glBindTexture(GL_TEXTURE_2D, m_spGLData->DEPTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, renderingSize.x(), renderingSize.y(), 0, GL_RGB, GL_FLOAT, NULL);

    // Setup depth texture
    glBindTexture(GL_TEXTURE_2D, m_spGLData->GeometryDepthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderingSize.x(), renderingSize.y(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // create and upload 1D resize kernel texture (used for image interpolation)
    ///////////////////////////////////////////////////////////////////////////
    #define LUTSIZE 512
    static float klut[LUTSIZE];
    for (int i = 0; i < LUTSIZE; i++)
    {
        double X = double(i) / double(LUTSIZE) * 2.0;
        klut[i] = float((std::sin(X * vpl::math::PI) / (X * vpl::math::PI)) * (std::sin((X * 0.5f) * vpl::math::PI) / ((X * 0.5) * vpl::math::PI)));
    }
    klut[0] = 1.0f;

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_1D, m_spGLData->BicKerTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE16F_ARB, LUTSIZE, 0, GL_LUMINANCE, GL_FLOAT, klut);

    // default settings
    glActiveTexture(GL_TEXTURE0);

    // BOX rendering
    VPL_ASSERT((m_Triangles.size() % conf::NumOfBatches) == 0);
    VPL_ASSERT(3 * sizeof(GLfloat) == sizeof(tCoords));

    // Generate the box VBOs
    glGenBuffersARB(conf::NumOfBatches, m_spGLData->VBOBox);

    // Init buffers
    int BatchSize = m_Triangles.size() / conf::NumOfBatches;
    for (int b = 0; b < conf::NumOfBatches; ++b)
    {
        // Bind the vertex buffer
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_spGLData->VBOBox[b]);
        // Load the data
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, BatchSize * sizeof(tCoords), &(m_Triangles[b * BatchSize]), GL_STATIC_DRAW_ARB);
    }

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

    GLint vertexArray;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArray);

    // Generate box VAOs
    glGenVertexArraysX(conf::NumOfBatches, m_spGLData->VAOBox);

    // Init VAOs
    for (int b = 0; b < conf::NumOfBatches; ++b)
    {
        // Bind the VAO
        glBindVertexArrayX(m_spGLData->VAOBox[b]);

        // Enable vertex arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        // And set up vertex attributes
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_spGLData->VBOBox[b]);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        //glEnableVertexAttribArray(0);
        //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // Bind with 0, so, switch back to normal pointer operation
    glBindVertexArrayX(vertexArray);

    // RECT rendering (two triangles)

    // Generate rect's VBOs
    glGenBuffersARB(1, &m_spGLData->VBORect);

    // Bind the vertex buffer
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_spGLData->VBORect);
    // Load the data
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, 18 * sizeof(float), conf::rectVertices, GL_STATIC_DRAW_ARB);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

#ifdef USE_VAORECT
    // Generate rect's VAO
    glGenVertexArraysX(1, &m_spGLData->VAORect);

    // Bind the VAO
    glBindVertexArrayX(m_spGLData->VAORect);

    // Enable vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // And set up vertex attributes
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_spGLData->VBORect);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
#endif

    glBindVertexArrayX(vertexArray);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, arrayBuffer);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, elementArrayBuffer);

    // Restore state so that OSG can continue its work
    bVertexArray            ? glEnableClientState(GL_VERTEX_ARRAY)          : glDisableClientState(GL_VERTEX_ARRAY);
    bTexCoordArray          ? glEnableClientState(GL_TEXTURE_COORD_ARRAY)   : glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    bNormalArray            ? glEnableClientState(GL_NORMAL_ARRAY)          : glDisableClientState(GL_NORMAL_ARRAY);
    bColorArray             ? glEnableClientState(GL_COLOR_ARRAY)           : glDisableClientState(GL_COLOR_ARRAY);
    bIndexArray             ? glEnableClientState(GL_INDEX_ARRAY)           : glDisableClientState(GL_INDEX_ARRAY);
    bSecondaryColorArray    ? glEnableClientState(GL_SECONDARY_COLOR_ARRAY) : glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    bEdgeFlagArray          ? glEnableClientState(GL_EDGE_FLAG_ARRAY)       : glDisableClientState(GL_EDGE_FLAG_ARRAY);
    bFogCoordArray          ? glEnableClientState(GL_FOG_COORD_ARRAY)       : glDisableClientState(GL_FOG_COORD_ARRAY);

    // O.K.
    if (retVal)
    {
        m_Flags |= INITIALIZED;
    }
    
    GLenum error = glGetError();

    return retVal;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::reloadShader()
{
#ifdef LOAD_SHADERS
    if (m_spParams->selectedShader == CUSTOM)
    {
        return;
    }

    int curr = m_spParams->selectedShader;
    loadShaderProgram(
        conf::shaderFilenames[curr],
        "shaders2/simple.vert",
        &m_spGLData->shaderRayCast[curr],
        &m_spGLData->vertexShader,
        &m_spGLData->PshaderRayCast[curr]);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalUploadTexture()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, m_spGLData->VolumeTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Create the 3D texture
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    storeVolumeToTexture(m_VolumeData);
    {
        GLenum error = glGetError();

        int w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        if ((error != GL_NO_ERROR) || (w != m_VolumeData.getXSize()) || (h != m_VolumeData.getYSize()) || (d != m_VolumeData.getZSize()))
        {
            m_Error |= CANNOT_CREATE_3D_TEXTURE;
            VPL_LOG_INFO("Error: Cannot create 3D texture!");
            return false;
        }
    }

    // Release the volume data
    m_VolumeData.resize(0, 0, 0, 0);

    return true;
}

bool PSVolumeRendering::internalUploadAuxTexture()
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, m_spGLData->AuxVolumeTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Create the 3D texture
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    storeVolumeToTexture(m_AuxVolumeData);
    {
        GLenum error = glGetError();

        int w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        if ((error != GL_NO_ERROR) || (w != m_AuxVolumeData.getXSize()) || (h != m_AuxVolumeData.getYSize()) || (d != m_AuxVolumeData.getZSize()))
        {
            m_Error |= CANNOT_CREATE_3D_TEXTURE;
            VPL_LOG_INFO("Error: Cannot create skipping 3D texture!");
            return false;
        }
    }

    // Release the volume data
    m_AuxVolumeData.resize(0, 0, 0, 0);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalUploadCustomData_bool()
{
    if (m_volume_bool.getZSize() <= 0)
    {
        return false;
    }

    m_spParams->CustomXSize = m_volume_bool.getXSize();
    m_spParams->CustomYSize = m_volume_bool.getYSize();
    m_spParams->CustomZSize = m_volume_bool.getZSize();

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->CustomXSize &= ROUNDING_MASK;
    m_spParams->CustomYSize &= ROUNDING_MASK;
    m_spParams->CustomZSize &= ROUNDING_MASK;

    m_customData_bool.resize(m_spParams->CustomXSize, m_spParams->CustomYSize, m_spParams->CustomZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < m_spParams->CustomZSize; z++)
    {
        for (vpl::tSize y = 0; y < m_spParams->CustomYSize; y++)
        {
            for (vpl::tSize x = 0; x < m_spParams->CustomXSize; x++)
            {
                m_customData_bool(x, y, z) = m_volume_bool(x, y, z) ? 255 : 0;
            }
        }
    }

    m_volume_bool.resize(0, 0, 0, 0);

    if (m_auxVolume.getZSize() != 0)
    {
        m_spParams->CustomAuxXSize = ((m_auxVolume.getXSize() + 3) / 4) * 4;
        m_spParams->CustomAuxYSize = ((m_auxVolume.getYSize() + 3) / 4) * 4;
        m_spParams->CustomAuxZSize = ((m_auxVolume.getZSize() + 3) / 4) * 4;

        m_spParams->CustomAuxTexXSize = float(m_spParams->CustomXSize) / float(m_spParams->CustomAuxXSize * 8);
        m_spParams->CustomAuxTexYSize = float(m_spParams->CustomYSize) / float(m_spParams->CustomAuxYSize * 8);
        m_spParams->CustomAuxTexZSize = float(m_spParams->CustomZSize) / float(m_spParams->CustomAuxZSize * 8);

        m_auxCustomData.resize(m_spParams->CustomAuxXSize, m_spParams->CustomAuxYSize, m_spParams->CustomAuxZSize);
        m_auxCustomData.fillEntire(0);

        #pragma omp parallel for
        for (vpl::tSize z = 0; z < m_auxVolume.getZSize(); ++z)
        {
            for (vpl::tSize y = 0; y < m_auxVolume.getYSize(); ++y)
            {
                for (vpl::tSize x = 0; x < m_auxVolume.getXSize(); ++x)
                {
                    m_auxCustomData(x, y, z) = m_auxVolume(x, y, z);
                }
            }
        }

        m_auxVolume.resize(0, 0, 0, 0);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalUploadCustomData_tPixel8()
{
    if (m_volume_tPixel8.getZSize() <= 0)
    {
        return false;
    }

    m_spParams->CustomXSize = m_volume_tPixel8.getXSize();
    m_spParams->CustomYSize = m_volume_tPixel8.getYSize();
    m_spParams->CustomZSize = m_volume_tPixel8.getZSize();

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->CustomXSize &= ROUNDING_MASK;
    m_spParams->CustomYSize &= ROUNDING_MASK;
    m_spParams->CustomZSize &= ROUNDING_MASK;

    m_customData_tPixel8.resize(m_spParams->CustomXSize, m_spParams->CustomYSize, m_spParams->CustomZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < m_spParams->CustomZSize; z++)
    {
        for (vpl::tSize y = 0; y < m_spParams->CustomYSize; y++)
        {
            for (vpl::tSize x = 0; x < m_spParams->CustomXSize; x++)
            {
                m_customData_tPixel8(x, y, z) = m_volume_tPixel8(x, y, z);
            }
        }
    }

    m_volume_tPixel8.resize(0, 0, 0, 0);

    if (m_auxVolume.getZSize() != 0)
    {
        m_spParams->CustomAuxXSize = ((m_auxVolume.getXSize() + 3) / 4) * 4;
        m_spParams->CustomAuxYSize = ((m_auxVolume.getYSize() + 3) / 4) * 4;
        m_spParams->CustomAuxZSize = ((m_auxVolume.getZSize() + 3) / 4) * 4;

        m_spParams->CustomAuxTexXSize = float(m_spParams->CustomXSize) / float(m_spParams->CustomAuxXSize * 8);
        m_spParams->CustomAuxTexYSize = float(m_spParams->CustomYSize) / float(m_spParams->CustomAuxYSize * 8);
        m_spParams->CustomAuxTexZSize = float(m_spParams->CustomZSize) / float(m_spParams->CustomAuxZSize * 8);

        m_auxCustomData.resize(m_spParams->CustomAuxXSize, m_spParams->CustomAuxYSize, m_spParams->CustomAuxZSize);
        m_auxCustomData.fillEntire(0);

        #pragma omp parallel for
        for (vpl::tSize z = 0; z < m_auxVolume.getZSize(); ++z)
        {
            for (vpl::tSize y = 0; y < m_auxVolume.getYSize(); ++y)
            {
                for (vpl::tSize x = 0; x < m_auxVolume.getXSize(); ++x)
                {
                    m_auxCustomData(x, y, z) = m_auxVolume(x, y, z);
                }
            }
        }

        m_auxVolume.resize(0, 0, 0, 0);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalUploadCustomData_tPixel16()
{
    if (m_volume_tPixel16.getZSize() <= 0)
    {
        return false;
    }

    m_spParams->CustomXSize = m_volume_tPixel16.getXSize();
    m_spParams->CustomYSize = m_volume_tPixel16.getYSize();
    m_spParams->CustomZSize = m_volume_tPixel16.getZSize();

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->CustomXSize &= ROUNDING_MASK;
    m_spParams->CustomYSize &= ROUNDING_MASK;
    m_spParams->CustomZSize &= ROUNDING_MASK;

    m_customData_tPixel16.resize(m_spParams->CustomXSize, m_spParams->CustomYSize, m_spParams->CustomZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < m_spParams->CustomZSize; z++)
    {
        for (vpl::tSize y = 0; y < m_spParams->CustomYSize; y++)
        {
            for (vpl::tSize x = 0; x < m_spParams->CustomXSize; x++)
            {
                m_customData_tPixel16(x, y, z) = m_volume_tPixel16(x, y, z);
            }
        }
    }

    m_volume_tPixel16.resize(0, 0, 0, 0);

    if (m_auxVolume.getZSize() != 0)
    {
        m_spParams->CustomAuxXSize = ((m_auxVolume.getXSize() + 3) / 4) * 4;
        m_spParams->CustomAuxYSize = ((m_auxVolume.getYSize() + 3) / 4) * 4;
        m_spParams->CustomAuxZSize = ((m_auxVolume.getZSize() + 3) / 4) * 4;

        m_spParams->CustomAuxTexXSize = float(m_spParams->CustomXSize) / float(m_spParams->CustomAuxXSize * 8);
        m_spParams->CustomAuxTexYSize = float(m_spParams->CustomYSize) / float(m_spParams->CustomAuxYSize * 8);
        m_spParams->CustomAuxTexZSize = float(m_spParams->CustomZSize) / float(m_spParams->CustomAuxZSize * 8);

        m_auxCustomData.resize(m_spParams->CustomAuxXSize, m_spParams->CustomAuxYSize, m_spParams->CustomAuxZSize);
        m_auxCustomData.fillEntire(0);

        #pragma omp parallel for
        for (vpl::tSize z = 0; z < m_auxVolume.getZSize(); ++z)
        {
            for (vpl::tSize y = 0; y < m_auxVolume.getYSize(); ++y)
            {
                for (vpl::tSize x = 0; x < m_auxVolume.getXSize(); ++x)
                {
                    m_auxCustomData(x, y, z) = m_auxVolume(x, y, z);
                }
            }
        }

        m_auxVolume.resize(0, 0, 0, 0);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalUploadCustomData_tRGBPixel()
{
    if (m_volume_tRGBPixel.getZSize() <= 0)
    {
        return false;
    }

    m_spParams->CustomXSize = m_volume_tRGBPixel.getXSize();
    m_spParams->CustomYSize = m_volume_tRGBPixel.getYSize();
    m_spParams->CustomZSize = m_volume_tRGBPixel.getZSize();

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->CustomXSize &= ROUNDING_MASK;
    m_spParams->CustomYSize &= ROUNDING_MASK;
    m_spParams->CustomZSize &= ROUNDING_MASK;

    m_customData_tRGBPixel.resize(m_spParams->CustomXSize, m_spParams->CustomYSize, m_spParams->CustomZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < m_spParams->CustomZSize; z++)
    {
        for (vpl::tSize y = 0; y < m_spParams->CustomYSize; y++)
        {
            for (vpl::tSize x = 0; x < m_spParams->CustomXSize; x++)
            {
                m_customData_tRGBPixel(x, y, z) = m_volume_tRGBPixel(x, y, z);
            }
        }
    }

    m_volume_tRGBPixel.resize(0, 0, 0, 0);

    if (m_auxVolume.getZSize() != 0)
    {
        m_spParams->CustomAuxXSize = ((m_auxVolume.getXSize() + 3) / 4) * 4;
        m_spParams->CustomAuxYSize = ((m_auxVolume.getYSize() + 3) / 4) * 4;
        m_spParams->CustomAuxZSize = ((m_auxVolume.getZSize() + 3) / 4) * 4;

        m_spParams->CustomAuxTexXSize = float(m_spParams->CustomXSize) / float(m_spParams->CustomAuxXSize * 8);
        m_spParams->CustomAuxTexYSize = float(m_spParams->CustomYSize) / float(m_spParams->CustomAuxYSize * 8);
        m_spParams->CustomAuxTexZSize = float(m_spParams->CustomZSize) / float(m_spParams->CustomAuxZSize * 8);

        m_auxCustomData.resize(m_spParams->CustomAuxXSize, m_spParams->CustomAuxYSize, m_spParams->CustomAuxZSize);
        m_auxCustomData.fillEntire(0);

        #pragma omp parallel for
        for (vpl::tSize z = 0; z < m_auxVolume.getZSize(); ++z)
        {
            for (vpl::tSize y = 0; y < m_auxVolume.getYSize(); ++y)
            {
                for (vpl::tSize x = 0; x < m_auxVolume.getXSize(); ++x)
                {
                    m_auxCustomData(x, y, z) = m_auxVolume(x, y, z);
                }
            }
        }

        m_auxVolume.resize(0, 0, 0, 0);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalUploadCustomTexture_bool()
{
    int currBinding = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &currBinding);
    glBindTexture(GL_TEXTURE_3D, m_currentVolume);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    storeVolumeToTexture(m_customData_bool);
    {
        GLenum error = glGetError();

        int w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        if ((error != GL_NO_ERROR) || (w != m_customData_bool.getXSize()) || (h != m_customData_bool.getYSize()) || (d != m_customData_bool.getZSize()))
        {
            m_Error |= CANNOT_CREATE_3D_TEXTURE;
            VPL_LOG_INFO("Error: Cannot create 3D texture for custom data (bool)!");
            return false;
        }
    }

    // Release the volume data
    m_customData_bool.resize(0, 0, 0, 0);

    glBindTexture(GL_TEXTURE_3D, currBinding);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalUploadCustomTexture_tPixel8()
{
    int currBinding = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &currBinding);
    glBindTexture(GL_TEXTURE_3D, m_currentVolume);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    storeVolumeToTexture(m_customData_tPixel8);
    {
        GLenum error = glGetError();

        int w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        if ((error != GL_NO_ERROR) || (w != m_customData_tPixel8.getXSize()) || (h != m_customData_tPixel8.getYSize()) || (d != m_customData_tPixel8.getZSize()))
        {
            m_Error |= CANNOT_CREATE_3D_TEXTURE;
            VPL_LOG_INFO("Error: Cannot create 3D texture for custom data (tPixel8)!");
            return false;
        }
    }

    // Release the volume data
    m_customData_tPixel8.resize(0, 0, 0, 0);

    glBindTexture(GL_TEXTURE_3D, currBinding);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalUploadCustomTexture_tPixel16()
{
    int currBinding = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &currBinding);
    glBindTexture(GL_TEXTURE_3D, m_currentVolume);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    storeVolumeToTexture(m_customData_tPixel16);
    {
        GLenum error = glGetError();

        int w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        if ((error != GL_NO_ERROR) || (w != m_customData_tPixel16.getXSize()) || (h != m_customData_tPixel16.getYSize()) || (d != m_customData_tPixel16.getZSize()))
        {
            m_Error |= CANNOT_CREATE_3D_TEXTURE;
            VPL_LOG_INFO("Error: Cannot create 3D texture for custom data (tPixel16)!");
            return false;
        }
    }

    // Release the volume data
    m_customData_tPixel16.resize(0, 0, 0, 0);

    glBindTexture(GL_TEXTURE_3D, currBinding);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::internalUploadCustomTexture_tRGBPixel()
{
    int currBinding = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &currBinding);
    glBindTexture(GL_TEXTURE_3D, m_currentVolume);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    storeVolumeToTexture(m_customData_tRGBPixel);
    {
        GLenum error = glGetError();

        int w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        if ((error != GL_NO_ERROR) || (w != m_customData_tRGBPixel.getXSize()) || (h != m_customData_tRGBPixel.getYSize()) || (d != m_customData_tRGBPixel.getZSize()))
        {
            m_Error |= CANNOT_CREATE_3D_TEXTURE;
            VPL_LOG_INFO("Error: Cannot create 3D texture for custom data (tRGBPixel)!");
            return false;
        }
    }

    // Release the volume data
    m_customData_tRGBPixel.resize(0, 0, 0, 0);

    glBindTexture(GL_TEXTURE_3D, currBinding);

    return true;
}

bool PSVolumeRendering::internalUploadCustomAuxTexture()
{
    int currBinding = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_3D, &currBinding);
    glBindTexture(GL_TEXTURE_3D, m_spGLData->CustomAuxVolumeTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    storeVolumeToTexture(m_auxCustomData);
    {
        GLenum error = glGetError();

        int w, h, d;
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &d);

        if ((error != GL_NO_ERROR) || (w != m_auxCustomData.getXSize()) || (h != m_auxCustomData.getYSize()) || (d != m_auxCustomData.getZSize()))
        {
            m_Error |= CANNOT_CREATE_3D_TEXTURE;
            VPL_LOG_INFO("Error: Cannot create 3D texture for custom data (auxiliary volume)!");
            return false;
        }
    }

    // Release the volume data
    m_auxCustomData.resize(0, 0, 0, 0);

    glBindTexture(GL_TEXTURE_3D, currBinding);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
vpl::img::CPoint3D PSVolumeRendering::getRenderingSize(PSVolumeRenderingParams *pParams, int Flags) const
{
    float desiredDimension = 0.0f;
    vpl::img::CPoint3D retValue;

    if (Flags & MOUSE_MODE)
    {
        desiredDimension = conf::MouseRenderingSize[pParams->currentQuality];
    }
    else
    {
        desiredDimension = pParams->renderingSize;
    }

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    float width = viewport[2];
    float height = viewport[3];

    if (width > height)
    {
        float ratio = height / width;
        float dimension = vpl::math::getMin<float>(width, desiredDimension);
        
        retValue = vpl::img::CPoint3D(dimension, vpl::math::round2Int(dimension * ratio));
    }
    else
    {
        float ratio = width / height;
        float dimension = vpl::math::getMin<float>(height, desiredDimension);
        retValue = vpl::img::CPoint3D(vpl::math::round2Int(dimension * ratio), dimension);    
    }

    return retValue;
}

///////////////////////////////////////////////////////////////////////////////
//
float PSVolumeRendering::getVolumeSamplingDistance(PSVolumeRenderingParams *pParams, int Flags) const
{
    if (Flags & MOUSE_MODE)
    {
        return conf::MouseTextureSampling[pParams->currentQuality];
    }
    else
    {
        return pParams->volumeSamplingDistance;
    }
}

///////////////////////////////////////////////////////////////////////////////
// change size of all internal rendering textures
bool PSVolumeRendering::internalSetRenderingSize(PSVolumeRenderingParams *pParams, int Flags)
{
    // The current rendering size
    vpl::img::CPoint3D renderingSize = getRenderingSize(pParams, Flags);

    glActiveTexture(GL_TEXTURE4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glBindTexture(GL_TEXTURE_3D, m_spGLData->RaysStartEnd);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F_ARB, renderingSize.x(), renderingSize.y(), 2, 0, GL_RGB, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_spGLData->RTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, renderingSize.x(), renderingSize.y(), 0, GL_RGB, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_spGLData->DEPTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, renderingSize.x(), renderingSize.y(), 0, GL_RGB, GL_FLOAT, NULL);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// upload new, user selected lookup texture to the GPU memory
bool PSVolumeRendering::internalSetLUT(PSVolumeRenderingParams *pParams)
{
    // upload new 1D texture
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, m_spGLData->LookUpTexture);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16, LUT_1D_SIZE, 0, GL_RGBA, GL_UNSIGNED_SHORT, m_internalLookupTables[pParams->selectedLut]);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setFlag(int Flag)
{
    m_Flags |= Flag;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setAndSignalFlag(int Flag)
{
    m_Mutex.lock();
    m_Flags |= Flag;
    m_Condition.notifyOne();
    m_Mutex.unlock();
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::clearFlag(int Flag)
{
    m_Flags &= (0xffff - Flag);
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::testFlag(int Flag)
{
    return (m_Flags & Flag) != 0;
}

///////////////////////////////////////////////////////////////////////////////
//
VPL_THREAD_ROUTINE(PSVolumeRendering::setupLoop)
{
    // Console object
    PSVolumeRendering *pRenderer = static_cast<PSVolumeRendering *>(pThread->getData());
    if (!pRenderer)
    {
        return -1;
    }

    // Mask of relevant flags
    int InvalidMask = DATA_INVALID;
    InvalidMask += CUSTOM_DATA_INVALID;

    // Main thread loop
    VPL_THREAD_MAIN_LOOP
    {
        pRenderer->m_Mutex.lock();
        if ((pRenderer->m_Flags & InvalidMask) == 0)
        {
            // Wait for the "anything changed" event
            if (!pRenderer->m_Condition.wait(pRenderer->m_Mutex, 250))
            {
                pRenderer->m_Mutex.unlock();
                continue;
            }
        }

        // Local copy of all flags and params
        int Flags = pRenderer->m_Flags;
        pRenderer->clearFlag(DATA_INVALID);
        pRenderer->clearFlag(CUSTOM_DATA_INVALID);

        //PSVolumeRenderingParams params = *pRenderer->m_spParams;

        // Release the mutex
        pRenderer->m_Mutex.unlock();

        int newFlags = PSVR_NO_FLAGS;

        // Prepare the data if required.
        if (Flags & DATA_INVALID)
        {
            if (pRenderer->internalUploadData())
            {
                newFlags |= TEXTURE_INVALID;
                newFlags |= AUX_TEXTURE_INVALID;
            }
        }

        // Prepare the data if required.
        if (Flags & CUSTOM_DATA_INVALID)
        {
            bool uploadData = false;
            switch (pRenderer->m_currentType)
            {
            case EVT_BOOL:
                uploadData = pRenderer->internalUploadCustomData_bool();
                break;

            case EVT_PIXEL8:
                uploadData = pRenderer->internalUploadCustomData_tPixel8();
                break;

            case EVT_PIXEL16:
                uploadData = pRenderer->internalUploadCustomData_tPixel16();
                break;

            case EVT_RGBPIXEL:
                uploadData = pRenderer->internalUploadCustomData_tRGBPixel();
                break;
            }

            if (uploadData)
            {
                newFlags |= CUSTOM_TEXTURE_INVALID;
            }
        }

        // set flags
        pRenderer->setFlag(newFlags);

        // on change request redraw
        if ((Flags & DATA_INVALID) || (Flags & CUSTOM_DATA_INVALID))
        {
            pRenderer->redraw();
        }

        // Sleep for a short period of time
        vpl::sys::sleep(1);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Latest version
// - num. of quads specified by the parameter
// - VAO (Vertex Array Object)
void PSVolumeRendering::prepareBox(int NumOfQuads)
{
    m_Triangles.clear();

    // Quad size - real and texture coordinates
    float rs = 2.0f / NumOfQuads;
    float ts = 1.0f / NumOfQuads;

    // First side
    float b = -1.0f, v = 0.0f;
    for (int j = 0; j < NumOfQuads; ++j, b += rs, v += ts)
    {
        float a = -1.0f, u = 0.0f;
        for (int i = 0; i < NumOfQuads; ++i, a += rs, u += ts)
        {
            m_Triangles.push_back(tCoords(a,      b + rs, -1.0f));
            m_Triangles.push_back(tCoords(a + rs, b + rs, -1.0f));
            m_Triangles.push_back(tCoords(a + rs, b,      -1.0f));

            m_Triangles.push_back(tCoords(a + rs, b,      -1.0f));
            m_Triangles.push_back(tCoords(a,      b,      -1.0f));
            m_Triangles.push_back(tCoords(a,      b + rs, -1.0f));

            m_Triangles.push_back(tCoords(a,      b + rs, 1.0f));
            m_Triangles.push_back(tCoords(a,      b,      1.0f));
            m_Triangles.push_back(tCoords(a + rs, b,      1.0f));

            m_Triangles.push_back(tCoords(a + rs, b,      1.0f));
            m_Triangles.push_back(tCoords(a + rs, b + rs, 1.0f));
            m_Triangles.push_back(tCoords(a,      b + rs, 1.0f));
        }
    }

    b = -1.0f, v = 0.0f;
    for (int j = 0; j < NumOfQuads; ++j, b += rs, v += ts)
    {
        float a = -1.0f, u = 0.0f;
        for (int i = 0; i < NumOfQuads; ++i, a += rs, u += ts)
        {
            m_Triangles.push_back(tCoords(a,      -1.0f, b));
            m_Triangles.push_back(tCoords(a + rs, -1.0f, b));
            m_Triangles.push_back(tCoords(a + rs, -1.0f, b + rs));

            m_Triangles.push_back(tCoords(a + rs, -1.0f, b + rs));
            m_Triangles.push_back(tCoords(a,      -1.0f, b + rs));
            m_Triangles.push_back(tCoords(a,      -1.0f, b));

            m_Triangles.push_back(tCoords(a,      1.0f,  b));
            m_Triangles.push_back(tCoords(a,      1.0f,  b + rs));
            m_Triangles.push_back(tCoords(a + rs, 1.0f,  b + rs));

            m_Triangles.push_back(tCoords(a + rs, 1.0f,  b + rs));
            m_Triangles.push_back(tCoords(a + rs, 1.0f,  b));
            m_Triangles.push_back(tCoords(a,      1.0f,  b));
        }
    }

    b = -1.0f, v = 0.0f;
    for (int j = 0; j < NumOfQuads; ++j, b += rs, v += ts)
    {
        float a = -1.0f, u = 0.0f;
        for (int i = 0; i < NumOfQuads; ++i, a += rs, u += ts)
        {
            m_Triangles.push_back(tCoords(-1.0f, a + rs, b + rs));
            m_Triangles.push_back(tCoords(-1.0f, a + rs, b));
            m_Triangles.push_back(tCoords(-1.0f, a,      b));

            m_Triangles.push_back(tCoords(-1.0f, a,      b));
            m_Triangles.push_back(tCoords(-1.0f, a,      b + rs));
            m_Triangles.push_back(tCoords(-1.0f, a + rs, b + rs));

            m_Triangles.push_back(tCoords(1.0f,  a + rs, b + rs));
            m_Triangles.push_back(tCoords(1.0f,  a,      b + rs));
            m_Triangles.push_back(tCoords(1.0f,  a,      b));

            m_Triangles.push_back(tCoords(1.0f,  a,      b));
            m_Triangles.push_back(tCoords(1.0f,  a + rs, b));
            m_Triangles.push_back(tCoords(1.0f,  a + rs, b + rs));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Latest version
// - a complex geometry (more than 6 basic quads)
// - VAO (Vertex Array Object)
void PSVolumeRendering::renderBox(PSVolumeRenderingParams *pParams, const tArray& Triangles)
{
    // height of the box (object)
    float Y = (float)pParams->YSize / (float)pParams->XSize * pParams->aspectRatio_YtoX;

    // depth of the box
    float Z = (float)pParams->ZSize / (float)pParams->XSize * pParams->aspectRatio_ZtoX;

    // Scaling of the box
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(1.0f, Y, Z);

    // grab current matrices and calculate inverse matrices for further processing
    osg::Matrix projectionMatrix;
    GLdouble glProjectionMatrix[16];
    glMatrixMode(GL_PROJECTION);
    glGetDoublev(GL_PROJECTION_MATRIX, glProjectionMatrix);
    projectionMatrix.set(glProjectionMatrix);
    projectionMatrix = osg::Matrix::inverse(projectionMatrix);

    osg::Matrix modelViewMatrix;
    GLdouble glModelViewMatrix[16];
    glMatrixMode(GL_MODELVIEW);
    glGetDoublev(GL_MODELVIEW_MATRIX, glModelViewMatrix);
    modelViewMatrix.set(glModelViewMatrix);
    modelViewMatrix = osg::Matrix::inverse(modelViewMatrix);

    // store inverted matrices
    for (int i = 0; i < 16; i++)
    {
        pParams->invModelViewMatrix[i] = modelViewMatrix.ptr()[i];
        pParams->invProjectionMatrix[i] = projectionMatrix.ptr()[i];
    }

    // Useless color definition - not visible under normal circumstances
    glColor3f(0.0f, 0.0f, 0.0f);

    // Save current state
    bool bVertexArray =         glIsEnabled(GL_VERTEX_ARRAY);
    bool bTexCoordArray =       glIsEnabled(GL_TEXTURE_COORD_ARRAY);
    bool bNormalArray =         glIsEnabled(GL_NORMAL_ARRAY);
    bool bColorArray =          glIsEnabled(GL_COLOR_ARRAY);
    bool bIndexArray =          glIsEnabled(GL_INDEX_ARRAY);
    bool bSecondaryColorArray = glIsEnabled(GL_SECONDARY_COLOR_ARRAY);
    bool bEdgeFlagArray =       glIsEnabled(GL_EDGE_FLAG_ARRAY);
    bool bFogCoordArray =       glIsEnabled(GL_FOG_COORD_ARRAY);
    GLint vertexArray;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArray);

    // Draw all VBOs
    int BatchSize = Triangles.size() / conf::NumOfBatches;
    for (int b = 0; b < conf::NumOfBatches; ++b)
    {
        // Bind the corresponding VAO
        glBindVertexArrayX(m_spGLData->VAOBox[b]);

        // Draw the box
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(BatchSize));
    }

    // Unbind the vertex array
    glBindVertexArrayX(vertexArray);

    // Restore state so that OSG can continue its work
    bVertexArray            ? glEnableClientState(GL_VERTEX_ARRAY)          : glDisableClientState(GL_VERTEX_ARRAY);
    bTexCoordArray          ? glEnableClientState(GL_TEXTURE_COORD_ARRAY)   : glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    bNormalArray            ? glEnableClientState(GL_NORMAL_ARRAY)          : glDisableClientState(GL_NORMAL_ARRAY);
    bColorArray             ? glEnableClientState(GL_COLOR_ARRAY)           : glDisableClientState(GL_COLOR_ARRAY);
    bIndexArray             ? glEnableClientState(GL_INDEX_ARRAY)           : glDisableClientState(GL_INDEX_ARRAY);
    bSecondaryColorArray    ? glEnableClientState(GL_SECONDARY_COLOR_ARRAY) : glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    bEdgeFlagArray          ? glEnableClientState(GL_EDGE_FLAG_ARRAY)       : glDisableClientState(GL_EDGE_FLAG_ARRAY);
    bFogCoordArray          ? glEnableClientState(GL_FOG_COORD_ARRAY)       : glDisableClientState(GL_FOG_COORD_ARRAY);

    // Restore the matrix
    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// Main volume rendering function called for every frame displayed
// Initilizes OpenGL when first called, than updates shaders and lookups
// and finally renders volume in a few steps.
void PSVolumeRendering::renderVolume()
{
    // Local copy of rendering parameters
    PSVolumeRenderingParams Params;
    int Flags = 0;

    // BEGIN: Locked part of the rendering
    {
        tLock Lock(*this);

        // Is rendering enabled?
        if (!m_Enabled)
        {
            return;
        }

        // Do not do anything if init failed many times
        if (constantFailure())
        {
            return;
        }

        // Initialize the renderer if required.
        init();

        // Is rendering correctly initialized?
        if (!testFlag(INITIALIZED))
        {
            return;
        }

        if (((APP_MODE.check(scene::CAppMode::COMMAND_DRAW_WINDOW) || APP_MODE.check(scene::CAppMode::COMMAND_DRAW_GEOMETRY)) && testFlag(MOUSE_PRESSED)) &&
            !testFlag(OSR_INVALID) &&
            !testFlag(TEXTURE_INVALID) &&
            !testFlag(AUX_TEXTURE_INVALID) &&
            !testFlag(CUSTOM_TEXTURE_INVALID))
        {
            setFlag(FAST_REDRAW);
        }

        // Local copy of internal flags
        Flags = m_Flags;
        clearFlag(TEXTURE_INVALID |
            AUX_TEXTURE_INVALID |
            CUSTOM_TEXTURE_INVALID |
            OSR_INVALID | 
            LUT_INVALID |
            FAST_REDRAW);

        // New texture data?
        if (Flags & TEXTURE_INVALID)
        {
            internalUploadTexture();
        }

        if (Flags & AUX_TEXTURE_INVALID)
        {
            internalUploadAuxTexture();
        }

        // New texture data?
        if (Flags & CUSTOM_TEXTURE_INVALID)
        {
            switch (m_currentType)
            {
            case EVT_BOOL:
                internalUploadCustomTexture_bool();
                break;

            case EVT_PIXEL8:
                internalUploadCustomTexture_tPixel8();
                break;

            case EVT_PIXEL16:
                internalUploadCustomTexture_tPixel16();
                break;

            case EVT_RGBPIXEL:
                internalUploadCustomTexture_tRGBPixel();
                break;
            }

            internalUploadCustomAuxTexture();
        }

        // Create a local copy of current rendering parameters
        Params = *m_spParams;
    }
    // END: Locked part of the rendering

    // Resolution changed?
    if (Flags & OSR_INVALID)
    {
        internalSetRenderingSize(&Params, Flags);
    }

    // LUT changed?
    if (Flags & LUT_INVALID)
    {
        internalSetLUT(&Params);
    }

    // general OpenGL settings
    //glClearColor(0.2f, 0.2f, 0.4f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    // reset to most likely original state
    //glActiveTexture(GL_TEXTURE0);

    // The current rendering size
    vpl::img::CPoint3D renderingSize = getRenderingSize(&Params, Flags);

    // Store the current matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // save current render target
    GLint framebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);

    // save current viewport size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Setup depth texture
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_spGLData->GeometryDepthTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewport[2], viewport[3], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, viewport[2], viewport[3]);

    // and change it to small off-screen texture
    glViewport(0, 0, renderingSize.x(), renderingSize.y());

    // modify OSG transformation matrix
    glScalef(Params.RealXSize * 0.5f, Params.RealXSize * 0.5f, Params.RealXSize * 0.5f);

    glEnable(GL_CULL_FACE);

    int val;
    // if fast redraw is not set, render volume
    if ((Flags & FAST_REDRAW) == 0)
    {
        GLenum drawBuffers0[] = { GL_COLOR_ATTACHMENT0_EXT };
        GLenum drawBuffers1[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };

        // render front side polygons of the box
        glCullFace(GL_BACK); // cull back-facing polygons
        glDepthFunc(GL_LESS); // keep front-most fragments
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_spGLData->OffScreenFramebuffer0);
        glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_3D, m_spGLData->RaysStartEnd, 0, 0);
        glDrawBuffers(1, drawBuffers0);
        glUseProgram(m_spGLData->PshaderFBO); // use special small shader
        glClearDepth(1.0);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderBox(&Params, m_Triangles);

        // render back side polygons of the box
        glCullFace(GL_FRONT); // cull front-facing polygons
        glDepthFunc(GL_GREATER); // keep back-most fragments
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_spGLData->OffScreenFramebuffer1);
        glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_3D, m_spGLData->RaysStartEnd, 0, 1);
        glDrawBuffers(1, drawBuffers0);
        glUseProgram(m_spGLData->PshaderFBO); // use special small shader
        glClearDepth(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderBox(&Params, m_Triangles);

        // render back side polygons of the box - this time using VR shaders and related stuff
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_spGLData->ResizeFramebuffer); // render to texture
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_spGLData->RTTexture, 0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, m_spGLData->DEPTexture, 0);
        glDrawBuffers(2, drawBuffers1);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use volume rendering shader
        unsigned int selectedShader = 0;
        if (Params.selectedShader == CUSTOM)
        {
            selectedShader = m_customShaderId;
        }
        else
        {
            selectedShader = m_spGLData->PshaderRayCast[Params.selectedShader];
        }
        glUseProgram(selectedShader);

        // bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, m_spGLData->VolumeTexture);
        setParameter(selectedShader, "t3D", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, (Params.selectedShader != CUSTOM ? m_spGLData->AuxVolumeTexture : m_spGLData->CustomAuxVolumeTexture));
        setParameter(selectedShader, "tSkip3D", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_1D, m_spGLData->LookUpTexture);
        setParameter(selectedShader, "LookUp", 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, m_spGLData->NoiseTexture);
        setParameter(selectedShader, "Noise", 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_3D, m_spGLData->RaysStartEnd);
        setParameter(selectedShader, "tRaysStartEnd", 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, m_spGLData->GeometryDepthTexture);
        setParameter(selectedShader, "Depth", 5);

        // nothing in Texture Unit #6
        //glActiveTexture(GL_TEXTURE6);
        //glBindTexture(GL_TEXTURE_3D, 0);
        //setParameter(selectedShader, "", 6);

        if (Params.selectedShader == CUSTOM)
        {
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_3D, m_currentVolume);
            setParameter(selectedShader, "tCustom3D", 7);
        }

        // set shader values
        float volumeSamplingDistance = getVolumeSamplingDistance(&Params, Flags);
        osg::Vec3 par_sVector = (Params.selectedShader == CUSTOM ? osg::Vec3(1.25f / float(Params.CustomXSize), 1.25f / float(Params.CustomYSize), 1.25f / float(Params.CustomZSize)) : osg::Vec3(1.25f / float(Params.XSize), 1.25f / float(Params.YSize), 1.25f / float(Params.ZSize)));
        osg::Vec3 par_skipTexSize = (Params.selectedShader == CUSTOM ? osg::Vec3(Params.CustomAuxTexXSize, Params.CustomAuxTexYSize, Params.CustomAuxTexZSize) : osg::Vec3(Params.AuxTexXSize, Params.AuxTexYSize, Params.AuxTexZSize));
        osg::Vec3 par_tResolution = (Params.selectedShader == CUSTOM ? osg::Vec3(volumeSamplingDistance / float(Params.CustomXSize), volumeSamplingDistance / float(Params.CustomYSize), volumeSamplingDistance / float(Params.CustomZSize)) : osg::Vec3(volumeSamplingDistance / float(Params.XSize), volumeSamplingDistance / float(Params.YSize), volumeSamplingDistance / float(Params.ZSize)));
        osg::Vec3 par_tSkipResolution = (Params.selectedShader == CUSTOM ? osg::Vec3(8.0f / float(Params.CustomXSize), 8.0f / float(Params.CustomYSize), 8.0f / float(Params.CustomZSize)) : osg::Vec3(8.0f / float(Params.XSize), 8.0f / float(Params.YSize), 8.0f / float(Params.ZSize)));
        osg::Matrix invProjectionMatrix = osg::Matrix(Params.invProjectionMatrix);
        osg::Matrix invModelViewMatrix = osg::Matrix(Params.invModelViewMatrix);

        setParameter(selectedShader, "textureSampling",     volumeSamplingDistance);
        setParameter(selectedShader, "inputAdjustment",     osg::Vec2(Params.dataPreMultiplication, -Params.dataOffset));
        setParameter(selectedShader, "imageAdjustment",     osg::Vec2(Params.imageBrightness, Params.imageContrast));
        setParameter(selectedShader, "sVector",             par_sVector);
        setParameter(selectedShader, "skipTexSize",         par_skipTexSize);
        setParameter(selectedShader, "tResolution",         par_tResolution);
        setParameter(selectedShader, "tSkipResolution",     par_tSkipResolution);
        setParameter(selectedShader, "StopCondition",       0.01f);
        setParameter(selectedShader, "wSize",               osg::Vec2(1.0f / float(renderingSize.x()), 1.0f / float(renderingSize.y())));
        setParameter(selectedShader, "pl",                  osg::Vec4(Params.planeA, Params.planeB, Params.planeC, Params.planeD + Params.planeDeltaNear * Sqrt3Div2));
        setParameter(selectedShader, "plNear",              osg::Vec4(Params.planeA, Params.planeB, Params.planeC, Params.planeD + Params.planeDeltaNear * Sqrt3Div2));
        setParameter(selectedShader, "plFar",               osg::Vec4(Params.planeA, Params.planeB, Params.planeC, Params.planeD + Params.planeDeltaFar * Sqrt3Div2));
        setParameter(selectedShader, "invProjectionMatrix", invProjectionMatrix);
        setParameter(selectedShader, "invModelViewMatrix",  invModelViewMatrix);
        setParameter(selectedShader, "skipCondition",       m_skipConditions[Params.selectedLut]);

        if (Params.selectedShader == SURFACE)
        {
            setParameter(selectedShader, "surfacePar",  osg::Vec2(Params.surfaceNormalMult, Params.surfaceNormalExp));
        }

        // let plugin fill custom shader parameters
        if (Params.selectedShader == CUSTOM)
        {
            shaderUpdateCallback();
        }

        // actual rendering
        renderBox(&Params, m_Triangles);

        // restore culling and depth func
        glCullFace(GL_BACK);
        glDepthFunc(GL_LESS);
        glClearDepth(1.0);
    }

    // now RESIZE rendered texture to fit window
    // AND RENDER BICUBIC RESIZED IMAGE TO WHOLE WINDOW - SCREEN
    glUseProgram(m_spGLData->PshaderResize);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_spGLData->RTTexture);
    setParameter(m_spGLData->PshaderResize, "image", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_spGLData->DEPTexture);
    setParameter(m_spGLData->PshaderResize, "outdepth", 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_1D, m_spGLData->BicKerTexture);
    setParameter(m_spGLData->PshaderResize, "kernel", 6);

    setParameter(m_spGLData->PshaderResize, "resolution", osg::Vec2(renderingSize.x(), renderingSize.y()));

    // end of parameters

    glColor3f(0, 1, 1); // no use
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_ALWAYS);
    glDisable(GL_CULL_FACE);
    //GLint binding;
    //glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &binding);

    // Save current state
    bool bVertexArray = glIsEnabled(GL_VERTEX_ARRAY);
    bool bTexCoordArray = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
    bool bNormalArray = glIsEnabled(GL_NORMAL_ARRAY);
    bool bColorArray = glIsEnabled(GL_COLOR_ARRAY);
    bool bIndexArray = glIsEnabled(GL_INDEX_ARRAY);
    bool bSecondaryColorArray = glIsEnabled(GL_SECONDARY_COLOR_ARRAY);
    bool bEdgeFlagArray = glIsEnabled(GL_EDGE_FLAG_ARRAY);
    bool bFogCoordArray = glIsEnabled(GL_FOG_COORD_ARRAY);

#ifdef USE_VAORECT
    GLint vertexArray;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArray);

    // Bind the corresponding VAO
    glBindVertexArrayX(m_spGLData->VAORect);

    // Draw the rect
    glDrawArrays(GL_TRIANGLES, 0, GLsizei(6));

    // Unboud a previously bound Vertex Array Object (just to be sure)
    glBindVertexArrayX(vertexArray);
#else
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // And set up vertex attributes
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_spGLData->VBORect);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, GLsizei(6));
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
#endif
    
    // Restore state so that OSG can continue its work
    bVertexArray            ? glEnableClientState(GL_VERTEX_ARRAY)          : glDisableClientState(GL_VERTEX_ARRAY);
    bTexCoordArray          ? glEnableClientState(GL_TEXTURE_COORD_ARRAY)   : glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    bNormalArray            ? glEnableClientState(GL_NORMAL_ARRAY)          : glDisableClientState(GL_NORMAL_ARRAY);
    bColorArray             ? glEnableClientState(GL_COLOR_ARRAY)           : glDisableClientState(GL_COLOR_ARRAY);
    bIndexArray             ? glEnableClientState(GL_INDEX_ARRAY)           : glDisableClientState(GL_INDEX_ARRAY);
    bSecondaryColorArray    ? glEnableClientState(GL_SECONDARY_COLOR_ARRAY) : glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    bEdgeFlagArray          ? glEnableClientState(GL_EDGE_FLAG_ARRAY)       : glDisableClientState(GL_EDGE_FLAG_ARRAY);
    bFogCoordArray          ? glEnableClientState(GL_FOG_COORD_ARRAY)       : glDisableClientState(GL_FOG_COORD_ARRAY);

    // restore state
    glPopMatrix();
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // From http://www.opengl.org/wiki/index.php/Common_Mistakes#Unsupported_formats_.231
    // 
    // Use glFlush if you are rendering directly to your window. It is better to
    // have a double buffered window but if you have a case where you want to render
    // to the window directly, then go ahead.
    //
    // There is a lot of tutorial website that show this 
    // 
    // glFlush();
    // SwapBuffers();
    //
    // Never call glFlush before calling SwapBuffers. The SwapBuffer command takes care of flushing and command processing.
    //glFlush();
}

///////////////////////////////////////////////////////////////////////////////
//
bool PSVolumeRendering::init()
{
    tLock Lock(*this);

    // Already initialized?
    if (testFlag(INITIALIZED))
    {
        return true;
    }

    // already
    if (!testFlag(INIT_INVALID))
    {
        // Initial test
        if (!canStart())
        {
            return false;
        }

        //setAndSignalFlag(INIT_INVALID);
        setFlag(INIT_INVALID);
    }

    if (testFlag(INIT_INVALID))
    {
        if (!internalInitRendering())
        {
            m_FailureCounter++;
            return false;
        }
        else
        {
            m_FailureCounter = 0;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Resets initialization failure counter
void PSVolumeRendering::resetFailureCounter()
{
    m_FailureCounter = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Returns if init has failed too many times in a row
bool PSVolumeRendering::constantFailure()
{
    return (m_FailureCounter > PSVR_FAIL_LIMIT);
}

///////////////////////////////////////////////////////////////////////////////
// Returns all error strings
std::vector<std::string> PSVolumeRendering::getErrorStrings()
{
    return m_ErrorStrings;
}

///////////////////////////////////////////////////////////////////////////////
// are we able to render?
void PSVolumeRendering::setMouseMode(bool bEnable)
{
    tLock Lock(*this);

    /*clearFlag(FAST_REDRAW);
    if (APP_MODE.check(scene::CAppMode::COMMAND_DRAW_WINDOW))
    {
        if (bEnable)
        {
            setFlag(FAST_REDRAW);
        }

        return;
    }*/

    if (bEnable)
    {
        setAndSignalFlag(MOUSE_MODE | OSR_INVALID);
    }
    else
    {
        clearFlag(MOUSE_MODE);
        setAndSignalFlag(OSR_INVALID);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setMousePressed(bool bPressed)
{
    tLock Lock(*this);

    if (bPressed)
    {
        setFlag(MOUSE_PRESSED);
    }
    else
    {
        clearFlag(MOUSE_PRESSED);
    }
}

//////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setShader(int shader)
{
    if (shader < 0 || shader >= SHADERS_COUNT)
    {
        return;
    }

    tLock Lock(*this);

    m_spParams->selectedShader = shader;

    VPL_SIGNAL(SigVRModeChange).invoke(shader);
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setLut(int lut)
{
    if (lut < 0 || lut >= LOOKUPS_COUNT)
    {
        return;
    }

    tLock Lock(*this);

    m_spParams->selectedLut = lut;

    //setAndSignalFlag(LUT_INVALID);
    setFlag(LUT_INVALID);

    VPL_SIGNAL(SigVRLutChange).invoke(lut);
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setQuality(int quality)
{
    if (quality < 0 || quality >= QUALITY_LEVELS)
    {
        return;
    }

    tLock Lock(*this);

    if (m_spParams->currentQuality == quality)
    {
        return;
    }

    // set new quality
    m_spParams->currentQuality = quality;
    m_spParams->volumeSamplingDistance = conf::TextureSampling[quality];
    m_spParams->renderingSize = conf::RenderingSize[quality];

    setAndSignalFlag(OSR_INVALID | DATA_INVALID);

    VPL_SIGNAL(SigVRQualityChange).invoke(quality);
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setDataRemap(float expand, float offset)
{
    tLock Lock(*this);

    m_spParams->dataPreMultiplication = expand;
    m_spParams->dataOffset = offset;

    //VPL_LOG_INFO("PSVolumeRendering::setDataRemap(): mult = " << m_spParams->dataPreMultiplication << ", offset = " << m_spParams->dataOffset);
    VPL_SIGNAL(SigVRDataRemapChange).invoke(expand, offset);
}

void PSVolumeRendering::getDataRemap(float& expand, float& offset)
{
    tLock Lock(*this);

    expand = m_spParams->dataPreMultiplication;
    offset = m_spParams->dataOffset;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setPicture(float brightness, float contrast)
{
    tLock Lock(*this);

    m_spParams->imageBrightness = brightness;
    m_spParams->imageContrast = contrast;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setSurfaceDetection(float mult, float exp)
{
    tLock Lock(*this);

    m_spParams->surfaceNormalMult = mult;
    m_spParams->surfaceNormalExp = exp;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setRenderingSize(int size)
{
    if (size < 0)
    {
        return;
    }

    tLock Lock(*this);

    m_spParams->renderingSize = size;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setCuttingPlane(float a, float b, float c, float d)
{
    tLock Lock(*this);

    m_spParams->planeA = a;
    m_spParams->planeB = b;
    m_spParams->planeC = c;
    m_spParams->planeD = d;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setCuttingPlaneDisplacement(float deltaNear, float deltaFar)
{
    tLock Lock(*this);

    vpl::math::limit<float>(deltaNear, -1.0f, 1.0f);
    vpl::math::limit<float>(deltaFar, -1.0f, 1.0f);
    m_spParams->planeDeltaNear = deltaNear;
    m_spParams->planeDeltaFar = deltaFar;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setNearCuttingPlaneDisplacement(float delta)
{
    vpl::math::limit<float>(delta, -1.0f, 1.0f);
    setCuttingPlaneDisplacement(delta, m_spParams->planeDeltaFar);
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setFarCuttingPlaneDisplacement(float delta)
{
    vpl::math::limit<float>(delta, -1.0f, 1.0f);
    setCuttingPlaneDisplacement(m_spParams->planeDeltaNear, delta);
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::getCuttingPlane(float &a, float &b, float &c, float &d)
{
    a = m_spParams->planeA;
    b = m_spParams->planeB;
    c = m_spParams->planeC;
    d = m_spParams->planeD;
}

///////////////////////////////////////////////////////////////////////////////
//
float PSVolumeRendering::getNearCuttingPlaneDisplacement()
{
    return m_spParams->planeDeltaNear;
}

///////////////////////////////////////////////////////////////////////////////
//
float PSVolumeRendering::getFarCuttingPlaneDisplacement()
{
    return m_spParams->planeDeltaFar;
}

///////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setSamplingDistance(float distance)
{
    if (distance < 0.0f)
    {
        return;
    }

    tLock Lock(*this);

    m_spParams->volumeSamplingDistance = distance;
}

///////////////////////////////////////////////////////////////////////////////
// get methods
int  PSVolumeRendering::getLut() const
{
    return m_spParams->selectedLut;
}

int  PSVolumeRendering::getShader() const
{
    return m_spParams->selectedShader;
}

int  PSVolumeRendering::getQuality() const
{
    return m_spParams->currentQuality;
}

} // namesapce PSVR

#endif // USE_PSVR
