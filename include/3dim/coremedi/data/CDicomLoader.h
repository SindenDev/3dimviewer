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

#ifndef CDicomLoader_H
#define CDicomLoader_H

// VPL
#include <VPL/Base/SharedPtr.h>
#include <VPL/ImageIO/DicomSlice.h>
#include <VPL/Module/Progress.h>
#include <VPL/Base/Exception.h>
#include <VPL/System/String.h>

// STL
#include <string>
#include <set>


namespace data
{
    class CSeries;
}

namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Global definitions.

//! Vector of dicom slices.
typedef std::vector<vpl::img::CDicomSlicePtr> tDicomSlices;

//! Exception thrown when DICOM image loader failes.
VPL_DECLARE_EXCEPTION(CDicomLoadingFailure, "Failed to load DICOM image.")

//! Loads a RTG from a given dicom file.
struct sExtendedTags
{
    double fDistanceSourceToDetector;
    double fDistanceSourceToPatient;
    double fEstimatedRadiographicMagnificationFactor;
    double fGridFocalDistance;
    int iPatientAge;
    double fPatientHeight;
    double fPatientWeight;
};

///////////////////////////////////////////////////////////////////////////////
//! Dicom dataset loader.

class CDicomLoader : public vpl::base::CObject, public vpl::mod::CProgress
{
public:
    //! Smart pointer declaration
    VPL_SHAREDPTR( CDicomLoader );

public:
    //! Default constructor.
    CDicomLoader();

    //! Destructor.
    virtual ~CDicomLoader();

    //! Runs through a directory and finds all dicom series.
    CSeries * preLoadDirectory( const vpl::sys::tString & path );

    //! Runs through a file and finds all dicom series.
    CSeries * preLoadFile( const vpl::sys::tString & path );

    //! Adds yet another dicom file extension.
    void addDicomExtension( const vpl::sys::tString & extension );

    //! Removes dicom file extension.
    void removeDicomExtension( const vpl::sys::tString & extension );

    //! Returns true if a given file extension is valid.
    bool isDicomExtension( const vpl::sys::tString & extension );

    //! Adds an empty extension.
    void allowNoExtension(bool bValue = true);

    //! Allows any extension (this option is enabled by default).
    void allowAnyExtension(bool bValue = true);

    //! Adds an empty extension.
    void allowNumExtension(bool bValue = true);

protected:
    //! Allowed dicom extensions
    std::set< vpl::sys::tString > m_DicomExtensions;

    //! Allows empty extension of dicom files.
    bool m_bAllowNoExtension;

    //! Allows any extension of dicom files that is a number.
    bool m_bAllowNumExtension;

    //! Allows any extension of dicom files.
    bool m_bAllowAnyExtension;
};

} // namespace data

#endif // CDicomLoader_H
