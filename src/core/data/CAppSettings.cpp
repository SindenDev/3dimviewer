//////////////////////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////////////////////

#include <3dim/core/data/CAppSettings.h>

#ifdef _WIN32
    #include <Shlobj.h>
#endif

//#define __APPLE__
#ifdef __APPLE__
    #ifdef _WIN32 // for testing only
        #include <io.h> 
        #define access _access
        #define R_OK 4
        #define F_OK    0
    #else
        #include <unistd.h> // 
    #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Default constructor. 
////////////////////////////////////////////////////////////////////////////////////////////////////
data::CAppSettings::CAppSettings()
    : m_clearColor( 0.0, 0.0, 0.0, 1.0 ),
      m_filter(NoFilter),
      m_arialPath("fonts/arial.ttf"),
      m_bNPOTTextures(true)
{
#ifdef _WIN32
    char szPath[MAX_PATH*2]={};
    SHGetFolderPathA(NULL, CSIDL_WINDOWS, NULL, 0, szPath);
    if (0!=szPath[0])
    {
        m_arialPath = szPath;
        m_arialPath+="/fonts/arialuni.ttf";
        DWORD attr = GetFileAttributesA(m_arialPath.c_str());
        if (INVALID_FILE_ATTRIBUTES==attr)
        {
            m_arialPath = szPath;
            m_arialPath+="/fonts/arial.ttf";
            attr = GetFileAttributesA(m_arialPath.c_str());
            if (INVALID_FILE_ATTRIBUTES==attr)
                m_arialPath="fonts/arial.ttf";
        }
    }
#endif
#ifdef __APPLE__
    m_arialPath.clear();
#define TESTFONTPATH(x) if (m_arialPath.empty()) { int res = access(x, R_OK); if (0==res) m_arialPath = x; }
    // search for unicode arial
    TESTFONTPATH("/System/Library/Fonts/Arial Unicode.ttf");
    TESTFONTPATH("/Library/Fonts/Arial Unicode.ttf");
    TESTFONTPATH("~/Library/Fonts/Arial Unicode.ttf");
    // search for ordinary arial
    TESTFONTPATH("/System/Library/Fonts/Arial.ttf");
    TESTFONTPATH("/Library/Fonts/Arial.ttf");
    TESTFONTPATH("~/Library/Fonts/Arial.ttf");
    // if not found then use the application provided one
    if (m_arialPath.empty())
        m_arialPath="fonts/arial.ttf";
#endif

    // Define default clear color
    m_clearColor = osg::Vec4f( 0.0, 0.0, 0.0, 1.0 );
    //m_clearColor = osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Initializes the default state. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void data::CAppSettings::init()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   Regenerate object according to the data storage changes. 
//!
//!\param   Changes The changes. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void data::CAppSettings::update( const data::CChangedEntries& Changes )
{
    // Do nothing for now
}