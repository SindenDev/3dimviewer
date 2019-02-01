///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// This file comes from proprietary 3Dim Laboratory s.r.o. software
// and was modified for 
// 
// BlueSkyPlan version 3.x
// Diagnostic and implant planning software for dentistry.
// 
// The original license can be found below.
//
// Changes are Copyright 2012 Blue Sky Bio, LLC
// All rights reserved 
//
// Changelog:
//    [2012/mm/dd] - ...
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2008-2012 3Dim Laboratory s.r.o. 
// All rights reserved 
//
///////////////////////////////////////////////////////////////////////////////

#include <render/glErrorReporting.h>
#include <osg/DisplaySettings>

std::string glErrorEnumString(GLenum value)
{
    std::string outString = "";
    switch (value)
    {
    case GL_INVALID_ENUM:
        outString = "GL_INVALID_ENUM";
        break;

    case GL_INVALID_VALUE:
        outString = "GL_INVALID_VALUE";
        break;

    case GL_INVALID_OPERATION:
        outString = "GL_INVALID_OPERATION";
        break;

    case GL_INVALID_FRAMEBUFFER_OPERATION:
        outString = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;

    case GL_OUT_OF_MEMORY:
        outString = "GL_OUT_OF_MEMORY";
        break;

   /* case GL_STACK_UNDERFLOW:
        outString = "GL_STACK_UNDERFLOW";
        break;

    case GL_STACK_OVERFLOW:
        outString = "GL_STACK_OVERFLOW";
        break;*/

    case GL_NO_ERROR:
        outString = "";
        break;

    default:
        outString = "UNKNOWN_ERROR";
        break;
    }
    return outString;
}

std::string glFramebufferStatusString(GLenum value)
{
    std::string outString = "";
    switch (value)
    {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        outString = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        outString = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        outString = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        outString = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;

    case GL_FRAMEBUFFER_UNSUPPORTED:
        outString = "GL_FRAMEBUFFER_UNSUPPORTED";
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        outString = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        outString = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        break;

    case GL_FRAMEBUFFER_COMPLETE:
        outString = "GL_FRAMEBUFFER_COMPLETE";
        break;

    default:
        outString = "";
        break;
    }
    return outString;
}

std::string glGetErrors(std::string functionName)
{
    std::string errorString = "";
    GLenum error = glGetError();
    int i = 0;
    while (error != GL_NO_ERROR && i<100)
    {
        if (i<10)
            errorString += (errorString.empty() ? "OGL function \"" + functionName + "\" generated these errors: " : ", ") + glErrorEnumString(error);
        error = glGetError();
        i++;
    }
    return errorString;
}

bool isDebugMessageCallbackAvailable()
{
#ifdef __APPLE__
    return false;
#endif
    int glVersion[2] = { -1, -1 };
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

    return (glVersion[0] > 4) || (glVersion[0] == 4 && glVersion[1] >= 3);
}

bool isDebugContextEnabled()
{
    int contextFlag;

    glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlag);

    return contextFlag & GL_CONTEXT_FLAG_DEBUG_BIT;
}

bool glGetErrorEnabled()
{
    static bool enabled = !isDebugMessageCallbackAvailable() && (osg::DisplaySettings::instance()->getGLContextFlags() & GL_CONTEXT_FLAG_DEBUG_BIT);

    return enabled;
}
