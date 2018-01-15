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

#ifndef CDicomGDCM_H
#define CDicomGDCM_H

#if defined( TRIDIM_USE_GDCM )

#include <data/CDicom.h>
#include <data/CDicomLoader.h>

//VPL
#include <VPL/Base/Object.h>
#include <VPL/Base/SharedPtr.h>
#include <VPL/ImageIO/DicomSliceLoaderGDCM.h>


namespace data
{
    class CSerieInfo;

///////////////////////////////////////////////////////////////////////////////
//! Class encapsulating GDCM dicom file containing one or more image frames.

class CDicomGDCM : public data::CDicom
{
public:

    //! Smart pointer declaration
    VPL_SHAREDPTR(CDicomGDCM);    

public:
    //! Default constructor.
    CDicomGDCM(bool loadBuggyFile = false);

    CDicomGDCM(data::CSerieInfo* serie, bool loadBuggyFile = false);

    //! Destructor.
    ~CDicomGDCM();

    //Virtual methods, see base class CDicom

    virtual bool loadFile(const std::string & file) override;
    virtual std::string getPatientName() override;
    virtual std::string getStudyId() override;
    virtual std::string getSerieId() override;
    virtual int	getNumberOfFrames() override;
    virtual int getBitsAllocated() override;
    virtual int getSliceId() override;
    virtual void getSliceIds(tDicomNumList& Numbers) override;
    virtual double getPixelSpacing() override;
    virtual bool setPatientName(const std::string & name) override;
    virtual bool anonymize(const std::string & name) override;
    virtual long saveToBuffer(char * buffer, long length) override;
    virtual bool compressLosslessJPEG() override;
    virtual bool loadDicom(const vpl::sys::tString &dir,
                           const std::string &filename,
                           vpl::img::CDicomSlice &slice,
                           sExtendedTags& tags,
                           bool bLoadImageData = true) override;
    virtual int loadDicom(const vpl::sys::tString &dir,
                         const std::string &filename,
                         tDicomSlices &slices,
                         sExtendedTags& tags,
                         bool bLoadImageData = true,
                         bool bIgnoreBitsStoredTag = false) override;

    //! Pre-loads file, ignores image data
    bool preLoadFile(const std::string & file);
    bool preLoadFile(const vpl::sys::tString &dir, const vpl::sys::tString &filename);
    bool preLoadFile(const vpl::sys::tString &dir, std::string &filename);

    //! Loads file by name. This function only tries to parse the file and checks whether it is valid DICOM file.   
    bool loadFile(const vpl::sys::tString &dir, const vpl::sys::tString &filename);
    bool loadFile(const vpl::sys::tString &dir, std::string &filename);

private:
  
    //! Loader used to load DICOM file into CDicomSlice
    vpl::img::CDicomSliceLoaderGDCM m_loader;

    //! Reference slice: assuming that all tag information are the same for all slices
    //! exept for slice position and thickness
    vpl::img::CDicomSlice m_referenceSlice;

    data::CSerieInfo *m_serie;

    bool m_loadBuggyFile;

private:

    //! Loads additional tags
    void loadExtendedTags(sExtendedTags& tags);

#ifdef WIN32 // Windows need nonunicode paths to be in ACP
	static std::string     wcs2ACP(const std::wstring &filename);
#endif

    //! Creates absolute path from dir and filename
    std::string createFullPath(const vpl::sys::tString dir, const vpl::sys::tString filename);
#if !defined(__APPLE__)
    std::string createFullPath(const vpl::sys::tString dir, std::string filename);
#endif
};

/*
* Smart pointer.
*/
typedef CDicomGDCM::tSmartPtr  CDicomGDCMPtr;

} // namespace data

#endif //TRIDIM_USE_GDCM
#endif // CDicomGDCM_H
