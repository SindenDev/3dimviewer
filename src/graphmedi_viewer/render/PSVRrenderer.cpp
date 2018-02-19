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

#include <render/PSVRrenderer.h>

#ifdef USE_PSVR

#include <osg/OSGCanvas.h>

#include <render/PSVRosg.h>
#include <render/PSVRshaders.h>
#include <render/glErrorReporting.h>
#include <osg/CSceneManipulator.h>
#include <osg/CSceneOSG.h>

#include <osg/CullFace>
#include <osg/Texture1D>
#include <osg/Texture3D>

#include <VPL/Math/Base.h>
#include <VPL/Math/Random.h>
#include <VPL/System/Sleep.h>
#include <VPL/Image/VolumeFilters/Sobel.h>
#include <VPL/Image/VolumeFilters/Gaussian.h>

#ifdef __APPLE__
    #define LOOKUP_TEXTURE_FORMAT   GL_RGBA12
#else
    #define LOOKUP_TEXTURE_FORMAT   GL_RGBA16
#endif

#define ROUNDING_MASK 0xFFFFFFFC

#define tridimGlR(name, glExp) glExp; { if(!glDebugCallbackReady()){std::string errorString = glGetErrors(name); if (!errorString.empty()) VPL_LOG_ERROR(errorString);} }

namespace PSVR
{

///////////////////////////////////////////////////////////////////////////////
//! sqrt(2)/2
const double Sqrt3Div2 = 0.866025404;

namespace conf
{
    //! Raycasting shaders
    const char *shaderFilenames[] =
        { "shaders2/suma.frag"
        , "shaders2/max.frag"
        , "shaders2/shade.frag"
        , "shaders2/add.frag"
        , "shaders2/surface.frag"
    };

    //! Standard deviation of the random noise generator.
    const double NoiseSigma = 35.0;

    //! Number of quads that forms side of the rendered box.
    const int NumOfQuads = 1;

    // Rendering parameters for different quality levels.
    /////////////////////////////////////////////////////

    //! Rendering resolution.
    const int RenderingSize[PSVolumeRendering::QUALITY_LEVELS]     = { 128, 256, 512, 1024 };

    //! 3D texture sampling step.
    const float TextureSampling[PSVolumeRendering::QUALITY_LEVELS] = { 0.7f, 0.6f, 0.5f, 0.4f };
    //const float TextureSampling[PSVolumeRendering::QUALITY_LEVELS] = { 0.5f, 0.5f, 0.5f, 0.5f };

    //! Input volume data sub-sampling coefficient.
    const float DataSampling[PSVolumeRendering::QUALITY_LEVELS]    = { 0.25f, 0.25f, 0.5f, 1.0f };

    // Rendering parameters while using mouse.
    /////////////////////////////////////////////////////

    //! Rendering resolution (mouse mode).
    const int MouseRenderingSize[PSVolumeRendering::QUALITY_LEVELS]     = { 128, 128, 256, 512 };

    //! 3D texture sampling (mouse mode).
    //const float MouseTextureSampling[PSVolumeRendering::QUALITY_LEVELS] = { 0.8f, 0.8f, 0.8f, 0.8f };
    const float MouseTextureSampling[PSVolumeRendering::QUALITY_LEVELS] = { 0.75f, 0.75f, 0.75f, 0.75f };
}

//! Parameters of the rendering.
struct PSVolumeRendering::PSVolumeRenderingParams
{
    ////////////////////////////////////////////////////////////////////////////
    // 3D volume information

    //! Volume data size in voxels.
    vpl::tSize XSize, YSize, ZSize;
    vpl::tSize CustomXSize, CustomYSize, CustomZSize;

    //! Real voxel size.
    float dX, dY, dZ;

    //! Real volume size (num. of voxels * real voxel size).
    float RealXSize, RealYSize, RealZSize;

    //! Aspect ratio.
    float aspectRatio_YtoX, aspectRatio_ZtoX;

    ////////////////////////////////////////////////////////////////////////////
    // Skipping volume information

    //! Skipping volume size
    vpl::tSize AuxXSize, AuxYSize, AuxZSize;
    vpl::tSize CustomAuxXSize, CustomAuxYSize, CustomAuxZSize;

    //! Skipping volume texture size
    float AuxTexXSize, AuxTexYSize, AuxTexZSize;
    float CustomAuxTexXSize, CustomAuxTexYSize, CustomAuxTexZSize;

    ////////////////////////////////////////////////////////////////////////////
    // Volume rendering parameters

    //! Intensity shifting and scaling.
    float dataPreMultiplication, dataOffset;

    //! Output image contrast and brightness.
    float imageContrast, imageBrightness;

    //! Surface detection parameters.
    float surfaceNormalMult, surfaceNormalExp;

    //! 3D texture sampling step along the ray...
    float volumeSamplingDistance;

    //! Cutting plane...
    float planeA, planeB, planeC, planeD, planeDeltaNear, planeDeltaFar;

    ////////////////////////////////////////////////////////////////////////////
    // Volume rendering state variables

    //! Current rendering quality.
    int currentQuality;

    //! Rendering resolution.
    int renderingSize;

    //! Currently selected shader.
    EShaders selectedShader;

    //! Currently selected color LUT.
    ELookups selectedLut;

    //! Inverted matrices for object-space position reconstruction of VR box
    osg::Matrix invProjectionMatrix;
    osg::Matrix invModelViewMatrix;
};

///////////////////////////////////////////////////////////////////////////////
//! constructor - initialize main variables
PSVolumeRendering::PSVolumeRendering()
    : m_pCanvas(NULL)
    , m_Enabled(false)
    , m_Error(DATA_NOT_SPECIFIED)
    , m_Flags(0)
    , m_FailureCounter(0)
    , m_Mutex(false)
    , m_Thread(setupLoop, this, false)
    , m_spParams(new PSVolumeRenderingParams)
    , m_spVolumeData(NULL)
{
    m_Thread.resume();

    // Initialize volume and surface data
    m_VolumeData.resize(INIT_SIZE, INIT_SIZE, INIT_SIZE, 0);
    m_VolumeData.fillEntire(0);

    m_spParams->AuxXSize = m_spParams->AuxYSize = m_spParams->AuxZSize = INIT_SIZE / 8;
    m_AuxVolumeData.resize(m_spParams->AuxXSize, m_spParams->AuxYSize, m_spParams->AuxZSize, 0);
    m_AuxVolumeData.fillEntire(vpl::img::tRGBPixel(0));
    m_spParams->AuxTexXSize = m_spParams->AuxTexYSize = m_spParams->AuxTexZSize = 1.0f;

    // Initialize rendering parameters
    m_spParams->XSize = m_spParams->YSize = m_spParams->ZSize = INIT_SIZE;
    m_spParams->CustomXSize = m_spParams->CustomYSize = m_spParams->CustomZSize = 0;
    m_spParams->RealXSize = m_spParams->RealYSize = m_spParams->RealZSize = INIT_SIZE;
    m_spParams->dX = m_spParams->dY = m_spParams->dZ = 1.0f;
    m_spParams->aspectRatio_YtoX = m_spParams->aspectRatio_ZtoX = 1.0f;

    m_spParams->AuxXSize = m_spParams->AuxYSize = m_spParams->AuxZSize = 0;
    m_spParams->CustomAuxXSize = m_spParams->CustomAuxYSize = m_spParams->CustomAuxZSize = 0;
    m_spParams->AuxTexXSize = m_spParams->AuxTexYSize = m_spParams->AuxTexZSize = 0;
    m_spParams->CustomAuxTexXSize = m_spParams->CustomAuxTexYSize = m_spParams->CustomAuxTexZSize = 0;

    m_spParams->dataPreMultiplication = 1.0f;
    m_spParams->dataOffset = 0.0f;

    m_spParams->imageContrast = 1.0f;
    m_spParams->imageBrightness = 0.0f;

    m_spParams->surfaceNormalMult = 15.0f;
    m_spParams->surfaceNormalExp = 2.0f;

    m_spParams->planeA = 1.0;
    m_spParams->planeB = 0.0;
    m_spParams->planeC = 0.0;
    m_spParams->planeD = -0.5;
    m_spParams->planeDeltaNear = 1.0;
    m_spParams->planeDeltaFar = -1.0;

    m_spParams->currentQuality = LOW_QUALITY;
    m_spParams->volumeSamplingDistance = conf::TextureSampling[LOW_QUALITY];
    m_spParams->renderingSize = conf::RenderingSize[LOW_QUALITY];

    m_spParams->selectedShader = EShaders::SURFACE;
    m_spParams->selectedLut = ELookups::SURFACE_BONE;

    // Prepare the box bounding volume data
    prepareBox(conf::NumOfQuads);
    prepareQuad();

    // create default lookup tables
    createLookupTables();

	noteMatrixSignalConnection = VPL_SIGNAL(SigNewTransformMatrixFromNote).connect(this, &PSVolumeRendering::setNewTransformMatrix);
}

void PSVolumeRendering::setNewTransformMatrix(osg::Matrix& newTransformMatrix, double distance) {

    auto cameraManipulator = dynamic_cast<osg::CSceneManipulator *>(getCanvas()->getView()->getCameraManipulator());

    if (cameraManipulator == nullptr)
        return;

    cameraManipulator->customSetPosition(newTransformMatrix, distance);

    this->redraw();
}

PSVolumeRendering::~PSVolumeRendering()
{
    // Stop the rendering
    release();
}

QSize PSVolumeRendering::getWindowSize()
{
    return m_pCanvas->size();
}

osg::Matrix PSVolumeRendering::getWorldMatrix()
{
    osg::Matrix retMatrix = osg::Matrix::identity();

    scene::CScene3D *vrScene = dynamic_cast<scene::CScene3D*>(getCanvas()->getView()->getSceneData());
    if (vrScene == NULL)
    {
        return retMatrix;
    }

    PSVR::osgPSVolumeRenderingGeode *vrGeode = dynamic_cast<PSVR::osgPSVolumeRenderingGeode *>(vrScene->getVRGeode());
    if (vrGeode == NULL)
    {
        return retMatrix;
    }
    
    osg::Vec3 scale = osg::Vec3(getRealXSize(), getRealYSize(), getRealZSize());
    scale *= 0.5f;
    osg::Matrix scaleMatrix = osg::Matrix::scale(scale);
    retMatrix = scaleMatrix * vrGeode->getWorldMatrices()[0];
    
    return retMatrix;
}

osg::Matrix PSVolumeRendering::getViewMatrix()
{
    return getCanvas()->getView()->getCamera()->getViewMatrix();
}

osg::Matrix PSVolumeRendering::getProjectionMatrix()
{
    return getCanvas()->getView()->getCamera()->getProjectionMatrix();
}

osg::Matrix PSVolumeRendering::getTransformMatrix()
{
    return getWorldMatrix() * getViewMatrix() * getProjectionMatrix();
}

void PSVolumeRendering::internalCreateCustomShader(std::string vertexShaderSource, std::string fragmentShaderSource)
{ 
    m_shaders[EShaders::CUSTOM] = new osg::Program();
    m_shaders[EShaders::CUSTOM]->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
    m_shaders[EShaders::CUSTOM]->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
}

void PSVolumeRendering::internalSetDataToCustomVolume()
{
    tLock(*this);
    m_Error &= (0xffff - CUSTOM_DATA_NOT_SPECIFIED);
    setAndSignalFlag(CUSTOM_DATA_INVALID);
}


void PSVolumeRendering::resetLookupTables()
{
    createLookupTables();
    updateLookupTables();
}

void PSVolumeRendering::updateLookupTables(std::string lutName)
{
    std::vector<std::pair<ELookups, std::string> > luts;
    luts.push_back(std::make_pair(ELookups::MIP_SOFT,     "MIP_SOFT"));
    luts.push_back(std::make_pair(ELookups::MIP_HARD,     "MIP_HARD"));
    luts.push_back(std::make_pair(ELookups::XRAY_SOFT,    "XRAY_SOFT"));
    luts.push_back(std::make_pair(ELookups::XRAY_HARD,    "XRAY_HARD"));
    luts.push_back(std::make_pair(ELookups::SHA_AIR,      "SHA_AIR"));
    luts.push_back(std::make_pair(ELookups::SHA_TRAN,     "SHA_TRAN"));
    luts.push_back(std::make_pair(ELookups::SHA_BONE0,    "SHA_BONE0"));
    luts.push_back(std::make_pair(ELookups::SHA_BONE1,    "SHA_BONE1"));
    luts.push_back(std::make_pair(ELookups::SHA_BONE2,    "SHA_BONE2"));
    luts.push_back(std::make_pair(ELookups::SURFACE_SKIN, "SURFACE_SKIN"));
    luts.push_back(std::make_pair(ELookups::SURFACE_BONE, "SURFACE_BONE"));

    #pragma omp parallel for
    for (int i = 0; i < luts.size(); ++i)
    {
        if ((lutName.empty()) || (lutName == m_lookupTables[luts[i].second].name()))
        {
            updateLookupTable(m_lookupTables[luts[i].second], m_internalLookupTables[static_cast<int>(luts[i].first)], m_skipConditions[static_cast<int>(luts[i].first)]);
        }
    }

    setLut(getLut());
}

void PSVolumeRendering::setCustomShaderUniforms(std::vector<osg::ref_ptr<osg::Uniform>>&& uniforms)
{
    m_customUniforms = std::move(uniforms);
}

void PSVolumeRendering::updateLookupTable(CLookupTable &lookupTable, unsigned short *internalLookupTable, osg::Vec4 &skipCondition)
{
    skipCondition[0] = 1.0;
    skipCondition[1] = 0.0;
    skipCondition[2] = 1.0;
    skipCondition[3] = 0.0;

    for (int y = LUT_2D_H - 1; y >= 0; --y)
    {
        for (int x = LUT_2D_W - 1; x >= 0; --x)
        {
            osg::Vec2 position = osg::Vec2(double(x) / double(LUT_2D_W), double(y) / double(LUT_2D_H));
            osg::Vec4 color = lookupTable.color(position);

            internalLookupTable[4 * (y * LUT_2D_W + x) + 0] = static_cast<unsigned short>(color.r() * 65535.0);
            internalLookupTable[4 * (y * LUT_2D_W + x) + 1] = static_cast<unsigned short>(color.g() * 65535.0);
            internalLookupTable[4 * (y * LUT_2D_W + x) + 2] = static_cast<unsigned short>(color.b() * 65535.0);
            internalLookupTable[4 * (y * LUT_2D_W + x) + 3] = static_cast<unsigned short>((1.0 - color.a()) * 65535.0);

            if (color.a() > 0.0f)
            {
                skipCondition[0] = std::min(skipCondition[0], position[0]);
                skipCondition[1] = std::max(skipCondition[1], position[0]);
                skipCondition[2] = std::min(skipCondition[2], position[1]);
                skipCondition[3] = std::max(skipCondition[3], position[1]);
            }
        }
    }
}

float PSVolumeRendering::getRealXSize() const
{
    return m_spParams->RealXSize;
}

float PSVolumeRendering::getRealYSize() const
{
    return m_spParams->RealYSize;
}

float PSVolumeRendering::getRealZSize() const
{
    return m_spParams->RealZSize;
}

void PSVolumeRendering::redraw(bool bEraseBackground)
{
    if (m_pCanvas)
    {
        m_pCanvas->Refresh(bEraseBackground);
    }
}

void PSVolumeRendering::enable(bool bEnable)
{
    m_Enabled = bEnable;

    // signal others that VR has been enabled
    VPL_SIGNAL(SigVREnabledChange).invoke(bEnable);

    // redraw
    if (m_pCanvas)
    {
        m_pCanvas->Refresh(false);
    }
}

//! Returns true of the rendering is enabled.

inline bool PSVolumeRendering::isEnabled() const { return m_Enabled; }

///////////////////////////////////////////////////////////////////////////////
// take CDensityVolume and copy data and information
void PSVolumeRendering::uploadData(vpl::img::CDensityVolume * pData)
{
    if (!pData)
    {
        return;
    }

    tLock Lock(*this);

    // Store reference to the data
	vpl::img::CDensityVolume *workingPtr;

	int datasetID = data::PATIENT_DATA;
	{	// get active data set
		data::CObjectPtr<data::CActiveDataSet> spDataSet(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
		datasetID = spDataSet->getId();
	}
    data::CObjectPtr<data::CDensityData> spVolumeData(APP_STORAGE.getEntry(datasetID));
    workingPtr = spVolumeData.get();

    m_spParams->RealXSize = float(workingPtr->getXSize() * workingPtr->getDX());
    m_spParams->RealYSize = float(workingPtr->getYSize() * workingPtr->getDY());
    m_spParams->RealZSize = float(workingPtr->getZSize() * workingPtr->getDZ());
    m_Error &= (0xffff - DATA_NOT_SPECIFIED);

    m_pCanvas->getView()->home();

    setAndSignalFlag(DATA_INVALID | LUT_INVALID | OSR_INVALID);
}

///////////////////////////////////////////////////////////////////////////////
//! - Updates specified surface
void PSVolumeRendering::updateRenderTargets()
{
    setFlag(OSR_INVALID);
}

void PSVolumeRendering::createLookupTables()
{
    m_internalLookupTables.clear();
    m_skipConditions.clear();
    for (int i = 0; i < static_cast<int>(ELookups::LOOKUPS_COUNT); ++i)
    {
        m_internalLookupTables.push_back(new unsigned short[4 * LUT_2D_W * LUT_2D_H]);
        m_skipConditions.push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
    }

    CLookupTable &mipSoft = m_lookupTables["MIP_SOFT"];
    CLookupTable &mipHard = m_lookupTables["MIP_HARD"];
    CLookupTable &xraySoft = m_lookupTables["XRAY_SOFT"];
    CLookupTable &xrayHard = m_lookupTables["XRAY_HARD"];
    CLookupTable &shadingAir = m_lookupTables["SHA_AIR"];
    CLookupTable &shadingTransparent = m_lookupTables["SHA_TRAN"];
    CLookupTable &shadingBone0 = m_lookupTables["SHA_BONE0"];
    CLookupTable &shadingBone1 = m_lookupTables["SHA_BONE1"];
    CLookupTable &shadingBone2 = m_lookupTables["SHA_BONE2"];
    CLookupTable &surfaceSkin = m_lookupTables["SURFACE_SKIN"];
    CLookupTable &surfaceBone = m_lookupTables["SURFACE_BONE"];

    mipSoft.setName("MIP soft");
    mipSoft.clear();
    mipSoft.addComponent();
    mipSoft.setName(0, "component0");
    mipSoft.addPoint(0, osg::Vec2d(0.000, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);
    mipSoft.addPoint(0, osg::Vec2d(0.123, 0.0), osg::Vec4(0.870, 0.705, 0.262, 1.000), true, false, 0.0);
    mipSoft.addPoint(0, osg::Vec2d(0.246, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);

    mipHard.setName("MIP hard");
    mipHard.clear();
    mipHard.addComponent();
    mipHard.setName(0, "component0");
    mipHard.addPoint(0, osg::Vec2d(0.174, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);
    mipHard.addPoint(0, osg::Vec2d(0.533, 0.0), osg::Vec4(0.988, 0.988, 0.988, 1.000), true, false, 0.0);
    mipHard.addPoint(0, osg::Vec2d(1.000, 0.0), osg::Vec4(1.000, 1.000, 1.000, 1.000), true, false, 0.0);

    xraySoft.setName("X-ray soft");
    xraySoft.clear();
    xraySoft.addComponent();
    xraySoft.setName(0, "component0");
    xraySoft.addPoint(0, osg::Vec2d(0.038, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);
    xraySoft.addPoint(0, osg::Vec2d(0.130, 0.0), osg::Vec4(0.341, 0.266, 0.101, 0.139), true, false, 0.0);
    xraySoft.addPoint(0, osg::Vec2d(0.309, 0.0), osg::Vec4(0.015, 0.039, 0.109, 0.000), true, false, 0.0);
    xraySoft.addPoint(0, osg::Vec2d(0.310, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);

    xrayHard.setName("X-ray hard");
    xrayHard.clear();
    xrayHard.addComponent();
    xrayHard.setName(0, "component0");
    xrayHard.addPoint(0, osg::Vec2d(0.147, 0.0), osg::Vec4(0.000, 0.000, 0.000, 1.000), true, false, 0.0);
    xrayHard.addPoint(0, osg::Vec2d(0.148, 0.0), osg::Vec4(0.043, 0.070, 0.101, 1.000), true, false, 0.0);
    xrayHard.addPoint(0, osg::Vec2d(1.000, 0.0), osg::Vec4(1.000, 1.000, 1.000, 1.000), true, false, 0.0);

    shadingAir.setName("Shading - air");
    shadingAir.clear();
    shadingAir.addComponent();
    shadingAir.setName(0, "component0");
    shadingAir.addPoint(0, osg::Vec2d(0.105, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);
    shadingAir.addPoint(0, osg::Vec2d(0.106, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.040), true, false, 0.0);
    shadingAir.addPoint(0, osg::Vec2d(0.139, 0.0), osg::Vec4(0.407, 0.988, 0.960, 0.204), true, false, 0.0);
    shadingAir.addPoint(0, osg::Vec2d(0.162, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);

    shadingTransparent.setName("Shading - transparent");
    shadingTransparent.clear();
    shadingTransparent.addComponent();
    shadingTransparent.setName(0, "component0");
    shadingTransparent.addPoint(0, osg::Vec2d(0.071, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);
    shadingTransparent.addPoint(0, osg::Vec2d(0.129, 0.0), osg::Vec4(0.988, 0.000, 0.000, 0.034), true, false, 0.0);
    shadingTransparent.addPoint(0, osg::Vec2d(0.181, 0.0), osg::Vec4(0.952, 0.968, 0.019, 0.034), true, false, 0.0);
    shadingTransparent.addPoint(0, osg::Vec2d(0.223, 0.0), osg::Vec4(0.082, 0.980, 0.000, 0.051), true, false, 0.0);
    shadingTransparent.addPoint(0, osg::Vec2d(0.273, 0.0), osg::Vec4(0.529, 1.000, 0.952, 0.170), true, false, 0.0);
    shadingTransparent.addPoint(0, osg::Vec2d(0.412, 0.0), osg::Vec4(0.788, 0.843, 1.000, 1.000), true, false, 0.0);
    shadingTransparent.addPoint(0, osg::Vec2d(1.000, 0.0), osg::Vec4(0.007, 0.027, 0.980, 1.000), true, false, 0.0);

    shadingBone0.setName("Shading - bone (skull)");
    shadingBone0.clear();
    shadingBone0.addComponent();
    shadingBone0.setName(0, "bone");
    shadingBone0.setAlphaFactor(0, 0.172);
    shadingBone0.addPoint(0, osg::Vec2d(0.265, 0.0), osg::Vec4(1.000, 0.992, 0.949, 0.000), true, false, 0.0);
    shadingBone0.addPoint(0, osg::Vec2d(0.273, 0.0), osg::Vec4(1.000, 0.988, 0.917, 1.000), true, false, 0.0);
    shadingBone0.addComponent();
    shadingBone0.setName(1, "skin");
    shadingBone0.setAlphaFactor(1, 0.028);
    shadingBone0.addPoint(1, osg::Vec2d(0.126, 0.0), osg::Vec4(1.000, 0.498, 0.498, 0.000), true, false, 0.0);
    shadingBone0.addPoint(1, osg::Vec2d(0.170, 0.0), osg::Vec4(1.000, 0.498, 0.498, 1.000), true, false, 0.0);
    shadingBone0.addPoint(1, osg::Vec2d(0.223, 0.0), osg::Vec4(1.000, 0.498, 0.498, 0.000), true, false, 0.0);

    shadingBone1.setName("Shading - bone (spine)");
    shadingBone1.clear();
    shadingBone1.addComponent();
    shadingBone1.setName(0, "bone");
    shadingBone1.setAlphaFactor(0, 0.217);
    shadingBone1.addPoint(0, osg::Vec2d(0.190, 0.0), osg::Vec4(1.000, 0.992, 0.949, 0.000), true, false, 0.0);
    shadingBone1.addPoint(0, osg::Vec2d(0.201, 0.0), osg::Vec4(1.000, 0.988, 0.917, 1.000), true, false, 0.0);
    shadingBone1.addComponent();
    shadingBone1.setName(1, "skin");
    shadingBone1.setAlphaFactor(1, 0.028);
    shadingBone1.addPoint(1, osg::Vec2d(0.126, 0.0), osg::Vec4(1.000, 0.498, 0.498, 0.000), true, false, 0.0);
    shadingBone1.addPoint(1, osg::Vec2d(0.170, 0.0), osg::Vec4(1.000, 0.498, 0.498, 1.000), true, false, 0.0);
    shadingBone1.addPoint(1, osg::Vec2d(0.223, 0.0), osg::Vec4(1.000, 0.498, 0.498, 0.000), true, false, 0.0);

    shadingBone2.setName("Shading - bone (pelvis)");
    shadingBone2.clear();
    shadingBone2.addComponent();
    shadingBone2.setName(0, "bone");
    shadingBone2.setAlphaFactor(0, 0.172);
    shadingBone2.addPoint(0, osg::Vec2d(0.184, 0.0), osg::Vec4(1.000, 0.992, 0.949, 0.000), true, false, 0.0);
    shadingBone2.addPoint(0, osg::Vec2d(0.198, 0.0), osg::Vec4(1.000, 0.988, 0.917, 1.000), true, false, 0.0);
    shadingBone2.addComponent();
    shadingBone2.setName(1, "skin");
    shadingBone2.setAlphaFactor(1, 0.037);
    shadingBone2.addPoint(1, osg::Vec2d(0.111, 0.0), osg::Vec4(1.000, 0.498, 0.498, 0.000), true, false, 0.0);
    shadingBone2.addPoint(1, osg::Vec2d(0.149, 0.0), osg::Vec4(1.000, 0.498, 0.498, 1.000), true, false, 0.0);
    shadingBone2.addPoint(1, osg::Vec2d(0.193, 0.0), osg::Vec4(1.000, 0.498, 0.498, 0.000), true, false, 0.0);

    surfaceSkin.setName("Surface - skin");
    surfaceSkin.clear();
    surfaceSkin.addComponent();
    surfaceSkin.setName(0, "component0");
    surfaceSkin.addPoint(0, osg::Vec2d(0.000, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);
    surfaceSkin.addPoint(0, osg::Vec2d(0.102, 0.0), osg::Vec4(1.000, 0.996, 0.988, 1.000), true, false, 0.0);
    surfaceSkin.addPoint(0, osg::Vec2d(1.000, 0.0), osg::Vec4(0.498, 0.498, 0.498, 1.000), true, false, 0.0);

    surfaceBone.setName("Surface - bones");
    surfaceBone.clear();
    surfaceBone.addComponent();
    surfaceBone.setName(0, "component0");
    surfaceBone.addPoint(0, osg::Vec2d(0.172, 0.0), osg::Vec4(0.000, 0.000, 0.000, 0.000), true, false, 0.0);
    surfaceBone.addPoint(0, osg::Vec2d(0.173, 0.0), osg::Vec4(0.380, 0.258, 0.066, 0.000), true, false, 0.0);
    surfaceBone.addPoint(0, osg::Vec2d(0.269, 0.0), osg::Vec4(1.000, 1.000, 1.000, 1.000), true, false, 0.0);

    updateLookupTables();
}

///////////////////////////////////////////////////////////////////////////////
// deallocate all used resources
void PSVolumeRendering::release()
{
    for (std::size_t i = 0; i < m_internalLookupTables.size(); ++i)
    {
        delete[] m_internalLookupTables[i];
    }
    m_internalLookupTables.clear();

    m_Thread.terminate(true);

    tLock Lock(*this);

    m_Enabled = false;
    m_Error = DATA_NOT_SPECIFIED;

    // Clear all flags
    m_Flags = PSVR_NO_FLAGS;
}

///////////////////////////////////////////////////////////////////////////////
// Faster gauss filter implementation
float getFilteredVal(vpl::img::CDensityVolume * pVolume, int x, int y, int z)
{
    vpl::tSize zSums[3] = { };
    vpl::tSize xOff = pVolume->getXOffset();
    for (int dz = -1; dz <= 1; dz++)
    {
        vpl::tSize ySums[3] = { };
        for (int dy = -1; dy <= 1; dy++)
        {
            vpl::tSize idx = pVolume->getIdx(x, y + dy, z + dz);
            ySums[dy + 1] = pVolume->at(idx - xOff) + 3 * pVolume->at(idx) + pVolume->at(idx + xOff);
        }
        zSums[dz + 1] = ySums[0] + 3 * ySums[1] + ySums[2];
    }
    //float val = pVolume->at(x,y,z);
    float sum = (zSums[0] + 3 * zSums[1] + zSums[2]) / 125.0;
    return sum;
}

bool PSVolumeRendering::internalUploadData()
{
	int datasetID = data::PATIENT_DATA;
	{	// get active data set
		data::CObjectPtr<data::CActiveDataSet> spDataSet(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
		datasetID = spDataSet->getId();
	}
    data::CObjectPtr<data::CDensityData> spVolumeData(APP_STORAGE.getEntry(datasetID));
    vpl::img::CDensityVolume *workingPtr = spVolumeData.get();

    if (!workingPtr || workingPtr->getZSize() <= 0)
    {
        return false;
    }

    tLock Lock(*this);

    // Estimate volume dimensions
    m_spParams->XSize = workingPtr->getXSize();
    m_spParams->YSize = workingPtr->getYSize();
    m_spParams->ZSize = workingPtr->getZSize();

    // Real voxel size
    m_spParams->dX = workingPtr->getDX();
    m_spParams->dY = workingPtr->getDY();
    m_spParams->dZ = workingPtr->getDZ();

    // Data sub-sampling coeff
    float SubSampling = conf::DataSampling[m_spParams->currentQuality];

    // Sub-sampling...
    //if (SubSampling < 1.0f)
    if (SubSampling != 1.0f)
    {
        m_spParams->XSize = vpl::math::round2Int(float(m_spParams->XSize) * SubSampling);
        m_spParams->YSize = vpl::math::round2Int(float(m_spParams->YSize) * SubSampling);
        m_spParams->ZSize = vpl::math::round2Int(float(m_spParams->ZSize) * SubSampling);

        float InvSubSampling = 1.0f / SubSampling;
        m_spParams->dX *= InvSubSampling;
        m_spParams->dY *= InvSubSampling;
        m_spParams->dZ *= InvSubSampling;
    }

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->XSize &= ROUNDING_MASK;
    m_spParams->YSize &= ROUNDING_MASK;
    m_spParams->ZSize &= ROUNDING_MASK;

    // Voxel size correction for even texture size
    vpl::img::CVector3d realVolumeSize;
    realVolumeSize.x() = workingPtr->getDX() * workingPtr->getXSize();
    realVolumeSize.y() = workingPtr->getDY() * workingPtr->getYSize();
    realVolumeSize.z() = workingPtr->getDZ() * workingPtr->getZSize();
    m_spParams->dX = realVolumeSize.x() / m_spParams->XSize;
    m_spParams->dY = realVolumeSize.y() / m_spParams->YSize;
    m_spParams->dZ = realVolumeSize.z() / m_spParams->ZSize;

    // Allocate the data
    try
    {
        m_VolumeData.resize(m_spParams->XSize, m_spParams->YSize, m_spParams->ZSize, 0);
    }
    catch (std::bad_alloc &e)
    {
        VPL_LOG_INFO("Exception: VR cannot create volume (" << e.what() << ")");

        SubSampling = 1.0;
        m_spParams->XSize = m_spParams->YSize = m_spParams->ZSize = vpl::math::getMin<int>(INIT_SIZE, m_spParams->XSize, m_spParams->YSize, m_spParams->ZSize);
        m_spParams->dX = m_spParams->dY = m_spParams->dZ = 1.0;
        m_VolumeData.resize(m_spParams->XSize, m_spParams->YSize, m_spParams->ZSize, 0);
    }

    // Copy volume shape
    m_spParams->aspectRatio_YtoX = m_spParams->dY / m_spParams->dX;
    m_spParams->aspectRatio_ZtoX = m_spParams->dZ / m_spParams->dX;

    // Skipping volume
    m_spParams->AuxXSize = (m_spParams->XSize + 7) / 8 + 3;
    m_spParams->AuxYSize = (m_spParams->YSize + 7) / 8 + 3;
    m_spParams->AuxZSize = (m_spParams->ZSize + 7) / 8 + 3;

    // Even texture size...
    m_spParams->AuxXSize &= ROUNDING_MASK;
    m_spParams->AuxYSize &= ROUNDING_MASK;
    m_spParams->AuxZSize &= ROUNDING_MASK;
    
    // Allocate the skipping volume
    m_AuxVolumeData.resize(m_spParams->AuxXSize, m_spParams->AuxYSize, m_spParams->AuxZSize, 0);
    m_spParams->AuxTexXSize = float(m_spParams->XSize) / float(m_spParams->AuxXSize * 8);
    m_spParams->AuxTexYSize = float(m_spParams->YSize) / float(m_spParams->AuxYSize * 8);
    m_spParams->AuxTexZSize = float(m_spParams->ZSize) / float(m_spParams->AuxZSize * 8);

    // Clear the volume
    m_AuxVolumeData.fillEntire(vpl::img::tRGBPixel(0));

    // Scaling factor
    static const vpl::img::tDensityPixel voxelMax = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMax();
    static const vpl::img::tDensityPixel voxelMin = vpl::img::CPixelTraits<vpl::img::tDensityPixel>::getPixelMin();

    static const float dataScale = 65536.0f / float(voxelMax - voxelMin);
    static const float skipScale = 255.0f / 65536.0f;

    // Check if the data are empty (i.e. reset of the storage, etc.)
    vpl::img::tDensityPixel MaxValue = vpl::img::getMax<vpl::img::tDensityPixel>(*workingPtr);
    bool bEmptyData = (MaxValue == voxelMin) ? true : false;

    // No need to interpolate and copy the data if they are empty
    if (bEmptyData)
    {
        // Just clear the internal volume
        m_VolumeData.fillEntire(vpl::img::CVolume<vpl::img::tPixel16>::tVoxel(0));
    }
    else
    {
        // Copy (and interpolate) data voxel by voxel
        //if (SubSampling < 1.0f)
        if (SubSampling != 1.0f)
        {
            double XStep = double(workingPtr->getXSize() - 1) / (m_spParams->XSize - 1);
            double YStep = double(workingPtr->getYSize() - 1) / (m_spParams->YSize - 1);
            double ZStep = double(workingPtr->getZSize() - 1) / (m_spParams->ZSize - 1);
#pragma omp parallel for schedule(static) default(shared)
            for (vpl::tSize z = 0; z < m_spParams->ZSize; z++)
            {
                vpl::img::CPoint3D Point(0.0, 0.0, z * ZStep);
                for (vpl::tSize y = 0; y < m_spParams->YSize; y++, Point.y() += YStep)
                {
                    Point.x() = 0.0;
                    for (vpl::tSize x = 0; x < m_spParams->XSize; x++, Point.x() += XStep)
                    {
                        auto normalizedPixel = vpl::img::CVolume<vpl::img::tPixel16>::tVoxel(float(workingPtr->interpolate(Point) - voxelMin) * dataScale);
                        m_VolumeData(x, y, z) = normalizedPixel;
                    }
                }
            }
        }

        // No sub-sampling is required...
        else
        {
//#define MDSTK_GAUSS
#ifdef MDSTK_GAUSS
            // Gaussian filter
            vpl::img::CVolumeGauss3Filter<vpl::img::CDensityVolume> GaussFilter;
            // initialize kernel (before multiple threads kick in)
            float tmpPixel = float(GaussFilter.getResponse(*workingPtr, 0, 0, 0));
            m_VolumeData(0, 0, 0) = tmpPixel;
#endif

#pragma omp parallel for schedule(static) default(shared)
            for (vpl::tSize z = 0; z < m_spParams->ZSize; z++)
            {
                for (vpl::tSize y = 0; y < m_spParams->YSize; y++)
                {
                    for (vpl::tSize x = 0; x < m_spParams->XSize; x++)
                    {
#ifdef MDSTK_GAUSS
                        float Pixel = float(GaussFilter.getResponse(*workingPtr, x, y, z));
#else
                        float Pixel = float(getFilteredVal(workingPtr, x, y, z));
#endif
                        auto normalizedPixel = vpl::img::CVolume<vpl::img::tPixel16>::tVoxel((Pixel - voxelMin) * dataScale);
                        m_VolumeData(x, y, z) = normalizedPixel;
                   }
                }
            }
        }

        // Sobel filters
        vpl::img::CVolumeSobelX<vpl::img::CVolume<vpl::img::tPixel16>> SobelX;
        vpl::img::CVolumeSobelY<vpl::img::CVolume<vpl::img::tPixel16>> SobelY;
        vpl::img::CVolumeSobelZ<vpl::img::CVolume<vpl::img::tPixel16>> SobelZ;

        // Gradient normalization
        float gradScale = 0.33f * skipScale;

        // Skipping volume - min/max
        vpl::img::tRGBPixel::tComponent pixelMin = vpl::img::CPixelTraits<vpl::img::tRGBPixel>::getPixelMin().r();
        vpl::img::tRGBPixel::tComponent pixelMax = vpl::img::CPixelTraits<vpl::img::tRGBPixel>::getPixelMax().r();

#pragma omp parallel for schedule(static,8) default(shared)
        for (vpl::tSize z = 0; z < m_spParams->ZSize; z++)
        {
            vpl::tSize sz = z / 8;
            for (vpl::tSize y = 0; y < m_spParams->YSize; y++)
            {
                vpl::tSize sy = y / 8;
                for (vpl::tSize x = 0; x < m_spParams->XSize; x++)
                {
                    vpl::tSize sx = x / 8;
                    vpl::img::tRGBPixel& RGBPixel = m_AuxVolumeData(sx, sy, sz);

                    // Min/Max
                    vpl::img::tRGBPixel::tComponent SkipPixel = vpl::img::tRGBPixel::tComponent(skipScale * float(m_VolumeData(x, y, z)));
                    RGBPixel.r() = vpl::math::getMin<vpl::img::tRGBPixel::tComponent>(RGBPixel.r(), vpl::math::getMax<vpl::img::tRGBPixel::tComponent>(SkipPixel - 1, pixelMin));
                    RGBPixel.g() = vpl::math::getMax<vpl::img::tRGBPixel::tComponent>(RGBPixel.g(), vpl::math::getMin<vpl::img::tRGBPixel::tComponent>(SkipPixel + 1, pixelMax));
                }
            }
        }

        // Skipping volume - gradient magnitude
#pragma omp parallel for schedule(static,8) default(shared)
        for (vpl::tSize z = 1; z < (m_spParams->ZSize - 1); z += 2)
        {
            vpl::tSize sz = z / 8;
            for (vpl::tSize y = 1; y < (m_spParams->YSize - 1); y += 2)
            {
                vpl::tSize sy = y / 8;
                for (vpl::tSize x = 1; x < (m_spParams->XSize - 1); x += 2)
                {
                    vpl::tSize sx = x / 8;
                    vpl::img::tRGBPixel& RGBPixel = m_AuxVolumeData(sx, sy, sz);
                
                    // Edge magnitude
                    float GradX = float(SobelX.getResponse(m_VolumeData, x, y, z));
                    float GradY = float(SobelY.getResponse(m_VolumeData, x, y, z));
                    float GradZ = float(SobelZ.getResponse(m_VolumeData, x, y, z));
                    float GradMag = vpl::math::getAbs(GradX) + vpl::math::getAbs(GradY) + vpl::math::getAbs(GradZ);
                    vpl::img::tRGBPixel::tComponent Grad = vpl::img::tRGBPixel::tComponent(gradScale * GradMag);
                    RGBPixel.b() = vpl::math::getMax(RGBPixel.b(), Grad);
                }
            }
        }
    }

    // O.K.
    return true;
}

void PSVolumeRendering::storeVolumeToTexture(vpl::img::CVolume<vpl::img::tPixel8>& volume, osg::Texture3D* texture)
{
    auto sliceSize = volume.getXSize() * volume.getYSize();
    auto data = new unsigned char[volume.getZSize() * sliceSize];

    // fill it with data slice by slice
    vpl::img::CImage<vpl::img::tPixel8, vpl::base::CRefData> slice(volume.getXSize(), volume.getYSize());

    for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
    {
        volume.getPlaneXY(z, slice);

        std::memcpy(data + z * sliceSize, slice.getPtr(), sliceSize);
    }

    texture->getImage()->setImage(volume.getXSize(), volume.getYSize(), volume.getZSize(), GL_R8, GL_RED, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE);
    texture->dirtyTextureObject();
}

void PSVolumeRendering::storeVolumeToTexture(vpl::img::CVolume<vpl::img::tPixel16>& volume, osg::Texture3D* texture)
{
    auto sliceSize = volume.getXSize() * volume.getYSize() * 2;
    auto data = new unsigned char[volume.getZSize() * sliceSize];

    // fill it with data slice by slice
    vpl::img::CImage<vpl::img::tPixel16, vpl::base::CRefData> slice(volume.getXSize(), volume.getYSize());

    for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
    {
        volume.getPlaneXY(z, slice);

        std::memcpy(data + z * sliceSize, slice.getPtr(), sliceSize);
    }

    texture->getImage()->setImage(volume.getXSize(), volume.getYSize(), volume.getZSize(), GL_R16, GL_RED, GL_UNSIGNED_SHORT, data, osg::Image::USE_NEW_DELETE);
    texture->dirtyTextureObject();
}

void PSVolumeRendering::storeVolumeToTexture(vpl::img::CVolume<vpl::img::tRGBPixel>& volume, osg::Texture3D* texture)
{
    auto sliceSize = volume.getXSize() * volume.getYSize() * 4;
    auto data = new unsigned char[volume.getZSize() * sliceSize];

    // fill it with data slice by slice
    vpl::img::CImage<vpl::img::tRGBPixel, vpl::base::CRefData> slice(volume.getXSize(), volume.getYSize());

    for (vpl::tSize z = 0; z < volume.getZSize(); ++z)
    {
        volume.getPlaneXY(z, slice);

        std::memcpy(data + z * sliceSize, slice.getPtr(), sliceSize);
    }

    texture->getImage()->setImage(volume.getXSize(), volume.getYSize(), volume.getZSize(), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE);
    texture->dirtyTextureObject();
}

///////////////////////////////////////////////////////////////////////////////
// first-time initialization of OpenGL state, textures, shaders, lookups, ...
bool PSVolumeRendering::internalInitRendering()
{
    // Already initialized?
    if (testFlag(INITIALIZED))
    {
        return true;
    }

    m_textureRaysStartEnd = new osg::Texture3D();
    m_textureRaysStartEnd->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    m_textureRaysStartEnd->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    m_textureRaysStartEnd->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_textureRaysStartEnd->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    m_textureRaysStartEnd->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    m_textureRaysStartEnd->setInternalFormat(GL_RGB32F);
    m_textureRaysStartEnd->setSourceFormat(GL_RGB);
    m_textureRaysStartEnd->setSourceType(GL_FLOAT);

    m_textureVolumeRender = new osg::Texture2D();
    m_textureVolumeRender->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    m_textureVolumeRender->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    m_textureVolumeRender->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_textureVolumeRender->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    m_textureVolumeRender->setInternalFormat(GL_RGBA); 

    m_textureDepth = new osg::Texture2D();
    m_textureDepth->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    m_textureDepth->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    m_textureDepth->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_textureDepth->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    m_textureDepth->setInternalFormat(GL_DEPTH24_STENCIL8);
    m_textureDepth->setSourceFormat(GL_DEPTH_STENCIL);
    m_textureDepth->setSourceType(GL_UNSIGNED_INT_24_8);

    m_fboFrontBox = new osg::FrameBufferObject();
    m_fboFrontBox->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment(m_textureRaysStartEnd, 0));

    m_fboBackBox = new osg::FrameBufferObject();
    m_fboBackBox->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment(m_textureRaysStartEnd, 1));

    m_fboDepth = new osg::FrameBufferObject();
    m_fboDepth->setAttachment(osg::Camera::DEPTH_BUFFER, osg::FrameBufferAttachment(m_textureDepth));

    m_fboVolumeRender = new osg::FrameBufferObject();
    m_fboVolumeRender->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment(m_textureVolumeRender));

    // Setup 3D texture
    ///////////////////////////////////////////////////////////////////////////
    m_textureVolume = new osg::Texture3D(new osg::Image());
    m_textureVolume->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    m_textureVolume->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    m_textureVolume->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_textureVolume->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    m_textureVolume->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    m_textureVolume->setUseHardwareMipMapGeneration(false);
    m_textureVolume->setResizeNonPowerOfTwoHint(false);

    storeVolumeToTexture(m_VolumeData, m_textureVolume);

    // Setup aux 3D texture
    ///////////////////////////////////////////////////////////////////////////
    m_textureCurrentVolume = new osg::Texture3D(new osg::Image());
    m_textureCurrentVolume->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    m_textureCurrentVolume->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    m_textureCurrentVolume->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_textureCurrentVolume->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    m_textureCurrentVolume->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    m_textureCurrentVolume->setUseHardwareMipMapGeneration(false);
    m_textureCurrentVolume->setResizeNonPowerOfTwoHint(false);

    // Setup skipping 3D texture
    ///////////////////////////////////////////////////////////////////////////
    m_textureAuxVolume = new osg::Texture3D(new osg::Image());
    m_textureAuxVolume->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    m_textureAuxVolume->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    m_textureAuxVolume->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_textureAuxVolume->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    m_textureAuxVolume->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    m_textureAuxVolume->setUseHardwareMipMapGeneration(false);
    m_textureAuxVolume->setResizeNonPowerOfTwoHint(false);

    storeVolumeToTexture(m_AuxVolumeData, m_textureAuxVolume);

    m_textureCustomAuxVolume = new osg::Texture3D(new osg::Image());
    m_textureCustomAuxVolume->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    m_textureCustomAuxVolume->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    m_textureCustomAuxVolume->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_textureCustomAuxVolume->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    m_textureCustomAuxVolume->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    m_textureCustomAuxVolume->setUseHardwareMipMapGeneration(false);
    m_textureCustomAuxVolume->setResizeNonPowerOfTwoHint(false);

    // LUT texture
    ///////////////////////////////////////////////////////////////////////////
    // setup 2D lookup texture in OpenGL and pre-load the first one
    m_textureLookUp = new osg::Texture2D(new osg::Image());
    m_textureLookUp->getImage()->setImage(LUT_2D_W, LUT_2D_H, 1, LOOKUP_TEXTURE_FORMAT, GL_RGBA, GL_UNSIGNED_SHORT, (unsigned char*)m_internalLookupTables[0], osg::Image::NO_DELETE);
    m_textureLookUp->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    m_textureLookUp->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    m_textureLookUp->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_textureLookUp->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    // noise texture
    ///////////////////////////////////////////////////////////////////////////
    // Prepare a random noise texture
    vpl::math::CNormalPRNG generator;
    unsigned char* noise = new unsigned char[NOISE_SIZE * NOISE_SIZE];

    std::generate(noise, noise + NOISE_SIZE * NOISE_SIZE, [&generator]() {return static_cast<unsigned char>(128 + generator.random(0.0, conf::NoiseSigma)); });

    m_textureNoise = new osg::Texture2D(new osg::Image());
    m_textureNoise->getImage()->setImage(NOISE_SIZE, NOISE_SIZE, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE, noise, osg::Image::USE_NEW_DELETE);
    m_textureNoise->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    m_textureNoise->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    m_textureNoise->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_textureNoise->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    // create and upload 1D resize kernel texture (used for image interpolation)
    ///////////////////////////////////////////////////////////////////////////
    unsigned char* klut = new unsigned char[LUT_SIZE * sizeof(float)];
    float* data = reinterpret_cast<float*>(klut);

    for (int i = 1; i < LUT_SIZE; i++)
    {
        double x = double(i) / double(LUT_SIZE) * 2.0;

        data[i] = static_cast<float>((std::sin(x * vpl::math::PI) / (x * vpl::math::PI)) * (std::sin((x * 0.5f) * vpl::math::PI) / ((x * 0.5) * vpl::math::PI)));
    }
    data[0] = 1.0f;

    osg::Image* bickerImage = new osg::Image();
    bickerImage->setImage(LUT_SIZE, 1, 1, GL_R32F, GL_RED, GL_FLOAT, klut, osg::Image::USE_NEW_DELETE);

    m_textureBicubicKernel = new osg::Texture1D(bickerImage);
    m_textureBicubicKernel->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    m_textureBicubicKernel->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    m_textureBicubicKernel->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);

    auto shaderVertex = new osg::Shader(osg::Shader::VERTEX, shader::Vert);

    m_shaderOsrt = new osg::Program();
    m_shaderOsrt->addShader(new osg::Shader(osg::Shader::VERTEX, shader::Vert2));
    m_shaderOsrt->addShader(new osg::Shader(osg::Shader::FRAGMENT, shader::OSRT));

    m_shaderResize = new osg::Program();
    m_shaderResize->addShader(new osg::Shader(osg::Shader::VERTEX, shader::FSQuadVS));
    m_shaderResize->addShader(new osg::Shader(osg::Shader::FRAGMENT, shader::Resize));

    m_uniform_image = new osg::Uniform("image", 4);
    m_uniform_kernel = new osg::Uniform("kernel", 6);
    m_uniform_resolution = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "resolution");

    for (int i = 0; i < static_cast<int>(EShaders::SHADERS_COUNT); i++)
    {
        m_shaders[static_cast<EShaders>(i)] = new osg::Program();
    }

    m_shaders[EShaders::XRAY]->addShader(shaderVertex);
    m_shaders[EShaders::XRAY]->addShader(new osg::Shader(osg::Shader::FRAGMENT, shader::XRay));

    m_shaders[EShaders::MIP]->addShader(shaderVertex);
    m_shaders[EShaders::MIP]->addShader(new osg::Shader(osg::Shader::FRAGMENT, shader::MAX_IP));

    m_shaders[EShaders::SHADING]->addShader(shaderVertex);
    m_shaders[EShaders::SHADING]->addShader(new osg::Shader(osg::Shader::FRAGMENT, shader::Shade));

    m_shaders[EShaders::ADDITIVE]->addShader(shaderVertex);
    m_shaders[EShaders::ADDITIVE]->addShader(new osg::Shader(osg::Shader::FRAGMENT, shader::Add));

    m_shaders[EShaders::SURFACE]->addShader(shaderVertex);
    m_shaders[EShaders::SURFACE]->addShader(new osg::Shader(osg::Shader::FRAGMENT, shader::Surface));

    m_uniform_t3D = new osg::Uniform("t3D", 0);
    m_uniform_tSkip3D = new osg::Uniform("tSkip3D", 1);
    m_uniform_LookUp = new osg::Uniform("LookUp", 2);
    m_uniform_Noise = new osg::Uniform("Noise", 3);
    m_uniform_tRaysStartEnd = new osg::Uniform("tRaysStartEnd", 4);
    m_uniform_Depth = new osg::Uniform("Depth", 5);

    //CUSTOM
    m_uniform_tCustom3D = new osg::Uniform("tCustom3D", 7);

    m_uniform_textureSampling = new osg::Uniform(osg::Uniform::FLOAT, "textureSampling");
    m_uniform_inputAdjustment = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "inputAdjustment");
    m_uniform_imageAdjustment = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "imageAdjustment");
    m_uniform_sVector = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "sVector");
    m_uniform_skipTexSize = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "skipTexSize");
    m_uniform_tResolution = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "tResolution");
    m_uniform_tSkipResolution = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "tSkipResolution");
    m_uniform_StopCondition = new osg::Uniform("StopCondition", 0.01f);
    m_uniform_wSize = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "wSize");
    m_uniform_pl = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "pl");
    m_uniform_plNear = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "plNear");
    m_uniform_plFar = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "plFar");
    m_uniform_invProjectionMatrix = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "invProjectionMatrix");
    m_uniform_invModelViewMatrix = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "invModelViewMatrix");
    m_uniform_skipCondition = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "skipCondition");

    //SURFACE
    m_uniform_surfacePar = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "surfacePar");

    m_stateSetFrontBox = new osg::StateSet(*m_box->getOrCreateStateSet());
    m_stateSetBackBox = new osg::StateSet(*m_box->getOrCreateStateSet());
    m_stateSetVolumeRenderBackup = new osg::StateSet(*m_box->getOrCreateStateSet());
    m_stateSetResize = new osg::StateSet(*m_quad->getOrCreateStateSet());

    m_stateSetFrontBox->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetFrontBox->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetFrontBox->setAttributeAndModes(m_shaderOsrt, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetFrontBox->setAttribute(m_fboFrontBox);

    m_stateSetBackBox->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetBackBox->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetBackBox->setAttributeAndModes(m_shaderOsrt, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetBackBox->setAttribute(m_fboBackBox);

    m_stateSetVolumeRenderBackup->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetVolumeRenderBackup->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetVolumeRenderBackup->setAttribute(m_fboVolumeRender);
    m_stateSetVolumeRenderBackup->setTextureAttribute(0, m_textureVolume);
    m_stateSetVolumeRenderBackup->setTextureAttribute(1, m_textureAuxVolume);
    m_stateSetVolumeRenderBackup->setTextureAttribute(2, m_textureLookUp);
    m_stateSetVolumeRenderBackup->setTextureAttribute(3, m_textureNoise);
    m_stateSetVolumeRenderBackup->setTextureAttribute(4, m_textureRaysStartEnd);
    m_stateSetVolumeRenderBackup->setTextureAttribute(5, m_textureDepth);
    m_stateSetVolumeRenderBackup->setTextureAttribute(7, m_textureCurrentVolume);

    m_stateSetResize->addUniform(m_uniform_image);
    m_stateSetResize->addUniform(m_uniform_kernel);
    m_stateSetResize->addUniform(m_uniform_resolution);

    m_stateSetResize->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetResize->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
    m_stateSetResize->setAttributeAndModes(m_shaderResize, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

    m_stateSetResize->setTextureAttribute(4, m_textureVolumeRender);
    m_stateSetResize->setTextureAttribute(6, m_textureBicubicKernel);

    setShaderInternal(static_cast<EShaders>(m_spParams->selectedShader));

    // O.K.
    m_Flags |= INITIALIZED;

    return true;
}

void PSVolumeRendering::setShaderInternal(EShaders shader)
{
    m_stateSetVolumeRender = new osg::StateSet(*m_stateSetVolumeRenderBackup);

    m_stateSetVolumeRender->addUniform(m_uniform_t3D);
    m_stateSetVolumeRender->addUniform(m_uniform_tSkip3D);
    m_stateSetVolumeRender->addUniform(m_uniform_LookUp);
    m_stateSetVolumeRender->addUniform(m_uniform_Noise);
    m_stateSetVolumeRender->addUniform(m_uniform_tRaysStartEnd);
    m_stateSetVolumeRender->addUniform(m_uniform_Depth);

    m_stateSetVolumeRender->addUniform(m_uniform_textureSampling);
    m_stateSetVolumeRender->addUniform(m_uniform_inputAdjustment);
    m_stateSetVolumeRender->addUniform(m_uniform_imageAdjustment);
    m_stateSetVolumeRender->addUniform(m_uniform_sVector);
    m_stateSetVolumeRender->addUniform(m_uniform_skipTexSize);
    m_stateSetVolumeRender->addUniform(m_uniform_tResolution);
    m_stateSetVolumeRender->addUniform(m_uniform_tSkipResolution);
    m_stateSetVolumeRender->addUniform(m_uniform_StopCondition);
    m_stateSetVolumeRender->addUniform(m_uniform_wSize);
    m_stateSetVolumeRender->addUniform(m_uniform_pl);
    m_stateSetVolumeRender->addUniform(m_uniform_plNear);
    m_stateSetVolumeRender->addUniform(m_uniform_plFar);
    m_stateSetVolumeRender->addUniform(m_uniform_invProjectionMatrix);
    m_stateSetVolumeRender->addUniform(m_uniform_invModelViewMatrix);
    m_stateSetVolumeRender->addUniform(m_uniform_skipCondition);

    if (shader == EShaders::SURFACE)
    {
        m_stateSetVolumeRender->addUniform(m_uniform_surfacePar);
    }

    if (shader == EShaders::CUSTOM)
    {
        for (auto uni : m_customUniforms)
        {
            m_stateSetVolumeRender->addUniform(uni);
        }

        m_stateSetVolumeRender->addUniform(m_uniform_tCustom3D);

        m_stateSetVolumeRender->setTextureAttribute(1, m_textureCustomAuxVolume);
    }
    else
    {
        m_stateSetVolumeRender->setTextureAttribute(1, m_textureAuxVolume);

    }

    m_stateSetVolumeRender->setAttributeAndModes(m_shaders[shader], osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
}

bool PSVolumeRendering::internalUploadTexture()
{
    VPL_LOG_TRACE("PSVolumeRendering::internalUploadTexture");

    storeVolumeToTexture(m_VolumeData, m_textureVolume);

    m_VolumeData.resize(0, 0, 0, 0);

    return true;
}

bool PSVolumeRendering::internalUploadAuxTexture()
{
    VPL_LOG_TRACE("PSVolumeRendering::internalUploadAuxTexture");

     storeVolumeToTexture(m_AuxVolumeData, m_textureAuxVolume);

    if (m_spParams->selectedShader != EShaders::CUSTOM)
    {
        m_stateSetVolumeRender->setTextureAttribute(1, m_textureAuxVolume);
    }

    m_AuxVolumeData.resize(0, 0, 0, 0);

    return true;
}

bool PSVolumeRendering::internalUploadCustomData_bool()
{
    if (m_volume_bool.getZSize() <= 0)
    {
        return false;
    }

    m_spParams->CustomXSize = m_volume_bool.getXSize();
    m_spParams->CustomYSize = m_volume_bool.getYSize();
    m_spParams->CustomZSize = m_volume_bool.getZSize();

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->CustomXSize &= ROUNDING_MASK;
    m_spParams->CustomYSize &= ROUNDING_MASK;
    m_spParams->CustomZSize &= ROUNDING_MASK;

    m_customData_bool.resize(m_spParams->CustomXSize, m_spParams->CustomYSize, m_spParams->CustomZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < m_spParams->CustomZSize; z++)
    {
        for (vpl::tSize y = 0; y < m_spParams->CustomYSize; y++)
        {
            for (vpl::tSize x = 0; x < m_spParams->CustomXSize; x++)
            {
                m_customData_bool(x, y, z) = m_volume_bool(x, y, z) ? 255 : 0;
            }
        }
    }

    m_volume_bool.resize(0, 0, 0, 0);

    if (m_auxVolume.getZSize() != 0)
    {
        m_spParams->CustomAuxXSize = ((m_auxVolume.getXSize() + 3) / 4) * 4;
        m_spParams->CustomAuxYSize = ((m_auxVolume.getYSize() + 3) / 4) * 4;
        m_spParams->CustomAuxZSize = ((m_auxVolume.getZSize() + 3) / 4) * 4;

        m_spParams->CustomAuxTexXSize = float(m_spParams->CustomXSize) / float(m_spParams->CustomAuxXSize * 8);
        m_spParams->CustomAuxTexYSize = float(m_spParams->CustomYSize) / float(m_spParams->CustomAuxYSize * 8);
        m_spParams->CustomAuxTexZSize = float(m_spParams->CustomZSize) / float(m_spParams->CustomAuxZSize * 8);

        m_auxCustomData.resize(m_spParams->CustomAuxXSize, m_spParams->CustomAuxYSize, m_spParams->CustomAuxZSize);
        m_auxCustomData.fillEntire(0);

        #pragma omp parallel for
        for (vpl::tSize z = 0; z < m_auxVolume.getZSize(); ++z)
        {
            for (vpl::tSize y = 0; y < m_auxVolume.getYSize(); ++y)
            {
                for (vpl::tSize x = 0; x < m_auxVolume.getXSize(); ++x)
                {
                    m_auxCustomData(x, y, z) = m_auxVolume(x, y, z);
                }
            }
        }

        m_auxVolume.resize(0, 0, 0, 0);
    }

    return true;
}

bool PSVolumeRendering::internalUploadCustomData_tPixel8()
{
    if (m_volume_tPixel8.getZSize() <= 0)
    {
        return false;
    }

    m_spParams->CustomXSize = m_volume_tPixel8.getXSize();
    m_spParams->CustomYSize = m_volume_tPixel8.getYSize();
    m_spParams->CustomZSize = m_volume_tPixel8.getZSize();

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->CustomXSize &= ROUNDING_MASK;
    m_spParams->CustomYSize &= ROUNDING_MASK;
    m_spParams->CustomZSize &= ROUNDING_MASK;

    m_customData_tPixel8.resize(m_spParams->CustomXSize, m_spParams->CustomYSize, m_spParams->CustomZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < m_spParams->CustomZSize; z++)
    {
        for (vpl::tSize y = 0; y < m_spParams->CustomYSize; y++)
        {
            for (vpl::tSize x = 0; x < m_spParams->CustomXSize; x++)
            {
                m_customData_tPixel8(x, y, z) = m_volume_tPixel8(x, y, z);
            }
        }
    }

    m_volume_tPixel8.resize(0, 0, 0, 0);

    if (m_auxVolume.getZSize() != 0)
    {
        m_spParams->CustomAuxXSize = ((m_auxVolume.getXSize() + 3) / 4) * 4;
        m_spParams->CustomAuxYSize = ((m_auxVolume.getYSize() + 3) / 4) * 4;
        m_spParams->CustomAuxZSize = ((m_auxVolume.getZSize() + 3) / 4) * 4;

        m_spParams->CustomAuxTexXSize = float(m_spParams->CustomXSize) / float(m_spParams->CustomAuxXSize * 8);
        m_spParams->CustomAuxTexYSize = float(m_spParams->CustomYSize) / float(m_spParams->CustomAuxYSize * 8);
        m_spParams->CustomAuxTexZSize = float(m_spParams->CustomZSize) / float(m_spParams->CustomAuxZSize * 8);

        m_auxCustomData.resize(m_spParams->CustomAuxXSize, m_spParams->CustomAuxYSize, m_spParams->CustomAuxZSize);
        m_auxCustomData.fillEntire(0);

        #pragma omp parallel for
        for (vpl::tSize z = 0; z < m_auxVolume.getZSize(); ++z)
        {
            for (vpl::tSize y = 0; y < m_auxVolume.getYSize(); ++y)
            {
                for (vpl::tSize x = 0; x < m_auxVolume.getXSize(); ++x)
                {
                    m_auxCustomData(x, y, z) = m_auxVolume(x, y, z);
                }
            }
        }

        m_auxVolume.resize(0, 0, 0, 0);
    }

    return true;
}

bool PSVolumeRendering::internalUploadCustomData_tPixel16()
{
    if (m_volume_tPixel16.getZSize() <= 0)
    {
        return false;
    }

    m_spParams->CustomXSize = m_volume_tPixel16.getXSize();
    m_spParams->CustomYSize = m_volume_tPixel16.getYSize();
    m_spParams->CustomZSize = m_volume_tPixel16.getZSize();

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->CustomXSize &= ROUNDING_MASK;
    m_spParams->CustomYSize &= ROUNDING_MASK;
    m_spParams->CustomZSize &= ROUNDING_MASK;

    m_customData_tPixel16.resize(m_spParams->CustomXSize, m_spParams->CustomYSize, m_spParams->CustomZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < m_spParams->CustomZSize; z++)
    {
        for (vpl::tSize y = 0; y < m_spParams->CustomYSize; y++)
        {
            for (vpl::tSize x = 0; x < m_spParams->CustomXSize; x++)
            {
                m_customData_tPixel16(x, y, z) = m_volume_tPixel16(x, y, z);
            }
        }
    }

    m_volume_tPixel16.resize(0, 0, 0, 0);

    if (m_auxVolume.getZSize() != 0)
    {
        m_spParams->CustomAuxXSize = ((m_auxVolume.getXSize() + 3) / 4) * 4;
        m_spParams->CustomAuxYSize = ((m_auxVolume.getYSize() + 3) / 4) * 4;
        m_spParams->CustomAuxZSize = ((m_auxVolume.getZSize() + 3) / 4) * 4;

        m_spParams->CustomAuxTexXSize = float(m_spParams->CustomXSize) / float(m_spParams->CustomAuxXSize * 8);
        m_spParams->CustomAuxTexYSize = float(m_spParams->CustomYSize) / float(m_spParams->CustomAuxYSize * 8);
        m_spParams->CustomAuxTexZSize = float(m_spParams->CustomZSize) / float(m_spParams->CustomAuxZSize * 8);

        m_auxCustomData.resize(m_spParams->CustomAuxXSize, m_spParams->CustomAuxYSize, m_spParams->CustomAuxZSize);
        m_auxCustomData.fillEntire(0);

        #pragma omp parallel for
        for (vpl::tSize z = 0; z < m_auxVolume.getZSize(); ++z)
        {
            for (vpl::tSize y = 0; y < m_auxVolume.getYSize(); ++y)
            {
                for (vpl::tSize x = 0; x < m_auxVolume.getXSize(); ++x)
                {
                    m_auxCustomData(x, y, z) = m_auxVolume(x, y, z);
                }
            }
        }

        m_auxVolume.resize(0, 0, 0, 0);
    }

    return true;
}

bool PSVolumeRendering::internalUploadCustomData_tRGBPixel()
{
    if (m_volume_tRGBPixel.getZSize() <= 0)
    {
        return false;
    }

    m_spParams->CustomXSize = m_volume_tRGBPixel.getXSize();
    m_spParams->CustomYSize = m_volume_tRGBPixel.getYSize();
    m_spParams->CustomZSize = m_volume_tRGBPixel.getZSize();

    // Even texture size...
    // TODO: This may cause a small error in the visualization...
    m_spParams->CustomXSize &= ROUNDING_MASK;
    m_spParams->CustomYSize &= ROUNDING_MASK;
    m_spParams->CustomZSize &= ROUNDING_MASK;

    m_customData_tRGBPixel.resize(m_spParams->CustomXSize, m_spParams->CustomYSize, m_spParams->CustomZSize, 0);

#pragma omp parallel for schedule(static) default(shared)
    for (vpl::tSize z = 0; z < m_spParams->CustomZSize; z++)
    {
        for (vpl::tSize y = 0; y < m_spParams->CustomYSize; y++)
        {
            for (vpl::tSize x = 0; x < m_spParams->CustomXSize; x++)
            {
                m_customData_tRGBPixel(x, y, z) = m_volume_tRGBPixel(x, y, z);
            }
        }
    }

    m_volume_tRGBPixel.resize(0, 0, 0, 0);

    if (m_auxVolume.getZSize() != 0)
    {
        m_spParams->CustomAuxXSize = ((m_auxVolume.getXSize() + 3) / 4) * 4;
        m_spParams->CustomAuxYSize = ((m_auxVolume.getYSize() + 3) / 4) * 4;
        m_spParams->CustomAuxZSize = ((m_auxVolume.getZSize() + 3) / 4) * 4;

        m_spParams->CustomAuxTexXSize = float(m_spParams->CustomXSize) / float(m_spParams->CustomAuxXSize * 8);
        m_spParams->CustomAuxTexYSize = float(m_spParams->CustomYSize) / float(m_spParams->CustomAuxYSize * 8);
        m_spParams->CustomAuxTexZSize = float(m_spParams->CustomZSize) / float(m_spParams->CustomAuxZSize * 8);

        m_auxCustomData.resize(m_spParams->CustomAuxXSize, m_spParams->CustomAuxYSize, m_spParams->CustomAuxZSize);
        m_auxCustomData.fillEntire(0);

        #pragma omp parallel for
        for (vpl::tSize z = 0; z < m_auxVolume.getZSize(); ++z)
        {
            for (vpl::tSize y = 0; y < m_auxVolume.getYSize(); ++y)
            {
                for (vpl::tSize x = 0; x < m_auxVolume.getXSize(); ++x)
                {
                    m_auxCustomData(x, y, z) = m_auxVolume(x, y, z);
                }
            }
        }

        m_auxVolume.resize(0, 0, 0, 0);
    }

    return true;
}

bool PSVolumeRendering::internalUploadCustomTexture_bool()
{
    VPL_LOG_TRACE("PSVolumeRendering::internalUploadCustomTexture_bool");

    storeVolumeToTexture(m_customData_bool, m_textureCurrentVolume);

    m_customData_bool.resize(0, 0, 0, 0);

    return true;
}

bool PSVolumeRendering::internalUploadCustomTexture_tPixel8()
{
    VPL_LOG_TRACE("PSVolumeRendering::internalUploadCustomTexture_tPixel8");

    storeVolumeToTexture(m_customData_tPixel8, m_textureCurrentVolume);

    m_customData_tPixel8.resize(0, 0, 0, 0);

    return true;
}

bool PSVolumeRendering::internalUploadCustomTexture_tPixel16()
{
    VPL_LOG_TRACE("PSVolumeRendering::internalUploadCustomTexture_tPixel16");

    storeVolumeToTexture(m_customData_tPixel16, m_textureCurrentVolume);;

    m_customData_tPixel16.resize(0, 0, 0, 0);

    return true;
}

bool PSVolumeRendering::internalUploadCustomTexture_tRGBPixel()
{
    VPL_LOG_TRACE("PSVolumeRendering::internalUploadCustomTexture_tRGBPixel");

    storeVolumeToTexture(m_customData_tRGBPixel, m_textureCurrentVolume);

    m_customData_tRGBPixel.resize(0, 0, 0, 0);

    return true;
}

bool PSVolumeRendering::internalUploadCustomAuxTexture()
{
    VPL_LOG_TRACE("PSVolumeRendering::internalUploadCustomAuxTexture");

    storeVolumeToTexture(m_auxCustomData, m_textureCustomAuxVolume);

    m_stateSetVolumeRender->setTextureAttribute(1, m_textureCustomAuxVolume);

    m_auxCustomData.resize(0, 0, 0, 0);

    return true;
}

osg::Vec2i PSVolumeRendering::getRenderingSize(PSVolumeRenderingParams& params, const osg::Vec2i& currentViewport, int flags) const
{
    int desiredDimension = 0;

    if (flags & MOUSE_MODE)
    {
        desiredDimension = conf::MouseRenderingSize[params.currentQuality];
    }
    else
    {
        desiredDimension = params.renderingSize;
    }

    int width = std::max(1, currentViewport.x());
    int height = std::max(1, currentViewport.y());

    if (width > height)
    {
        float ratio = static_cast<float>(height) / width;
        int dimension = vpl::math::getMin<int>(width, desiredDimension);

        return osg::Vec2i(dimension, vpl::math::round2Int(dimension * ratio));
    }
    else
    {
        float ratio = static_cast<float>(width) / height;
        int dimension = vpl::math::getMin<int>(height, desiredDimension);

        return osg::Vec2i(vpl::math::round2Int(dimension * ratio), dimension);
    }
}

float PSVolumeRendering::getVolumeSamplingDistance(PSVolumeRenderingParams *pParams, int Flags) const
{
    if (Flags & MOUSE_MODE)
    {
        return conf::MouseTextureSampling[pParams->currentQuality];
    }
    else
    {
        return pParams->volumeSamplingDistance;
    }
}

///////////////////////////////////////////////////////////////////////////////
// change size of all internal rendering textures
bool PSVolumeRendering::internalSetRenderingSize(PSVolumeRenderingParams& params, const osg::Vec2i& currentViewport, const osg::Vec2i& newViewport, int flags)
{
    VPL_LOG_TRACE("PSVolumeRendering::internalSetRenderingSize");

    m_textureRaysStartEnd->setTextureSize(newViewport.x(), newViewport.y(), 2);
    m_textureRaysStartEnd->dirtyTextureObject();

    m_textureVolumeRender->setTextureSize(newViewport.x(), newViewport.y());
    m_textureVolumeRender->dirtyTextureObject();

    m_textureDepth->setTextureSize(currentViewport.x(), currentViewport.y());
    m_textureDepth->dirtyTextureObject();

    m_fboFrontBox->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment(m_textureRaysStartEnd, 0));
    m_fboBackBox->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment(m_textureRaysStartEnd, 1));

    m_fboVolumeRender->setAttachment(osg::Camera::COLOR_BUFFER0, osg::FrameBufferAttachment(m_textureVolumeRender));

    m_fboDepth->setAttachment(osg::Camera::DEPTH_BUFFER, osg::FrameBufferAttachment(m_textureDepth));

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// upload new, user selected lookup texture to the GPU memory
bool PSVolumeRendering::internalSetLUT(PSVolumeRenderingParams *pParams)
{
    VPL_LOG_TRACE("PSVolumeRendering::internalSetLUT");

    m_textureLookUp->getImage()->setImage(LUT_2D_W, LUT_2D_H, 1, LOOKUP_TEXTURE_FORMAT, GL_RGBA, GL_UNSIGNED_SHORT,
        (unsigned char*)m_internalLookupTables[static_cast<int>(pParams->selectedLut)], osg::Image::NO_DELETE);
    m_textureLookUp->dirtyTextureObject();

    return true;
}

void PSVolumeRendering::setFlag(int Flag)
{
    m_Flags |= Flag;
}

void PSVolumeRendering::setAndSignalFlag(int Flag)
{
    m_Mutex.lock();
    m_Flags |= Flag;
    m_Condition.notifyOne();
    m_Mutex.unlock();
}

void PSVolumeRendering::clearFlag(int Flag)
{
    m_Flags &= (0xffff - Flag);
}

bool PSVolumeRendering::testFlag(int Flag)
{
    return (m_Flags & Flag) != 0;
}

VPL_THREAD_ROUTINE(PSVolumeRendering::setupLoop)
{
    // Console object
    PSVolumeRendering *pRenderer = static_cast<PSVolumeRendering *>(pThread->getData());
    if (!pRenderer)
    {
        return -1;
    }

    // Mask of relevant flags
    int InvalidMask = DATA_INVALID;
    InvalidMask += CUSTOM_DATA_INVALID;

    // Main thread loop
    VPL_THREAD_MAIN_LOOP
    {
        pRenderer->m_Mutex.lock();
        if ((pRenderer->m_Flags & InvalidMask) == 0)
        {
            // Wait for the "anything changed" event
            if (!pRenderer->m_Condition.wait(pRenderer->m_Mutex, 250))
            {
                pRenderer->m_Mutex.unlock();
                continue;
            }
        }

        // Local copy of all flags and params
        int Flags = pRenderer->m_Flags;
        pRenderer->clearFlag(DATA_INVALID);
        pRenderer->clearFlag(CUSTOM_DATA_INVALID);

        //PSVolumeRenderingParams params = *pRenderer->m_spParams;

        // Release the mutex
        pRenderer->m_Mutex.unlock();

        int newFlags = PSVR_NO_FLAGS;

        // Prepare the data if required.
        if (Flags & DATA_INVALID)
        {
            if (pRenderer->internalUploadData())
            {
                newFlags |= TEXTURE_INVALID;
                newFlags |= AUX_TEXTURE_INVALID;
            }
        }

        // Prepare the data if required.
        if (Flags & CUSTOM_DATA_INVALID)
        {
            bool uploadData = false;
            switch (pRenderer->m_currentType)
            {
            case EVT_BOOL:
                uploadData = pRenderer->internalUploadCustomData_bool();
                break;

            case EVT_PIXEL8:
                uploadData = pRenderer->internalUploadCustomData_tPixel8();
                break;

            case EVT_PIXEL16:
                uploadData = pRenderer->internalUploadCustomData_tPixel16();
                break;

            case EVT_RGBPIXEL:
                uploadData = pRenderer->internalUploadCustomData_tRGBPixel();
                break;
            }

            if (uploadData)
            {
                newFlags |= CUSTOM_TEXTURE_INVALID;
            }
        }

        // set flags
        pRenderer->setFlag(newFlags);

        // on change request redraw
        if ((Flags & DATA_INVALID) || (Flags & CUSTOM_DATA_INVALID))
        {
            pRenderer->redraw();
        }

        // Sleep for a short period of time
        vpl::sys::sleep(1);
    }

    return 0;
}

void PSVolumeRendering::prepareBox(int numOfQuads)
{
    auto vertices = new osg::Vec3Array();

    // Quad size - real and texture coordinates
    float rs = 2.0f / numOfQuads;
    float ts = 1.0f / numOfQuads;

    // First side
    float b = -1.0f, v = 0.0f;
    for (int j = 0; j < numOfQuads; ++j, b += rs, v += ts)
    {
        float a = -1.0f, u = 0.0f;
        for (int i = 0; i < numOfQuads; ++i, a += rs, u += ts)
        {
            vertices->push_back(osg::Vec3(a, b + rs, -1.0f));
            vertices->push_back(osg::Vec3(a + rs, b + rs, -1.0f));
            vertices->push_back(osg::Vec3(a + rs, b, -1.0f));

            vertices->push_back(osg::Vec3(a + rs, b, -1.0f));
            vertices->push_back(osg::Vec3(a, b, -1.0f));
            vertices->push_back(osg::Vec3(a, b + rs, -1.0f));

            vertices->push_back(osg::Vec3(a, b + rs, 1.0f));
            vertices->push_back(osg::Vec3(a, b, 1.0f));
            vertices->push_back(osg::Vec3(a + rs, b, 1.0f));

            vertices->push_back(osg::Vec3(a + rs, b, 1.0f));
            vertices->push_back(osg::Vec3(a + rs, b + rs, 1.0f));
            vertices->push_back(osg::Vec3(a, b + rs, 1.0f));
        }
    }

    b = -1.0f, v = 0.0f;
    for (int j = 0; j < numOfQuads; ++j, b += rs, v += ts)
    {
        float a = -1.0f, u = 0.0f;
        for (int i = 0; i < numOfQuads; ++i, a += rs, u += ts)
        {
            vertices->push_back(osg::Vec3(a, -1.0f, b));
            vertices->push_back(osg::Vec3(a + rs, -1.0f, b));
            vertices->push_back(osg::Vec3(a + rs, -1.0f, b + rs));

            vertices->push_back(osg::Vec3(a + rs, -1.0f, b + rs));
            vertices->push_back(osg::Vec3(a, -1.0f, b + rs));
            vertices->push_back(osg::Vec3(a, -1.0f, b));

            vertices->push_back(osg::Vec3(a, 1.0f, b));
            vertices->push_back(osg::Vec3(a, 1.0f, b + rs));
            vertices->push_back(osg::Vec3(a + rs, 1.0f, b + rs));

            vertices->push_back(osg::Vec3(a + rs, 1.0f, b + rs));
            vertices->push_back(osg::Vec3(a + rs, 1.0f, b));
            vertices->push_back(osg::Vec3(a, 1.0f, b));
        }
    }

    b = -1.0f, v = 0.0f;
    for (int j = 0; j < numOfQuads; ++j, b += rs, v += ts)
    {
        float a = -1.0f, u = 0.0f;
        for (int i = 0; i < numOfQuads; ++i, a += rs, u += ts)
        {
            vertices->push_back(osg::Vec3(-1.0f, a + rs, b + rs));
            vertices->push_back(osg::Vec3(-1.0f, a + rs, b));
            vertices->push_back(osg::Vec3(-1.0f, a, b));

            vertices->push_back(osg::Vec3(-1.0f, a, b));
            vertices->push_back(osg::Vec3(-1.0f, a, b + rs));
            vertices->push_back(osg::Vec3(-1.0f, a + rs, b + rs));

            vertices->push_back(osg::Vec3(1.0f, a + rs, b + rs));
            vertices->push_back(osg::Vec3(1.0f, a, b + rs));
            vertices->push_back(osg::Vec3(1.0f, a, b));

            vertices->push_back(osg::Vec3(1.0f, a, b));
            vertices->push_back(osg::Vec3(1.0f, a + rs, b));
            vertices->push_back(osg::Vec3(1.0f, a + rs, b + rs));
        }
    }

    m_box = new osg::Geometry();
    m_box->setVertexArray(vertices);
    m_box->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 3 * 12 * numOfQuads * numOfQuads));
}

void PSVolumeRendering::prepareQuad()
{
    auto vertices = new osg::Vec3Array();
    vertices->push_back(osg::Vec3(-1.0f, -1.0f, 0.0f));
    vertices->push_back(osg::Vec3(1.0f, -1.0f, 0.0f));
    vertices->push_back(osg::Vec3(1.0f, 1.0f, 0.0f));
    vertices->push_back(osg::Vec3(-1.0f, 1.0f, 0.0f));

    m_quad = new osg::Geometry();
    m_quad->setVertexArray(vertices);
    m_quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, 4));
}

///////////////////////////////////////////////////////////////////////////////
// Main volume rendering function called for every frame displayed
// Initilizes OpenGL when first called, than updates shaders and lookups
// and finally renders volume in a few steps.
void PSVolumeRendering::renderVolume(osg::RenderInfo& renderInfo)
{
    // Local copy of rendering parameters
    PSVolumeRenderingParams params;
    int flags = 0;

    // BEGIN: Locked part of the rendering
    {
        tLock Lock(*this);

        // Do not do anything if init failed many times
        if (constantFailure())
        {
            return;
        }

        // Initialize the renderer if required.
        init();

        // Is rendering enabled and correctly initialized?
        if (!m_Enabled || !testFlag(INITIALIZED))
        {
            return;
        }

        if (((APP_MODE.check(scene::CAppMode::COMMAND_DRAW_WINDOW) || APP_MODE.check(scene::CAppMode::COMMAND_DRAW_GEOMETRY)) && testFlag(MOUSE_PRESSED)) &&
            !testFlag(OSR_INVALID) &&
            !testFlag(TEXTURE_INVALID) &&
            !testFlag(AUX_TEXTURE_INVALID) &&
            !testFlag(CUSTOM_TEXTURE_INVALID))
        {
            setFlag(FAST_REDRAW);
        }

        // Local copy of internal flags
        flags = m_Flags;
        clearFlag(TEXTURE_INVALID |
            AUX_TEXTURE_INVALID |
            CUSTOM_TEXTURE_INVALID |
            OSR_INVALID |
            LUT_INVALID |
            FAST_REDRAW);

        // New texture data?
        if (flags & TEXTURE_INVALID)
        {
            internalUploadTexture();
        }

        if (flags & AUX_TEXTURE_INVALID)
        {
            internalUploadAuxTexture();
        }

        // New texture data?
        if (flags & CUSTOM_TEXTURE_INVALID)
        {
            switch (m_currentType)
            {
            case EVT_BOOL:
                internalUploadCustomTexture_bool();
                break;

            case EVT_PIXEL8:
                internalUploadCustomTexture_tPixel8();
                break;

            case EVT_PIXEL16:
                internalUploadCustomTexture_tPixel16();
                break;

            case EVT_RGBPIXEL:
                internalUploadCustomTexture_tRGBPixel();
                break;
            }

            internalUploadCustomAuxTexture();
        }

        // Create a local copy of current rendering parameters
        params = *m_spParams;
    }
    // END: Locked part of the rendering

    auto preViewport = renderInfo.getCurrentCamera()->getViewport();

    osg::Vec2i viewportSize = osg::Vec2i(preViewport->width(), preViewport->height());
    osg::Vec2i renderingSize = getRenderingSize(params, viewportSize, flags);

    auto newViewport = new osg::Viewport(0, 0, renderingSize.x(), renderingSize.y());

    // Resolution changed?
    if (flags & OSR_INVALID)
    {
        internalSetRenderingSize(params, viewportSize, renderingSize, flags);
    }

    // LUT changed?
    if (flags & LUT_INVALID)
    {
        internalSetLUT(&params);
    }

    float Y = static_cast<float>(params.YSize) / params.XSize * params.aspectRatio_YtoX;
    float Z = static_cast<float>(params.ZSize) / params.XSize * params.aspectRatio_ZtoX;

    osg::Matrix prevModelViewMatrix = renderInfo.getState()->getModelViewMatrix();
    osg::Matrix quadModelViewMatrix = osg::Matrix::scale(params.RealXSize * 0.5f, params.RealXSize * 0.5f, params.RealXSize * 0.5f) * prevModelViewMatrix;
    osg::Matrix boxModelViewMatrix = osg::Matrix::scale(1.0f, Y, Z) * quadModelViewMatrix;

    params.invModelViewMatrix = osg::Matrix::inverse(boxModelViewMatrix);
    params.invProjectionMatrix = osg::Matrix::inverse(renderInfo.getState()->getProjectionMatrix());

    auto stateSet = new osg::StateSet();
    renderInfo.getState()->captureCurrentState(*stateSet);

    m_fboDepth->apply(*renderInfo.getState(), osg::FrameBufferObject::DRAW_FRAMEBUFFER);

    renderInfo.getState()->haveAppliedAttribute(m_fboDepth);

    tridimGlR("glBlitFramebuffer", glBlitFramebuffer(0, 0, viewportSize.x(), viewportSize.y(), 0, 0, viewportSize.x(), viewportSize.y(), GL_DEPTH_BUFFER_BIT, GL_NEAREST););

    renderInfo.getState()->apply(stateSet);

    renderInfo.getState()->applyModelViewMatrix(boxModelViewMatrix);

    // if fast redraw is not set, render volume
    if (!(flags & FAST_REDRAW))
    {
        tridimGlR("glClearColor", glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

        // render front side polygons of the box
        renderInfo.getState()->apply(m_stateSetFrontBox);
        renderInfo.getState()->applyModelViewAndProjectionUniformsIfRequired();
        renderInfo.getState()->applyAttribute(newViewport);

        tridimGlR("glClear", glClear(GL_COLOR_BUFFER_BIT));

        m_box->draw(renderInfo);

        // render back side polygons of the box
        renderInfo.getState()->apply(m_stateSetBackBox);
        renderInfo.getState()->applyModelViewAndProjectionUniformsIfRequired();
        renderInfo.getState()->applyAttribute(newViewport);

        tridimGlR("glClear", glClear(GL_COLOR_BUFFER_BIT));

        m_box->draw(renderInfo);

        updateShaderUniforms(params, renderingSize, flags);

        if (params.selectedShader == EShaders::CUSTOM)
        {
            shaderUpdateCallback();
        }

        m_stateSetVolumeRender->setTextureAttribute(2, m_textureLookUp);


        renderInfo.getState()->apply(m_stateSetVolumeRender);
        renderInfo.getState()->applyModelViewAndProjectionUniformsIfRequired();
        renderInfo.getState()->applyAttribute(newViewport);

        tridimGlR("glClear", glClear(GL_COLOR_BUFFER_BIT));

        m_box->draw(renderInfo);
    }

    m_uniform_image->set(4);
    m_uniform_kernel->set(6);

    m_uniform_resolution->set(osg::Vec2(renderingSize.x(), renderingSize.y()));

    renderInfo.getState()->apply(m_stateSetResize);
    renderInfo.getState()->applyModelViewMatrix(quadModelViewMatrix);
    renderInfo.getState()->applyModelViewAndProjectionUniformsIfRequired();
    renderInfo.getState()->applyAttribute(preViewport);

    m_quad->draw(renderInfo);

    renderInfo.getState()->applyModelViewMatrix(prevModelViewMatrix);
    renderInfo.getState()->applyModelViewAndProjectionUniformsIfRequired();
}

void PSVolumeRendering::updateShaderUniforms(PSVR::PSVolumeRendering::PSVolumeRenderingParams& params, const osg::Vec2i& newViewport, int flags)
{
    float vsd = getVolumeSamplingDistance(&params, flags);

    m_uniform_t3D->set(0);
    m_uniform_tSkip3D->set(1);
    m_uniform_LookUp->set(2);
    m_uniform_Noise->set(3);
    m_uniform_tRaysStartEnd->set(4);
    m_uniform_Depth->set(5);

    if (params.selectedShader == EShaders::CUSTOM)
    {
        m_uniform_tCustom3D->set(7);

        m_uniform_sVector->set(osg::Vec3(1.25f / float(params.CustomXSize), 1.25f / float(params.CustomYSize), 1.25f / float(params.CustomZSize)));
        m_uniform_skipTexSize->set(osg::Vec3(params.CustomAuxTexXSize, params.CustomAuxTexYSize, params.CustomAuxTexZSize));
        m_uniform_tResolution->set(osg::Vec3(vsd / float(params.CustomXSize), vsd / float(params.CustomYSize), vsd / float(params.CustomZSize)));
        m_uniform_tSkipResolution->set(osg::Vec3(8.0f / float(params.CustomXSize), 8.0f / float(params.CustomYSize), 8.0f / float(params.CustomZSize)));
    }
    else
    {
        m_uniform_sVector->set(osg::Vec3(1.25f / float(params.XSize), 1.25f / float(params.YSize), 1.25f / float(params.ZSize)));
        m_uniform_skipTexSize->set(osg::Vec3(params.AuxTexXSize, params.AuxTexYSize, params.AuxTexZSize));
        m_uniform_tResolution->set(osg::Vec3(vsd / float(params.XSize), vsd / float(params.YSize), vsd / float(params.ZSize)));
        m_uniform_tSkipResolution->set(osg::Vec3(8.0f / float(params.XSize), 8.0f / float(params.YSize), 8.0f / float(params.ZSize)));
    }

    m_uniform_textureSampling->set(vsd);
    m_uniform_inputAdjustment->set(osg::Vec2(params.dataPreMultiplication, -params.dataOffset));
    m_uniform_imageAdjustment->set(osg::Vec2(params.imageBrightness, params.imageContrast));
    m_uniform_StopCondition->set(0.01f);
    m_uniform_wSize->set(osg::Vec2(1.0f / newViewport.x(), 1.0f / newViewport.y()));
    m_uniform_pl->set(osg::Vec4(params.planeA, params.planeB, params.planeC, params.planeD + params.planeDeltaNear * Sqrt3Div2));
    m_uniform_plNear->set(osg::Vec4(params.planeA, params.planeB, params.planeC, params.planeD + params.planeDeltaNear * Sqrt3Div2));
    m_uniform_plFar->set(osg::Vec4(params.planeA, params.planeB, params.planeC, params.planeD + params.planeDeltaFar * Sqrt3Div2));
    m_uniform_invProjectionMatrix->set(params.invProjectionMatrix);
    m_uniform_invModelViewMatrix->set(params.invModelViewMatrix);
    m_uniform_skipCondition->set(m_skipConditions[static_cast<int>(params.selectedLut)]);

    // surface rendering parameters
    if (params.selectedShader == EShaders::SURFACE)
    {
        m_uniform_surfacePar->set(osg::Vec2(params.surfaceNormalMult, params.surfaceNormalExp));
    }
}

bool PSVolumeRendering::init()
{
    tLock Lock(*this);

    // Already initialized?
    if (testFlag(INITIALIZED))
    {
        return true;
    }

    // already
    if (!testFlag(INIT_INVALID))
    {
        setFlag(INIT_INVALID);
    }

    if (testFlag(INIT_INVALID))
    {
        if (!internalInitRendering())
        {
            m_FailureCounter++;
            return false;
        }
        else
        {
            m_FailureCounter = 0;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Resets initialization failure counter
void PSVolumeRendering::resetFailureCounter()
{
    m_FailureCounter = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Returns if init has failed too many times in a row
bool PSVolumeRendering::constantFailure()
{
    return (m_FailureCounter > PSVR_FAIL_LIMIT);
}

///////////////////////////////////////////////////////////////////////////////
// are we able to render?
void PSVolumeRendering::setMouseMode(bool bEnable)
{
    tLock Lock(*this);

    if (bEnable)
    {
        setAndSignalFlag(MOUSE_MODE | OSR_INVALID);
    }
    else if(testFlag(MOUSE_MODE))
    {
        clearFlag(MOUSE_MODE);
        setAndSignalFlag(OSR_INVALID);
    }
}

////////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setMousePressed(bool bPressed)
{
    tLock Lock(*this);

    if (bPressed)
    {
        setFlag(MOUSE_PRESSED);
    }
    else
    {
        clearFlag(MOUSE_PRESSED);
    }
}

//////////////////////////////////////////////////////////////////////////////
//
void PSVolumeRendering::setShader(EShaders shader)
{
    tLock Lock(*this);

    if (shader == m_spParams->selectedShader)
        return;

    m_spParams->selectedShader = shader;

    if (testFlag(INITIALIZED))
    {
        setShaderInternal(shader);
    }

    VPL_SIGNAL(SigVRModeChange).invoke(static_cast<int>(shader));
}

void PSVolumeRendering::setLut(ELookups lut)
{
    tLock Lock(*this);

    m_spParams->selectedLut = lut;

    setFlag(LUT_INVALID);

    VPL_SIGNAL(SigVRLutChange).invoke(static_cast<int>(lut));
}

void PSVolumeRendering::setQuality(int quality)
{
    if (quality < 0 || quality >= QUALITY_LEVELS)
    {
        return;
    }

    tLock Lock(*this);

    if (m_spParams->currentQuality == quality)
    {
        return;
    }

    // set new quality
    m_spParams->currentQuality = quality;
    m_spParams->volumeSamplingDistance = conf::TextureSampling[quality];
    m_spParams->renderingSize = conf::RenderingSize[quality];

    setAndSignalFlag(OSR_INVALID | DATA_INVALID);

    VPL_SIGNAL(SigVRQualityChange).invoke(quality);
}

void PSVolumeRendering::setDataRemap(float expand, float offset)
{
    tLock Lock(*this);

    m_spParams->dataPreMultiplication = expand;
    m_spParams->dataOffset = offset;

    //VPL_LOG_INFO("PSVolumeRendering::setDataRemap(): mult = " << m_spParams->dataPreMultiplication << ", offset = " << m_spParams->dataOffset);
    VPL_SIGNAL(SigVRDataRemapChange).invoke(expand, offset);
}

void PSVolumeRendering::getDataRemap(float& expand, float& offset)
{
    tLock Lock(*this);

    expand = m_spParams->dataPreMultiplication;
    offset = m_spParams->dataOffset;
}

void PSVolumeRendering::setPicture(float brightness, float contrast)
{
    tLock Lock(*this);

    m_spParams->imageBrightness = brightness;
    m_spParams->imageContrast = contrast;
}

void PSVolumeRendering::setSurfaceDetection(float mult, float exp)
{
    tLock Lock(*this);

    m_spParams->surfaceNormalMult = mult;
    m_spParams->surfaceNormalExp = exp;
}

void PSVolumeRendering::setRenderingSize(int size)
{
    if (size < 0)
    {
        return;
    }

    tLock Lock(*this);

    m_spParams->renderingSize = size;
}

void PSVolumeRendering::setCuttingPlane(float a, float b, float c, float d)
{
    tLock Lock(*this);

    m_spParams->planeA = a;
    m_spParams->planeB = b;
    m_spParams->planeC = c;
    m_spParams->planeD = d;
}

void PSVolumeRendering::setCuttingPlaneDisplacement(float deltaNear, float deltaFar)
{
    tLock Lock(*this);

    vpl::math::limit<float>(deltaNear, -1.0f, 1.0f);
    vpl::math::limit<float>(deltaFar, -1.0f, 1.0f);
    m_spParams->planeDeltaNear = deltaNear;
    m_spParams->planeDeltaFar = deltaFar;
}

void PSVolumeRendering::setNearCuttingPlaneDisplacement(float delta)
{
    vpl::math::limit<float>(delta, -1.0f, 1.0f);
    setCuttingPlaneDisplacement(delta, m_spParams->planeDeltaFar);
}

void PSVolumeRendering::setFarCuttingPlaneDisplacement(float delta)
{
    vpl::math::limit<float>(delta, -1.0f, 1.0f);
    setCuttingPlaneDisplacement(m_spParams->planeDeltaNear, delta);
}

void PSVolumeRendering::getCuttingPlane(float &a, float &b, float &c, float &d)
{
    a = m_spParams->planeA;
    b = m_spParams->planeB;
    c = m_spParams->planeC;
    d = m_spParams->planeD;
}

float PSVolumeRendering::getNearCuttingPlaneDisplacement()
{
    return m_spParams->planeDeltaNear;
}

float PSVolumeRendering::getFarCuttingPlaneDisplacement()
{
    return m_spParams->planeDeltaFar;
}

void PSVolumeRendering::setSamplingDistance(float distance)
{
    if (distance < 0.0f)
    {
        return;
    }

    tLock Lock(*this);

    m_spParams->volumeSamplingDistance = distance;
}

///////////////////////////////////////////////////////////////////////////////
// get methods
PSVolumeRendering::ELookups PSVolumeRendering::getLut() const
{
    return m_spParams->selectedLut;
}

PSVolumeRendering::EShaders PSVolumeRendering::getShader() const
{
    return m_spParams->selectedShader;
}

int  PSVolumeRendering::getQuality() const
{
    return m_spParams->currentQuality;
}

} // namesapce PSVR

#endif // USE_PSVR
