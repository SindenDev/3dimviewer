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

#ifndef CSlice_H
#define CSlice_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/Lock.h>
#include <VPL/Image/Image.h>
#include <VPL/Base/SharedPtr.h>
#include <VPL/Module/Signal.h>

#include <osg/Texture2D>
#include <vector>
#include <map>

#include "data/CObjectHolder.h"
#include "data/CObjectPtr.h"


namespace data
{

class CCSlicePropertyUpdateProvider
{
public:
    CCSlicePropertyUpdateProvider()
    { }

    ~CCSlicePropertyUpdateProvider()
    { }
};
    
class CSliceBaseProperty
{
protected:
    std::string m_name;
    int m_propertySourceStorageId;
    int m_sliceStorageId;
    bool m_validData;

public:
    CSliceBaseProperty(std::string name, int propertySourceStorageId, int sliceStorageId)
        : m_name(name)
        , m_propertySourceStorageId(propertySourceStorageId)
        , m_sliceStorageId(sliceStorageId)
        , m_validData(false)
    { }

    ~CSliceBaseProperty()
    { }

    std::string name() const
    {
        return m_name;
    }

    int propertySourceStorageId() const
    {
        return m_propertySourceStorageId;
    }

    int sliceStorageId() const
    {
        return m_sliceStorageId;
    }

    bool hasValidData() const
    {
        return m_validData;
    };

    virtual bool update(CCSlicePropertyUpdateProvider *updater) = 0;
};
    
template <class StorageType, class ValueType>
class CSlicePropertyLeanHandle
{
public:
    typedef StorageType tStorageType;
    typedef ValueType tValueType;

protected:
    int m_id;
    std::string m_name;

public:
    CSlicePropertyLeanHandle()
        : m_id(-1)
        , m_name("")
    { }

    ~CSlicePropertyLeanHandle()
    { }

    bool isValid() const
    {
        return (m_id >= 0);
    }

    int id() const
    {
        return m_id;
    }
    
    std::string name() const
    {
        return m_name;
    }

protected:
    void setId(int id)
    {
        m_id = id;
    }
    
    void setName(std::string name)
    {
        m_name = name;
    }

    friend class CSlicePropertyContainer;
};

template <class UpdateProvider, class StorageType, class ValueType>
class CSlicePropertyHandle : public CSlicePropertyLeanHandle<StorageType, ValueType>
{
public:
    typedef UpdateProvider tUpdateProvider;

public:
    CSlicePropertyHandle()
        : CSlicePropertyLeanHandle<StorageType, ValueType>()
    { }

    ~CSlicePropertyHandle()
    { }
};

template <class StorageType, class ValueType>
class CSliceLeanProperty : public vpl::img::CImage<ValueType>, public CSliceBaseProperty
{
private:
    typedef ValueType tValueType;

public:
    CSliceLeanProperty(std::string name, int propertySourceStorageId, int sliceStorageId)
        : CSliceBaseProperty(name, propertySourceStorageId, sliceStorageId)
    { }

    ~CSliceLeanProperty()
    { }
};

template <class UpdateProvider, class StorageType, class ValueType>
class CSliceProperty : public CSliceLeanProperty<StorageType, ValueType>
{
public:
    CSliceProperty(std::string name, int propertySourceStorageId, int sliceStorageId)
        : CSliceLeanProperty<StorageType, ValueType>(name, propertySourceStorageId, sliceStorageId)
    { }

    ~CSliceProperty()
    { }

    virtual bool update(CCSlicePropertyUpdateProvider *updater)
    {
        CObjectPtr<StorageType> spStorageObject(APP_STORAGE.getEntry(this->m_propertySourceStorageId));
        this->m_validData = static_cast<UpdateProvider *>(updater)->updateProperty(spStorageObject.get(), this);
        return this->m_validData;
    }
};

class CSlicePropertyContainer
{
public:
    typedef std::vector<CSliceBaseProperty *> tPropertyList;

protected:
    int m_newId;
    std::map<int, CSliceBaseProperty *> m_properties;

public:
    CSlicePropertyContainer()
        : m_newId(0)
    { }

    ~CSlicePropertyContainer()
    { }

    template <class UpdateProvider, class StorageType, class ValueType>
    bool add(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle, std::string name, int propertySourceStorageId, int sliceStorageId)
    {
        for (std::map<int, CSliceBaseProperty *>::iterator it = m_properties.begin(); it != m_properties.end(); ++it)
        {
            if (it->second->name() == name)
            {
                return false;
            }
        }

        CSliceBaseProperty *property = new CSliceProperty<UpdateProvider, StorageType, ValueType>(name, propertySourceStorageId, sliceStorageId);
        m_properties[m_newId] = property;
        handle.setId(m_newId);
        handle.setName(name);
        m_newId++;

        return true;
    }

    template <class UpdateProvider, class StorageType, class ValueType>
    void remove(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle)
    {
        if (handle.isValid())
        {
            delete m_properties[handle.id()];
            m_properties.erase(handle.id());
        }
    }

    template <class UpdateProvider, class StorageType, class ValueType>
    bool get(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle, std::string name) const
    {
        for (std::map<int, CSliceBaseProperty *>::const_iterator it = m_properties.begin(); it != m_properties.end(); ++it)
        {
            if (it->second->name() == name)
            {
                handle.setId(it->first);
                handle.setName(name);
                return true;
            }
        }
        return false;
    }

    template <class StorageType, class ValueType>
    bool get(CSlicePropertyLeanHandle<StorageType, ValueType> &handle, std::string name) const
    {
        for (std::map<int, CSliceBaseProperty *>::const_iterator it = m_properties.begin(); it != m_properties.end(); ++it)
        {
            if (it->second->name() == name)
            {
                handle.setId(it->first);
                handle.setName(name);
                return true;
            }
        }
        return false;
    }

    template <class UpdateProvider, class StorageType, class ValueType>
    CSliceProperty<UpdateProvider, StorageType, ValueType> &property(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle) const
    {
        assert(handle.isValid());
        CSliceProperty<UpdateProvider, StorageType, ValueType> *targetProperty = dynamic_cast<CSliceProperty<UpdateProvider, StorageType, ValueType> *>(m_properties.find(handle.id())->second);
        return (*targetProperty);
    }

    template <class StorageType, class ValueType>
    CSliceLeanProperty<StorageType, ValueType> &property(CSlicePropertyLeanHandle<StorageType, ValueType> &handle) const
    {
        assert(handle.isValid());
        CSliceLeanProperty<StorageType, ValueType> *targetProperty = dynamic_cast<CSliceLeanProperty<StorageType, ValueType> *>(m_properties.find(handle.id())->second);
        return (*targetProperty);
    }

    template <class UpdateProvider, class StorageType, class ValueType>
    ValueType &value(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle, int x, int y) const
    {
        return property(handle)(x, y);
    }

    template <class StorageType, class ValueType>
    ValueType &value(CSlicePropertyLeanHandle<StorageType, ValueType> &handle, int x, int y) const
    {
        return property(handle)(x, y);
    }

    template <class UpdateProvider, class StorageType, class ValueType>
    bool hasValidData(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle) const
    {
        return property(handle).hasValidData();
    }

    template <class StorageType, class ValueType>
    bool hasValidData(CSlicePropertyLeanHandle<StorageType, ValueType> &handle) const
    {
        return property(handle).hasValidData();
    }

    tPropertyList propertyList() const
    {
        tPropertyList properties;
        for (std::map<int, CSliceBaseProperty *>::const_iterator it = m_properties.begin(); it != m_properties.end(); ++it)
        {
            properties.push_back(it->second);
        }
        return properties;
    }
};

///////////////////////////////////////////////////////////////////////////////
//! Base class for all slices (orthogonal, planar, curved) through density
//! volume data.
class CSlice : public vpl::base::CObject, public CCSlicePropertyUpdateProvider//, public vpl::base::CLockableObject<CSlice>
{
public:
    //! Lock type.
//    typedef CLockableObject<CSlice>::CLock tLock;

    //! Used OSG texture.
    typedef osg::Texture2D tTexture;

    //! Initial size.
    enum { INIT_SIZE = 64 };

    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CSlice);

public:
    //! Default constructor.
    CSlice();

    //! Destructor.
    virtual ~CSlice();

    template <class UpdateProvider, class StorageType, class ValueType>
    bool addProperty(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle, std::string name, int propertySourceStorageId, int sliceStorageId)
    {
        bool retVal = m_properties.add(handle, name, propertySourceStorageId, sliceStorageId);
        if (retVal)
        {
            m_signalConnections[name] = APP_STORAGE.getEntrySignal(propertySourceStorageId).connect(this, &CSlice::onPropertySourceChanged);
        }
        return retVal;
    }

    template <class UpdateProvider, class StorageType, class ValueType>
    void removeProperty(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle)
    {
        if (handle.isValid() && m_signalConnections.find(handle.name()) != m_signalConnections.end())
        {
            APP_STORAGE.getEntrySignal(m_properties.property(handle).propertySourceStorageId()).disconnect(m_signalConnections[handle.name()]);
            m_properties.remove(handle);
            m_signalConnections.erase(handle.name());
        }
    }

    template <class StorageType, class ValueType>
    bool getProperty(CSlicePropertyLeanHandle<StorageType, ValueType> &handle, std::string name) const
    {
        return m_properties.get(handle, name);
    }

    template <class UpdateProvider, class StorageType, class ValueType>
    bool getProperty(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle, std::string name) const
    {
        return m_properties.get(handle, name);
    }

    template <class StorageType, class ValueType>
    ValueType &propertyValue(CSlicePropertyLeanHandle<StorageType, ValueType> &handle, int x, int y) const
    {
        return m_properties.value(handle, x, y);
    }

    template <class UpdateProvider, class StorageType, class ValueType>
    ValueType &propertyValue(CSlicePropertyHandle<UpdateProvider, StorageType, ValueType> &handle, int x, int y) const
    {
        return m_properties.value(handle, x, y);
    }

    //! Returns width (x-size) of the original image.
    virtual vpl::tSize getWidth() const { return m_DensityData.getXSize(); }

    //! Returns height (y-size) of the original image.
    virtual vpl::tSize getHeight() const { return m_DensityData.getYSize(); }

    //! Returns pointer to the OSG texture data.
    tTexture *getTexturePtr() { return m_spTexture.get(); }
    const tTexture *getTexturePtr() const { return m_spTexture.get(); }

    //! Returns maximal allowed texture coordinates.
    float getTextureWidth() const { return m_fTextureWidth; }
    float getTextureHeight() const { return m_fTextureHeight; }

    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries& Changes) = 0;

    //! Re-initializes the slice.
    virtual void init();

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    //! Disconnect properties
    void disconnectProperties();

protected:
    //! Density data.
    vpl::img::CDImage m_DensityData;
    
    //! Segmented/labelled density data.
    vpl::img::CImage16 m_RegionData;

    //! Custom properties
    CSlicePropertyContainer m_properties;
    std::map<std::string, vpl::mod::tSignalConnection> m_signalConnections;

    //! RGB data.
    vpl::img::CRGBImage m_RGBData;
    
    //! OSG texture.
    osg::ref_ptr<osg::Image> m_spImage;
    osg::ref_ptr<tTexture> m_spTexture;

    //! Texture coordinates.
    float m_fTextureWidth, m_fTextureHeight;

protected:
    //! Initializes all texture properties.
    void setupTexture();

    //! Calculates optimal texture size - power of two.
    //! - Modifies given parameters.
    void estimateTextureSize(vpl::tSize& Width, vpl::tSize& Height);

    //! Regenerates OSG texture data from internal RGB data.
    void updateTexture(bool bRecreateImage);

    //! Regenerates the internal RGB image from density and region data.
    bool updateRGBData(bool bSizeChanged);

    //! Regenerates the internal RGB image from density data only.
    //! - Linear contrast enhancement algorithm.
    bool updateRGBData2(bool bSizeChanged);

private:
    //! Applies current selected filter on m_DensityData and returns backup of original data
    vpl::img::CDImage* applyFilterAndGetBackup();

    //! Restores m_DensityData and deletes backup
    void restoreFromBackupAndFree(vpl::img::CDImage* pBackup);

    // Handles update when property's source has changed
    void onPropertySourceChanged(data::CStorageEntry *entry);
};


///////////////////////////////////////////////////////////////////////////////
// 

typedef enum 
{
    INTERPOLATION_NEAREST	=	0,
    INTERPOLATION_BILINEAR	=	4,
    OPG_XRAY                =   8,
    OPG_MIP                 =   16
} TInterpolationType;


} // namespace data

#endif // CSlice_H

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
