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

#ifndef CDicomDCTk_H
#define CDicomDCTk_H

#include <data/CDicom.h>


///////////////////////////////////////////////////////////////////////////////
// Forward declarations

class DcmFileFormat;

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Class encapsulating DCMTk dicom file containing one or more image frames.

class CDicomDCTk : public CDicom
{
public:
    //! Smart pointer declaration
    VPL_SHAREDPTR(CDicomDCTk);
    
public:
    //! Default constructor.
    CDicomDCTk();

    //! Constructor parametrized by the filename.
    CDicomDCTk(const std::string & file);

    //! Destructor.
    ~CDicomDCTk();

    //Virtual methods, see base class CDicom

    virtual bool loadFile( const std::string & file ) override;
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


    ///////////////////////////////////////////////////////////////////////////////
    // Dicom dataset loader.

    //! Loads a single frame (i.e. slice) from a given dicom file.
    virtual bool loadDicom(const vpl::sys::tString &dir,
                       const std::string &filename,
                       vpl::img::CDicomSlice &slice,
                       sExtendedTags& tags,
                       bool bLoadImageData = true) override;

    //! Loads all frames/slices from a given dicom file.
    //! - Returns the number of successfully read images.
    virtual int loadDicom(const vpl::sys::tString &dir,
                      const std::string &filename,
                      tDicomSlices &slices,
                      sExtendedTags& tags,
                      bool bLoadImageData = true,
                      bool bIgnoreBitsStoredTag = false) override;

    //TODO this static methods are not implemented in CDicomGDCM, move them as global to CSeries.h?
    static bool loadDicomDCTk2D(const vpl::sys::tString &dir,
                         const std::string &filename,
                         vpl::img::CDicomSlice &slice,
                         sExtendedTags& tags,
                         bool bLoadImageData,
                         int nDesiredBits);

    //! Retrieves data from dicom file tags
    static bool getDicomFileInfo(const vpl::sys::tString &dir, const std::string &filename, int& nFrames);

private:
    //! Pointer to the DCMTk main handle
    DcmFileFormat *m_pHandle;

private:
    //! Computes hash of a null terminated string.
    std::string	hash(const char * input);
};

/*
* Smart pointer.
*/
typedef CDicomDCTk::tSmartPtr  CDicomDCTkPtr;

} // namespace data

#endif // CDicom_H
//loadDicom dat ako static do triedy