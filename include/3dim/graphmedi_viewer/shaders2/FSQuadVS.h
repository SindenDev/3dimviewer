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

#ifndef FSQuadVS_H
#define FSQuadVS_H

namespace shader
{

const char FSQuadVS[] =
    "void main() \n"
    "{ \n"
    "    // Transforming The Vertex \n"
    "    gl_Position = gl_Vertex; \n"
    "    gl_TexCoord[0] = gl_Vertex * 0.5 + 0.5; \n"
    "} \n";

} // namespace shader

#endif // FSQuadVS_H
