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

#ifndef PSVRrenderer_H
#define PSVRrenderer_H

#include <configure.h>

#ifdef USE_PSVR

#include <VPL/Base/Lock.h>
#include <VPL/System/Thread.h>
#include <VPL/System/Condition.h>
#include <VPL/System/Stopwatch.h>

#include <render/CVolumeRenderer.h>

#include <osg/Texture3D>
#include <osg/FrameBufferObject>

#define PSVR_FAIL_LIMIT 10


///////////////////////////////////////////////////////////////////////////////
// forward declarations
class OSGCanvas;

namespace PSVR
{
///////////////////////////////////////////////////////////////////////////////
//! Volume rendering routines suitable for OSG
class PSVolumeRendering : public vpl::base::CLockableObject<PSVolumeRendering>, public CVolumeRenderer
{
public:
    // Friend classes
    friend class osgPSVolumeRendering;
    friend class osgPSVolumeRenderingGeode;

    //! Predefined shaders.
    enum class EShaders
    {
        XRAY = 0,
        MIP,
        SHADING,
        ADDITIVE,
        SURFACE,
        CUSTOM,

        SHADERS_COUNT
    };

    //! Predefined lookup tables.
    enum class ELookups
    {
        MIP_SOFT = 0,
        MIP_HARD,
        XRAY_SOFT,
        XRAY_HARD,
        SHA_AIR,
        SHA_TRAN,
        SHA_BONE0,
        SHA_BONE1,
        SHA_BONE2,
        SURFACE_SKIN,
        SURFACE_BONE,

        LOOKUPS_COUNT
    };

    //! Rendering quality.
    enum EQuality
    {
        VERY_LOW_QUALITY = 0,
        LOW_QUALITY,
        MEDIUM_QUALITY,
        HIGH_QUALITY,

        QUALITY_LEVELS
    };

    //! Error codes.
    enum EError
    {
        PSVR_NO_ERROR = 0,
        DATA_NOT_SPECIFIED = 1 << 0,
        GLEW_INIT_FAILED = 1 << 1,
        UNDEFINED_GL_CANVAS = 1 << 2,
        UNSUPPORTED_SHADER_MODEL = 1 << 3,
        UNSUPPORTED_GRAPHIC_CARD = 1 << 4,
        LOOKUP_NOT_FOUND = 1 << 5,
        SHADER_NOT_FOUND = 1 << 6,
        INIT_FAILED = 1 << 7,
        CANNOT_CREATE_3D_TEXTURE = 1 << 8,
        CUSTOM_DATA_NOT_SPECIFIED = 1 << 9,
    };

    //! Scoped lock.
    typedef vpl::base::CLockableObject<PSVolumeRendering>::CLock tLock;

public:
    //! Default constructor.
    PSVolumeRendering();

    //! Destructor.
    virtual ~PSVolumeRendering();

    // implementing CVolumeRenderer interface
public:
    // window
    virtual QSize getWindowSize() override;

    // transform matrix
    virtual osg::Matrix getWorldMatrix() override;
    virtual osg::Matrix getViewMatrix() override;
    virtual osg::Matrix getProjectionMatrix() override;
    virtual osg::Matrix getTransformMatrix() override;

    // lookup tables
    virtual void resetLookupTables() override;
    virtual void updateLookupTables(std::string lutName = "") override;

    void setCustomShaderUniforms(std::vector<osg::ref_ptr<osg::Uniform>>&& uniforms);

protected:
    // custom volume
    virtual void internalSetDataToCustomVolume() override;

    // custom shader
    virtual void internalCreateCustomShader(std::string vertexShaderSource, std::string fragmentShaderSource) override;

public:
    //! Returns pointer to the used canvas.
    OSGCanvas * getCanvas() { return m_pCanvas; }

    //! Sets pointer to canvas.
    PSVolumeRendering& setCanvas(OSGCanvas * pCanvas)
    {
        m_pCanvas = pCanvas;
        return *this;
    }

    //! Enforce redrawing of the OpenGL canvas.
    virtual void redraw(bool bEraseBackground = false) override;

    //! Enables the rendering.
    virtual void enable(bool bEnable = true) override;
    
    //! Returns true of the rendering is enabled.
    virtual bool isEnabled() const override;

    //! Initializes the volume rendering.
    //! - This method must be called once at the begining!
    //! - A valid pointer to the GL canvas must be already set!
    virtual bool init() override;

    //! Resets initialization failure counter
    void resetFailureCounter();

    //! Returns if init has failed too many times in a row
    bool constantFailure();

    //! Returns true if some error has occured during the initialization phase
    //! or during the rendering itself.
    bool isError() const { return ((m_Error & (0xffff - DATA_NOT_SPECIFIED)) != 0); }

    //! Returns the current error code.
    int getStatus() const { return m_Error; }

    //! Disables volume rendering and frees all resources.
    void release();

    //! Uploads new volumetric data.
    void uploadData(vpl::img::CDensityVolume * Data);

    //! Updates dimensions of render targets
    void updateRenderTargets();

private:
    //! Creates default lookup tables
    void createLookupTables();

    //! Updates internal lookup table (converts from point representation to pixel representation)
    void updateLookupTable(CLookupTable &lookupTable, unsigned short *internalLookupTable, osg::Vec4 &skipCondition);

    void setNewTransformMatrix(osg::Matrix& newTransformMatrix, double distance);

	vpl::mod::tSignalConnection noteMatrixSignalConnection;
public:
    // Methods for VR control
    ELookups getLut() const;
    EShaders getShader() const;
    int getQuality() const;

    void setMouseMode(bool bEnable);
    void setMousePressed(bool bPressed);
    void setShader(EShaders shader);
    void setLut(ELookups lut);
    void setQuality(int quality);
    void setSamplingDistance(float distance);
    void setPicture(float brightness, float contrast);
    void setSurfaceDetection(float mult, float exp);
    void setDataRemap(float expand, float offset);
    void setRenderingSize(int size);
    void setCuttingPlane(float a, float b, float c, float d);
    void setCuttingPlaneDisplacement(float deltaNear, float deltaFar = 0.0f);
    void setNearCuttingPlaneDisplacement(float delta);
    void setFarCuttingPlaneDisplacement(float delta);

    void getCuttingPlane(float &a, float &b, float &c, float &d);
    float getNearCuttingPlaneDisplacement();
    float getFarCuttingPlaneDisplacement();
    void getDataRemap(float &expand, float &offset);

    //! Returns real size of the currently used volume data.
    float getRealXSize() const;
    float getRealYSize() const;
    float getRealZSize() const;

    //! Renders the data.
    //! - This method is usually called during 'OSG drawable' rendering.
    void renderVolume(osg::RenderInfo& renderInfo);

    //! Sets specified flags.
    void setFlag(int Flag);

    //! Sets specified flags.
    void setAndSignalFlag(int Flag);

    //! Clears specified flags.
    void clearFlag(int Flag);

    //! Returns true if a given flag is set.
    bool testFlag(int Flag);

    //! Internal flags.
    enum EFlag
    {
        PSVR_NO_FLAGS    = 0,

        //! First-time init.
        INITIALIZED     = 1 << 0,

        //! user is manipulating mouse.
        MOUSE_MODE      = 1 << 1,

        //! Initialize the renderer.
        INIT_INVALID    = 1 << 2,

        //! Re-send LUT.
        LUT_INVALID     = 1 << 3,

        //! Re-size render textures.
        OSR_INVALID     = 1 << 4,

        //! Prepare density data.
        DATA_INVALID    = 1 << 5,

        //! 3D texture with density data is invalid.
        TEXTURE_INVALID = 1 << 6,

        //! Auxiliary 3D texture with density data is invalid.
        AUX_TEXTURE_INVALID = 1 << 7,

        //! Prepare custom data
        CUSTOM_DATA_INVALID = 1 << 8,

        //! 3D texture with custom data is invalid
        CUSTOM_TEXTURE_INVALID = 1 << 9,

        //! Skip redrawing of volume and just use previously rendered image
        FAST_REDRAW = 1 << 11,

        //! Mouse pressed flag
        MOUSE_PRESSED = 1 << 12,

        //! All "invalid" flags...
        INVALID = INIT_INVALID | LUT_INVALID | OSR_INVALID | DATA_INVALID | TEXTURE_INVALID | AUX_TEXTURE_INVALID | CUSTOM_DATA_INVALID | CUSTOM_TEXTURE_INVALID,
    };

protected:
    //! Predefined constants...
    enum EConf
    {
        //! Init size of volumes
        INIT_SIZE = 32,

        //! Size of the 1D color lookup tables.
        LUT_2D_W = 4096,
        LUT_2D_H = 128,

        //! Size of the noise table.
        NOISE_SIZE = 1024,

        LUT_SIZE = 512,
    };

    // Forward declaration (rendering parameters).
    struct PSVolumeRenderingParams;

protected:
    void storeVolumeToTexture(vpl::img::CVolume<vpl::img::tPixel8>& volume, osg::Texture3D* texture);
    void storeVolumeToTexture(vpl::img::CVolume<vpl::img::tPixel16>& volume, osg::Texture3D* texture);
    void storeVolumeToTexture(vpl::img::CVolume<vpl::img::tRGBPixel>& volume, osg::Texture3D* texture);

    //! First-time initialization of the OpenGL.
    bool internalInitRendering();

    void setShaderInternal(EShaders shader);

    //! Prepares the volume data.
    bool internalUploadData();

    //! Uploads the 3D texture.
    bool internalUploadTexture();
    bool internalUploadAuxTexture();

    //! Prepares custom data.
    bool internalUploadCustomData_bool();
    bool internalUploadCustomData_tPixel8();
    bool internalUploadCustomData_tPixel16();
    bool internalUploadCustomData_tRGBPixel();

    //! Uploads the 3D texture with custom data.
    bool internalUploadCustomTexture_bool();
    bool internalUploadCustomTexture_tPixel8();
    bool internalUploadCustomTexture_tPixel16();
    bool internalUploadCustomTexture_tRGBPixel();
    bool internalUploadCustomAuxTexture();

    //! Changes rendering resolution.
    bool internalSetRenderingSize(PSVolumeRenderingParams& params, const osg::Vec2i& currentViewport, const osg::Vec2i& newViewport, int flags);

    //! Changes color lookup table.
    bool internalSetLUT(PSVolumeRenderingParams *pParams);

    //! Helper thread used to prepare volume data, etc.
    static VPL_THREAD_ROUTINE(setupLoop);

    //! Returns the rendering resolution.
    //! - The value depends on the current rendering resolution
    //!   and whether the mouse mode is enabled.
    osg::Vec2i getRenderingSize(PSVolumeRenderingParams& params, const osg::Vec2i& currentViewport, int flags) const;

    //! Return the 3D texture sampling step along a ray.
    //! - The value depends on the current rendering resolution
    //!   and whether the mouse mode is enabled.
    float getVolumeSamplingDistance(PSVolumeRenderingParams *pParams, int Flags) const;

    void updateShaderUniforms(PSVR::PSVolumeRendering::PSVolumeRenderingParams& params, const osg::Vec2i& newViewport, int flags);

protected:
    //! Used canvas.
    OSGCanvas* m_pCanvas;

    //! Enables/disables volume rendering.
    volatile bool m_Enabled;

    //! True in case of some error.
    volatile int m_Error;

    //! Internal flags.
    volatile int m_Flags;

    //! Init failure counter
    volatile int m_FailureCounter;

    //! Mutex for mutual access to internal flags.
    vpl::sys::CMutex m_Mutex;

    //! Condition signalled on any change of internal flags.
    vpl::sys::CCondition m_Condition;

    //! Helper thread.
    vpl::sys::CThread m_Thread;

    //! Rendering parameters.
    vpl::base::CScopedPtr<PSVolumeRenderingParams> m_spParams;

    //! Original volume data.
    vpl::img::CDensityVolume::tSmartPtr m_spVolumeData;

    //! Pre-processed volume data
    vpl::img::CVolume<vpl::img::tPixel16> m_VolumeData;

    //! Pre-processed subvolume data
    vpl::img::CVolume<vpl::img::tRGBPixel> m_AuxVolumeData;

    //! Pre-processed custom data
    vpl::img::CVolume<vpl::img::tPixel8> m_customData_bool;
    vpl::img::CVolume<vpl::img::tPixel8> m_customData_tPixel8;
    vpl::img::CVolume<vpl::img::tPixel16> m_customData_tPixel16;
    vpl::img::CVolume<vpl::img::tRGBPixel> m_customData_tRGBPixel;
    vpl::img::CVolume<vpl::img::tPixel8> m_auxCustomData;

protected:
    void prepareBox(int numOfQuads);
    void prepareQuad();

protected:
    std::vector<unsigned short*> m_internalLookupTables;
    std::vector<osg::Vec4> m_skipConditions;

    osg::ref_ptr<osg::Geometry> m_box;
    osg::ref_ptr<osg::Geometry> m_quad;

    std::map<EShaders, osg::ref_ptr<osg::Program>> m_shaders;

    osg::ref_ptr<osg::Program> m_shaderOsrt;
    osg::ref_ptr<osg::Program> m_shaderResize;

    osg::ref_ptr<osg::Uniform> m_uniform_t3D;
    osg::ref_ptr<osg::Uniform> m_uniform_tSkip3D;
    osg::ref_ptr<osg::Uniform> m_uniform_LookUp;
    osg::ref_ptr<osg::Uniform> m_uniform_Noise;
    osg::ref_ptr<osg::Uniform> m_uniform_tRaysStartEnd;
    osg::ref_ptr<osg::Uniform> m_uniform_Depth;

    //CUSTOM
    std::vector<osg::ref_ptr<osg::Uniform>> m_customUniforms;
    osg::ref_ptr<osg::Uniform> m_uniform_tCustom3D;
    //CUSTOM

    osg::ref_ptr<osg::Uniform> m_uniform_textureSampling;
    osg::ref_ptr<osg::Uniform> m_uniform_inputAdjustment;
    osg::ref_ptr<osg::Uniform> m_uniform_imageAdjustment;
    osg::ref_ptr<osg::Uniform> m_uniform_sVector;
    osg::ref_ptr<osg::Uniform> m_uniform_skipTexSize;
    osg::ref_ptr<osg::Uniform> m_uniform_tResolution;
    osg::ref_ptr<osg::Uniform> m_uniform_tSkipResolution;
    osg::ref_ptr<osg::Uniform> m_uniform_StopCondition;
    osg::ref_ptr<osg::Uniform> m_uniform_wSize;
    osg::ref_ptr<osg::Uniform> m_uniform_pl;
    osg::ref_ptr<osg::Uniform> m_uniform_plNear;
    osg::ref_ptr<osg::Uniform> m_uniform_plFar;
    osg::ref_ptr<osg::Uniform> m_uniform_invProjectionMatrix;
    osg::ref_ptr<osg::Uniform> m_uniform_invModelViewMatrix;
    osg::ref_ptr<osg::Uniform> m_uniform_skipCondition;

    //SURFACE
    osg::ref_ptr<osg::Uniform> m_uniform_surfacePar;
    //SURFACE

    osg::ref_ptr<osg::Uniform> m_uniform_image;
    osg::ref_ptr<osg::Uniform> m_uniform_kernel;
    osg::ref_ptr<osg::Uniform> m_uniform_resolution;

    osg::ref_ptr<osg::StateSet> m_stateSetFrontBox;
    osg::ref_ptr<osg::StateSet> m_stateSetBackBox;
    osg::ref_ptr<osg::StateSet> m_stateSetVolumeRender;
    osg::ref_ptr<osg::StateSet> m_stateSetVolumeRenderBackup;
    osg::ref_ptr<osg::StateSet> m_stateSetResize;

    osg::ref_ptr<osg::FrameBufferObject> m_fboFrontBox;
    osg::ref_ptr<osg::FrameBufferObject> m_fboBackBox;
    osg::ref_ptr<osg::FrameBufferObject> m_fboDepth;
    osg::ref_ptr<osg::FrameBufferObject> m_fboVolumeRender;

    osg::ref_ptr<osg::Texture3D> m_textureRaysStartEnd;
    osg::ref_ptr<osg::Texture2D> m_textureVolumeRender;

    osg::ref_ptr<osg::Texture2D> m_textureDepth;

    osg::ref_ptr<osg::Texture3D> m_textureVolume;
    osg::ref_ptr<osg::Texture3D> m_textureAuxVolume;
    osg::ref_ptr<osg::Texture3D> m_textureCustomAuxVolume;
    osg::ref_ptr<osg::Texture3D> m_textureCurrentVolume;
    osg::ref_ptr<osg::Texture2D> m_textureLookUp;
    osg::ref_ptr<osg::Texture1D> m_textureBicubicKernel;
    osg::ref_ptr<osg::Texture2D> m_textureNoise;
};

} // namespace PSVR

#endif // USE_PSVR

#endif // PSVRrenderer_H
