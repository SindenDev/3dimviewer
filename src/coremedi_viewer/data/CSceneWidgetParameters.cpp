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

#include <data/CSceneWidgetParameters.h>

// Default widgets visibility
#define DEF_WIDGETS_VISIBILITY true
#define MAX2( x, y )  ( ((x) > (y))? (x) : (y) )
#define MAX3( x, y, z ) MAX2( MAX2( (x), (y) ), (z) )
#define RGB256( r, g, b, a ) osg::Vec4( float(r)/255.0f, float(g)/255.0f, float(b)/255.0f, float(a)/255.0f )

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Default constructor. 
////////////////////////////////////////////////////////////////////////////////////////////////////
data::CSceneWidgetParameters::CSceneWidgetParameters(void)
   : m_bWidgetsVisible( DEF_WIDGETS_VISIBILITY )
   , m_widgetsScale(1)
   , m_defaultWidgetsScale(1)
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Regenerates the object state according to any changes in the data storage. 
//!
//!\param   Changes  The changes. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void data::CSceneWidgetParameters::update(const data::CChangedEntries&Changes)
{
	// Do nothing
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Initializes the object to its default state. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void data::CSceneWidgetParameters::init(void)
{
   m_bWidgetsVisible= DEF_WIDGETS_VISIBILITY;
//   m_backgroundColor = osg::Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
   m_backgroundColor = osg::Vec4( 0.2f, 0.2f, 0.6f, 0.0f );
   m_markersColor = osg::Vec4( 0.95f, 0.95f, 0.0f, 1.0f );
   m_textColor = osg::Vec4( 0.95f, 0.95f, 0.0f, 1.0f );

   m_normalSlicesColors[0] = RGB256( 238, 241, 124, 255 );
   m_normalSlicesColors[1] = RGB256( 150, 150, 216, 255 ); //-15%
   m_normalSlicesColors[2] = RGB256( 203, 203, 255, 255 ); //+15%
   m_normalSlicesColors[3] = RGB256( 216, 108, 108, 255 );
   m_normalSlicesColors[4] = RGB256( 255, 143, 143, 255 );

   // Ortho slices color
   m_orthoSlicesColors[ 0 ] = RGB256(0, 0, 255, 255 ); // Axial slice
   m_orthoSlicesColors[ 1 ] = RGB256(0, 255, 0, 255 ); // Coronal slice
   m_orthoSlicesColors[ 2 ] = RGB256(255, 0, 0, 255 ); // Sagittal slice

   // Widgets scale
   m_widgetsScale = m_defaultWidgetsScale;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief   ! Show/hide all. 
//!
//!\param   bVisible true to show, false to hide. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void data::CSceneWidgetParameters::show(bool bVisible)
{
	m_bWidgetsVisible = bVisible;
}
