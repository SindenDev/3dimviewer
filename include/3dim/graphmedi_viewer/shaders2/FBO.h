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

#ifndef FBO_H
#define FBO_H

namespace shader
{
const char OSRT[] =
"#version 330 core \n"
" \n"
"uniform mat4 osg_ModelViewMatrix; \n"
"uniform mat4 osg_ProjectionMatrix; \n"
" \n"
"in vec4 fragWorldPosition; \n"
" \n"
"out vec4 outPosition; \n"
" \n"
"void main() \n"
"{ \n"
"    vec4 wPos = fragWorldPosition; \n"
"    vec4 vPos = osg_ModelViewMatrix * wPos; \n"
"    vec4 pPos = osg_ProjectionMatrix * vPos; \n"
"    pPos /= pPos.w; \n"
" \n"
"    outPosition = vec4(pPos.xyz, 0.0); \n"
"} \n";

} // namespace shader

#endif // FBO_H

