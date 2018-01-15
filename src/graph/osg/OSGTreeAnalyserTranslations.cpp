
#include <osg/OSGTreeAnalyser.h>

#include <osg/Texture>
#include <osg/CullSettings>
#include <osg/Camera>


#include <qdebug.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>    //isnan, isinf


//this should prevent any other type as argument
template <>
std::string osg::OSGTreeAnalyser::translateFloat<float>(const float& number) {

    if (std::isnan(number))
        return "NaN";

    if (number < 0 && std::isinf(number))
        return "-INF";
    else if (number > 0 && std::isinf(number))
        return "INF";


    return std::to_string(number);
}

template <>
std::string osg::OSGTreeAnalyser::translateDouble<double>(const double& number) {

    if (std::isnan(number))
        return "NaN";

    if (std::isinf(number))
        return "INF";

    return std::to_string(number);
}


std::string osg::OSGTreeAnalyser::strigifyMatrix(osg::Matrixd matrix) {

    std::string str;
    str.append(std::string(level++, '\t') + "<matrix att=\"ignore\">\n");
    for (size_t row = 0; row < 4; row++) {
        str.append(std::string(level, '\t'));
        for (size_t column = 0; column < 4; column++) {
            str.append(translateDouble(matrix(row, column)) + " ");
        }
        str.replace(str.size() - 1, str.size() - 1, "\n");  //remove trailing space
    }

    str.append(std::string(--level, '\t') + "</matrix>");
    return str;
}

std::string osg::OSGTreeAnalyser::stringifyVec4(osg::Vec4 vec) {
    std::string str;

    for (size_t i = 0; i < 4; i++) {
        str.append(translateFloat(vec[i]) + " ");
    }
    str.replace(str.size() - 1, str.size() - 1, "");  //remove trailing space

    return str;

}






std::string osg::OSGTreeAnalyser::translateReferenceFrame(osg::Transform::ReferenceFrame reference_frame) {

    switch (reference_frame) {
    case osg::Transform::ReferenceFrame::RELATIVE_RF: return "RELATIVE_RF";
    case osg::Transform::ReferenceFrame::ABSOLUTE_RF:return "ABSOLUTE_RF";
    case osg::Transform::ReferenceFrame::ABSOLUTE_RF_INHERIT_VIEWPOINT:return "ABSOLUTE_RF_INHERIT_VIEWPOINT";
    default: return "unknown reference frame";
    }
}

std::string osg::OSGTreeAnalyser::translateBinding(osg::Array::Binding binding) {

    switch (binding) {
    case osg::Array::Binding::BIND_UNDEFINED: return "BIND_UNDEFINED";
    case osg::Array::Binding::BIND_OFF: return "BIND_OFF";
    case osg::Array::Binding::BIND_OVERALL: return "BIND_OVERALL";
    case osg::Array::Binding::BIND_PER_PRIMITIVE_SET: return "BIND_PER_PRIMITIVE_SET";
    case osg::Array::Binding::BIND_PER_VERTEX: return "BIND_PER_VERTEX";
    default: return "unknown array binding";
    }
}


std::string osg::OSGTreeAnalyser::translateBool(bool value) {

    if (value)
        return "true";
    else return "false";
}


std::string osg::OSGTreeAnalyser::translateBoolToOnOff(bool value) {

    if (value)
        return "On";
    else return "Off";
}

/*
std::string translateBinding(osg::PrimitiveSet::Type type) {


case P : return "P";rimitiveType,
case D : return "D";rawArraysPrimitiveType,
case D : return "D";rawArrayLengthsPrimitiveType,
case D : return "D";rawElementsUBytePrimitiveType,
case D : return "D";rawElementsUShortPrimitiveType,
case D : return "D";rawElementsUIntPrimitiveType
};
*/
std::string osg::OSGTreeAnalyser::translatePrimitiveSetMode(/*osg::PrimitiveSet::Mode*/ GLenum mode) {
    switch (mode) {
    case GL_POINTS: return "POINTS";
    case GL_LINES: return "LINES";
    case GL_LINE_STRIP: return "LINE_STRIP";
    case GL_LINE_LOOP: return "LINE_LOOP";
    case GL_TRIANGLES: return "TRIANGLES";
    case GL_TRIANGLE_STRIP: return "TRIANGLE_STRIP";
    case GL_TRIANGLE_FAN: return "TRIANGLE_FAN";
    case GL_QUADS: return "QUADS";
    case GL_QUAD_STRIP: return "QUAD_STRIP";
    case GL_POLYGON: return "POLYGON";
    case GL_LINES_ADJACENCY_EXT: return "LINES_ADJACENCY";
    case GL_LINE_STRIP_ADJACENCY_EXT: return "LINE_STRIP_ADJACENCY";
    case GL_TRIANGLES_ADJACENCY_EXT: return "TRIANGLES_ADJACENCY";
    case GL_TRIANGLE_STRIP_ADJACENCY_EXT: return "TRIANGLE_STRIP_ADJACENCY";
    case GL_PATCHES: return "PATCHES";
    default: return "unknown primitive set mode";
    }
};

std::string osg::OSGTreeAnalyser::translateModeValue(/*osg::StateAttribute::Values*/unsigned int value_mask) {

    std::string tmp;

    if (value_mask == 0)
        return "OFF";

    //reason for this is that attributes don't have explicit ON so if I don't want to have leading space I have to do this this way..
    bool on = false;

    if ((value_mask & osg::StateAttribute::Values::ON) == osg::StateAttribute::Values::ON) {
        tmp.append("ON");
        on = true;
    }

    if ((value_mask & osg::StateAttribute::Values::OVERRIDE) == osg::StateAttribute::Values::OVERRIDE) {
        
        if (not on) {
            tmp.append("OVERRIDE");
            on = true;
        } else
            tmp.append(" OVERRIDE");
    }

    if ((value_mask & osg::StateAttribute::Values::PROTECTED) == osg::StateAttribute::Values::PROTECTED) {
        if (not on) {
            tmp.append("PROTECTED");
            on = true;
        } else
            tmp.append(" PROTECTED");
    }

    if ((value_mask & osg::StateAttribute::Values::INHERIT) == osg::StateAttribute::Values::INHERIT) {
        if (not on) {
            tmp.append("INHERIT");
            on = true;
        } else
            tmp.append(" INHERIT");
    }

    return tmp;

}

//from https://cvs.khronos.org/svn/repos/ogl/trunk/doc/registry/public/api/gl.xml
std::string osg::OSGTreeAnalyser::translateStateAttributeMode(/*osg::StateAttribute::GLMode*/ GLenum mode) {


    //maybe some stuff is missing or some is redundant.. no definitive list exists..
    //this was grabbed from OGL list of enable enums linked above
    switch (mode) {
    case GL_ALPHA_TEST: return "GL_ALPHA_TEST";
    case GL_AUTO_NORMAL: return "GL_AUTO_NORMAL";
    case GL_BLEND: return "GL_BLEND";
    case GL_CLIP_PLANE0: return "GL_CLIP_PLANE0";
    case GL_CLIP_PLANE1: return "GL_CLIP_PLANE1";
    case GL_CLIP_PLANE2: return "GL_CLIP_PLANE2";
    case GL_CLIP_PLANE3: return "GL_CLIP_PLANE3";
    case GL_CLIP_PLANE4: return "GL_CLIP_PLANE4";
    case GL_CLIP_PLANE5: return "GL_CLIP_PLANE5";
    case GL_COLOR_ARRAY: return "GL_COLOR_ARRAY";
    case GL_COLOR_LOGIC_OP: return "GL_COLOR_LOGIC_OP";
    case GL_COLOR_MATERIAL: return "GL_COLOR_MATERIAL";
    case GL_CULL_FACE: return "GL_CULL_FACE";
    case GL_DEPTH_TEST: return "GL_DEPTH_TEST";
    case GL_DITHER: return "GL_DITHER";
    case GL_EDGE_FLAG_ARRAY: return "GL_EDGE_FLAG_ARRAY";
    case GL_FOG: return "GL_FOG";
    case GL_INDEX_ARRAY: return "GL_INDEX_ARRAY";
    case GL_INDEX_LOGIC_OP: return "GL_INDEX_LOGIC_OP";
    case GL_LIGHT0: return "GL_LIGHT0";
    case GL_LIGHT1: return "GL_LIGHT1";
    case GL_LIGHT2: return "GL_LIGHT2";
    case GL_LIGHT3: return "GL_LIGHT3";
    case GL_LIGHT4: return "GL_LIGHT4";
    case GL_LIGHT5: return "GL_LIGHT5";
    case GL_LIGHT6: return "GL_LIGHT6";
    case GL_LIGHT7: return "GL_LIGHT7";
    case GL_LIGHTING: return "GL_LIGHTING";
    case GL_LINE_SMOOTH: return "GL_LINE_SMOOTH";
    case GL_LINE_STIPPLE: return "GL_LINE_STIPPLE";
    case GL_MAP1_COLOR_4: return "GL_MAP1_COLOR_4";
    case GL_MAP1_INDEX: return "GL_MAP1_INDEX";
    case GL_MAP1_NORMAL: return "GL_MAP1_NORMAL";
    case GL_MAP1_TEXTURE_COORD_1: return "GL_MAP1_TEXTURE_COORD_1";
    case GL_MAP1_TEXTURE_COORD_2: return "GL_MAP1_TEXTURE_COORD_2";
    case GL_MAP1_TEXTURE_COORD_3: return "GL_MAP1_TEXTURE_COORD_3";
    case GL_MAP1_TEXTURE_COORD_4: return "GL_MAP1_TEXTURE_COORD_4";
    case GL_MAP1_VERTEX_3: return "GL_MAP1_VERTEX_3";
    case GL_MAP1_VERTEX_4: return "GL_MAP1_VERTEX_4";
    case GL_MAP2_COLOR_4: return "GL_MAP2_COLOR_4";
    case GL_MAP2_INDEX: return "GL_MAP2_INDEX";
    case GL_MAP2_NORMAL: return "GL_MAP2_NORMAL";
    case GL_MAP2_TEXTURE_COORD_1: return "GL_MAP2_TEXTURE_COORD_1";
    case GL_MAP2_TEXTURE_COORD_2: return "GL_MAP2_TEXTURE_COORD_2";
    case GL_MAP2_TEXTURE_COORD_3: return "GL_MAP2_TEXTURE_COORD_3";
    case GL_MAP2_TEXTURE_COORD_4: return "GL_MAP2_TEXTURE_COORD_4";
    case GL_MAP2_VERTEX_3: return "GL_MAP2_VERTEX_3";
    case GL_MAP2_VERTEX_4: return "GL_MAP2_VERTEX_4";
    case GL_NORMALIZE: return "GL_NORMALIZE";
    case GL_NORMAL_ARRAY: return "GL_NORMAL_ARRAY";
    case GL_POINT_SMOOTH: return "GL_POINT_SMOOTH";
    case GL_POLYGON_OFFSET_FILL: return "GL_POLYGON_OFFSET_FILL";
    case GL_POLYGON_OFFSET_LINE: return "GL_POLYGON_OFFSET_LINE";
    case GL_POLYGON_OFFSET_POINT: return "GL_POLYGON_OFFSET_POINT";
    case GL_POLYGON_SMOOTH: return "GL_POLYGON_SMOOTH";
    case GL_POLYGON_STIPPLE: return "GL_POLYGON_STIPPLE";
    case GL_SCISSOR_TEST: return "GL_SCISSOR_TEST";
    case GL_STENCIL_TEST: return "GL_STENCIL_TEST";
    case GL_TEXTURE_1D: return "GL_TEXTURE_1D";
    case GL_TEXTURE_2D: return "GL_TEXTURE_2D";
    case GL_TEXTURE_COORD_ARRAY: return "GL_TEXTURE_COORD_ARRAY";
    case GL_TEXTURE_GEN_Q: return "GL_TEXTURE_GEN_Q";
    case GL_TEXTURE_GEN_R: return "GL_TEXTURE_GEN_R";
    case GL_TEXTURE_GEN_S: return "GL_TEXTURE_GEN_S";
    case GL_TEXTURE_GEN_T: return "GL_TEXTURE_GEN_T";
    case GL_VERTEX_ARRAY: return "GL_VERTEX_ARRAY";
        //form osg
    case GL_RESCALE_NORMAL: return "GL_RESCALE_NORMAL";

    default: return "unknown state attribute mode";
    }
}


std::string osg::OSGTreeAnalyser::translateRenderingHint(/*osg::StateSet::RenderingHint*/int hint) {
    switch (hint) {

    case osg::StateSet::RenderingHint::DEFAULT_BIN: return "DEFAULT_BIN";
    case osg::StateSet::RenderingHint::OPAQUE_BIN: return "OPAQUE_BIN";
    case osg::StateSet::RenderingHint::TRANSPARENT_BIN: return "TRANSPARENT_BIN";
    default: return "unknown rendering hint";
    }
};


std::string osg::OSGTreeAnalyser::translateRenderbinMode(osg::StateSet::RenderBinMode mode) {

    switch (mode) {
    case osg::StateSet::RenderBinMode::INHERIT_RENDERBIN_DETAILS: return "INHERIT_RENDERBIN_DETAILS";
    case osg::StateSet::RenderBinMode::USE_RENDERBIN_DETAILS: return "USE_RENDERBIN_DETAILS";
    case osg::StateSet::RenderBinMode::OVERRIDE_RENDERBIN_DETAILS: return "OVERRIDE_RENDERBIN_DETAILS";
    default: return "unknown renderbin mode";
    }

}
std::string osg::OSGTreeAnalyser::translateStateAttributeType(/*osg::StateAttribute::Type*/int attribute) {

    switch (attribute) {

    case osg::StateAttribute::Type::TEXTURE: return "TEXTURE";

    case osg::StateAttribute::Type::POLYGONMODE: return "POLYGONMODE";
    case osg::StateAttribute::Type::POLYGONOFFSET: return "POLYGONOFFSET";
    case osg::StateAttribute::Type::MATERIAL: return "MATERIAL";
    case osg::StateAttribute::Type::ALPHAFUNC: return "ALPHAFUNC";
    case osg::StateAttribute::Type::ANTIALIAS: return "ANTIALIAS";
    case osg::StateAttribute::Type::COLORTABLE: return "COLORTABLE";
    case osg::StateAttribute::Type::CULLFACE: return "CULLFACE";
    case osg::StateAttribute::Type::FOG: return "FOG";
    case osg::StateAttribute::Type::FRONTFACE: return "FRONTFACE";

    case osg::StateAttribute::Type::LIGHT: return "LIGHT";

    case osg::StateAttribute::Type::POINT: return "POINT";
    case osg::StateAttribute::Type::LINEWIDTH: return "LINEWIDTH";
    case osg::StateAttribute::Type::LINESTIPPLE: return "LINESTIPPLE";
    case osg::StateAttribute::Type::POLYGONSTIPPLE: return "POLYGONSTIPPLE";
    case osg::StateAttribute::Type::SHADEMODEL: return "SHADEMODEL";
    case osg::StateAttribute::Type::TEXENV: return "TEXENV";
    case osg::StateAttribute::Type::TEXENVFILTER: return "TEXENVFILTER";
    case osg::StateAttribute::Type::TEXGEN: return "TEXGEN";
    case osg::StateAttribute::Type::TEXMAT: return "TEXMAT";
    case osg::StateAttribute::Type::LIGHTMODEL: return "LIGHTMODEL";
    case osg::StateAttribute::Type::BLENDFUNC: return "BLENDFUNC";
    case osg::StateAttribute::Type::BLENDEQUATION: return "BLENDEQUATION";
    case osg::StateAttribute::Type::LOGICOP: return "LOGICOP";
    case osg::StateAttribute::Type::STENCIL: return "STENCIL";
    case osg::StateAttribute::Type::COLORMASK: return "COLORMASK";
    case osg::StateAttribute::Type::DEPTH: return "DEPTH";
    case osg::StateAttribute::Type::VIEWPORT: return "VIEWPORT";
    case osg::StateAttribute::Type::SCISSOR: return "SCISSOR";
    case osg::StateAttribute::Type::BLENDCOLOR: return "BLENDCOLOR";
    case osg::StateAttribute::Type::MULTISAMPLE: return "MULTISAMPLE";
    case osg::StateAttribute::Type::CLIPPLANE: return "CLIPPLANE";
    case osg::StateAttribute::Type::COLORMATRIX: return "COLORMATRIX";
    case osg::StateAttribute::Type::VERTEXPROGRAM: return "VERTEXPROGRAM";
    case osg::StateAttribute::Type::FRAGMENTPROGRAM: return "FRAGMENTPROGRAM";
    case osg::StateAttribute::Type::POINTSPRITE: return "POINTSPRITE";
    case osg::StateAttribute::Type::PROGRAM: return "PROGRAM";
    case osg::StateAttribute::Type::CLAMPCOLOR: return "CLAMPCOLOR";
    case osg::StateAttribute::Type::HINT: return "HINT";
    case osg::StateAttribute::Type::SAMPLEMASKI: return "SAMPLEMASKI";
    case osg::StateAttribute::Type::PRIMITIVERESTARTINDEX: return "PRIMITIVERESTARTINDEX";

        /// osgFX namespace
    case osg::StateAttribute::Type::VALIDATOR: return "VALIDATOR";
    case osg::StateAttribute::Type::VIEWMATRIXEXTRACTOR: return "VIEWMATRIXEXTRACTOR";

        /// osgNV namespace
    case osg::StateAttribute::Type::OSGNV_PARAMETER_BLOCK: return "OSGNV_PARAMETER_BLOCK";

        // osgNVExt namespace
    case osg::StateAttribute::Type::OSGNVEXT_TEXTURE_SHADER: return "OSGNVEXT_TEXTURE_SHADER";
    case osg::StateAttribute::Type::OSGNVEXT_VERTEX_PROGRAM: return "OSGNVEXT_VERTEX_PROGRAM";
    case osg::StateAttribute::Type::OSGNVEXT_REGISTER_COMBINERS: return "OSGNVEXT_REGISTER_COMBINERS";

        /// osgNVCg namespace
    case osg::StateAttribute::Type::OSGNVCG_PROGRAM: return "OSGNVCG_PROGRAM";

        // osgNVSlang namespace
    case osg::StateAttribute::Type::OSGNVSLANG_PROGRAM: return "OSGNVSLANG_PROGRAM";

        // osgNVParse
    case osg::StateAttribute::Type::OSGNVPARSE_PROGRAM_PARSER: return "OSGNVPARSE_PROGRAM_PARSER";

    case osg::StateAttribute::Type::UNIFORMBUFFERBINDING: return "UNIFORMBUFFERBINDING";
    case osg::StateAttribute::Type::TRANSFORMFEEDBACKBUFFERBINDING: return "TRANSFORMFEEDBACKBUFFERBINDING";

    case osg::StateAttribute::Type::ATOMICCOUNTERBUFFERBINDING: return "ATOMICCOUNTERBUFFERBINDING";

    case osg::StateAttribute::Type::PATCH_PARAMETER: return "PATCH_PARAMETER";

    case osg::StateAttribute::Type::FRAME_BUFFER_OBJECT: return "FRAME_BUFFER_OBJECT";

    default: return "unknown state attribute type";
    }
}

std::string osg::OSGTreeAnalyser::translateTextureFilterMode(/*osg::Texture::FilterMode*/int mode) {

    switch (mode) {

    case osg::Texture::LINEAR: return "LINEAR";
    case osg::Texture::LINEAR_MIPMAP_LINEAR: return "LINEAR_MIPMAP_LINEAR";
    case osg::Texture::LINEAR_MIPMAP_NEAREST: return "LINEAR_MIPMAP_NEAREST";
    case osg::Texture::NEAREST: return "NEAREST";
    case osg::Texture::NEAREST_MIPMAP_LINEAR: return "NEAREST_MIPMAP_LINEAR";
    case osg::Texture::NEAREST_MIPMAP_NEAREST: return "NEAREST_MIPMAP_NEAREST";
    default: return "unknown texture filter mode";


    }
}

std::string osg::OSGTreeAnalyser::translateTextureWrapMode(/*osg::Texture::WrapMode*/int mode) {

    switch (mode) {
        case osg::Texture::CLAMP : return "CLAMP";
        case osg::Texture::CLAMP_TO_EDGE : return "CLAMP_TO_EDGE";
        case osg::Texture::CLAMP_TO_BORDER : return "CLAMP_TO_BORDER";
        case osg::Texture::REPEAT : return "REPEAT";
        case osg::Texture::MIRROR : return "MIRROR";
        default: return "unknown texture wrap mode";
    }
}

std::string osg::OSGTreeAnalyser::translateInternalFormatMode(int mode) {
    switch (mode) {
        case osg::Texture::USE_IMAGE_DATA_FORMAT: return "USE_IMAGE_DATA_FORMAT";
        case osg::Texture::USE_USER_DEFINED_FORMAT: return "USE_USER_DEFINED_FORMAT";
        case osg::Texture::USE_ARB_COMPRESSION: return "USE_ARB_COMPRESSION";
        case osg::Texture::USE_S3TC_DXT1_COMPRESSION: return "USE_S3TC_DXT1_COMPRESSION";
        case osg::Texture::USE_S3TC_DXT3_COMPRESSION: return "USE_S3TC_DXT3_COMPRESSION";
        case osg::Texture::USE_S3TC_DXT5_COMPRESSION: return "USE_S3TC_DXT5_COMPRESSION";
        case osg::Texture::USE_PVRTC_2BPP_COMPRESSION: return "USE_PVRTC_2BPP_COMPRESSION";
        case osg::Texture::USE_PVRTC_4BPP_COMPRESSION: return "USE_PVRTC_4BPP_COMPRESSION";
        case osg::Texture::USE_ETC_COMPRESSION: return "USE_ETC_COMPRESSION";
        case osg::Texture::USE_ETC2_COMPRESSION: return "USE_ETC2_COMPRESSION";
        case osg::Texture::USE_RGTC1_COMPRESSION: return "USE_RGTC1_COMPRESSION";
        case osg::Texture::USE_RGTC2_COMPRESSION: return "USE_RGTC2_COMPRESSION";
        case osg::Texture::USE_S3TC_DXT1c_COMPRESSION: return "USE_S3TC_DXT1c_COMPRESSION";
        case osg::Texture::USE_S3TC_DXT1a_COMPRESSION: return "USE_S3TC_DXT1a_COMPRESSION";
        default: return "unknown internal format mode";

    }
}

std::string osg::OSGTreeAnalyser::translateTextureFormatType(int type) {
    switch(type){
        //! default OpenGL format (clamped values to [0,1) or [0,255])
        case osg::Texture::NORMALIZED: return "NORMALIZED";
        
                //! float values, Shader Model 3.0 (see ARB_texture_float)
        case osg::Texture::FLOAT:return "FLOAT";
        
                //! Signed integer values (see EXT_texture_integer)
        case osg::Texture::SIGNED_INTEGER:return "SIGNED_INTEGER";
        
                //! Unsigned integer value (see EXT_texture_integer)
        case osg::Texture::UNSIGNED_INTEGER:return "UNSIGNED_INTEGER";

        default: return "unknown texture format type";
    }
}


// TODO - vyhazet zbytecne veci..
std::string osg::OSGTreeAnalyser::translateTextureFormat(int format) {




  //  qDebug() << QString::fromStdString(int_to_hex(format));

    //probably unneded stuff here somewhere still.. + Not sure which osg uses and how..
    switch (format) {

    //viz computeInternalFormatWithImage from osg/texture.cpp   -- I suspect that osg is being weird here
    case 0 : return "no internal format";
    case 1 : return "GL_LUMINANCE";
    case 2 : return "GL_LUMINANCE_ALPHA";
    case 3 : return "GL_RGB";
    case 4 : return "GL_RGBA";

/*
case GL_TEXTURE_2D : return "GL_TEXTURE_2D";
case GL_TEXTURE_CUBE_MAP_POSITIVE_X : return "GL_TEXTURE_CUBE_MAP_POSITIVE_X";
case GL_TEXTURE_CUBE_MAP_NEGATIVE_X : return "GL_TEXTURE_CUBE_MAP_NEGATIVE_X";
case GL_TEXTURE_CUBE_MAP_POSITIVE_Y : return "GL_TEXTURE_CUBE_MAP_POSITIVE_Y";
case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y : return "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y";
case GL_TEXTURE_CUBE_MAP_POSITIVE_Z : return "GL_TEXTURE_CUBE_MAP_POSITIVE_Z";
case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z : return "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z";
*/
case GL_RED : return "GL_RED";
//case GL_RED_INTEGER : return "GL_RED_INTEGER";

case GL_RGB : return "GL_RGB";
//case GL_RGB_INTEGER : return "GL_RGB_INTEGER";
case GL_RGBA : return "GL_RGBA";
//case GL_RGBA_INTEGER : return "GL_RGBA_INTEGER";
case GL_DEPTH_COMPONENT : return "GL_DEPTH_COMPONENT";
//case GL_DEPTH_STENCIL : return "GL_DEPTH_STENCIL";
case GL_LUMINANCE_ALPHA : return "GL_LUMINANCE_ALPHA";
case GL_LUMINANCE : return "GL_LUMINANCE";
case GL_ALPHA : return "GL_ALPHA";



 /* 
case GL_UNSIGNED_BYTE: return "GL_UNSIGNED_BYTE";
case GL_BYTE: return "GL_BYTE";
case GL_UNSIGNED_SHORT: return "GL_UNSIGNED_SHORT";
case GL_SHORT: return "GL_SHORT";
case GL_UNSIGNED_INT: return "GL_UNSIGNED_INT";
case GL_INT: return "GL_INT";
case GL_HALF_FLOAT: return "GL_HALF_FLOAT";
case GL_FLOAT: return "GL_FLOAT";
case GL_UNSIGNED_SHORT_5_6_5: return "GL_UNSIGNED_SHORT_5_6_5";
//case GL_UNSIGNED_SHORT_4_4_4_4: return "GL_UNSIGNED_SHORT_4_4_4_4";
case GL_UNSIGNED_SHORT_5_5_5_1: return "GL_UNSIGNED_SHORT_5_5_5_1";
case GL_UNSIGNED_INT_2_10_10_10_REV: return "GL_UNSIGNED_INT_2_10_10_10_REV";
//case GL_UNSIGNED_INT_10F_11F_11F_REV: return "GL_UNSIGNED_INT_10F_11F_11F_REV";
//case GL_UNSIGNED_INT_5_9_9_9_REV: return "GL_UNSIGNED_INT_5_9_9_9_REV";
//case GL_UNSIGNED_INT_24_8: return "GL_UNSIGNED_INT_24_8";
//case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: return "GL_FLOAT_32_UNSIGNED_INT_24_8_REV";

*/


    /*
case GL_TEXTURE_3D: return "GL_TEXTURE_3D";



case GL_TEXTURE_2D_ARRAY_EXT: return "GL_TEXTURE_2D_ARRAY_EXT";
case GL_PROXY_TEXTURE_2D_ARRAY_EXT: return "GL_PROXY_TEXTURE_2D_ARRAY_EXT";
case GL_TEXTURE_BINDING_2D_ARRAY_EXT: return "GL_TEXTURE_BINDING_2D_ARRAY_EXT";
case GL_MAX_ARRAY_TEXTURE_LAYERS_EXT: return "GL_MAX_ARRAY_TEXTURE_LAYERS_EXT";
case GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT: return "GL_COMPARE_REF_DEPTH_TO_TEXTURE_EXT";
case GL_SAMPLER_2D_ARRAY_EXT: return "GL_SAMPLER_2D_ARRAY_EXT";
case GL_SAMPLER_2D_ARRAY_SHADOW_EXT: return "GL_SAMPLER_2D_ARRAY_SHADOW_EXT";
case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT: return "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT";



case GL_TEXTURE_CUBE_MAP: return "GL_TEXTURE_CUBE_MAP";
case GL_TEXTURE_BINDING_CUBE_MAP: return "GL_TEXTURE_BINDING_CUBE_MAP";
case GL_TEXTURE_CUBE_MAP_POSITIVE_X: return "GL_TEXTURE_CUBE_MAP_POSITIVE_X";
case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: return "GL_TEXTURE_CUBE_MAP_NEGATIVE_X";
case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: return "GL_TEXTURE_CUBE_MAP_POSITIVE_Y";
case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: return "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y";
case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: return "GL_TEXTURE_CUBE_MAP_POSITIVE_Z";
case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: return "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z";
case GL_PROXY_TEXTURE_CUBE_MAP: return "GL_PROXY_TEXTURE_CUBE_MAP";
case GL_MAX_CUBE_MAP_TEXTURE_SIZE: return "GL_MAX_CUBE_MAP_TEXTURE_SIZE";



case GL_TEXTURE_BINDING_3D: return "GL_TEXTURE_BINDING_3D";




case GL_MAX_TEXTURE_UNITS: return "GL_MAX_TEXTURE_UNITS";



case GL_TEXTURE_DEPTH: return "GL_TEXTURE_DEPTH";



case GL_TEXTURE_2D_MULTISAMPLE: return "GL_TEXTURE_2D_MULTISAMPLE";

*/






    case GL_COMPRESSED_ALPHA_ARB: return "GL_COMPRESSED_ALPHA_ARB";
    case GL_COMPRESSED_LUMINANCE_ARB: return "GL_COMPRESSED_LUMINANCE_ARB";
    case GL_COMPRESSED_LUMINANCE_ALPHA_ARB: return "GL_COMPRESSED_LUMINANCE_ALPHA_ARB";
    case GL_COMPRESSED_INTENSITY_ARB: return "GL_COMPRESSED_INTENSITY_ARB";
    case GL_COMPRESSED_RGB_ARB: return "GL_COMPRESSED_RGB_ARB";
    case GL_COMPRESSED_RGBA_ARB: return "GL_COMPRESSED_RGBA_ARB";
    case GL_TEXTURE_COMPRESSION_HINT_ARB: return "GL_TEXTURE_COMPRESSION_HINT_ARB";
    case GL_TEXTURE_COMPRESSED_ARB: return "GL_TEXTURE_COMPRESSED_ARB";
    case GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB: return "GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB";
    case GL_COMPRESSED_TEXTURE_FORMATS_ARB: return "GL_COMPRESSED_TEXTURE_FORMATS_ARB";



    case GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB: return "GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB";



    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: return "GL_COMPRESSED_RGB_S3TC_DXT1_EXT";
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT";
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: return "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT";
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: return "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT";



    case GL_COMPRESSED_RED_RGTC1_EXT: return "GL_COMPRESSED_RED_RGTC1_EXT";
    case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT: return "GL_COMPRESSED_SIGNED_RED_RGTC1_EXT";
    case GL_COMPRESSED_RED_GREEN_RGTC2_EXT: return "GL_COMPRESSED_RED_GREEN_RGTC2_EXT";
    case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT: return "GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT";



    case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG: return "GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG";
    case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG: return "GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG";
    case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG: return "GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG";
    case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG: return "GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG";



    case GL_ETC1_RGB8_OES: return "GL_ETC1_RGB8_OES";


    case GL_RGBA32F_ARB: return "GL_RGBA32F_ARB";
    case GL_RGB32F_ARB: return "GL_RGB32F_ARB";
    case GL_ALPHA32F_ARB: return "GL_ALPHA32F_ARB";
    case GL_INTENSITY32F_ARB: return "GL_INTENSITY32F_ARB";
    case GL_LUMINANCE32F_ARB: return "GL_LUMINANCE32F_ARB";
    case GL_LUMINANCE_ALPHA32F_ARB: return "GL_LUMINANCE_ALPHA32F_ARB";
    case GL_RGBA16F_ARB: return "GL_RGBA16F_ARB";
    case GL_RGB16F_ARB: return "GL_RGB16F_ARB";
    case GL_ALPHA16F_ARB: return "GL_ALPHA16F_ARB";
    case GL_INTENSITY16F_ARB: return "GL_INTENSITY16F_ARB";
    case GL_LUMINANCE16F_ARB: return "GL_LUMINANCE16F_ARB";
    case GL_LUMINANCE_ALPHA16F_ARB: return "GL_LUMINANCE_ALPHA16F_ARB";
        
         //collides with above block ^^ 

    //case GL_RGBA_FLOAT32_ATI: return "GL_RGBA_FLOAT32_ATI";
    //case GL_RGB_FLOAT32_ATI: return "GL_RGB_FLOAT32_ATI";
    //case GL_ALPHA_FLOAT32_ATI: return "GL_ALPHA_FLOAT32_ATI";
    //case GL_INTENSITY_FLOAT32_ATI: return "GL_INTENSITY_FLOAT32_ATI";
    //case GL_LUMINANCE_FLOAT32_ATI: return "GL_LUMINANCE_FLOAT32_ATI";
    //case GL_LUMINANCE_ALPHA_FLOAT32_ATI: return "GL_LUMINANCE_ALPHA_FLOAT32_ATI";
    //case GL_RGBA_FLOAT16_ATI: return "GL_RGBA_FLOAT16_ATI";
    //case GL_RGB_FLOAT16_ATI: return "GL_RGB_FLOAT16_ATI";
    //case GL_ALPHA_FLOAT16_ATI: return "GL_ALPHA_FLOAT16_ATI";
    //case GL_INTENSITY_FLOAT16_ATI: return "GL_INTENSITY_FLOAT16_ATI";
    //case GL_LUMINANCE_FLOAT16_ATI: return "GL_LUMINANCE_FLOAT16_ATI";
    //case GL_LUMINANCE_ALPHA_FLOAT16_ATI: return "GL_LUMINANCE_ALPHA_FLOAT16_ATI";

    


    case GL_HILO_NV: return "GL_HILO_NV";
    case GL_DSDT_NV: return "GL_DSDT_NV";
    case GL_DSDT_MAG_NV: return "GL_DSDT_MAG_NV";
    case GL_DSDT_MAG_VIB_NV: return "GL_DSDT_MAG_VIB_NV";
    case GL_HILO16_NV: return "GL_HILO16_NV";
    case GL_SIGNED_HILO_NV: return "GL_SIGNED_HILO_NV";
    case GL_SIGNED_HILO16_NV: return "GL_SIGNED_HILO16_NV";
    case GL_SIGNED_RGBA_NV: return "GL_SIGNED_RGBA_NV";
    case GL_SIGNED_RGBA8_NV: return "GL_SIGNED_RGBA8_NV";
    case GL_SIGNED_RGB_NV: return "GL_SIGNED_RGB_NV";
    case GL_SIGNED_RGB8_NV: return "GL_SIGNED_RGB8_NV";
    case GL_SIGNED_LUMINANCE_NV: return "GL_SIGNED_LUMINANCE_NV";
    case GL_SIGNED_LUMINANCE8_NV: return "GL_SIGNED_LUMINANCE8_NV";
    case GL_SIGNED_LUMINANCE_ALPHA_NV: return "GL_SIGNED_LUMINANCE_ALPHA_NV";
    case GL_SIGNED_LUMINANCE8_ALPHA8_NV: return "GL_SIGNED_LUMINANCE8_ALPHA8_NV";
    case GL_SIGNED_ALPHA_NV: return "GL_SIGNED_ALPHA_NV";
    case GL_SIGNED_ALPHA8_NV: return "GL_SIGNED_ALPHA8_NV";
    case GL_SIGNED_INTENSITY_NV: return "GL_SIGNED_INTENSITY_NV";
    case GL_SIGNED_INTENSITY8_NV: return "GL_SIGNED_INTENSITY8_NV";
    case GL_DSDT8_NV: return "GL_DSDT8_NV";
    case GL_DSDT8_MAG8_NV: return "GL_DSDT8_MAG8_NV";
    case GL_DSDT8_MAG8_INTENSITY8_NV: return "GL_DSDT8_MAG8_INTENSITY8_NV";
    case GL_SIGNED_RGB_UNSIGNED_ALPHA_NV: return "GL_SIGNED_RGB_UNSIGNED_ALPHA_NV";
    case GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV: return "GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV";



    case GL_FLOAT_R_NV: return "GL_FLOAT_R_NV";
    case GL_FLOAT_RG_NV: return "GL_FLOAT_RG_NV";
    case GL_FLOAT_RGB_NV: return "GL_FLOAT_RGB_NV";
    case GL_FLOAT_RGBA_NV: return "GL_FLOAT_RGBA_NV";
    case GL_FLOAT_R16_NV: return "GL_FLOAT_R16_NV";
    case GL_FLOAT_R32_NV: return "GL_FLOAT_R32_NV";
    case GL_FLOAT_RG16_NV: return "GL_FLOAT_RG16_NV";
    case GL_FLOAT_RG32_NV: return "GL_FLOAT_RG32_NV";
    case GL_FLOAT_RGB16_NV: return "GL_FLOAT_RGB16_NV";
    case GL_FLOAT_RGB32_NV: return "GL_FLOAT_RGB32_NV";
    case GL_FLOAT_RGBA16_NV: return "GL_FLOAT_RGBA16_NV";
    case GL_FLOAT_RGBA32_NV: return "GL_FLOAT_RGBA32_NV";





    // OpenGL ES1 and ES2 doesn't provide GL_INTENSITY
    case GL_INTENSITY: return "GL_INTENSITY";




        // Integer texture extension as in http://www.opengl.org/registry/specs/EXT/texture_integer.txt

    case GL_RGBA32UI_EXT: return "GL_RGBA32UI_EXT";
    case GL_RGB32UI_EXT: return "GL_RGB32UI_EXT";
    case GL_ALPHA32UI_EXT: return "GL_ALPHA32UI_EXT";
    case GL_INTENSITY32UI_EXT: return "GL_INTENSITY32UI_EXT";
    case GL_LUMINANCE32UI_EXT: return "GL_LUMINANCE32UI_EXT";
    case GL_LUMINANCE_ALPHA32UI_EXT: return "GL_LUMINANCE_ALPHA32UI_EXT";

    case GL_RGBA16UI_EXT: return "GL_RGBA16UI_EXT";
    case GL_RGB16UI_EXT: return "GL_RGB16UI_EXT";
    case GL_ALPHA16UI_EXT: return "GL_ALPHA16UI_EXT";
    case GL_INTENSITY16UI_EXT: return "GL_INTENSITY16UI_EXT";
    case GL_LUMINANCE16UI_EXT: return "GL_LUMINANCE16UI_EXT";
    case GL_LUMINANCE_ALPHA16UI_EXT: return "GL_LUMINANCE_ALPHA16UI_EXT";

    case GL_RGBA8UI_EXT: return "GL_RGBA8UI_EXT";
    case GL_RGB8UI_EXT: return "GL_RGB8UI_EXT";
    case GL_ALPHA8UI_EXT: return "GL_ALPHA8UI_EXT";
    case GL_INTENSITY8UI_EXT: return "GL_INTENSITY8UI_EXT";
    case GL_LUMINANCE8UI_EXT: return "GL_LUMINANCE8UI_EXT";
    case GL_LUMINANCE_ALPHA8UI_EXT: return "GL_LUMINANCE_ALPHA8UI_EXT";

    case GL_RGBA32I_EXT: return "GL_RGBA32I_EXT";
    case GL_RGB32I_EXT: return "GL_RGB32I_EXT";
    case GL_ALPHA32I_EXT: return "GL_ALPHA32I_EXT";
    case GL_INTENSITY32I_EXT: return "GL_INTENSITY32I_EXT";
    case GL_LUMINANCE32I_EXT: return "GL_LUMINANCE32I_EXT";
    case GL_LUMINANCE_ALPHA32I_EXT: return "GL_LUMINANCE_ALPHA32I_EXT";

    case GL_RGBA16I_EXT: return "GL_RGBA16I_EXT";
    case GL_RGB16I_EXT: return "GL_RGB16I_EXT";
    case GL_ALPHA16I_EXT: return "GL_ALPHA16I_EXT";
    case GL_INTENSITY16I_EXT: return "GL_INTENSITY16I_EXT";
    case GL_LUMINANCE16I_EXT: return "GL_LUMINANCE16I_EXT";
    case GL_LUMINANCE_ALPHA16I_EXT: return "GL_LUMINANCE_ALPHA16I_EXT";

    case GL_RGBA8I_EXT: return "GL_RGBA8I_EXT";
    case GL_RGB8I_EXT: return "GL_RGB8I_EXT";
    case GL_ALPHA8I_EXT: return "GL_ALPHA8I_EXT";
    case GL_INTENSITY8I_EXT: return "GL_INTENSITY8I_EXT";
    case GL_LUMINANCE8I_EXT: return "GL_LUMINANCE8I_EXT";
    case GL_LUMINANCE_ALPHA8I_EXT: return "GL_LUMINANCE_ALPHA8I_EXT";

    case GL_RED_INTEGER_EXT: return "GL_RED_INTEGER_EXT";
    case GL_GREEN_INTEGER_EXT: return "GL_GREEN_INTEGER_EXT";
    case GL_BLUE_INTEGER_EXT: return "GL_BLUE_INTEGER_EXT";
    case GL_ALPHA_INTEGER_EXT: return "GL_ALPHA_INTEGER_EXT";
    case GL_RGB_INTEGER_EXT: return "GL_RGB_INTEGER_EXT";
    case GL_RGBA_INTEGER_EXT: return "GL_RGBA_INTEGER_EXT";
    case GL_BGR_INTEGER_EXT: return "GL_BGR_INTEGER_EXT";
    case GL_BGRA_INTEGER_EXT: return "GL_BGRA_INTEGER_EXT";
    case GL_LUMINANCE_INTEGER_EXT: return "GL_LUMINANCE_INTEGER_EXT";
    case GL_LUMINANCE_ALPHA_INTEGER_EXT: return "GL_LUMINANCE_ALPHA_INTEGER_EXT";

    case GL_RGBA_INTEGER_MODE_EXT: return "GL_RGBA_INTEGER_MODE_EXT";



    case GL_RG: return "GL_RG";
    case GL_RG_INTEGER: return "GL_RG_INTEGER";
    case GL_R8: return "GL_R8";
    case GL_R16: return "GL_R16";
    case GL_RG8: return "GL_RG8";
    case GL_RG16: return "GL_RG16";
    case GL_R16F: return "GL_R16F";
    case GL_R32F: return "GL_R32F";
    case GL_RG16F: return "GL_RG16F";
    case GL_RG32F: return "GL_RG32F";
    case GL_R8I: return "GL_R8I";
    case GL_R8UI: return "GL_R8UI";
    case GL_R16I: return "GL_R16I";
    case GL_R16UI: return "GL_R16UI";
    case GL_R32I: return "GL_R32I";
    case GL_R32UI: return "GL_R32UI";
    case GL_RG8I: return "GL_RG8I";
    case GL_RG8UI: return "GL_RG8UI";
    case GL_RG16I: return "GL_RG16I";
    case GL_RG16UI: return "GL_RG16UI";
    case GL_RG32I: return "GL_RG32I";
    case GL_RG32UI: return "GL_RG32UI";
    default: return"unknown internal format";




    }
}



std::string osg::OSGTreeAnalyser::translateCullMode(int value) {


    switch (value) {
        case osg::CullSettings::NO_CULLING : return "NO_CULLING";
        case osg::CullSettings::VIEW_FRUSTUM_SIDES_CULLING : return "VIEW_FRUSTUM_SIDES_CULLING";
        case osg::CullSettings::NEAR_PLANE_CULLING : return "NEAR_PLANE_CULLING";
        case osg::CullSettings::FAR_PLANE_CULLING : return "FAR_PLANE_CULLING";
        case osg::CullSettings::VIEW_FRUSTUM_CULLING : return "VIEW_FRUSTUM_CULLING";
        case osg::CullSettings::SMALL_FEATURE_CULLING : return "SMALL_FEATURE_CULLING";
        case osg::CullSettings::SHADOW_OCCLUSION_CULLING : return "SHADOW_OCCLUSION_CULLING";
        case osg::CullSettings::CLUSTER_CULLING : return "CLUSTER_CULLING";
        case osg::CullSettings::DEFAULT_CULLING : return "DEFAULT_CULLING";
        case osg::CullSettings::ENABLE_ALL_CULLING : return "ENABLE_ALL_CULLING";
        default: return "unknown cull settings";
    }



}


std::string osg::OSGTreeAnalyser::translateRenderOrder(int value) {
       
    switch (value) {
        case osg::Camera::RenderOrder::PRE_RENDER : return "PRE_RENDER";
        case osg::Camera::RenderOrder::NESTED_RENDER : return "NESTED_RENDER";
        case osg::Camera::RenderOrder::POST_RENDER : return "POST_RENDER";
        default : return "unknown render order";
    }
}

std::string osg::OSGTreeAnalyser::translateTransformOrder(int value) {

    switch (value) {
        case osg::Camera::TransformOrder::PRE_MULTIPLY: return "PRE_MULTIPLY";
        case osg::Camera::TransformOrder::POST_MULTIPLY: return "POST_MULTIPLY";
        default: return "unknown transform order";
    }
}

std::string osg::OSGTreeAnalyser::translateBufferComponent(int value) {
    switch (value) {

        case osg::Camera::BufferComponent::DEPTH_BUFFER: return "DEPTH_BUFFER";
        case osg::Camera::BufferComponent::STENCIL_BUFFER: return "STENCIL_BUFFER";
        case osg::Camera::BufferComponent::PACKED_DEPTH_STENCIL_BUFFER: return "PACKED_DEPTH_STENCIL_BUFFER";
        case osg::Camera::BufferComponent::COLOR_BUFFER: return "COLOR_BUFFER";
        case osg::Camera::BufferComponent::COLOR_BUFFER0: return "COLOR_BUFFER0";
        case osg::Camera::BufferComponent::COLOR_BUFFER1: return "COLOR_BUFFER1";
        case osg::Camera::BufferComponent::COLOR_BUFFER2: return "COLOR_BUFFER2";
        case osg::Camera::BufferComponent::COLOR_BUFFER3: return "COLOR_BUFFER3";
        case osg::Camera::BufferComponent::COLOR_BUFFER4: return "COLOR_BUFFER4";
        case osg::Camera::BufferComponent::COLOR_BUFFER5: return "COLOR_BUFFER5";
        case osg::Camera::BufferComponent::COLOR_BUFFER6: return "COLOR_BUFFER6";
        case osg::Camera::BufferComponent::COLOR_BUFFER7: return "COLOR_BUFFER7";
        case osg::Camera::BufferComponent::COLOR_BUFFER8: return "COLOR_BUFFER8";
        case osg::Camera::BufferComponent::COLOR_BUFFER9: return "COLOR_BUFFER9";
        case osg::Camera::BufferComponent::COLOR_BUFFER10: return "COLOR_BUFFER10";
        case osg::Camera::BufferComponent::COLOR_BUFFER11: return "COLOR_BUFFER11";
        case osg::Camera::BufferComponent::COLOR_BUFFER12: return "COLOR_BUFFER12";
        case osg::Camera::BufferComponent::COLOR_BUFFER13: return "COLOR_BUFFER13";
        case osg::Camera::BufferComponent::COLOR_BUFFER14: return "COLOR_BUFFER14";
        case osg::Camera::BufferComponent::COLOR_BUFFER15: return "COLOR_BUFFER15";

        default: return "unknown buffer component";
    }
}


/*
ProjectionResizePolicy
RenderTargetImplementation
Attachment
typedef std::map< BufferComponent, Attachment> BufferAttachmentMap;

*/

/*
enum ShadowCompareFunc {
    NEVER = GL_NEVER,
    LESS = GL_LESS,
    EQUAL = GL_EQUAL,
    LEQUAL = GL_LEQUAL,
    GREATER = GL_GREATER,
    NOTEQUAL = GL_NOTEQUAL,
    GEQUAL = GL_GEQUAL,
    ALWAYS = GL_ALWAYS
};

enum ShadowTextureMode {
    LUMINANCE = GL_LUMINANCE,
    INTENSITY = GL_INTENSITY,
    ALPHA = GL_ALPHA
};


*/