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

#ifndef PSVRrenderer_H
#define PSVRrenderer_H

#include "AppConfigure.h"
#include <configure.h>

// Check predefined type of volume rendering algorithm
#ifdef USE_PSVR

///////////////////////////////////////////////////////////////////////////////
// includes
#include <VPL/Base/Lock.h>
#include <VPL/System/Thread.h>
#include <VPL/System/Condition.h>
#include <VPL/System/Mutex.h>
#include <VPL/Image/DensityVolume.h>
#include <VPL/Image/Point3.h>
#include <VPL/System/Stopwatch.h>
#include <QWidget>
#include <osg/Array>

#include <render/CVolumeRenderer.h>

// STL
#include <vector>

//! Enables dynamic loading of shaders and lookup tables at runtime.
#ifndef LOAD_SHADERS
//#define LOAD_SHADERS
#endif

//! Enables 16bit 3D texture.
//#ifndef FULL_3D_TEXTURE
#define FULL_3D_TEXTURE
//#endif

#define PSVR_FAIL_LIMIT 10

///////////////////////////////////////////////////////////////////////////////
// forward declarations
class OSGCanvas;

namespace PSVR
{

///////////////////////////////////////////////////////////////////////////////
// forward declarations
class osgPSVolumeRendering;
class osgPSVolumeRenderingGeode;

///////////////////////////////////////////////////////////////////////////////
//! Volume rendering routines suitable for OSG
class PSVolumeRendering : public vpl::base::CLockableObject<PSVolumeRendering>, public CVolumeRenderer
{
public:
    //! Predefined shaders.
    enum EShaders
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
    enum ELookups
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
        PSVR_NO_ERROR               = 0,
        DATA_NOT_SPECIFIED          = 1 << 0,
        GLEW_INIT_FAILED            = 1 << 1,
        UNDEFINED_GL_CANVAS         = 1 << 2,
        UNSUPPORTED_SHADER_MODEL    = 1 << 3,
        UNSUPPORTED_GRAPHIC_CARD    = 1 << 4,
        LOOKUP_NOT_FOUND            = 1 << 5,
        SHADER_NOT_FOUND            = 1 << 6,
        INIT_FAILED                 = 1 << 7,
        CANNOT_CREATE_3D_TEXTURE    = 1 << 8,
        CUSTOM_DATA_NOT_SPECIFIED   = 1 << 9,
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
    //! returns flag if custom shader is really used a displayed
    virtual bool isCustomShaderActive();

    // window
    virtual QSize getWindowSize();

    // transform matrix
    virtual osg::Matrix getWorldMatrix();
    virtual osg::Matrix getViewMatrix();
    virtual osg::Matrix getProjectionMatrix();
    virtual osg::Matrix getTransformMatrix();

    // custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, int value);
    virtual void setParameter(unsigned int shaderId, std::string name, float value);
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec2 value);
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec3 value);
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec4 value);
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Matrix value);
    virtual void setParameter(unsigned int shaderId, std::string name, int *value, int count);
    virtual void setParameter(unsigned int shaderId, std::string name, float *value, int count);
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec2 *value, int count);
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec3 *value, int count);
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec4 *value, int count);
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Matrix *value, int count);

    // custom volume
    virtual void getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<bool> &volume);
    virtual void getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel16> &volume);
    virtual void getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel8> &volume);
    virtual void getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tRGBPixel> &volume);

    // lookup tables
    virtual void resetLookupTables();
    virtual void updateLookupTables();

protected:
    // custom volume
    virtual unsigned int internalCreateCustomVolume();
    virtual void internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<bool> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume);
    virtual void internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel16> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume);
    virtual void internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel8> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume);
    virtual void internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tRGBPixel> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume);
    virtual void internalDeleteCustomVolume(unsigned int volumeId);

    // custom shader
    unsigned int internalCreateCustomShader(std::string vertexShaderSource, std::string fragmentShaderSource);
    void internalUseCustomShader(unsigned int shaderId);
    void internalDeleteCustomShader(unsigned int shaderId);

private:
    std::map<unsigned int, std::vector<unsigned int> > m_programShaders;
    unsigned int m_customShaderId;
    std::vector<unsigned short *> m_internalLookupTables;
    std::vector<float> m_skipConditions;

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
    virtual void redraw(bool bEraseBackground = false);

    //! Enables the rendering.
    virtual void enable(bool bEnable = true);

    //! Reloads current shader from file
    void reloadShader();
    
    //! Returns true of the rendering is enabled.
    virtual bool isEnabled() const { return m_Enabled; }

    //! Checks the shader model, graphic memory, etc.
    bool canStart();

    //! Initializes the volume rendering.
    //! - This method must be called once at the begining!
    //! - A valid pointer to the GL canvas must be already set!
    virtual bool init();

    //! Resets initialization failure counter
    void resetFailureCounter();

    //! Returns if init has failed too many times in a row
    bool constantFailure();

    //! Returns all error strings
    std::vector<std::string> getErrorStrings();

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
    void updateLookupTable(CLookupTable &lookupTable, unsigned short *internalLookupTable, float &skipCondition);

public:
    // Methods for VR control
    int getLut() const;
    int getShader() const;
    int getQuality() const;

    void setMouseMode(bool bEnable);
    void setMousePressed(bool bPressed);
    void setShader(int shader);
    void setLut(int lut);
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
    void renderVolume();

protected:
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

    // Forward declaration (OpenGL variables).
    struct PSVolumeRenderingData;

    // Forward declaration (rendering parameters).
    struct PSVolumeRenderingParams;

protected:
    void storeVolumeToTexture(vpl::img::CVolume<vpl::img::tPixel8> volume);
    void storeVolumeToTexture(vpl::img::CVolume<vpl::img::tPixel16> volume);
    void storeVolumeToTexture(vpl::img::CVolume<vpl::img::tRGBPixel> volume);

    //! Checks the shader model, graphic memory, etc.
    bool internalCanStart();

    //! First-time initialization of the OpenGL.
    bool internalInitRendering();

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
    bool internalSetRenderingSize(PSVolumeRenderingParams *pParams, int Flags);

    //! Changes color lookup table.
    bool internalSetLUT(PSVolumeRenderingParams *pParams);

    //! Helper thread used to prepare volume data, etc.
    static VPL_THREAD_ROUTINE(setupLoop);

    //! Sets specified flags.
    void setFlag(int Flag);

    //! Sets specified flags.
    void setAndSignalFlag(int Flag);

    //! Clears specified flags.
    void clearFlag(int Flag);

    //! Returns true if a given flag is set.
    bool testFlag(int Flag);

    //! Returns the rendering resolution.
    //! - The value depends on the current rendering resolution
    //!   and whether the mouse mode is enabled.
    vpl::img::CPoint3D getRenderingSize(PSVolumeRenderingParams *pParams, int Flags) const;

    //! Return the 3D texture sampling step along a ray.
    //! - The value depends on the current rendering resolution
    //!   and whether the mouse mode is enabled.
    float getVolumeSamplingDistance(PSVolumeRenderingParams *pParams, int Flags) const;

protected:
    //! Pre-processed volume data
#ifdef FULL_3D_TEXTURE
    typedef vpl::img::CVolume<vpl::img::tPixel16> tVolumeData;
#else
    typedef vpl::img::CVolume<vpl::img::tPixel8> tVolumeData;
#endif // FULL_3D_TEXTURE

    //! Pre-processed subvolume data
    typedef vpl::img::CVolume<vpl::img::tRGBPixel> tAuxVolumeData;

    //! Pre-processed custom data
    typedef vpl::img::CVolume<vpl::img::tPixel8> tCustomData_bool;
    typedef vpl::img::CVolume<vpl::img::tPixel8> tCustomData_tPixel8;
    typedef vpl::img::CVolume<vpl::img::tPixel16> tCustomData_tPixel16;
    typedef vpl::img::CVolume<vpl::img::tRGBPixel> tCustomData_tRGBPixel;

    //! Pre-processed custom data subvolume
    typedef vpl::img::CVolume<vpl::img::tPixel8> tAuxCustomData;

protected:
    //! Is the GLEW library correctly initialized?
    volatile int m_GlewInit;

    //! maximum available size of 3D texture for volume
    long m_maximumVolumeSize;

    //! Used canvas.
    OSGCanvas * m_pCanvas;

    //! Enables/disables volume rendering.
    volatile bool m_Enabled;

    //! True in case of some error.
    volatile int m_Error;

    //! Internal flags.
    volatile int m_Flags;

    //! Init failure counter
    volatile int m_FailureCounter;

    //! Error strings
    std::vector<std::string> m_ErrorStrings;

    //! Mutex for mutual access to internal flags.
    vpl::sys::CMutex m_Mutex;

    //! Condition signalled on any change of internal flags.
    vpl::sys::CCondition m_Condition;

    //! Helper thread.
    vpl::sys::CThread m_Thread;

    //! OpenGL variables used by the renderer.
    vpl::base::CScopedPtr<PSVolumeRenderingData> m_spGLData;

    //! Rendering parameters.
    vpl::base::CScopedPtr<PSVolumeRenderingParams> m_spParams;

    //! Original volume data.
    vpl::img::CDensityVolume::tSmartPtr m_spVolumeData;

    //! Pre-processed volume data
    tVolumeData m_VolumeData;

    //! Pre-processed subvolume data
    tAuxVolumeData m_AuxVolumeData;

    //! Pre-processed custom data
    tCustomData_bool m_customData_bool;
    tCustomData_tPixel8 m_customData_tPixel8;
    tCustomData_tPixel16 m_customData_tPixel16;
    tCustomData_tRGBPixel m_customData_tRGBPixel;
    tAuxCustomData m_auxCustomData;

    ////////////////////////////////////////////////////////////////////////////
    // Look-Up Tables (LUT)
    
    //! Predefined constants...
    enum EConf
    {
        //! Init size of volumes
        INIT_SIZE = 32,

        //! Number of 1D color lookup tables.
        NUM_OF_LUTS_1D = LOOKUPS_COUNT,

        //! Size of the 1D color lookup tables.
        LUT_1D_SIZE = 4096,

        //! Size of the noise table.
        NOISE_SIZE = 1024
    };

    //! Random noise table.
    unsigned char noise[NOISE_SIZE * NOISE_SIZE];

    ////////////////////////////////////////////////////////////////////////////
    // Latest version of the geometry (The Cube)
    // - GL_TRIANGLES, VAO (Vertex Array Object)
    
    //! Single vertex (or texture) coordinates.
    typedef vpl::img::CPoint3<float> tCoords;
    
    //! Vector of vertices (or texture coordinates).
    typedef std::vector<tCoords> tArray;

    //! Auxilliary vector of vertices (i.e. triangles).
    tArray m_Triangles;

protected:
    //! Prepares the box that bounds volume data.
    void prepareBox(int NumOfQuads);

    //! Rendering of the box that bounds the volume data...
    void renderBox(PSVolumeRenderingParams *pParams, const tArray& Triangles);

    // Friend classes
    friend class osgPSVolumeRendering;
    friend class osgPSVolumeRenderingGeode;
};

} // namespace PSVR

#endif // USE_PSVR

#endif // PSVRrenderer_H
