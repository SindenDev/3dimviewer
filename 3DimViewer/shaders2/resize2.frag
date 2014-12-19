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

#ifndef __GLSL_CG_DATA_TYPES 
#define half4 vec4 
#define half3 vec3 
#define half2 vec2 
#define half float 
#define saturate(type) clamp(type,0.0,1.0) 
#endif 
 
uniform sampler2D outdepth; 
uniform sampler2D image; 
uniform sampler1D kernel; 
 
uniform vec2 resolution; 
 
void main() 
{ 
    vec2 offset = 1.0 / resolution; 
    vec2 position = gl_TexCoord[0].st * resolution; 
    vec2 sub_pos  = fract(position); 
     
    if (sub_pos.x >= 0.5) 
        sub_pos.x -= 0.5; 
    else 
        sub_pos.x += 0.5; 
    if (sub_pos.y >= 0.5) 
        sub_pos.y -= 0.5; 
    else 
        sub_pos.y += 0.5; 
     
    float XO1 = -sub_pos.x*offset.x-offset.x; 
    float XO2 = -sub_pos.x*offset.x; 
    float XO3 = (1.0-sub_pos.x)*offset.x; 
    float XO4 = (1.0-sub_pos.x)*offset.x+offset.x; 
     
    float Kw1 = texture1D(kernel,     (sub_pos.x+1.0) *0.5).r; 
    float Kw2 = texture1D(kernel,      sub_pos.x      *0.5).r; 
    float Kw3 = texture1D(kernel, (1.0-sub_pos.x)     *0.5).r; 
    float Kw4 = texture1D(kernel,((1.0-sub_pos.x)+1.0)*0.5).r; 
     
    float SW = Kw1 + Kw2 + Kw3 + Kw4; 
    if( abs(SW) < 0.001 )
    {
        SW = 0.001; 
    }
    
    float YO1 = -sub_pos.y*offset.y-offset.y; 
    vec3 FL1  = texture2D(image,gl_TexCoord[0].st+vec2(XO1,YO1)).rgb; 
    vec3 NL1  = texture2D(image,gl_TexCoord[0].st+vec2(XO2,YO1)).rgb; 
    vec3 NR1  = texture2D(image,gl_TexCoord[0].st+vec2(XO3,YO1)).rgb; 
    vec3 FR1  = texture2D(image,gl_TexCoord[0].st+vec2(XO4,YO1)).rgb; 
    vec3 C1 = (FL1 * Kw1 + 
              NL1 * Kw2 + 
              NR1 * Kw3 + 
              FR1 * Kw4) / SW; 
               
    float YO2 = -sub_pos.y*offset.y; 
    vec3 FL2  = texture2D(image,gl_TexCoord[0].st+vec2(XO1,YO2)).rgb; 
    vec3 NL2  = texture2D(image,gl_TexCoord[0].st+vec2(XO2,YO2)).rgb; 
    vec3 NR2  = texture2D(image,gl_TexCoord[0].st+vec2(XO3,YO2)).rgb; 
    vec3 FR2  = texture2D(image,gl_TexCoord[0].st+vec2(XO4,YO2)).rgb; 
    vec3 C2 = (FL2 * Kw1 + 
              NL2 * Kw2 + 
              NR2 * Kw3 + 
              FR2 * Kw4) / SW; 
     
    float YO3 = (1.0-sub_pos.y)*offset.y; 
    vec3 FL3  = texture2D(image,gl_TexCoord[0].st+vec2(XO1,YO3)).rgb; 
    vec3 NL3  = texture2D(image,gl_TexCoord[0].st+vec2(XO2,YO3)).rgb; 
    vec3 NR3  = texture2D(image,gl_TexCoord[0].st+vec2(XO3,YO3)).rgb; 
    vec3 FR3  = texture2D(image,gl_TexCoord[0].st+vec2(XO4,YO3)).rgb; 
    vec3 C3 = (FL3 * Kw1 + 
              NL3 * Kw2 + 
              NR3 * Kw3 + 
              FR3 * Kw4) / SW; 
               
    float YO4 = (1.0-sub_pos.y)*offset.y+offset.y; 
    vec3 FL4  = texture2D(image,gl_TexCoord[0].st+vec2(XO1,YO4)).rgb; 
    vec3 NL4  = texture2D(image,gl_TexCoord[0].st+vec2(XO2,YO4)).rgb; 
    vec3 NR4  = texture2D(image,gl_TexCoord[0].st+vec2(XO3,YO4)).rgb; 
    vec3 FR4  = texture2D(image,gl_TexCoord[0].st+vec2(XO4,YO4)).rgb; 
    vec3 C4 = (FL4 * Kw1 + 
              NL4 * Kw2 + 
              NR4 * Kw3 + 
              FR4 * Kw4) / SW; 
     
    float Kw5 = texture1D(kernel,     (sub_pos.y+1.0) *0.5).r; 
    float Kw6 = texture1D(kernel,      sub_pos.y      *0.5).r; 
    float Kw7 = texture1D(kernel, (1.0-sub_pos.y)     *0.5).r; 
    float Kw8 = texture1D(kernel,((1.0-sub_pos.y)+1.0)*0.5).r; 
     
    SW = Kw5 + Kw6 + Kw7 + Kw8; 
     
    vec3 C = (C1 * Kw5+ 
             C2 * Kw6+ 
             C3 * Kw7+ 
             C4 * Kw8) / SW; 
     
    gl_FragColor = vec4(C, texture2D(outdepth,gl_TexCoord[0].st).g); 
    gl_FragDepth = saturate(texture2D(outdepth,gl_TexCoord[0].st).r); 
} 
 
