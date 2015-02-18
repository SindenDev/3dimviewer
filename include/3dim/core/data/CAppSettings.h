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

#ifndef CAPPSETTINGS_H_INCLUDED
#define CAPPSETTINGS_H_INCLUDED

#include "CObjectHolder.h"
#include <osg/Vec4f>

#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{
class CAppSettings: public vpl::base::CObject
{
public:
    //! Smart pointer type.
    VPL_SHAREDPTR(CAppSettings);

public:
    //! Default constructor.
    CAppSettings();

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& Changes);

    //! Initializes the object to its default state.
    void init();

    //! We have no serializable data...
    bool hasData() { return false; }

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    //! Get default clear color
    const osg::Vec4f & getClearColor() { return m_clearColor; }

    //! Set custom clear color
    void setClearColor(const osg::Vec4f color) { m_clearColor = color; }

    enum ETextureFilter
    {
        NoFilter = 0,
        Sharpen,
        Blur,
        SmoothSharpen,
		Equalize,
		DetectEdges
    };
    //! Get default clear color
    const ETextureFilter & getFilter() { return m_filter; }

    //! Set custom clear color
    void setFilter(const ETextureFilter filter) { m_filter = filter; }

    //! Return path to arial font for osg
    const std::string& getArialFontPath() const { return m_arialPath; }

    //! Set whether Non power of two textures can be used
    void setNPOTTextures(bool bNPOT) { m_bNPOTTextures = bNPOT; }

    //! Get whether NPOT textures are enabled
    bool getNPOTTextures() const { return m_bNPOTTextures; } 

protected:
    //! Window clear color
    osg::Vec4f m_clearColor;

    //! Active Texture Filter
    ETextureFilter m_filter;

    //! Arial font path
    std::string m_arialPath;

    //! Non-Power-Of-Two textures
    bool    m_bNPOTTextures;

}; // class CAppSettings

namespace Storage
{
	//! Application settings 
	DECLARE_OBJECT(AppSettings, CAppSettings, CORE_STORAGE_APP_SETTINGS_ID );
}

} // namespace data

// CAPPSETTINGS_H_INCLUDED
#endif

