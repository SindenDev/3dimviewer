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

#ifndef CDicomSaver_H
#define CDicomSaver_H

//VPL
#include <VPL/Base/Object.h>
#include <VPL/Module/Progress.h>


class CProgress;

namespace data
{

//////////////////////////////////////////////////////////////////////////
//! Dicom dataset saver.
class CDicomSaver : public vpl::base::CObject, public vpl::mod::CProgress
{
public:

    //! Smart pointer declaration
    VPL_SHAREDPTR( CDicomSaver );

    //!Exceptions thrown when saving fails
    VPL_DECLARE_EXCEPTION(CDicomSaverExceptionMS, "Invalid media storage.");
    VPL_DECLARE_EXCEPTION(CDicomSaverExceptionFile, "Failed to create file.");
    VPL_DECLARE_EXCEPTION(CDicomSaverExceptionCompression, "Compression failed.");

public:
    //! Default constructor.
    CDicomSaver(){}

    //! Destructor.
    virtual ~CDicomSaver(){}

    //! Saves current DICOM serie or segmentation (segmentation plugin) in directory dirName.
    //! Serie and segmentation can be saved compressed, anonymized
    virtual bool saveSerie(std::string dirName, bool bSaveSegmented, bool bSaveCompressed,
                           bool bSaveActive, bool bSaveVOI, 
                           bool bAnonymize, std::string anonymString, std::string anonymID, vpl::mod::CProgress::tProgressFunc &progress) = 0;

};

} // namespace data

#endif // CDicomLoader_H
