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

#include <data/CDicomGDCM.h>

#if defined( TRIDIM_USE_GDCM )

#include <data/CSeries.h>
//for insert method
#include <data/CDicomSaverGDCM.h>
#include <data/DicomTags.h>

//STL
#include <codecvt>

#ifdef _WIN32 // Windows specific
#include <Windows.h>
#include <Wbemcli.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")
#endif


namespace data
{

CDicomGDCM::CDicomGDCM(bool loadBuggyFile)
:CDicom()
, m_serie(nullptr)
, m_loadBuggyFile(loadBuggyFile)
{
}

CDicomGDCM::CDicomGDCM(data::CSerieInfo* serie, bool loadBuggyFile)
: CDicom()
, m_serie(serie)
, m_loadBuggyFile(loadBuggyFile)
{
}
//=================================================================================================
CDicomGDCM::~CDicomGDCM()
{   
}

#ifdef WIN32 // Windows need nonunicode paths to be in ACP
std::string CDicomGDCM::wcs2ACP(const std::wstring &filename)
{
	std::string convFilename;
	{
		// get buffer size
		int size = WideCharToMultiByte(CP_ACP, 0, filename.c_str(), filename.size(), 0, 0, 0, 0);
		if (size>0)
		{
			// convert
			char* buffer = new char[size + 1];
			int sizeC = WideCharToMultiByte(CP_ACP, 0, filename.c_str(), filename.size(), buffer, size, 0, 0);
			if (sizeC>0)
			{
				assert(sizeC<size + 1);
				buffer[sizeC] = 0;
				convFilename = buffer;
			}
			delete buffer;
		}
	}
	return convFilename;
}
#endif

void CDicomGDCM::getSliceIds(tDicomNumList& Numbers)
{    
    std::vector<std::vector<double>> &vec = m_loader.getImagePositions();

    if (vec.empty())
        return;

    // Process all slice numbers
    std::vector<std::vector<double>>::iterator it = vec.begin();
    std::vector<std::vector<double>>::iterator itEnd = vec.end();

    for (; it != itEnd; ++it)
    {
        Numbers.push_back(convPos2Id((*it)[0], (*it)[1], (*it)[2]));
    }  
   
}

std::string CDicomGDCM::getSerieId()
{
    return m_loader.readTag(gdcm::Tag(VPL_GDCM_SeriesInstanceUID));
}

bool CDicomGDCM::setPatientName(const std::string& name)
{
    gdcm::File f = m_loader.getGDCMReader().GetFile();
    gdcm::DataSet &ds = f.GetDataSet();
    
    CDicomSaverGDCM::insertDataElement<VPL_GDCM_PatientName>(ds,name.c_str());

    return true;
}

bool CDicomGDCM::anonymize(const std::string& name)
{
    gdcm::File f = m_loader.getGDCMReader().GetFile();
    gdcm::DataSet &ds = f.GetDataSet();

    CDicomSaverGDCM::anonymize(f);

    if (!name.empty())
        CDicomSaverGDCM::insertDataElement<VPL_GDCM_PatientName>(ds, name.c_str());

    return true;
}

int CDicomGDCM::getBitsAllocated()
{
    return m_loader.getAttributeValue<VPL_GDCM_BitsAllocated>();
}

int CDicomGDCM::getSliceId()
{
    std::vector<std::vector<double>> &vec = m_loader.getImagePositions();
    if (vec.empty())
        return -1 ;

    std::vector<double> pos = vec.front();

    return convPos2Id(pos[0], pos[1], pos[2]);
}

double CDicomGDCM::getPixelSpacing()
{
    std::string pixelSpacing = m_loader.readTag(gdcm::Tag(VPL_GDCM_PixelSpacing));
    return std::atof(pixelSpacing.c_str());
}

long CDicomGDCM::saveToBuffer(char* buffer, long length)
{
    //TODO not tested (used in dataExpress Plugin)
    gdcm::Writer writer;

    writer.SetFile(m_loader.getGDCMReader().GetFile());

    std::ostringstream ss;

    writer.SetStream(ss);  
   
    std::string str = ss.str();

    return str.copy(buffer, str.size());
}

bool CDicomGDCM::compressLosslessJPEG()
{
    //TODO changing original image should not be possible
   
    //throw std::exception("Not implemented") 
    return false;

}

bool CDicomGDCM::preLoadFile(const std::string& file)
{   
    return m_loader.preLoadFile(file);
}

std::string CDicomGDCM::createFullPath(const vpl::sys::tString dir, const vpl::sys::tString filename)
{
    //get full file name

    std::string converted_dir;
    std::string converted_fn;
    
#ifdef __APPLE__    
    converted_dir = vpl::sys::tStringConv::toUtf8(dir);
    converted_fn = vpl::sys::tStringConv::toUtf8(filename);
#else
    converted_dir = wcs2ACP(dir);
	converted_fn = wcs2ACP(filename);
#endif
    std::string fn = converted_dir + "\\" + converted_fn;
    return fn;
}

#if !defined(__APPLE__)
std::string CDicomGDCM::createFullPath(const vpl::sys::tString dir, std::string filename)
{
    //get full file name
	std::string converted_dir;
    std::string fn;

	converted_dir = wcs2ACP(dir);

    fn = converted_dir + "\\" + filename;
    return fn;
}
#endif
    
bool CDicomGDCM::preLoadFile(const vpl::sys::tString &dir, const vpl::sys::tString &filename)
{
    return m_loader.preLoadFile(createFullPath(dir, filename));
}

bool CDicomGDCM::preLoadFile(const vpl::sys::tString &dir, std::string &filename)
{
    return m_loader.preLoadFile(createFullPath(dir, filename));
}
bool CDicomGDCM::loadFile(const std::string& file)
{
    try
    {
        m_bOk = m_loader.readFile(file);
        return m_bOk;
    }
    // GDCM reader failed to load given file
    catch (vpl::img::CDicomSliceLoaderGDCM::CDicomSliceLoaderGDCMExeption &e)
    {
       
        if (m_serie == nullptr) 
        {
            m_bOk = false;
            return false;
        }

        //if flag was already set, load file using custom load function
        if (m_loadBuggyFile)
        {
            m_bOk = m_loader.tryToLoadBuggyFile(file);
        }
        else
        {
			VPL_LOG_INFO("Loading serie with ID: " << m_loader.getAttributeValue<VPL_GDCM_SeriesInstanceUID>() << " as buggy.");
            
            //Tells serie that, all buggy files in this serie should be loaded without asking the user
            //TODO this could be replaced with signal and no pointer to serie will be needed
            m_serie->setLoadBuggySerie(true);

            m_bOk = m_loader.tryToLoadBuggyFile(file);
        }
        
        return m_bOk;

    }
    return false;
}

bool CDicomGDCM::loadFile(const vpl::sys::tString& dir, const vpl::sys::tString& filename)
{
    return loadFile(createFullPath(dir, filename));
}

bool CDicomGDCM::loadFile(const vpl::sys::tString& dir, std::string& filename)
{
    return loadFile(createFullPath(dir, filename));
}

bool CDicomGDCM::loadDicom(const vpl::sys::tString &dir,
                           const std::string &filename,
                           vpl::img::CDicomSlice &slice,
                           sExtendedTags& tags,
                           bool bLoadImageData)
{
    bool bOK = false;
    //if it's multiframe dicom file, load the middle frame, becasue this function is used just to preload middle image
    if (m_loader.isMultiframe())
        bOK = m_loader.loadMultiframeDicom(createFullPath(dir, filename), slice, m_loader.getNumberOfFrames() / 2, bLoadImageData);
    else
        bOK = m_loader.loadDicom(createFullPath(dir, filename), slice, bLoadImageData);
   
    if (bOK)
    {
        loadExtendedTags(tags);
    }

    return bOK;
}

int CDicomGDCM::loadDicom(const vpl::sys::tString &dir,
                          const std::string &filename,
                          tDicomSlices &slices,
                          sExtendedTags& tags,
                          bool bLoadImageData,
                          bool bIgnoreBitsStoredTag)
{
    bool bOK = false;

    if (m_loader.isMultiframe())
    {
        bOK = m_loader.loadMultiframeDicom(createFullPath(dir, filename), slices, true);
    }
    else
    {
        vpl::img::CDicomSlicePtr ptr(new vpl::img::CDicomSlice);
        if ((bOK = m_loader.loadDicom(createFullPath(dir, filename), *ptr, true)))
            slices.push_back(ptr);
    }

    if (bOK)
        loadExtendedTags(tags);

    return bOK;
       
}

std::string CDicomGDCM::getPatientName()
{
    return m_loader.readTag(gdcm::Tag(VPL_GDCM_PatientName));
}

int CDicomGDCM::getNumberOfFrames()
{
	return m_loader.getAttributeValue<VPL_GDCM_NumberOfFrames>();
}

std::string CDicomGDCM::getStudyId()
{
    return m_loader.readTag(gdcm::Tag(VPL_GDCM_StudyID));
}

void CDicomGDCM::loadExtendedTags(sExtendedTags& tags)
{

    tags.fDistanceSourceToDetector = m_loader.getAttributeValue<VPL_GDCM_DistanceSourceToDetector>();
    tags.fDistanceSourceToPatient = m_loader.getAttributeValue<VPL_GDCM_DistanceSourceToPatient>();
    tags.fEstimatedRadiographicMagnificationFactor = m_loader.getAttributeValue<VPL_GDCM_EstimatedRadiographicMagnificationFactor>();
    tags.fGridFocalDistance = m_loader.getAttributeValue<VPL_GDCM_GridFocalDistance>();
    tags.fPatientHeight = m_loader.getAttributeValue<VPL_GDCM_PatientSize>();
    tags.fPatientWeight = m_loader.getAttributeValue<VPL_GDCM_PatientWeight>();
    //safe conversion to int
    tags.iPatientAge = atoi(m_loader.getAttributeValue<VPL_GDCM_PatientAge>());
   
}

}   //namespace data

#endif //TRIDIM_USE_GDCM