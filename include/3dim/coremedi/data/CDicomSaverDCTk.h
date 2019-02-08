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

#ifndef CDicomSaverDCTK_H
#define CDicomSaverDCTK_H

#if !defined( TRIDIM_USE_GDCM )

#include <data/CDicomSaver.h>
#include <VPL/Module/Progress.h>

//forward declaration
namespace Ui
{
    class MainWindow;

}
namespace data
{

//////////////////////////////////////////////////////////////////////////
//! Dicom dataset saver (using DCTk).
class CDicomSaverDCTk : public data::CDicomSaver
{
public:
    //! Default constructor.
    CDicomSaverDCTk();

    //! Destructor.
    ~CDicomSaverDCTk();

    //! Saves current DICOM serie
    virtual bool saveSerie(std::string dirName, bool bSaveSegmented, bool bSaveCompressed,
                           bool bSaveActive, bool bSaveVOI, bool bAnonymize, 
                           std::string anonymString, std::string anonymID, vpl::mod::CProgress::tProgressFunc & progress) override;

};

} // namespace data

#endif // TRIDIM_USE_GDCM
#endif // CDicomSaverDCTK_H
