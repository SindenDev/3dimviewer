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

#ifndef _3DIMAPPVERSION_H
#define _3DIMAPPVERSION_H

#define VER_MAJOR               3
#define VER_MINOR               2
#define VER_BUILD               1

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define VER_FILEVERSION             VER_MAJOR,VER_MINOR,0,VER_BUILD
#define VER_FILEVERSION_STR         STR(VER_MAJOR) "." STR(VER_MINOR) ".0." STR(VER_BUILD) "\0"

#define VER_PRODUCTVERSION          VER_MAJOR,VER_MINOR
#define VER_PRODUCTVERSION_STR      STR(VER_MAJOR) "." STR(VER_MINOR) "\0"

#endif

