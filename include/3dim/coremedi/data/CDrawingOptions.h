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

#ifndef CDrawingOptions_H
#define CDrawingOptions_H

#include <VPL/Base/Object.h>
#include <VPL/Base/SharedPtr.h>

#include <data/CObjectHolder.h>

#include <osg/Vec4>

#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Manages mouse drawing modes.

class CDrawingOptions : public vpl::base::CObject
{
public:
    //! Input handling modes
    enum EDrawingMode
    {
        DRAW_NOTHING = 0,
        DRAW_POINT   = 1,   // Return one point on mouse down, draw marker
        DRAW_LINE    = 2,   // Get two points ( mouse down -> mouse up, draw interactive gizmo )
        DRAW_STROKE  = 3,   // Draw stroke ( mouse down -> mouse up ), return as a points array
        DRAW_LASO    = 4    // Draw stroke and one line is moving to the end position
    };

    //! Handler type - describes scene and handling type 
    enum EHandlerType
    {
        FOCUS_LOST = 0,     // Handler has lost the focus
        FOCUS_ON = 1,       // Handler has focus now
	    HANDLER_3D = 2,
	    HANDLER_XY = 3,
	    HANDLER_XZ = 4,
	    HANDLER_YZ = 5,
	    HANDLER_NORMAL = 6,
        HANDLER_WINDOW = 7,
        HANDLER_IMPLANT = 8,
	    HANDLER_UNKNOWN = 9
    };

public:
    //! Smart pointer type.
    VPL_SHAREDPTR( CDrawingOptions );
   
public:
    //! Default constructor.
    CDrawingOptions() {  }

    //! Regenerates the object state according to any changes in the data storage.
    void update(const data::CChangedEntries& VPL_UNUSED(Changes))
    {
       // viz. existujici kod ve tridach COrthoSliceXY, CDensityWindow, apod. 
    }

    //! Initializes the object to its default state.
    void init() 
    { 
       m_drawingMode = DRAW_POINT;
       m_lineColor = osg::Vec4( 1.0, 0.0, 0.0, 1.0 );
       m_lineWidth = 1.0;
    }

    //! Set drawing mode
    void setDrawingMode( EDrawingMode mode ){ m_drawingMode = mode; }

    //! Get drawing mode
    EDrawingMode getDrawingMode(){ return m_drawingMode; }

    //! Set color of marks
    void setColor( const osg::Vec4 & color ){ m_lineColor = color; }

    //! Get color
    osg::Vec4 getColor(){ return m_lineColor; }

    //! Set line width
    void setWidth( float width ){ m_lineWidth = width; }

    //! Get line width
    float getWidth() { return m_lineWidth; }

    //! Does object contain relevant data?
    virtual bool hasData(){ return false; }

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency( data::CStorageEntry * VPL_UNUSED(pParent) ) { return true; }

protected:
    //! Type of the handler - what type of the scene is it...
    EDrawingMode m_drawingMode;

    //! Line color
    osg::Vec4 m_lineColor;

    //! ine width
    float m_lineWidth;

}; // class CDrawingOptions

namespace Storage
{
	//! Drawing options
	DECLARE_OBJECT(DrawingOptions, CDrawingOptions, CORE_STORAGE_DRAWING_OPTIONS_ID);
}

} // namespace data

#endif // CDrawingOptions_H
