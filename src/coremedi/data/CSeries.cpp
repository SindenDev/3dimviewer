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

#include <data/CSeries.h>
#include <data/DicomTagUtils.h>
#include <data/CDicomLoader.h>
#include <data/CDicom.h>
#include <VPL/System/FileBrowser.h>


//==============================================================================================
data::CSerieInfo::CSerieInfo() : m_sId( "" ), m_bHas8BitData(false)
{
}

//==============================================================================================
data::CSerieInfo::~CSerieInfo()
{
}

//==============================================================================================
void data::CSerieInfo::setId( const std::string & id )
{
    m_sId = id;
}

//==============================================================================================
const std::string& data::CSerieInfo::getId() const
{
    return m_sId;
}

//==============================================================================================
int	data::CSerieInfo::getNumOfSlices() const
{
    return int(m_DicomNumSet.size());
}

//==============================================================================================
bool data::CSerieInfo::hasSlice( int number ) const
{
    return (m_DicomNumSet.find(number) != m_DicomNumSet.end());
}

//==============================================================================================
bool data::CSerieInfo::addSlice( int id )
{
    m_DicomNumSet.insert( id );
    return true;
}

//==============================================================================================
int	data::CSerieInfo::getNumOfDicomFiles() const
{
    return int(m_DicomList.size());
}

//==============================================================================================
bool data::CSerieInfo::addDicomFile( const vpl::sys::tString & path )
{
	SDCMTkFilename dcmtkFilename;
#ifdef WIN32
    dcmtkFilename.filename = wcs2ACP(path);
#else
    dcmtkFilename.filename = vpl::sys::tStringConv::toUtf8(path);
#endif
	m_DCMTkList.push_back(dcmtkFilename);
	m_DicomList.push_back(path);
    return true;
}

//==============================================================================================
bool data::CSerieInfo::addDicomFile( const vpl::sys::tString &dir, const vpl::sys::tString &filename )
{
	SDCMTkFilename dcmtkFilename;
	dcmtkFilename.directory = dir;
#ifdef WIN32
    dcmtkFilename.filename = wcs2ACP(filename);
#else
	dcmtkFilename.filename = vpl::sys::tStringConv::toUtf8(filename);
#endif
	m_DCMTkList.push_back(dcmtkFilename);
	vpl::sys::tChar CharSlash = vplT('/');
	m_DicomList.push_back(dir + CharSlash + filename);
    return true;
}

//==============================================================================================
void data::CSerieInfo::getDicomFileList( tDicomFileList& Files )
{
    Files.assign(m_DicomList.begin(), m_DicomList.end());
}

//==============================================================================================
bool data::CSerieInfo::loadDicomFile(int FileNum, vpl::img::CDicomSlice& Slice, sExtendedTags& tags, bool bLoadImageData)
{
    if( FileNum >= 0 && FileNum < int(m_DicomList.size()) )
    {
		return loadDicomDCTk( m_DCMTkList[FileNum].directory, m_DCMTkList[FileNum].filename, Slice, tags, bLoadImageData );
    }
    else
    {
        return false;
    }
}

//==============================================================================================
int data::CSerieInfo::loadDicomFile(int FileNum, tDicomSlices& Slices, sExtendedTags& tags, bool bLoadImageData, bool bCompatibilityMode)
{
    if( FileNum >= 0 && FileNum < int(m_DicomList.size()) )
    {
        return loadDicomDCTk( m_DCMTkList[FileNum].directory, m_DCMTkList[FileNum].filename, Slices, tags, bLoadImageData, bCompatibilityMode );
    }
    else
    {
        return 0;
    }
}

//==============================================================================================
//==============================================================================================
//==============================================================================================
data::CSeries::CSeries()
{
}

//==============================================================================================
data::CSeries::~CSeries()
{
}

//==============================================================================================
int	data::CSeries::getNumSeries() const
{
    return int(m_Series.size());
}

//==============================================================================================
data::CSerieInfo * data::CSeries::getSerie( const std::string & id )
{
    tSerieTable::iterator it = m_SerieTable.find( id );
    if ( it != m_SerieTable.end() ) return it->second.get();
    else return NULL;
}

//==============================================================================================
data::CSerieInfo * data::CSeries::getSerie( int number )
{
    if ( number < getNumSeries() && number >= 0 ) return m_Series[number].get();
    else return NULL;
}

//==============================================================================================
data::CSerieInfo * data::CSeries::addSerie( const std::string & id )
{
    CSerieInfo * serie = getSerie( id );
    if( serie )
    {
        return serie;
    }

    CSerieInfo * new_serie = new CSerieInfo();
    new_serie->setId( id );
    m_Series.push_back( new_serie );
    m_SerieTable[id] = new_serie;
    return new_serie;
}

//==============================================================================================
data::CSerieInfo * data::CSeries::addDicomFile( const vpl::sys::tString & path )
{
#ifdef WIN32
    std::string convPath = wcs2ACP(path);
#else
	std::string convPath = vpl::sys::tStringConv::toUtf8(path);
#endif
	CDicom dicom;
    if( dicom.loadFile( convPath ) )
    {
        // Retrieve serie id
        std::string	id = dicom.getSerieId();
        if( id.empty() )
        {
            return NULL;
        }
        CSerieInfo * serie = addSerie( id );
        if( !serie )
        {
            return NULL;
        }

        // Add dicom file to the serie
        serie->addDicomFile( path );

        // Retreive slice numbers of all frames stored in the dicom file
        CDicom::tDicomNumList Ids;
        dicom.getSliceIds( Ids );

        // Process all slice numbers
        CDicom::tDicomNumList::iterator it = Ids.begin();
        CDicom::tDicomNumList::iterator itEnd = Ids.end();
        for( ; it != itEnd; ++it )
        {
            serie->addSlice( *it );
        }

        // O.K.
        return serie;
    }
    
    return NULL;
}

#ifdef WIN32 // Windows need nonunicode paths to be in ACP
std::string data::wcs2ACP(const std::wstring &filename)
{
    std::string convFilename;
    {
        // get buffer size
        int size=WideCharToMultiByte(CP_ACP,0,filename.c_str(),filename.size(),0,0,0,0);
        if (size>0)
        {
            // convert
            char* buffer=new char[size+1];
            int sizeC=WideCharToMultiByte(CP_ACP,0,filename.c_str(),filename.size(),buffer,size,0,0);
            if (sizeC>0)
            {
                assert(sizeC<size+1);
                buffer[sizeC]=0;
                convFilename=buffer;
            }
            delete buffer;
        }
    }
    return convFilename;
}
#endif

data::CSerieInfo * data::CSeries::addDicomFile( const vpl::sys::tString &dir, const vpl::sys::tString &filename )
{
	vpl::sys::CFileBrowserU browser;
	vpl::sys::tString oldDir = browser.getDirectory();
	browser.setDirectory(dir);
	
#ifdef WIN32
    // Windows need nonunicode paths to be in ACP
    std::string convFilename = wcs2ACP(filename);
#else
    std::string convFilename = vpl::sys::tStringConv::toUtf8(filename);
#endif

	CDicom dicom;
    if( dicom.loadFile( convFilename ) )
    {
        // Retrieve serie id
        std::string	id = dicom.getSerieId();
        if( id.empty() )
        {
			browser.setDirectory(oldDir);
			return NULL;
        }
        CSerieInfo * serie = addSerie( id );
        if( !serie )
        {
			browser.setDirectory(oldDir);
            return NULL;
        }
		// Add dicom file to the serie
        serie->addDicomFile( dir, filename );

        // Retreive slice numbers of all frames stored in the dicom file
        CDicom::tDicomNumList Ids;
        dicom.getSliceIds( Ids );

        // Process all slice numbers
        CDicom::tDicomNumList::iterator it = Ids.begin();
        CDicom::tDicomNumList::iterator itEnd = Ids.end();
        for( ; it != itEnd; ++it )
        {
            serie->addSlice( *it );
        }

        // get dicom info
        int nBits = dicom.getBitsAllocated();
        if (8==nBits)
            serie->setHas8BitData(true);

        // O.K.
		browser.setDirectory(oldDir);
        return serie;
    }
    
	browser.setDirectory(oldDir);
    return NULL;
}
