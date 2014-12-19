////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////

#ifndef DebuggingMacros_H
#define DebuggingMacros_H

////////////////////////////////////////////////////////////
// Include files

#ifdef _WIN32
#   include <windows.h>
#endif // _WIN32

//#include <wx/debug.h>

#include <cassert>
#include <sstream>
#include <iostream>


////////////////////////////////////////////////////////////
// Printing function name macro, through OutputDebugString

#ifdef NDEBUG
#   define _PRINT_FUNCTION_NAME
#else
#   ifdef _WIN32
#       define _PRINT_FUNCTION_NAME { OutputDebugString((LPCTSTR)__PRETTY_FUNCTION__); }
#   else
#       define _PRINT_FUNCTION_NAME
#   endif // _WIN32
#endif

////////////////////////////////////////////////////////////
// Printing debug information macro, through OutputDebugString

#ifdef NDEBUG
#   define _TRACE(text)
#else
#   ifdef _WIN32
#       define _TRACE(text) { std::stringstream stream; stream << text; OutputDebugStringA((LPCTSTR)stream.str().c_str()); }
#   else
#       define _TRACE(text)
#   endif // _WIN32
#endif

#ifdef NDEBUG
#   define _TRACEW(text)
#else
#   ifdef _WIN32
#       define _TRACEW(text) { std::wstringstream stream; stream << text; OutputDebugStringW((LPCTSTR)stream.str().c_str()); }
#   else
#       define _TRACEW(text)
#   endif // _WIN32
#endif

////////////////////////////////////////////////////////////
// Assert macro encapsulation, for generalization under wxWidgets

#ifndef _ASSERT
#   define _ASSERT(condition) assert(condition)
//#   define _ASSERT(condition) wxASSERT(condition)
#endif


#endif // DebuggingMacros_H

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

