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

#ifndef CVolumeRenderer_H
#define CVolumeRenderer_H


#include <app/Signals.h>
#include <VPL/Image/DensityVolume.h>
#include <QWidget>
#include <osg/Array>
#include <string>
#include <render/CLookupTable.h>


///////////////////////////////////////////////////////////////////////////////
//! Interface for volume renderer usable by plugins
//! - Plugins may want to use some custom volume and custom shader for
//!   visualization. This interface puts necessary requirements on renderer
//!   so that it can handle plugins' needs (this may not necessary mean, that
//!   renderer is capable doing all these things, but it is possible that it
//!   can - check renderer itself)
//! - Plugin can create custom volume and custom shader. Renderer can
//!   internally handle this in whatever way it needs but it has to return
//!   unique ID of volume/shader
//! - Because of memory limitations of hardware, there is only one volume
//!   stored in GPU at a single point of time. There is mechanism for switching
//!   custom volumes of different plugins:
//!   - plugin creates custom volume and shader
//!   - when some plugin need to store and use custom volume, owner of previous
//!     custom volume is notified (if it is registered to being notified) and
//!     it can grab current data from renderer
//!   - current custom volume is updated with data provided by plugin
//!   - current custom volume is rendered using current custom shader
//!   - plugin deletes custom volume and shader
//!   - plugin can register various callbacks if needed (called when renderer
//!     is destroyed, volume is replaced by data of another plugin, shader's
//!     data need to be updated)
class CVolumeRenderer
{
protected:
    //! possible types of custom volume
    enum EVolumeType
    {
        EVT_NONE = 0,
        EVT_BOOL,
        EVT_PIXEL8,
        EVT_PIXEL16,
        EVT_RGBPIXEL
    };

public:
    //! class for handling calls by renderer that may execute custom code
    class CRendererCallback
    {
    public:
        //! Ctor
        CRendererCallback()
        { }

        //! Dtor
        virtual ~CRendererCallback()
        { }

        //! operator for executing custom code
        virtual void operator()()
        { }
    };

    typedef CRendererCallback tRendererCallback;

protected:
    //! connection of signal for returning pointer to renderer with necessary capabilities
    vpl::mod::tSignalConnection m_conGetRenderer;

    //! flag if renderer is connected to signal above
    bool m_connected;

    //! list of callbacks called when custom volume is changed
    std::map<unsigned int, tRendererCallback *> m_volumeChangeCallbacks;

    //! list of callbacks called when custom shader's data should be updated
    std::map<unsigned int, tRendererCallback *> m_shaderUpdateCallbacks;

    //! list of callbacks called when renderer is destroyed
    std::vector<tRendererCallback *> m_rendererDeleteCallbacks;

protected:
    //! ID of current custom volume
    unsigned int m_currentVolume;

    //! type of current custom volume
    EVolumeType m_currentType;

    //! ID of current custom shader
    unsigned int m_currentShader;

    //! containers for current custom volume
    vpl::img::CVolume<bool> m_volume_bool;
    vpl::img::CVolume<vpl::img::tPixel16> m_volume_tPixel16;
    vpl::img::CVolume<vpl::img::tPixel8> m_volume_tPixel8;
    vpl::img::CVolume<vpl::img::tRGBPixel> m_volume_tRGBPixel;
    vpl::img::CVolume<vpl::img::tPixel8> m_auxVolume;

    //! list of lookup tables
    std::map<std::string, CLookupTable> m_lookupTables;

public:
    //! Ctor
    CVolumeRenderer()
    {
        // connect to signal only if there is not another renderer already connected
        m_connected = false;
        if (VPL_SIGNAL(SigGetRenderer).invoke2() == NULL)
        {
            m_conGetRenderer = VPL_SIGNAL(SigGetRenderer).connect(this, &CVolumeRenderer::getRenderer);
            m_connected = true;
        }

        // initial values
        m_currentVolume = 0;
        m_currentType = EVT_NONE;
    }

    //! Dtor
    virtual ~CVolumeRenderer()
    {
        cleanup();
    }

    //! Cleanup function
    void cleanup()
    {
        // notify owner of current custom volume
        if (m_currentVolume != 0)
        {
            tRendererCallback *callback = m_volumeChangeCallbacks[m_currentVolume];
            if (callback != NULL)
            {
                (*callback)();
            }
            m_currentVolume = 0;
        }

        // notify all registered users of renderer
        while (m_rendererDeleteCallbacks.size() > 0)
        {
            tRendererCallback *callback = m_rendererDeleteCallbacks[m_rendererDeleteCallbacks.size() - 1];
            if (callback != NULL)
            {
                (*callback)();
            }
            m_rendererDeleteCallbacks.resize(m_rendererDeleteCallbacks.size() - 1);
        }

        // disconnect
        if (m_connected)
        {
            VPL_SIGNAL(SigGetRenderer).disconnect(m_conGetRenderer);
            m_connected = false;
        }
    }

    //! returns pointer to itself (called by signal invokation)
    CVolumeRenderer *getRenderer()
    {
        return this;
    }

public:
    //! registers callback executed when renderer is destroyed
    void registerOnDeleteCallback(tRendererCallback *callback)
    {
        for (std::size_t i = 0; i < m_rendererDeleteCallbacks.size(); ++i)
        {
            if (m_rendererDeleteCallbacks[i] == callback)
            {
                return;
            }
        }

        m_rendererDeleteCallbacks.push_back(callback);
    }

    //! deregisters callback executed when renderer is destroyed
    void deregisterOnDeleteCallback(tRendererCallback *callback)
    {
        for (std::vector<tRendererCallback *>::iterator it = m_rendererDeleteCallbacks.begin(); it != m_rendererDeleteCallbacks.end(); ++it)
        {
            if (*it == callback)
            {
                it = m_rendererDeleteCallbacks.erase(it);
                if (it == m_rendererDeleteCallbacks.end())
                {
                    break;
                }
            }
        }
    }

private:
    //! notifies previous owner that it should grab current data from renderer
    void notifyPreviousAndClear(unsigned int newVolumeId, EVolumeType newVolumeType)
    {
        // notify
        if (newVolumeId != m_currentVolume)
        {
            tRendererCallback *callback = m_volumeChangeCallbacks[m_currentVolume];
            if (callback != NULL)
            {
                (*callback)();
            }
        }
    
        // clear current if new type is different
        if (newVolumeType != m_currentType)
        {
            switch (m_currentType)
            {
            case EVT_BOOL:
                m_volume_bool.resize(0, 0, 0);
                break;

            case EVT_PIXEL8:
                m_volume_tPixel8.resize(0, 0, 0);
                break;

            case EVT_PIXEL16:
                m_volume_tPixel16.resize(0, 0, 0);
                break;

            case EVT_RGBPIXEL:
                m_volume_tRGBPixel.resize(0, 0, 0);
                break;

            case EVT_NONE:
            default:
                break;
            }

            m_auxVolume.resize(0, 0, 0);
        }
    }

public:
    //! creates custom volume
    unsigned int createCustomVolume(tRendererCallback *callback = NULL)
    {
        unsigned int id = internalCreateCustomVolume();
        if ((id != 0) && (callback != NULL))
        {
            m_volumeChangeCallbacks[id] = callback;
        }
        return id;
    }
    
    //! sets data to custom volume
    void setDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<bool> &volume, bool updateData = true)
    {
        vpl::img::CVolume<vpl::img::tPixel8> auxVolume;
        setDataToCustomVolume(volumeId, volume, auxVolume, updateData);
    }
    void setDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<bool> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume, bool updateData = true)
    {
        notifyPreviousAndClear(volumeId, EVT_BOOL);
        if ((m_currentVolume == volumeId) && (!updateData))
        {
            return;
        }
        m_currentVolume = volumeId;
        m_currentType = EVT_BOOL;
        m_volume_bool.copy(volume);
        m_auxVolume.copy(auxVolume);
        internalSetDataToCustomVolume(volumeId, volume, auxVolume);
    }

    //! sets data to custom volume
    void setDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel16> &volume, bool updateData = true)
    {
        vpl::img::CVolume<vpl::img::tPixel8> auxVolume;
        setDataToCustomVolume(volumeId, volume, auxVolume, updateData);
    }
    void setDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel16> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume, bool updateData = true)
    {
        notifyPreviousAndClear(volumeId, EVT_PIXEL16);
        if ((m_currentVolume == volumeId) && (!updateData))
        {
            return;
        }
        m_currentVolume = volumeId;
        m_currentType = EVT_PIXEL16;
        m_volume_tPixel16.copy(volume);
        m_auxVolume.copy(auxVolume);
        internalSetDataToCustomVolume(volumeId, volume, auxVolume);
    }

    //! sets data to custom volume
    void setDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel8> &volume, bool updateData = true)
    {
        vpl::img::CVolume<vpl::img::tPixel8> auxVolume;
        setDataToCustomVolume(volumeId, volume, auxVolume, updateData);
    }
    void setDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel8> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume, bool updateData = true)
    {
        notifyPreviousAndClear(volumeId, EVT_PIXEL8);
        if ((m_currentVolume == volumeId) && (!updateData))
        {
            return;
        }
        m_currentVolume = volumeId;
        m_currentType = EVT_PIXEL8;
        m_volume_tPixel8.copy(volume);
        m_auxVolume.copy(auxVolume);
        internalSetDataToCustomVolume(volumeId, volume, auxVolume);
    }

    //! sets data to custom volume
    void setDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tRGBPixel> &volume, bool updateData = true)
    {
        vpl::img::CVolume<vpl::img::tPixel8> auxVolume;
        setDataToCustomVolume(volumeId, volume, auxVolume, updateData);
    }
    void setDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tRGBPixel> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume, bool updateData = true)
    {
        notifyPreviousAndClear(volumeId, EVT_RGBPIXEL);
        if ((m_currentVolume == volumeId) && (!updateData))
        {
            return;
        }
        m_currentVolume = volumeId;
        m_currentType = EVT_RGBPIXEL;
        m_volume_tRGBPixel.copy(volume);
        m_auxVolume.copy(auxVolume);
        internalSetDataToCustomVolume(volumeId, volume, auxVolume);
    }

    //! deletes custom volume
    virtual void deleteCustomVolume(unsigned int volumeId)
    {
        if (volumeId == 0)
        {
            return;
        }
        internalDeleteCustomVolume(volumeId);
        m_volumeChangeCallbacks.erase(volumeId);
    }

    //! creates custom shader
    virtual unsigned int createCustomShader(std::string vertexShaderSource, std::string fragmentShaderSource, tRendererCallback *callback = NULL)
    {
        unsigned int id = internalCreateCustomShader(vertexShaderSource, fragmentShaderSource);
        if ((id != 0) && (callback != NULL))
        {
            m_shaderUpdateCallbacks[id] = callback;
        }
        return id;
    }

    //! activates specified custom shader
    virtual void useCustomShader(unsigned int shaderId)
    {
        m_currentShader = shaderId;
        internalUseCustomShader(shaderId);
    }

    //! deletes specified custom shader
    virtual void deleteCustomShader(unsigned int shaderId)
    {
        if (shaderId == 0)
        {
            return;
        }
        internalDeleteCustomShader(shaderId);
        m_shaderUpdateCallbacks.erase(shaderId);
    }

    //! returns active custom volume and custom shadr IDs
    void getActiveCustomIds(int &volumeId, int &shaderId)
    {
        volumeId = m_currentVolume;
        shaderId = m_currentShader;
    }

    //! returns list of lookup tables
    std::map<std::string, CLookupTable> &getLookupTables()
    {
        return m_lookupTables;
    }

protected:
    //! executes appropriate callback when data of active custom shader should be updated
    void shaderUpdateCallback()
    {
        tRendererCallback *callback = m_shaderUpdateCallbacks[m_currentShader];
        if (callback != NULL)
        {
            (*callback)();
        }
    }

public:
    //! initializes VR if it is possible
    virtual bool init() = 0;
    //! returns flag if VR is enabled
    virtual bool isEnabled() const = 0;
    //! enables/disables VR
    virtual void enable(bool bEnable = true) = 0;
    //! disables VR
    void disable() { enable(false); }

    //! returns flag if custom shader is really used a displayed
    virtual bool isCustomShaderActive() = 0;
    //! forces renderer to redraw itself
    virtual void redraw(bool bEraseBackground = false) = 0;
    //! returns current size of renderer's window
    virtual QSize getWindowSize() = 0;

    //! returns world matrix of volume
    virtual osg::Matrix getWorldMatrix() = 0;
    //! returns view matrix used for rendering volume
    virtual osg::Matrix getViewMatrix() = 0;
    //! returns projection matrix used for rendering volume
    virtual osg::Matrix getProjectionMatrix() = 0;
    //! returns world-to-screen matrix (world * view * projection)
    virtual osg::Matrix getTransformMatrix() = 0;

    //! sets int parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, int value) = 0;
    //! sets float parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, float value) = 0;
    //! sets osg::Vec2 parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec2 value) = 0;
    //! sets osg::Vec3 parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec3 value) = 0;
    //! sets osg::Vec4 parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec4 value) = 0;
    //! sets osg::Matrix parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Matrix value) = 0;
    //! sets array-of-ints parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, int *value, int count) = 0;
    //! sets array-of-floats parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, float *value, int count) = 0;
    //! sets array-of-osg::Vec2 parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec2 *value, int count) = 0;
    //! sets array-of-osg::Vec3 parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec3 *value, int count) = 0;
    //! sets array-of-osg::Vec4 parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Vec4 *value, int count) = 0;
    //! sets array-of-osg::Matrix parameter of custom shader
    virtual void setParameter(unsigned int shaderId, std::string name, osg::Matrix *value, int count) = 0;

    //! returns data of custom volume
    virtual void getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<bool> &volume) = 0;
    //! returns data of custom volume
    virtual void getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel16> &volume) = 0;
    //! returns data of custom volume
    virtual void getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel8> &volume) = 0;
    //! returns data of custom volume
    virtual void getDataFromCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tRGBPixel> &volume) = 0;

    //! resets lookup tables
    virtual void resetLookupTables() = 0;
    //! updates internal representation of LUTs
    virtual void updateLookupTables() = 0;

protected:
    //! internal method for creating custom volume, should return unique ID as it is used later
    virtual unsigned int internalCreateCustomVolume() = 0;
    //! internal update of data of custom volume
    virtual void internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<bool> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume) = 0;
    //! internal update of data of custom volume
    virtual void internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel16> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume) = 0;
    //! internal update of data of custom volume
    virtual void internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tPixel8> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume) = 0;
    //! internal update of data of custom volume
    virtual void internalSetDataToCustomVolume(unsigned int volumeId, vpl::img::CVolume<vpl::img::tRGBPixel> &volume, vpl::img::CVolume<vpl::img::tPixel8> &auxVolume) = 0;
    //! internal delete of custom volume
    virtual void internalDeleteCustomVolume(unsigned int volumeId) = 0;

    //! internal method for creating custom shader, should return unique ID as it is used later
    virtual unsigned int internalCreateCustomShader(std::string vertexShaderSource, std::string fragmentShaderSource) = 0;
    //! internal method for activating custom shader
    virtual void internalUseCustomShader(unsigned int shaderId) = 0;
    //! internal delete of custom shader
    virtual void internalDeleteCustomShader(unsigned int shaderId) = 0;
};


#endif // CVolumeRenderer_H
