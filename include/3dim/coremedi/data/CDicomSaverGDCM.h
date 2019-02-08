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

#ifndef CDicomSaverGDCM_H
#define CDicomSaverGDCM_H

#if defined( TRIDIM_USE_GDCM )

#include <data/CDicomSaver.h>

//GDCM
#include <gdcmDataSet.h>
#include <gdcmImageWriter.h>

namespace data
{

//////////////////////////////////////////////////////////////////////////
//! Dicom dataset saver (using GDCM).
class CDicomSaverGDCM : public data::CDicomSaver
{
public:
    //! Default constructor.
    CDicomSaverGDCM();

    //! Destructor.
    ~CDicomSaverGDCM();

    //! Saves current DICOM serie
    virtual bool saveSerie(std::string dirName, bool bSaveSegmented, bool bSaveCompressed,
        bool bSaveActive, bool bSaveVOI, bool bAnonymize, std::string anonymString, std::string anonymID, vpl::mod::CProgress::tProgressFunc & progress) override;
    
    //! Inserts or replaces tag with value val
    template<uint16_t Group, uint16_t Element>
    static void insertDataElement(gdcm::DataSet &ds, const char* val);

    //! Anonymizer, dump mode, just removes certain tags 
    static void anonymize(gdcm::File &file);

    //! Compresses image data 
    static bool compress(gdcm::Image &image);

private:
   
    //! Inserts study date and time
    bool addStudyDateTime(gdcm::DataSet &ds, const char *filename);

    //! Inserts content date and time
    bool addContentDateTime(gdcm::DataSet &ds, const char *filename);

    //! Inserts UIDs, including Media Storage SOP Class UID, Study Instance UID and Serie Instance UID
    bool addUIDs(bool sopclassuid, std::string const & sopclass, std::string const & study_uid, std::string const & series_uid, gdcm::ImageWriter& writer);

    //! Loads pixel data into pData 
    void loadPixelData(int minX, int maxX, int minY, int maxY, int minZ, int maxZ, 
                       bool bSaveSegmented, bool bSaveActive, int16_t* pData);
};

} // namespace data

#endif //TRIDIM_USE_GDCM
#endif // CDicomLoader_H
