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

#ifndef PositionedWindow_H_included
#define PositionedWindow_H_included

#include <graph/widgets/CSizeableWindowManager.h>

#include <osgWidget/Window>


namespace scene
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Positioned window. 
//
//	This class adds automatic positioning system to the window. This positioning is based on
//	the current window manager size.
//	Positioning rules:
//	- Positioning type can be NONE - do nothing, PROPORTIONAL - stored coordinate is recomputed 
//	  according to the window manager size (position and size) or to the current window size (origin)
//	  Last possibility is FIXED - use absolute positioning (no anchoring, just place coordinate).
//	- The first is computed new window width and height
//	- The second is computed new window origin.
//	- The last is computed new window position based on origin.
//	- Then the updateWindowSizePosition method is called - derived classes must rewrite this method. 

class CPositionedWindow
{
public:
    //! Positioning type
    enum EPositioning
    {
        NONE,
        PROPORTIONAL_WM,
        PROPORTIONAL_WINDOW,
        FIXED
    };

public:
    //! Default constructor
    CPositionedWindow();

    //! Virtual desctructor;
    virtual ~CPositionedWindow() {}

    //! Set window manager - this thing works only with SizeableWindowManager
    void setWindowManager( scene::CSizeableWindowManager * wm );

    //! Set position
    void setWindowPosition( float x, float y ) { m_x = x; m_y = y; }

    //! Set size
    void setWindowSize( float w, float h ) { m_width = w; m_height = h; }

    //! Set window origin 
    void setWindowOrigin( float x, float y ) { m_ox = x; m_oy = y; }

    //! Set position type
    void setPositionType( EPositioning xPosition, EPositioning yPosition ) { m_xType = xPosition; m_yType = yPosition; }

    //! Set size type
    void setSizeType( EPositioning width, EPositioning height ) { m_widthType = width; m_heightType = height; }

    //! Set origin type
    void setOriginType( EPositioning xType, EPositioning yType ) { m_xOriginType = xType; m_yOriginType = yType; }

protected:
    //! On window manager size changed signal answer
    void onWMSizeChanged( int wmw, int wmh );

    //! Recompute position
    virtual void computePosition( int wmw, int wmh, int  ox, int  oy, int & x, int & y );

    //! Recompute size
    virtual void computeSize( int wmw, int wmh, int & w, int & h );

    //! Recompute origin
    virtual void computeOrigin( int wmw, int wmh, int  ww, int wh, int & ox, int & oy );

    //! Update window parameters implementation
    virtual void updateWindowSizePosition( int VPL_UNUSED(width), int VPL_UNUSED(height), int VPL_UNUSED(x), int VPL_UNUSED(y) ) {}

    //! Called before all calculations are made - ancestors can update window sizes, origin position here.
    virtual void recomputeSizes() {}

protected:
    //! Position type - width
    EPositioning m_widthType;

    //! Position type - height
    EPositioning m_heightType;

    //! Position type - x
    EPositioning m_xType;

    //! Position type - y
    EPositioning m_yType;

    //! Position type - origin x
    EPositioning m_xOriginType;

    //! Position type - origin y
    EPositioning m_yOriginType;

    //! Window size 
    float m_width, m_height;

    //! Window position
    float m_x, m_y;

    //! Window origin
    float m_ox, m_oy;

    //! Signal connection
    vpl::mod::tSignalConnection m_conSizeChanged;

}; // class CPositionedWindow


} // namespace scene

#endif // PositionedWindow_H_included
