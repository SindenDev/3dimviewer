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

#ifndef CSceneWidgetParameters_H_included
#define CSceneWidgetParameters_H_included

#include <VPL/Base/BasePtr.h>
#include <VPL/Base/SharedPtr.h>
#include <data/CSerializableData.h>
#include <data/CChangedEntries.h>
#include <data/CStorageEntry.h>
#include <osg/Vec4>
#include <data/storage_ids_core.h>
#include <data/CStorageInterface.h>

namespace data
{

class CDentalExamination;

///////////////////////////////////////////////////////////////////////////////
//! Parameters of special widgets shown in corners of 3D and ortho scenes.

class CSceneWidgetParameters: public vpl::base::CObject //, public vpl::mod::CSerializable
{
public:
    //! Smart pointer type.
    VPL_SHAREDPTR(CSceneWidgetParameters);

    //! Default class name.
    VPL_ENTITY_NAME("SceneWidgetParameters");

    //! Default compression method.
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

public:
    //! Default constructor.
    CSceneWidgetParameters();

    //! Regenerates the object state according to any changes in the data storage.
    void update(const data::CChangedEntries& Changes);

    //! Initializes the object to its default state.
    void init();

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    //! Show/hide all
    void show( bool bVisible );

    //! Has data
    virtual bool hasData() { return true; }

    //! Are widgets visible
    bool widgetsVisible( ){ return m_bWidgetsVisible; }

    //! Get widget background color
    osg::Vec4 getBackgroundColor( ) { return m_backgroundColor; }

    //! Get text color
    osg::Vec4 getTextColor( ) { return m_textColor; }

    //! Get text color
    osg::Vec4 getMarkersColor( ) { return m_markersColor; }

    //! Get normal plane color
    const osg::Vec4 & getNormalPlaneColor( int id ) const { return m_normalSlicesColors[id]; }

    //! Get ortho slice color
    const osg::Vec4 & getOrthoSliceColor( int id ) const { return m_orthoSlicesColors[id]; }

	//! Set widgets scale
	void setWidgetsScale(double scale) { m_widgetsScale = scale; }

	//! Set default widgets scale
	void setDefaultWidgetsScale(double scale) { m_defaultWidgetsScale = scale; }	

	//! Get widgets scale
	double getWidgetsScale() const { return m_widgetsScale; }

	//! Get default widgets scale
	double getDefaultWidgetsScale() const { return m_defaultWidgetsScale; }

protected:
    //! Are widgets visible?
    bool m_bWidgetsVisible;

    //! Widget background color
    osg::Vec4 m_backgroundColor;

    //! Text color
    osg::Vec4 m_textColor;

    //! Ruler markers color
    osg::Vec4 m_markersColor;

    //! Normal slices decoration color
    osg::Vec4 m_normalSlicesColors[5];

    //! Ortho slices color
    osg::Vec4 m_orthoSlicesColors[3];

	//! Scaling factor for widgets
	double m_widgetsScale;

	//! Default widgets scale set on storage reset
	double m_defaultWidgetsScale;

    friend class CDentalExamination;
};

namespace Storage
{
	//! Scene widgets
	DECLARE_OBJECT(SceneWidgetsParameters, CSceneWidgetParameters, CORE_STORAGE_SCENE_WIDGETS_PARAMETERS_ID);
}

} // namespace data

#endif // CSceneWidgetParameters_H_included
