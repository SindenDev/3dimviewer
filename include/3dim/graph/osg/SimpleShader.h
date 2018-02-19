

#ifndef SimpleShader_H
#define SimpleShader_H

const char texturedShaderVert[] =
"#version 330 core\n\
\n\
uniform mat4 osg_ModelViewProjectionMatrix;\n\
\n\
in vec4 osg_Vertex;\n\
in vec4 osg_Color;\n\
in vec4 osg_MultiTexCoord0;\n\
\n\
out VertexData\n\
{\n\
    vec2 texCoords;\n\
    vec4 vertexColor;\n\
} VertexOut;\n\
\n\
void main()\n\
{\n\
    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n\
    VertexOut.texCoords = osg_MultiTexCoord0.xy;\n\
    VertexOut.vertexColor = osg_Color;\n\
}\n";

const char texturedShaderFrag[] =
"#version 330 core\n\
\n\
uniform sampler2D texSampler;\n\
\n\
in VertexData\n\
{\n\
    vec2 texCoords;\n\
    vec4 vertexColor;\n\
} VertexIn;\n\
\n\
out vec4 outColor;\n\
\n\
void main()\n\
{\n\
    outColor = VertexIn.vertexColor * texture(texSampler, VertexIn.texCoords);\n\
}\n";

const char texturedShaderAlphaToCoverageFrag[] =
"#version 330 core\n\
\n\
uniform sampler2D texSampler;\n\
\n\
in VertexData\n\
{\n\
    vec2 texCoords;\n\
    vec4 vertexColor;\n\
} VertexIn;\n\
\n\
out vec4 outColor;\n\
\n\
void main()\n\
{\n\
    outColor = VertexIn.vertexColor * texture(texSampler, VertexIn.texCoords); \n\
    outColor.a = (outColor.a - 0.5) / max(fwidth(outColor.a), 0.0001) + 0.5;\n\
}\n";

const char texturedShaderAlphaTestFrag[] =
"#version 330 core\n\
\n\
uniform sampler2D texSampler;\n\
\n\
in VertexData\n\
{\n\
    vec2 texCoords;\n\
    vec4 vertexColor;\n\
} VertexIn;\n\
\n\
out vec4 outColor;\n\
\n\
void main()\n\
{\n\
    vec4 color = texture(texSampler, VertexIn.texCoords);\n\
    if (color.a < 0.5)\n\
    {\n\
        discard; \n\
    }\n\
    outColor = VertexIn.vertexColor * texture(texSampler, VertexIn.texCoords); \n\
}\n";

const char colorShaderVert[] =
"#version 330 core\n\
\n\
uniform mat4 osg_ModelViewProjectionMatrix;\n\
\n\
in vec4 osg_Vertex;\n\
in vec4 osg_Color;\n\
\n\
out VertexData\n\
{\n\
    vec4 vertexColor;\n\
} VertexOut;\n\
\n\
void main()\n\
{\n\
    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;\n\
    VertexOut.vertexColor = osg_Color;\n\
}\n";

const char colorShaderFrag[] =
"#version 330 core\n\
\n\
in VertexData\n\
{\n\
    vec4 vertexColor;\n\
} VertexIn;\n\
\n\
out vec4 outColor;\n\
\n\
void main()\n\
{\n\
    outColor = VertexIn.vertexColor;\n\
}\n";

#endif