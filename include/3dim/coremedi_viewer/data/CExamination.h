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

#ifndef CExamination_H
#define CExamination_H

///////////////////////////////////////////////////////////////////////////////
// include files
#include "data/CStorageInterface.h"
#include <VPL/Image/Vector3.h>
#include <VPL/Image/Size.h>
#include <VPL/Image/Volume.h>
#include <data/storage_ids_core.h>
#include <data/CColorVector.h>
#include <data/CDicomLoader.h>

namespace data
{
	struct SDensityWindow;
	struct SVolumeOfInterest;
	class CCoordinatesConv;
	class CSerieInfo;
	class CDensityData;
    template <typename T> class CColoringFunc;
    typedef CColoringFunc<unsigned char> CColoringFunc4b;

///////////////////////////////////////////////////////////////////////////////
//! Class manages all data for actually processed volume (CT/MR data, slices,
//! segmentation data, density window, volume view, etc.).

class CExamination : public CStorageInterface
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CExamination);

public:
    enum ESubsamplingType
    {
        EST_MAXIMUM_QUALITY = 0,
        EST_MANUAL,
        EST_3D_OPTIMIZED,
    };

    enum ELoadState
    {
        ELS_FAILED = 0,
        ELS_OK,
        ELS_WARNING_GPU_MEMORY,
    };

public:
    //! Default constructor.
    CExamination();

    //! Destructor.
    virtual ~CExamination();

    //! Initializes the storage.
    //! - Registers creation functions of all storage entries.
    void init();


    //! Returns current density window.
    //! - Method can be called directly, or using appropriate signal.
    virtual const SDensityWindow &getDensityWindow(int id);

    //! Calculates optimal density window regarding the active density data.
    //! - Linear contrast enhancement method.
    //! - Method can be called directly, or using appropriate signal.
    virtual SDensityWindow estimateDensityWindow();

    //! Called upon to change the density window.
    //! - Method can be called directly, or using appropriate signal.
    virtual void setDensityWindow(int Center, int Width, int id);

    //! Called upon to change the coloring of density data.
    //! - Method can be called directly, or using appropriate signal.
    virtual void setColoring(data::CColoringFunc4b *pFunc);

    //! Returns type of the coloring function.
    //! - Method can be called directly, or using appropriate signal.
    virtual int getColoringType();


    //! Returns Id of the active density data.
    //! - Method can be called directly, or using appropriate signal.
    virtual int getActiveDataSet();

    //! Sets the active data set.
    virtual void setActiveDataSet(int Id);

    //! Returns Id of the active volume2real coordinates conversion object.
    //! - Method can be called directly, or using appropriate signal.
    virtual int getActiveConv();

    //! Returns copy of the active coordinates conversion object.
    //! - Method can be called directly, or using appropriate signal.
    virtual CCoordinatesConv getActiveConvObject();

    //! Returns copy of the patient data coordinaste conversion object
    //! - Method can be called directly, or using appropriate signal.
    virtual CCoordinatesConv getPatientConvObject();

    //! Returns copy of the auxiliary data coordinate conversion object
    //! - Method can be called directly, or using appropriate signal.
    virtual CCoordinatesConv getAuxConvObject();

    //! Enables region coloring.
    //! - Method can be called directly, or using appropriate signal.
    virtual void enableRegionColoring(bool bEnable);

    //! Returns true if the region coloring is enabled.
    //! - Method can be called directly, or using appropriate signal.
    virtual bool isRegionColoringEnabled();

    //! Sets a region color.
    //! - Method can be called directly, or using appropriate signal.
    virtual void setRegionColor(int i, const CColor4b& Color);

    //! Returns a region color.
    virtual CColor4b getRegionColor(int i);

    //! Enables region coloring.
    //! - Method can be called directly, or using appropriate signal.
    virtual void enableMultiClassRegionColoring(bool bEnable);

    //! Returns true if the region coloring is enabled.
    //! - Method can be called directly, or using appropriate signal.
    virtual bool isMultiClassRegionColoringEnabled();

    //! Sets a region color.
    //! - Method can be called directly, or using appropriate signal.
    virtual void setMultiClassRegionColor(int i, const CColor4b& Color);

    //! Returns a region color.
    virtual CColor4b getMultiClassRegionColor(int i);


    //! Called upon to change slice position.
    //! - Method can be called directly, or using appropriate signal.
    virtual void setSlicePositionXY(int Position);
    virtual void setSlicePositionXZ(int Position);
    virtual void setSlicePositionYZ(int Position);
    virtual void setSlicePositionARB(int Position);
    virtual double getSliceDoublePositionARB();

    //! Called upon to change ortho slices rendering mode.
    //! - Method can be called directly, or using appropriate signal.
    virtual void setSliceModeXY(int Mode);
    virtual void setSliceModeXZ(int Mode);
    virtual void setSliceModeYZ(int Mode);


    //! Loads the density data from a given file.
    virtual bool loadDensityData(const std::string & ssFilename,
                                 vpl::mod::CProgress::tProgressFunc & Progress,
                                 EDataSet Id = PATIENT_DATA
                                 );

    virtual bool loadDensityDataU(const vpl::sys::tString & ssFilename,
                                 vpl::mod::CProgress::tProgressFunc & Progress,
                                 EDataSet Id = PATIENT_DATA
                                 );

    //! Loads the density data from DICOM files.
/*    virtual bool loadDicomData(std::vector<std::string> & DicomFiles,
                               vpl::mod::CProgress::tProgressFunc & Progress,
                               EDataSet Id = PATIENT_DATA
                               );*/

    //! Loads the density data from serie info
    virtual ELoadState loadDicomData(data::CSerieInfo * serie,
                               data::sExtendedTags& tags,
                               vpl::mod::CProgress::tProgressFunc & Progress,
                               EDataSet Id = PATIENT_DATA,
                               ESubsamplingType subsamplingType = EST_MAXIMUM_QUALITY,
                               vpl::img::CVector3d subsampling = vpl::img::CVector3d(1.0, 1.0, 1.0),
                               bool bCompatibilityMode = false // attempts to fix dicom data which are not according to specs
                               );

    //! Saves the density data to given file.
    virtual bool saveDensityData(const std::string & ssFilename,
                                 vpl::mod::CProgress::tProgressFunc & Progress,
                                 EDataSet Id = PATIENT_DATA
                                 );

    virtual bool saveDensityDataU(const vpl::sys::tString & ssFilename,
                                  vpl::mod::CProgress::tProgressFunc & Progress,
                                  EDataSet Id = PATIENT_DATA
                                  );

    //! Changes size of the volume data.
    virtual bool setLimits(SVolumeOfInterest Limits, EDataSet Id = PATIENT_DATA, bool bForce = false);

    //! Function called after data has been loaded
    virtual void onDataLoad( EDataSet Id );

    //! Changes adapter ram information which affects subsampling
    virtual void setAdapterProperties(unsigned long long adapterRAM, unsigned long textureSize2D, unsigned long textureSize3D) { m_adapterRAM = adapterRAM; m_textureSize2D = textureSize2D; m_textureSize3D = textureSize3D; }

private:
    //! Subsamples given volume (returns true if volume should fit into GPU, false otherwise)
    bool subsample(data::CDensityData &densityData, ESubsamplingType subsamplingType, vpl::img::CVector3d& subsampling);

    //! Corrects volume dimensions to a specified integer multiple
    void correctVolumeSize(vpl::img::CDVolume &densityData, vpl::tSize multiple);

    // Corrects limits
    void correctLimits(SVolumeOfInterest &limits, vpl::img::CSize3i volumeSize, vpl::tSize multiple);

private:
    //! Private copy constructor.
    CExamination(const CExamination&);

    //! Private assignment operator.
    CExamination& operator =(const CExamination&);

    //! Adapter ram
    unsigned long long m_adapterRAM;
    //! Max 2D texture size
    unsigned long m_textureSize2D;
    //! Max 3D texture size
    unsigned long m_textureSize3D;
};


} // namespace data

#endif // CExamination_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
