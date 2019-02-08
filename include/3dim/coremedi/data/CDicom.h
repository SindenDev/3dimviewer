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

#ifndef CDicom_H
#define CDicom_H

//VPL
#include <VPL/Base/Object.h>
#include <VPL/Base/SharedPtr.h>

//for sExtendedTags and exceptions
#include <data/CDicomLoader.h>

// STL
#include <vector>
#include <VPL/System/String.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Class encapsulating DCMTk dicom file containing one or more image frames.

class CDicom : public vpl::base::CObject
{
public:
    //! Smart pointer declaration
    VPL_SHAREDPTR( CDicom );

    //! Compression types.
    enum ECompressionType
    {
        //! Raw data without any compression.
        RAW	        = 0x0,

        //! Lossless JPEG compression.
        LOSSLESS    = 0x1
    };

    //! Vector of dicom slice numbers.
    typedef std::vector<double> tDicomNumList;

public:
    //! Default constructor.
    CDicom();

    //! Constructor parametrized by the filename.
    CDicom( const std::string & file );

    //! Destructor.
    virtual ~CDicom();

    //! Returns true if the dicom file was loaded succesfully.
    bool ok() const  { return m_bOk; }

    //! Returns the original filename.
    const std::string& getFileName() const { return m_sFileName; }

    //! Returns size of a specified file.
    static long getFileSize(const std::string & file);

    //! Loads file by name.
    virtual bool loadFile(const std::string & file) = 0;

    //! Returns patient name, or an empty string on failure.
    virtual std::string getPatientName() = 0;

    //! Returns the study id string, or an empty string on failure.
    virtual std::string getStudyId() = 0;

    //! Returns the serie id string, or an empty string on failure.
    virtual std::string getSerieId() = 0;

    //! Returns number of frames (for multiframe dicoms)
    virtual int	getNumberOfFrames() = 0;

    //! Returns the bits allocated tag value
    virtual int getBitsAllocated() = 0;

    //! Returns the first slice number found in the DICOM file,
    virtual int getSliceId() = 0;

    //! Returns all the slice numbers found in the DICOM file.
    virtual void getSliceIds(tDicomNumList& Numbers) = 0;

    //! Returns the pixel spacing found in the DICOM file.
    virtual double getPixelSpacing() = 0;

    //! Resets the patient name.
    virtual bool setPatientName(const std::string & name) = 0;

    //! Removes all sensitive data and substitutes given string for the patient name.
    virtual bool anonymize(const std::string & name) = 0;

    //! Serializes dicom into a buffer.
    virtual long saveToBuffer(char * buffer, long length) = 0;

    //! Compresses data with losslessJpeg
    virtual bool compressLosslessJPEG() = 0;    
   
    //! Loads a single frame (i.e. slice) from a given dicom file.
    virtual bool loadDicom(const vpl::sys::tString &dir,
                        const std::string &filename,
                        vpl::img::CDicomSlice &slice,
                        sExtendedTags& tags,
                        bool bLoadImageData = true) = 0;

    //! Loads all frames/slices from a given dicom file.
    //! - Returns the number of successfully read images.
    virtual int loadDicom(const vpl::sys::tString &dir,
                        const std::string &filename,
                        tDicomSlices &slices,
                        sExtendedTags& tags,
                        bool bLoadImageData = true,
                        bool bIgnoreBitsStoredTag = false) = 0;

protected:
    
    //! True if file was succesfully loaded.
    bool m_bOk;

    //! Size of the data in bytes
    long m_lSize;

    //! Original filename.
    std::string	m_sFileName;

    //! Used compression type
    unsigned m_Compression;

protected:
    //! Converts the slice position to a slice identifier.
    long long convPos2Id( double a, double b, double c );
};

} // namespace data

#endif // CDicom_H
