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

#ifndef CDicom_H
#define CDicom_H

#include <VPL/Base/Object.h>
#include <VPL/Base/SharedPtr.h>

// STL
#include <vector>
#include <string>


///////////////////////////////////////////////////////////////////////////////
// Forward declarations

class DcmFileFormat;


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
    typedef std::vector<int> tDicomNumList;

public:
    //! Default constructor.
    CDicom();

    //! Constructor parametrized by the filename.
    CDicom( const std::string & file );

    //! Destructor.
    virtual ~CDicom();

    //! Loads file by name.
    bool loadFile( const std::string & file );

    //! Returns true if the dicom file was loaded succesfully.
    bool ok() { return m_bOk; }

    //! Returns the original filename.
    const std::string& getFileName();


    //! Returns patient name, or an empty string on failure.
    std::string getPatientName();

    //! Returns the study id string, or an empty string on failure.
    std::string getStudyId();

    //! Returns the serie id string, or an empty string on failure.
    std::string getSerieId();

    //! Returns the bits allocated tag value
    int getBitsAllocated();

    //! Returns the first slice number found in the DICOM file,
    int getSliceId();

    //! Returns all the slice numbers found in the DICOM file.
    void getSliceIds(tDicomNumList& Numbers);


    //! Resets the patient name.
    bool setPatientName( const std::string & name );

    //! Removes all sensitive data and substitutes given string for the patient name.
    bool anonymize( const std::string & name );

    //! Serializes dicom into a buffer.
    long saveToBuffer( char * buffer, long length );

    //! Compresses data with losslessJpeg
    bool compressLosslessJPEG();

    //! Returns size of a specified file.
    static long getFileSize( const std::string & file );

protected:
    //! Pointer to the DCMTk main handle
    DcmFileFormat *m_pHandle;

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
    int convPos2Id( double a, double b, double c );

    //! Computes hash of a null terminated string.
    std::string	hash( const char * input );
};


} // namespace data

#endif // CDicom_H
