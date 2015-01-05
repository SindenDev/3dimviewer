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

#include <data/CDicomLoader.h>
#include <data/DicomTagUtils.h>
#include <data/CDicom.h>

// VPL
#include <VPL/Base/ScopedPtr.h>
#include <VPL/Image/Vector3.h>
#include <VPL/System/FileBrowser.h>

// DCMTk
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dctk.h>
//#include <dcmtk/dcmdata/dcdebug.h>
#include <dcmtk/dcmdata/dcdicdir.h>
#include <dcmtk/dcmdata/cmdlnarg.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/ofstd/ofconapp.h>
#include <dcmtk/dcmjpeg/djdecode.h>
#include <dcmtk/dcmjpeg/dipijpeg.h>

// STL
#include <deque>
#include <cctype>

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

//==============================================================================================
CSeries * CDicomLoader::preLoadDirectory( const vpl::sys::tString & path )
{
    // Initialize the progress
    setProgressMax(0);
    beginProgress();

    vpl::sys::CFileBrowserU::SFileAttr files;
    vpl::sys::CFileBrowserU browser;

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
        // Get directory from the queue
        vpl::sys::tString current = directories.front();
        directories.pop_front();

        // Change the directory
        browser.setDirectory( current );

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

		bool bFound = browser.findFirst(StrStar, files);
        for( ; bFound; bFound = browser.findNext(files) )
        {
            // If a directory was found
            if( files.m_bDirectory )
            {
                if( files.m_sName != StrDot && files.m_sName != StrDotDot )
                {
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



//=============================================================================
//=============================================================================
//=============================================================================

const std::string trimSpaces(const std::string& str)
{
    static const std::string WHITESPACE = " \t";

    const size_t beginStr = str.find_first_not_of(WHITESPACE);
    if( beginStr == std::string::npos )
    {
        // no content
        return std::string("");
    }

    const size_t endStr = str.find_last_not_of(WHITESPACE);
    const size_t range = endStr - beginStr + 1;

    return str.substr(beginStr, range);
}


//! Reads all informative DICOM tags (patient name, etc.) from the dataset.
//! - Throws exception on failure.
void readTagsDCTk(DcmDataset * dataset, vpl::img::CDicomSlice & slice)
{
    if( !dataset )
    {
        throw CDicomLoadingFailure();
    }

    dataset->convertToUTF8();

    OFString buffer;
    DcmTag tag;
    OFCondition	status;

    // slice/image number
    tag	= TAG_SLICE_NUMBER;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
    {
        throw CDicomLoadingFailure();
    }
    else
    {
        int val = 0;
        sscanf( buffer.c_str(), "%d", &val );
        slice.m_iSliceNumber = val;
    }

    // patients name 
    tag	= TAG_PATIENTS_NAME;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        // assign default value
        slice.m_sPatientName = std::string("");
    }
    else
    {		
        slice.m_sPatientName = trimSpaces( std::string(buffer.c_str()) );
    }

    // patient id 
    tag	= TAG_PATIENTS_ID;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        // assign default value
        slice.m_sPatientId = std::string("");
    }
    else
    {
        slice.m_sPatientId = trimSpaces( std::string(buffer.c_str()) );
    }

    // patient birthday
    tag	= TAG_PATIENTS_BIRTHDAY;
    status = dataset->findAndGetOFString( tag, buffer );
    if ( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sPatientBirthday = std::string("");
    }
    else
    {
        slice.m_sPatientBirthday = trimSpaces( std::string( buffer.c_str() ) );
    }

    // patient sex
    tag	= TAG_PATIENTS_SEX;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sPatientSex = std::string("");
    }
    else
    {
        slice.m_sPatientSex = trimSpaces( std::string( buffer.c_str() ) );
    }

    // patient description
    tag	= TAG_PATIENTS_DESCRIPTION;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        // assign default value
        slice.m_sPatientDescription = "";
    }
    else
    {
        slice.m_sPatientDescription = trimSpaces( std::string( buffer.c_str() ) );
    }

    // study uid
    tag = TAG_STUDY_UID;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        // assign default value
        slice.m_sStudyUid = std::string( "" );
    }
    else
    {
        slice.m_sStudyUid = std::string( buffer.c_str() );
    }

    // study id
    tag = TAG_STUDY_ID;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        // assign default value
        slice.m_sStudyId = std::string( "" );
    }
    else
    {
        slice.m_sStudyId = std::string( buffer.c_str() );
    }

    // study date
    tag	= TAG_STUDY_DATE;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        // assign default value
        slice.m_sStudyDate = std::string( "" );
    }
    else
    {
        slice.m_sStudyDate = trimSpaces( std::string( buffer.c_str() ) );
    }

    // study description
    tag	= TAG_STUDY_DESCRIPTION;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sStudyDescription = std::string( "" );
    }
    else
    {
        slice.m_sStudyDescription = trimSpaces( std::string( buffer.c_str() ) );
    }

    // series uid
    tag	= TAG_SERIES_UID; 
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        // assign default value
        slice.m_sSeriesUid = std::string( "" );
    }
    else
    {
        slice.m_sSeriesUid = std::string( buffer.c_str() );
    }

    // series number
    Uint32	u;	
    tag	= TAG_SERIES_NUMBER;
    status = dataset->findAndGetUint32( tag, u );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_iSeriesNumber = 0;
    }
    else
    {
        slice.m_iSeriesNumber = u;
    }

    // modality
    tag	= TAG_MODALITY;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sModality = "CT";
    }
    else
    {
        slice.m_sModality = trimSpaces( std::string( buffer.c_str() ) );
    }

    // image type
    tag	= TAG_IMAGE_TYPE;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sImageType = "";
    }
    else
    {
        slice.m_sImageType = trimSpaces( std::string( buffer.c_str() ) );
    }

    // scan options
    tag	= TAG_SCAN_OPTIONS;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sScanOptions = "";
    }
    else
    {
        slice.m_sScanOptions = trimSpaces( std::string( buffer.c_str() ) );
    }

    // series date
    tag	= TAG_SERIES_DATE;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        // assign default value
        slice.m_sSeriesDate = std::string( "" );
    }
    else
    {
        slice.m_sSeriesDate = trimSpaces( std::string( buffer.c_str() ) );
    }

    // series time
    tag	= TAG_SERIES_TIME;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sSeriesTime = "";
    }
    else
    {
        slice.m_sSeriesTime = trimSpaces( std::string( buffer.c_str() ) );
    }

    // series description
    tag	= TAG_SERIES_DESCRIPTION;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sSeriesDescription = "";
    }
    else
    {
        slice.m_sSeriesDescription = trimSpaces( std::string( buffer.c_str() ) );
    }

    Float64	f1, f2;

    // window center 
    tag	= TAG_WINDOW_CENTER;
    status = dataset->findAndGetFloat64( tag, f1 );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_iWindowCenter = 1000;
    }
    else
    {
        slice.m_iWindowCenter = static_cast< int >( floor( f1 ) );
    }

    // window width
    tag	= TAG_WINDOW_WIDTH;
    status = dataset->findAndGetFloat64( tag, f1 );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_iWindowWidth = 1000;
    }
    else
    {
        slice.m_iWindowWidth = static_cast< int >( floor( f1 ) );
    }

    // pixel representation
    tag	= TAG_PIXEL_REPRESENTATION;
    status = dataset->findAndGetUint32( tag, u );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_uPixelRepresentation = 0;
    }
    else
    {
        slice.m_uPixelRepresentation = u;
    }

    // This doesn't work for Planmeca's multi-frame dicom files
    // thickness
/*    tag	= TAG_THICKNESS;
    status = dataset->findAndGetFloat64( tag, f1 );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        // assign default value
        slice.setThickness( 1.0 );
    }
    else
    {
        slice.setThickness( f1 );
    }

    // dx and dy
    tag = TAG_PIXEL_SIZE;
    status = dataset->findAndGetFloat64( tag, f1, 0 );
    status = dataset->findAndGetFloat64( tag, f2, 1 );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.setPixel( 1.0, 1.0 );
    }
    else
    {
        slice.setPixel( f1, f2 );
    }*/

    DcmStack stack;

    // thickness
    tag	= TAG_THICKNESS;
    status = dataset->findAndGetElements( tag, stack );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        double spacing = 0;
        DcmStack Stack;                
        OFCondition	status = dataset->findAndGetElements( TAG_SPACING_BETWEEN_SLICES, Stack );
        if (!status.bad() && Stack.card()>0)
        {            
            DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
            if (elem)
            {
                Float64 f1 = 0;
                status = elem->getFloat64(f1, 0);
                if (!status.bad() && f1!=0)
                    spacing = f1;
            }
        }
        if (spacing>0)
            slice.setThickness( spacing );
        else
            slice.setThickness( 1.0 );
    }
    else
    {
        DcmElement * elem = dynamic_cast<DcmElement *>(stack.elem(0));
        if( !elem && COMPULSORY_TAGS.contains( tag ) )
        {
            throw CDicomLoadingFailure();
        }
        else if ( !elem )
        {
            slice.setThickness( 1.0 );
        }
        else
        {
            // retrieve the position
            status = elem->getFloat64(f1, 0);
            if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
            {
                throw CDicomLoadingFailure();
            }
            else if ( status.bad() )
            {
                slice.setThickness( 1.0 );
            }
            else
            {
                slice.setThickness( f1 );
            }
        }
    }

    // dx and dy
    tag = TAG_PIXEL_SIZE;
    status = dataset->findAndGetElements( tag, stack );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.setPixel( 1.0, 1.0 );
    }
    else
    {
        DcmElement * elem = dynamic_cast<DcmElement *>(stack.elem(0));
        if( !elem && COMPULSORY_TAGS.contains( tag ) )
        {
            throw CDicomLoadingFailure();
        }
        else if ( !elem )
        {
            slice.setPixel( 1.0, 1.0 );
        }
        else
        {
            // retrieve the position
            status = elem->getFloat64(f1, 0);
            status = elem->getFloat64(f2, 1);
            if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
            {
                throw CDicomLoadingFailure();
            }
            else if ( status.bad() )
            {
                slice.setPixel( 1.0, 1.0 );
            }
            else
            {
                slice.setPixel( f1, f2 );
            }
        }
    }

    // slope
/*    tag	= TAG_SLOPE;
    status = dataset->findAndGetFloat64( tag, f1 );
    if( status.good() ) 
    {
        slice.m_dSlope = f1;
    }*/

    // intercept
/*    tag	= TAG_INTERCEPT;
    status = dataset->findAndGetFloat64(tag, f1 );
    if( status.good() ) 
    {
        slice.m_dIntercept = f1;
    }*/

    // manufacturer
    tag	= TAG_MANUFACTURER;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sManufacturer = "";
    }
    else
    {
        slice.m_sManufacturer = trimSpaces( std::string( buffer.c_str() ) );
    }

    // manufacturer's model name
    tag	= TAG_MODEL_NAME;
    status = dataset->findAndGetOFString( tag, buffer );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) ) 
    {
        throw CDicomLoadingFailure();
    }
    else if ( status.bad() )
    {
        slice.m_sModelName = "";
    }
    else
    {
        slice.m_sModelName = trimSpaces( std::string( buffer.c_str() ) );
    }
}

//=============================================================================

//! Reads tags representing orientation of ccordinate system from the dataset.
//! - Throws exception on failure.
void readOrientTagDCTk(DcmDataset * dataset,
                       vpl::img::CVector3D & XAxis,
                       vpl::img::CVector3D & YAxis,
                       vpl::img::CVector3D & ZAxis
                       )
{
    if( !dataset )
    {
        throw CDicomLoadingFailure();
    }
    
    // Orientation tag found flag
    bool bFound = false;
    
    DcmTag tag;
    OFCondition	status;
    DcmStack orientlist;
    
    // image orientation
    tag	= TAG_IMAGE_ORIENTATION;
    status = dataset->findAndGetElements( tag, orientlist );
    if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
    {
        throw CDicomLoadingFailure();
    }
    else if( status.good() )
    {
        DcmElement * elem = dynamic_cast<DcmElement *>( orientlist.elem(0) );
        if( !elem && COMPULSORY_TAGS.contains( tag ) )
        {
            throw CDicomLoadingFailure();
        }
        else if( elem )
        {
            // retrieve the position
            Float64 f1, f2, f3, f4, f5, f6;
            status = elem->getFloat64(f1, 0);
            status = elem->getFloat64(f2, 1);
            status = elem->getFloat64(f3, 2);
            status = elem->getFloat64(f4, 3);
            status = elem->getFloat64(f5, 4);
            status = elem->getFloat64(f6, 5);
            if( status.bad() && COMPULSORY_TAGS.contains( tag ) )
            {
                throw CDicomLoadingFailure();
            }
            else if( status.good() )
            {
                XAxis.setXYZ( f1, f2, f3 );
                YAxis.setXYZ( f4, f5, f6 );
                bFound = true;
            }
        }
    }

    // Use default values
    if( !bFound )
    {
        XAxis.setXYZ( 1, 0, 0 );
        YAxis.setXYZ( 0, 1, 0 );
    }

    // Calculate the z-axis
    ZAxis.vectorProduct( XAxis, YAxis );

    // Final normalization
    XAxis.normalize();
    YAxis.normalize();
    ZAxis.normalize();
}

//=============================================================================

bool loadDicomDCTk( const vpl::sys::tString &dir, const std::string &filename, vpl::img::CDicomSlice &slice, bool bLoadImageData )
{
	vpl::sys::CFileBrowserU browser;
	vpl::sys::tString oldDir = browser.getDirectory();
	browser.setDirectory(dir);
	
	// DCMTk image
    vpl::base::CScopedPtr<DicomImage> image(NULL);

    // Try to load input DICOM image
    try
    {
        DcmFileFormat file_format;
        OFCondition	status = file_format.loadFile( filename.c_str() );
        if ( !status.good() )
        {
            throw CDicomLoadingFailure();
        }

        DcmDataset * dataset = file_format.getDataset();
        if( !dataset )
        {
            throw CDicomLoadingFailure();
        }

        OFString buffer;
        DcmTag tag;
        DcmStack poslist, orientlist;
        Float64	f1, f2, f3;
        unsigned long slicepos = 0;
        double spacing = 0;
        f1 = f2 = f3 = 0;
        
        // This code doesn't work for Planmeca's multi-frame files
/*        tag = TAG_IMAGE_POSITION;
        status = dataset->findAndGetFloat64( tag, f1, 0 );
        status = dataset->findAndGetFloat64( tag, f2, 1 );
        status = dataset->findAndGetFloat64( tag, f3, 2 );
        if( status.bad() )
        {
            throw CDicomLoadingFailure();
        }
        else
        {
            slice.m_ImagePosition.setXYZ( f1, f2, f3 );
        }*/

        // try to locate all slice positions
        tag	= TAG_IMAGE_POSITION;
        status = dataset->findAndGetElements( tag, poslist );
        if (status.bad())
        {
            DcmStack Stack;
            OFCondition	status = dataset->findAndGetElements( TAG_SPACING_BETWEEN_SLICES, Stack );
            if (!status.bad() && Stack.card()>0)
            {            
                DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
                if (elem)
                {
                    // retrieve the position
                    Float64 f1 = 0;
                    status = elem->getFloat64(f1, 0);
                    if (!status.bad() && f1!=0)
                        spacing = f1;
                }
            }
        }
        if( (status.bad() || poslist.empty()) && 0==spacing )
        {
            throw CDicomLoadingFailure();
        }

        if (spacing>0)
        {
            slice.m_ImagePosition.setXYZ( f1, f2, f3 );
        }
        else
        {
            // get the image position 	    
            slicepos = poslist.card() / 2;
            DcmElement * elem = dynamic_cast<DcmElement *>(poslist.elem(slicepos));
            if( !elem )
            {
                throw CDicomLoadingFailure();
            }

            // retrieve the position
            status = elem->getFloat64(f1, 0);
            status = elem->getFloat64(f2, 1);
            status = elem->getFloat64(f3, 2);
            if( status.bad() )
            {
                throw CDicomLoadingFailure();
            }
            else
            {
                slice.m_ImagePosition.setXYZ( f1, f2, f3 );
            }
        }

        // get the image orientation
        vpl::img::CVector3D normal_image;
        readOrientTagDCTk( dataset, slice.m_ImageOrientationX, slice.m_ImageOrientationY, normal_image );

        // slice position calculation
        vpl::img::CPoint3D zero_point( 0, 0, 0 );
        vpl::img::CVector3D position_vector( zero_point, slice.m_ImagePosition );
        slice.setPosition( normal_image.dotProduct(normal_image, position_vector) );

        // prepare for potential decompression
        OFBool opt_verbose = OFFalse;
        E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;
        E_UIDCreation opt_uidcreation = EUC_default;
        E_PlanarConfiguration opt_planarconfig = EPC_default;
        DJDecoderRegistration::registerCodecs(opt_decompCSconversion, opt_uidcreation, opt_planarconfig, opt_verbose);

        E_TransferSyntax es = dataset->getOriginalXfer();
        OFCondition error = dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL );
        if ( error.bad() )
        {
            throw CDicomLoadingFailure();
        }

        // format conversion
        vpl::img::tDensityPixel pMin = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin();
        vpl::img::tDensityPixel pMax = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMax();
//        double dMin = pMin;
//        double dMax = pMax;
//        double dMinRelative = dMin;
//        double dMaxRelative = dMax;
//        image->getMinMaxValues( dMinRelative, dMaxRelative, 0 );

        if( bLoadImageData )
        {
            // Create a new image
            // - Load a single frame from the dataset!
            image = new DicomImage( dataset, es, 0UL, slicepos, 1UL );
            if( !image.get() || image->getStatus() != EIS_Normal )
            {
                throw CDicomLoadingFailure();
            }

            // create pixel data
            vpl::tSize xSize = static_cast< vpl::tSize >( image->getWidth() );
            vpl::tSize ySize = static_cast< vpl::tSize >( image->getHeight() );
//            unsigned short depth = static_cast< unsigned short >( image->getDepth() );

            // resize the slice
            slice.vpl::img::CDImage::resize( xSize, ySize, slice.getMargin() );

            image->deleteDisplayLUT(0);
            image->hideAllOverlays();
            image->removeAllOverlays();
            image->deleteOverlayData();
            image->setNoDisplayFunction();
            image->setNoVoiTransformation();
            image->getOverlayCount();

            // take pixel data pointer 
            const DiPixel * theData = image->getInterData();
            
            // take pixel data representation
            EP_Representation theData_representation = theData->getRepresentation();

            // test pixel data representation
            if( theData_representation != EPR_Uint16 && theData_representation != EPR_Sint16 &&
                theData_representation != EPR_Uint8 && theData_representation != EPR_Sint8 && 
                theData_representation != EPR_Uint32 && theData_representation != EPR_Sint32)
            {
                throw CDicomLoadingFailure();
            }

            // take slice image data pointer from pixel data
            const vpl::img::tDensityPixel * thePixels	= reinterpret_cast< const vpl::img::tDensityPixel* >( theData->getData() );
            if( !thePixels )
            {
                throw CDicomLoadingFailure();
            }
            const vpl::img::tPixel8 * thePixels8u	= reinterpret_cast< const vpl::img::tPixel8* >( theData->getData() );
            const vpl::sys::tInt8   * thePixels8	= reinterpret_cast< const vpl::sys::tInt8* >( theData->getData() );
            const vpl::sys::tInt32  * thePixels32	= reinterpret_cast< const vpl::sys::tInt32* >( theData->getData() );

#pragma omp parallel for
            for ( vpl::tSize y = 0; y < ySize; y++ )
            {
                for ( vpl::tSize x = 0; x < xSize; x++ )
                {
                    vpl::img::tDensityPixel Value = 0;
                    switch(theData_representation)
                    {
                    case EPR_Uint8:
                        Value = (8000/255.0 * thePixels8u[ y * xSize + x ]) - 1000;
                        break;
                    case EPR_Sint8:
                        Value = (8000/255.0 * (thePixels8[ y * xSize + x ] + 128)) - 1000;
                        break;
                    case EPR_Uint32:
                    case EPR_Sint32:
                        Value = thePixels32[ y * xSize + x ];
                        break;
                    default:
                        Value = thePixels[ y * xSize + x ];
                    }
                    Value = std::min(Value,pMax);
                    Value = std::max(Value,pMin);
                    slice( x,y ) = Value;
                }
            }
        }

        // Read all required DICOM tags
        readTagsDCTk( dataset, slice );
    }

    // Any failure?
    catch ( CDicomLoadingFailure & )
    {
		browser.setDirectory(oldDir);
		DJDecoderRegistration::cleanup();
        return false;
    }

	browser.setDirectory(oldDir);
    DJDecoderRegistration::cleanup();
    return true;
}

//=============================================================================
int loadDicomDCTk( const vpl::sys::tString &dir, const std::string &filename, tDicomSlices &slices, bool bLoadImageData, bool bIgnoreBitsStoredTag )
{
	vpl::sys::CFileBrowserU browser;
	vpl::sys::tString oldDir = browser.getDirectory();
	browser.setDirectory(dir);

	// DCMTk image
    vpl::base::CScopedPtr<DicomImage> image(NULL);

    // Try to load input DICOM image
    try
    {
        DcmFileFormat file_format;
        OFCondition	status = file_format.loadFile( filename.c_str() );
        if( !status.good() )
        {
            throw CDicomLoadingFailure();
        }

        DcmDataset * dataset = file_format.getDataset();
        if( !dataset )
        {
            throw CDicomLoadingFailure();
        }

        if (bIgnoreBitsStoredTag)
        {
            // we have met dicom data that have BitsStored tag set but use all allocated bits
            Uint16  bitsAllocated = 0, 
                    bitsStored = 0, 
                    highestBit = 0;
            dataset->findAndGetUint16(DCM_BitsAllocated, bitsAllocated);            
            if (bitsAllocated>0)
            {
                dataset->findAndGetUint16(DCM_BitsStored, bitsStored);
                bitsStored = bitsAllocated;
                dataset->putAndInsertUint16( DCM_BitsStored, bitsStored);
                dataset->findAndGetUint16(DCM_HighBit, highestBit);
                highestBit = bitsStored-1;
                dataset->putAndInsertUint16( DCM_HighBit, highestBit);
            }
        }

        OFString buffer;
        DcmTag tag;
        DcmStack poslist;
        double spacing = 0;
        
        // try to locate all slice position
        tag	= TAG_IMAGE_POSITION;
        status = dataset->findAndGetElements( tag, poslist );        
        if (status.bad())
        {
            DcmStack Stack;                
            OFCondition	status = dataset->findAndGetElements( TAG_SPACING_BETWEEN_SLICES, Stack );
            if (!status.bad() && Stack.card()>0)
            {            
                DcmElement * elem = dynamic_cast<DcmElement *>(Stack.elem(0));
                if (elem)
                {
                    Float64 f1 = 0;
                    status = elem->getFloat64(f1, 0);
                    if (!status.bad() && f1!=0)
                        spacing = f1;
                }
            }
        }
        if( (status.bad() || poslist.empty()) && 0==spacing )
        {
            throw CDicomLoadingFailure();
        }

        double sliceThickness = 0;
        {   // get slice thickness
            DcmStack stack;
            DcmTag  tag = TAG_THICKNESS;
            status = dataset->findAndGetElements( tag, stack );
            if (!status.bad() && stack.card()>0)
            {
                DcmElement * elem = dynamic_cast<DcmElement *>(stack.elem(0));
                if (elem)
                {
                    // retrieve the position
                    Float64 f1 = 0;
                    status = elem->getFloat64(f1, 0);
                    if (!status.bad() && f1!=0)
                        sliceThickness = f1;
                }
            }
        }
        if (0==spacing && 0==sliceThickness)
            sliceThickness = 1;
        
        // get the image orientation
        vpl::img::CVector3D ImageOrientationX, ImageOrientationY, normal_image;
        readOrientTagDCTk( dataset, ImageOrientationX, ImageOrientationY, normal_image );

        // Parse all required tags and prepare a slice prototype
        vpl::img::CDicomSlice slicetags;
        readTagsDCTk( dataset, slicetags );
        slicetags.m_ImageOrientationY = ImageOrientationY;
        slicetags.m_ImageOrientationX = ImageOrientationX;
        slicetags.m_ImagePosition.setXYZ( 0, 0, 0 );
        slicetags.setPosition( 0.0 );

        // Prepare for potential decompression
        OFBool opt_verbose = OFFalse;
        E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;
        E_UIDCreation opt_uidcreation = EUC_default;
        E_PlanarConfiguration opt_planarconfig = EPC_default;
        DJDecoderRegistration::registerCodecs(opt_decompCSconversion, opt_uidcreation, opt_planarconfig, opt_verbose);

        E_TransferSyntax es = dataset->getOriginalXfer();
        OFCondition error = dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL );
        if ( error.bad() )
        {
            throw CDicomLoadingFailure();
        }

        // Format conversion
        vpl::img::tDensityPixel pMin = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin();
        vpl::img::tDensityPixel pMax = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMax();
//        double dMin = pMin;
//        double dMax = pMax;
//        double dMinRelative = dMin;
//        double dMaxRelative = dMax;

        if( bLoadImageData )
        {
            // - Load all frames from the dataset!
            image = new DicomImage( dataset, es, 0UL, 0UL, 0UL );
            if( !image.get() || image->getStatus() != EIS_Normal || (0==poslist.card() && 0==spacing) || 0==image->getFrameCount() /*image->getFrameCount() != poslist.card()*/ )
            {
                throw CDicomLoadingFailure();
            }

            // Setup the DCMTk image to read raw image data
            image->deleteDisplayLUT(0);
            image->hideAllOverlays();
            image->removeAllOverlays();
            image->deleteOverlayData();
            image->setNoDisplayFunction();
            image->setNoVoiTransformation();
            image->getOverlayCount();

            // Format conversion
//            image->getMinMaxValues( dMinRelative, dMaxRelative, 0 );
        }

        // Load all frames
        //for( unsigned long i = 0; i < poslist.card(); ++i ) // we have a file which has multiple frames but only one position record
        for( unsigned long i = 0; i < image->getFrameCount(); ++i )
        {
            // Create a new slice
            vpl::img::CDicomSlicePtr pSlice = new vpl::img::CDicomSlice(slicetags);

            // Get image position 	
            //DcmElement * elem = dynamic_cast<DcmElement *>(poslist.elem(i));

            // Planmeca dicom files were upside down - it is because posList entries are in stack like structure,
            // so we have to take them in reverse order!
            if (0==poslist.card()) // there are also some iCATSystem files without image position tag, but have slicethickness and spacing defined
            {
                Float64 f1, f2, f3;
                f1 = f2 = f3 = 0;
                f3 = -i * std::max(sliceThickness,spacing); // just a guess
                pSlice->m_ImagePosition.setXYZ( f1, f2, f3 );
                vpl::img::CPoint3D zero_point( 0, 0, 0 );
                vpl::img::CVector3D position_vector( zero_point, pSlice->m_ImagePosition );
                pSlice->setPosition( normal_image.dotProduct(normal_image, position_vector) );
            }
            else
            {
                double correctionF3 = 0;
                DcmElement * elem = NULL;
                if (i>=poslist.card())
                {
                    elem = dynamic_cast<DcmElement *>(poslist.elem(0));
                    correctionF3 = -i * sliceThickness; // just a guess
                }
                else
                    elem = dynamic_cast<DcmElement *>(poslist.elem(poslist.card()-i-1));            
            
                if( elem )
                {
                    // Retrieve the position
                    Float64 f1, f2, f3;
                    status = elem->getFloat64(f1, 0);
                    status = elem->getFloat64(f2, 1);
                    status = elem->getFloat64(f3, 2);
                    f3 += correctionF3;
                    if( status.good() )
                    {
                        pSlice->m_ImagePosition.setXYZ( f1, f2, f3 );
                    }

                    // slice position vector calculation. length and normalization
                    vpl::img::CPoint3D zero_point( 0, 0, 0 );
                    vpl::img::CVector3D position_vector( zero_point, pSlice->m_ImagePosition );
                    pSlice->setPosition( normal_image.dotProduct(normal_image, position_vector) );
                }
            }

            if( bLoadImageData )
            {
                // Get the image size
                vpl::tSize xSize = static_cast< vpl::tSize >( image->getWidth() );
                vpl::tSize ySize = static_cast< vpl::tSize >( image->getHeight() );
//                unsigned short depth = static_cast< unsigned short >( image->getDepth() );

                // Resize the slice
                pSlice->vpl::img::CDImage::resize( xSize, ySize, pSlice->getMargin() );

                // take pixel data pointer 
                const DiPixel * theData = image->getInterData();
                // take pixel data representation
                EP_Representation theData_representation = theData->getRepresentation();

                // test pixel data representation
                if( theData_representation != EPR_Uint16 && theData_representation != EPR_Sint16 &&
                    theData_representation != EPR_Uint8 && theData_representation != EPR_Sint8 && 
                    theData_representation != EPR_Uint32 && theData_representation != EPR_Sint32)
                {
                    //image->getOutputData(16);
                    //deleteOutputData();
                    throw CDicomLoadingFailure();
                }

                // take slice image data pointer from pixel data
                const vpl::img::tDensityPixel * thePixels	= reinterpret_cast< const vpl::img::tDensityPixel* >( theData->getData() );                
                if( !thePixels )
                {
                    throw CDicomLoadingFailure();
                }

                const vpl::img::tPixel8 * thePixels8u	= reinterpret_cast< const vpl::img::tPixel8* >( theData->getData() );
                const vpl::sys::tInt8   * thePixels8	= reinterpret_cast< const vpl::sys::tInt8* >( theData->getData() );
                const vpl::sys::tInt32  * thePixels32	= reinterpret_cast< const vpl::sys::tInt32* >( theData->getData() );

                // move to the corresponding slice
                thePixels += i * xSize * ySize;
                thePixels8 += i * xSize * ySize;
                thePixels8u += i * xSize * ySize;
                thePixels32 += i * xSize * ySize;

#pragma omp parallel for
                for ( vpl::tSize y = 0; y < ySize; y++ )
                {
                    for ( vpl::tSize x = 0; x < xSize; x++ )
                    {
                        vpl::img::tDensityPixel Value = 0;
                        switch(theData_representation)
                        {
                        case EPR_Uint8:
                            Value = vpl::img::tDensityPixel((8000 * int(thePixels8u[y * xSize + x]) / 255) - 1000);
                            break;
                        case EPR_Sint8:
                            Value = vpl::img::tDensityPixel((8000 * (int(thePixels8[y * xSize + x]) + 128) / 255) - 1000);
                            break;
                        case EPR_Uint32:
                        case EPR_Sint32:
                            Value = vpl::img::tDensityPixel(thePixels32[y * xSize + x]);
                            break;
                        default:
                            Value = vpl::img::tDensityPixel(thePixels[y * xSize + x]);
                        }
                        Value = std::min(Value,pMax);
                        Value = std::max(Value,pMin);
                        pSlice->set( x, y, Value );
                    }
                }
            }

            // Store the new slice
            slices.push_back(pSlice);
        }
    }

    // Any failure?
    catch ( CDicomLoadingFailure & )
    {
		browser.setDirectory(oldDir);
        DJDecoderRegistration::cleanup();
        return false;
    }

	browser.setDirectory(oldDir);
    DJDecoderRegistration::cleanup();
    return true;
}

//=============================================================================
bool getDicomFileInfo( const vpl::sys::tString &dir, const std::string &filename, int& nFrames)
{
	vpl::sys::CFileBrowserU browser;
	vpl::sys::tString oldDir = browser.getDirectory();
	browser.setDirectory(dir);
    
	// DCMTk image
    vpl::base::CScopedPtr<DicomImage> image(NULL);

    nFrames = 0;

    // Try to load input DICOM image
    try
    {
        DcmFileFormat file_format;
        OFCondition	status = file_format.loadFile( filename.c_str() );
        if( !status.good() )
        {
            throw CDicomLoadingFailure();
        }

        DcmDataset * dataset = file_format.getDataset();
        if( !dataset )
        {
            throw CDicomLoadingFailure();
        }

        OFString buffer;
        DcmTag tag;
        DcmStack stack;
        
        tag	= TAG_NUMBER_OF_FRAMES;
        long int numOfFrames=0;
        dataset->findAndGetLongInt(DCM_NumberOfFrames,numOfFrames);
        nFrames = numOfFrames;
    }

    // Any failure?
    catch ( CDicomLoadingFailure & )
    {
		browser.setDirectory(oldDir);
        DJDecoderRegistration::cleanup();
        return false;
    }

	browser.setDirectory(oldDir);
    DJDecoderRegistration::cleanup();
    return true;
}

} // namespace data
