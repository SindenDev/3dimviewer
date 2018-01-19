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

#ifndef Resize_H
#define Resize_H

namespace shader
{

const char Resize[] =
"#ifndef __GLSL_CG_DATA_TYPES \n"
"#define half4 vec4 \n"
"#define half3 vec3 \n"
"#define half2 vec2 \n"
"#define half float \n"
"#define saturate(type) clamp(type,0.0,1.0) \n"
"#endif \n"
" \n"
"uniform sampler2D outdepth; \n"
"uniform sampler2D image; \n"
"uniform sampler1D kernel; \n"
" \n"
"uniform vec2 resolution; \n"
" \n"
"void main() \n"
"{ \n"
"    vec2 offset = 1.0 / resolution; \n"
"    vec2 position = gl_TexCoord[0].st * resolution; \n"
"    vec2 sub_pos  = fract(position); \n"
"     \n"
"    if (sub_pos.x >= 0.5) \n"
"        sub_pos.x -= 0.5; \n"
"    else \n"
"        sub_pos.x += 0.5; \n"
"    if (sub_pos.y >= 0.5) \n"
"        sub_pos.y -= 0.5; \n"
"    else \n"
"        sub_pos.y += 0.5; \n"
"     \n"
"    float XO1 = -sub_pos.x*offset.x-offset.x; \n"
"    float XO2 = -sub_pos.x*offset.x; \n"
"    float XO3 = (1.0-sub_pos.x)*offset.x; \n"
"    float XO4 = (1.0-sub_pos.x)*offset.x+offset.x; \n"
"     \n"
"    float Kw1 = texture1D(kernel,     (sub_pos.x+1.0) *0.5).r; \n"
"    float Kw2 = texture1D(kernel,      sub_pos.x      *0.5).r; \n"
"    float Kw3 = texture1D(kernel, (1.0-sub_pos.x)     *0.5).r; \n"
"    float Kw4 = texture1D(kernel,((1.0-sub_pos.x)+1.0)*0.5).r; \n"
"     \n"
"    float SW = Kw1 + Kw2 + Kw3 + Kw4; \n"
"     \n"
"    float YO1 = -sub_pos.y*offset.y-offset.y; \n"
"    vec3 FL1  = texture2D(image,gl_TexCoord[0].st+vec2(XO1,YO1)).rgb; \n"
"    vec3 NL1  = texture2D(image,gl_TexCoord[0].st+vec2(XO2,YO1)).rgb; \n"
"    vec3 NR1  = texture2D(image,gl_TexCoord[0].st+vec2(XO3,YO1)).rgb; \n"
"    vec3 FR1  = texture2D(image,gl_TexCoord[0].st+vec2(XO4,YO1)).rgb; \n"
"    vec3 C1 = (FL1 * Kw1 + \n"
"              NL1 * Kw2 + \n"
"              NR1 * Kw3 + \n"
"              FR1 * Kw4) / SW; \n"
"               \n"
"    float YO2 = -sub_pos.y*offset.y; \n"
"    vec3 FL2  = texture2D(image,gl_TexCoord[0].st+vec2(XO1,YO2)).rgb; \n"
"    vec3 NL2  = texture2D(image,gl_TexCoord[0].st+vec2(XO2,YO2)).rgb; \n"
"    vec3 NR2  = texture2D(image,gl_TexCoord[0].st+vec2(XO3,YO2)).rgb; \n"
"    vec3 FR2  = texture2D(image,gl_TexCoord[0].st+vec2(XO4,YO2)).rgb; \n"
"    vec3 C2 = (FL2 * Kw1 + \n"
"              NL2 * Kw2 + \n"
"              NR2 * Kw3 + \n"
"              FR2 * Kw4) / SW; \n"
"     \n"
"    float YO3 = (1.0-sub_pos.y)*offset.y; \n"
"    vec3 FL3  = texture2D(image,gl_TexCoord[0].st+vec2(XO1,YO3)).rgb; \n"
"    vec3 NL3  = texture2D(image,gl_TexCoord[0].st+vec2(XO2,YO3)).rgb; \n"
"    vec3 NR3  = texture2D(image,gl_TexCoord[0].st+vec2(XO3,YO3)).rgb; \n"
"    vec3 FR3  = texture2D(image,gl_TexCoord[0].st+vec2(XO4,YO3)).rgb; \n"
"    vec3 C3 = (FL3 * Kw1 + \n"
"              NL3 * Kw2 + \n"
"              NR3 * Kw3 + \n"
"              FR3 * Kw4) / SW; \n"
"               \n"
"    float YO4 = (1.0-sub_pos.y)*offset.y+offset.y; \n"
"    vec3 FL4  = texture2D(image,gl_TexCoord[0].st+vec2(XO1,YO4)).rgb; \n"
"    vec3 NL4  = texture2D(image,gl_TexCoord[0].st+vec2(XO2,YO4)).rgb; \n"
"    vec3 NR4  = texture2D(image,gl_TexCoord[0].st+vec2(XO3,YO4)).rgb; \n"
"    vec3 FR4  = texture2D(image,gl_TexCoord[0].st+vec2(XO4,YO4)).rgb; \n"
"    vec3 C4 = (FL4 * Kw1 + \n"
"              NL4 * Kw2 + \n"
"              NR4 * Kw3 + \n"
"              FR4 * Kw4) / SW; \n"
"     \n"
"    float Kw5 = texture1D(kernel,     (sub_pos.y+1.0) *0.5).r; \n"
"    float Kw6 = texture1D(kernel,      sub_pos.y      *0.5).r; \n"
"    float Kw7 = texture1D(kernel, (1.0-sub_pos.y)     *0.5).r; \n"
"    float Kw8 = texture1D(kernel,((1.0-sub_pos.y)+1.0)*0.5).r; \n"
"     \n"
"    SW = Kw5 + Kw6 + Kw7 + Kw8; \n"
"     \n"
"    vec3 C = (C1 * Kw5+ \n"
"             C2 * Kw6+ \n"
"             C3 * Kw7+ \n"
"             C4 * Kw8) / SW; \n"
"     \n"
"    gl_FragColor = vec4(C, texture2D(outdepth,gl_TexCoord[0].st).g); \n"
"    gl_FragDepth = saturate(texture2D(outdepth,gl_TexCoord[0].st).r);     \n"
"} \n";

} // namespace shader

#endif // Resize_H

