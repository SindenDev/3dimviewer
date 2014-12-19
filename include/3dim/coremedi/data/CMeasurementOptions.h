///////////////////////////////////////////////////////////////////////////////
// $Id: CMeasurementsOptions.h 
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

#ifndef CMeasurementOptions_H
#define CMeasurementOptions_H

#include <osg/Vec4>
#include <VPL/Base/Object.h>
#include <data/CObjectHolder.h>
#include <VPL/Base/SharedPtr.h>
#include <string>

#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Measurement markers settings in the storage.

class CMeasurementOptions : public vpl::base::CObject
{
public:
    //! Smart pointer type.
    VPL_SHAREDPTR( CMeasurementOptions );

    //! Regenerates the object state according to any changes in the data storage.
    void update(const data::CChangedEntries& VPL_UNUSED(Changes))
    {
       // viz. existujici kod ve tridach COrthoSliceXY, CDensityWindow, apod. 
    }

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency( data::CStorageEntry * VPL_UNUSED(pParent) ) { return true; }

    //! Does object contain relevant data?
    virtual bool hasData(){ return false; }

    //! Initializes the object to its default state.
    void init();

    //! Set ruler line color
    void SetRulerLineColor( const osg::Vec4 & color ){ m_rulerLineColor = color; }

    //! Get ruler line color
    osg::Vec4 GetRulerLineColor(){ return m_rulerLineColor; }

    //! Set ruler font color
    void SetRulerFontColor( const osg::Vec4 & color ){ m_rulerFontColor = color; }

    //! Get ruler font color
    osg::Vec4 GetRulerFontColor(){ return m_rulerFontColor; }

    //! Set ruler font shadow color
    void SetRulerFontShadowColor( const osg::Vec4 & color ){ m_rulerFontShadowColor = color; }

    //! Get ruler font shadow color
    osg::Vec4 GetRulerFontShadowColor(){ return m_rulerFontShadowColor; }

    //! Set ruler font size
    void SetRulerFontSize( unsigned int size ){ m_rulerFontSize = size; }

    //! Ger ruler font size
    unsigned int GetRulerFontSize(){ return m_rulerFontSize; }

    //! Set ruler font name
    void SetRulerFontName( const std::string & name ) { m_rulerFontName = name; }

    //! GetRulerFontName
    std::string GetRulerFontName() { return m_rulerFontName; }

    //! Set dropper line color
    void SetDropperLineColor( const osg::Vec4 & color ){ m_dropperLineColor = color; }

    //! Get dropper line color
    osg::Vec4 GetDropperLineColor(){ return m_dropperLineColor; }

    //! Set dropper font color
    void SetDropperFontColor( const osg::Vec4 & color ){ m_dropperFontColor = color; }

    //! Get dropper font color
    osg::Vec4 GetDropperFontColor(){ return m_dropperFontColor; }

    //! Set dropper font shadow color
    void SetDropperFontShadowColor( const osg::Vec4 & color ){ m_dropperFontShadowColor = color; }

    //! Get dropper font shadow color
    osg::Vec4 GetDropperFontShadowColor(){ return m_dropperFontShadowColor; }

    //! Set dropper font size
    void SetDropperFontSize( unsigned int size ){ m_dropperFontSize = size; }

    //! Ger dropper font size
    unsigned int GetDropperFontSize(){ return m_dropperFontSize; }

    //! Set dropper font name
    void SetDropperFontName( const std::string & name ) { m_dropperFontName = name; }

    //! Get dropper font name
    std::string GetDropperFontName() { return m_dropperFontName; }

protected:
    //! Ruler line color
    osg::Vec4 m_rulerLineColor;

    //! Ruler font color
    osg::Vec4 m_rulerFontColor;

    //! Ruler font shadow color
    osg::Vec4 m_rulerFontShadowColor;

    //! Ruler font size
    unsigned int m_rulerFontSize;

    //! Ruler font name
    std::string m_rulerFontName;

    //! Density dropper circle color
    osg::Vec4 m_dropperLineColor;

    //! Density dropper font color
    osg::Vec4 m_dropperFontColor;

    //! Density dropper font shadow color
    osg::Vec4 m_dropperFontShadowColor;

    //! Density dropper font size
    unsigned int m_dropperFontSize;

    //! Density dropper font name
    std::string m_dropperFontName;

}; // class CMeasurementOptions

namespace Storage
{
	//! Measurement markers options
	DECLARE_OBJECT(MeasurementOptions, CMeasurementOptions, CORE_STORAGE_MEASUREMENT_OPTIONS_ID);
}

} // namespace data

#endif // CMeasurementOptions_H
