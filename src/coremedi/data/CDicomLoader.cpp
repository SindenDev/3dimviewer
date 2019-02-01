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

#include <data/CDicomLoader.h>
#include <data/CDicom.h>
#include <data/CSeries.h>

// VPL
#include <VPL/Base/ScopedPtr.h>
#include <VPL/Image/Vector3.h>
#include <VPL/System/FileBrowser.h>


// STL
#include <deque>
#include <cctype>
#include <locale>
#ifdef _WIN32
#include <codecvt>
#endif
#if !defined(TRIDIM_USE_GDCM)

#include <data/DicomTagUtils.h>

#endif


namespace data
{

//==============================================================================================
CDicomLoader::CDicomLoader()
{
    m_bAllowAnyExtension = true;
    m_bAllowNoExtension = false;
    m_bAllowNumExtension = false;
	/*
	#ifdef _WIN32
    addDicomExtension( L"dicom" );
    addDicomExtension( L"dcm" );
	#else
    addDicomExtension( "dicom" );
    addDicomExtension( "dcm" );
	#endif
	*/
}

//==============================================================================================
CDicomLoader::~CDicomLoader()
{
}

std::string getenvSafe(const char* var)
{
    std::string res;
    char* v = getenv(var);
    if (nullptr != v)
        res = v;
    return res;
}

//==============================================================================================
CSeries * CDicomLoader::preLoadDirectory( const vpl::sys::tString & path )
{
    // Initialize the progress
    setProgressMax(0);
    beginProgress();

    vpl::sys::CFileBrowserU::SFileAttr files;
    vpl::sys::CFileBrowserU browser;

    // get system dir
    static std::string sysDir = getenvSafe("SystemRoot");
    vpl::sys::tString systemDir = vpl::sys::tStringConv::fromUtf8(sysDir);
    vpl::sys::tString systemDirFw(systemDir);
    std::replace(systemDirFw.begin(), systemDirFw.end(), vplT('\\'), vplT('/'));

    // Remember the current working directory
    vpl::sys::tString oldDir = browser.getDirectory();

    // Initialize a queue of directories to process
    std::deque< vpl::sys::tString > directories;
    directories.push_back( path );

    // Create a new dicom series
    CSeries::tSmartPtr new_series;

    // Number of loaded dicom files
    int	total_dicoms = 0;
    bool bCanceled = false;
    while( !directories.empty() && !bCanceled )
    {
        if (!this->progress())
        {
            bCanceled = true;
            break;
        }

        // Get directory from the queue
        vpl::sys::tString current = directories.front();
        directories.pop_front();

        // skip system dir
        if (0 == current.compare(systemDir) || 0 == current.compare(systemDirFw))
        {
            continue;
        }

        // Change the directory
        vpl::sys::CFileBrowserU browserX; // use new instance of browser, because CFileBrowserU doesn't work ok for multiple findFirst calls (handle leak)
        browserX.setDirectory( current );

        // Check if the file DICOMDIR exists in the directory
        // - This part of the code is not yet finished!
        // - Hence, instead of the DICOMDIR, found DICOM files are parsed directly!
/*        {
            // Load the DICOMDIR
            preLoadDicomDir(current, new_series.get());

            // Continue with the next directory
            continue;
        }*/

        // Browse all files in the directory
		vpl::sys::tString StrStar = vplT("*");
		vpl::sys::tString StrDot = vplT(".");
		vpl::sys::tString StrDotDot = vplT("..");
        vpl::sys::tChar CharSlash = vplT('/');
		vpl::sys::tChar CharDot = vplT('.');

		bool bFound = browserX.findFirst(StrStar, files);
        for( ; bFound; bFound = browserX.findNext(files) )
        {
            // If a directory was found
            if( files.m_bDirectory )
            {
                if( files.m_sName != StrDot && files.m_sName != StrDotDot )
                {
                    if (!current.empty() && current[current.size()-1] == CharSlash)
                        directories.push_back(current + files.m_sName);
                    else
                        directories.push_back( current + CharSlash + files.m_sName );
                }
            }
            else
            {
                // Verify the file extension
                if( !m_bAllowAnyExtension )
                {
                    std::size_t ext_start = files.m_sName.find_last_of( CharDot );
                    if ( ext_start != vpl::sys::tString::npos )
                    {
                        vpl::sys::tString ext = files.m_sName.substr( ext_start + 1 );
                        if ( !isDicomExtension( ext ) )
                        {
                            continue;
                        }
                    }
                    else if ( !m_bAllowNoExtension )
                    {
                        continue;
                    }
                }

				// Preload the file - provide directory path and filename separately if possible (CPath would be nice)
				if ( new_series->addDicomFile(current, files.m_sName) )
                {
                    total_dicoms++;
                    if ( total_dicoms % 20 == 0 && !this->progress() )
                    {
                        bCanceled = true;
                        break;
                    }
                }
            }
        }
    }

    // Change the working directory back
    browser.setDirectory( oldDir );

    // Finish the progress
    endProgress();

    if ( new_series->getNumSeries() > 0 ) 
    {
        return new_series.release();
    }
    else
    {
        return NULL;
    }
}

//==============================================================================================
CSeries * CDicomLoader::preLoadFile( const vpl::sys::tString & path )
{
    // Create a new dicom series
    CSeries::tSmartPtr new_series;

    {
        vpl::sys::tChar CharDot = vplT('.');
        // Verify the file extension
        if( !m_bAllowAnyExtension )
        {
            std::size_t ext_start = path.find_last_of( CharDot );
            if ( ext_start != vpl::sys::tString::npos )
            {
                vpl::sys::tString ext = path.substr( ext_start + 1 );
                if ( !isDicomExtension( ext ) )
                    return NULL;
            }
            else if ( !m_bAllowNoExtension )
            {
                return NULL;
            }
        }

	    // Preload the file - provide directory path and filename separately        
        vpl::sys::tChar CharBackSlash = vplT('\\');
        vpl::sys::tChar CharSlash = vplT('/');
        std::size_t fileNameStart1 = path.find_last_of( CharBackSlash );
        std::size_t fileNameStart2 = path.find_last_of( CharSlash );
        std::size_t fileNameStart = fileNameStart1;
        if (fileNameStart == vpl::sys::tString::npos)
            fileNameStart = fileNameStart2;
        else
            if (fileNameStart2!=vpl::sys::tString::npos)
                fileNameStart = std::max(fileNameStart,fileNameStart2);
        vpl::sys::tString dir = path.substr(0,fileNameStart);
	    if ( new_series->addDicomFile(dir, path) )
        {
        }
    }
    
    if ( new_series->getNumSeries() > 0 ) 
    {
        return new_series.release();
    }
    else
    {
        return NULL;
    }
}

//==============================================================================================
void CDicomLoader::allowNoExtension(bool bValue)
{
    m_bAllowNoExtension = bValue;
}

//==============================================================================================
void CDicomLoader::allowAnyExtension(bool bValue)
{
    m_bAllowAnyExtension = bValue;
}

//==============================================================================================
void CDicomLoader::allowNumExtension(bool bValue)
{
    m_bAllowNumExtension = bValue;
}

//==============================================================================================
void CDicomLoader::addDicomExtension( const vpl::sys::tString & extension )
{
    vpl::sys::tString extensionTemp(extension);
    std::transform( extensionTemp.begin(), extensionTemp.end(), extensionTemp.begin(), tolower );
    m_DicomExtensions.insert( extensionTemp );
}

//==============================================================================================
void CDicomLoader::removeDicomExtension( const vpl::sys::tString & extension )
{
    vpl::sys::tString extensionTemp(extension);
    std::transform( extensionTemp.begin(), extensionTemp.end(), extensionTemp.begin(), tolower );
    m_DicomExtensions.erase( extensionTemp );
}

//==============================================================================================

bool is_number(const vpl::sys::tString& s)
{
    vpl::sys::tString::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool CDicomLoader::isDicomExtension( const vpl::sys::tString & extension )
{
    vpl::sys::tString extensionTemp(extension);
    std::transform( extensionTemp.begin(), extensionTemp.end(), extensionTemp.begin(), tolower );    
    if (m_bAllowNumExtension && is_number(extensionTemp))
        return true;
    return ( m_DicomExtensions.find( extensionTemp ) != m_DicomExtensions.end() );
}

} // namespace data
