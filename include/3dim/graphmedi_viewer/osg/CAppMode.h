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

#ifndef CAppMode_H
#define CAppMode_H

#include <VPL/Base/Singleton.h>
#include <VPL/Module/Signal.h>

#include <data/CDrawingOptions.h>

#include <osg/Array>

namespace osgManipulator
{
    class Dragger;
}

namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! Singleton switching between different mouse modes (trackball,
//! geometry manipulation, etc.).

class CAppMode : vpl::base::CSingleton<vpl::base::SL_LONG>
{
public:
    //! Helper flags.
    enum EFlags
    {
        //! Modes marked with this flag cannot be stored and restored automatically.
        COMMAND_MODE            = 0x8000
    };

    //! Main restorable mouse modes.
    enum EMode
    {
        //! Scene manipulation (trackball) mode.
	    MODE_TRACKBALL          = 0,

        //! Geometry manipulation mode.
	    MODE_SLICE_MOVE		    = 1,

        //! Density window adjusting mode.
	    MODE_DENSITY_WINDOW     = 2,

        //! Default mouse mode.
        DEFAULT_MODE            = MODE_TRACKBALL
    };

    //! Mouse commands.
    //! - Temporary mouse modes which cannot be stored and restored automatically.
    enum ECommands
    {
        //! Automatically scales the scene in a clicked window.
        COMMAND_SCENE_ZOOM          = 100 | COMMAND_MODE,

        //! Allows you to register a callback function called when mouse button is clicked.
        COMMAND_SCENE_HIT           = 101 | COMMAND_MODE,

		//! Measurement commands.
		COMMAND_DENSITY_MEASURE     = 200 | COMMAND_MODE,
		COMMAND_DISTANCE_MEASURE    = 201 | COMMAND_MODE,

		//! Drawing
		COMMAND_DRAW_WINDOW         = 300 | COMMAND_MODE,
		COMMAND_DRAW_GEOMETRY       = 301 | COMMAND_MODE,

        //! Implant commands (used in DentalViewer).
        COMMAND_ADD_IMPLANT         = 400 | COMMAND_MODE,
        COMMAND_REMOVE_IMPLANT	    = 401 | COMMAND_MODE,
        COMMAND_REPLACE_IMPLANT	    = 402 | COMMAND_MODE,
        COMMAND_IMPLANT_INFO	    = 403 | COMMAND_MODE,

        //! Protosegmentation (used in DentalViewer).
		COMMAND_AUTO_SEGMENTATION   = 500 | COMMAND_MODE
    };

    //! Type representing the current application mode.
    typedef int tMode;

	//! Type representing the "mode changed" signal.
	typedef vpl::mod::CSignal<void, tMode> tSigModeChanged;

    ///////////////////////////////////////////////////////////////////////////

    //! Special flags passed to command handlers.
    enum EMouseFlags
    {
        //! Left mouse button pressed.
        PUSH        = 1,

        //! Mouse move.
        DRAG        = 2,

        //! Left mouse button released.
        RELEASE     = 4,

        //! Left mouse button doubleclicked.
        DOUBLECLICK = 8
    };

    //! Signal invoked when the COMMAND_SCENE_HIT mode is active and user clicks
    //! on a slice geometry in any OSG window.
    //! - All connected signal handlers receive (x,y,z) volume coordinates
    //!   of the mouse cursor and additional flags.
	typedef vpl::mod::CSignal<void, float, float, float, int> tSigSceneHit;

    //! Signal invoked when the COMMAND_DENSITY_MEASURE mode is active and user clicks
    //! on a slice geometry in any OSG window.
    //! - Connected signal handlers receive the estimated density value [Hu].
	typedef vpl::mod::CSignal<void, int> tSigDensityMeasure;

    //! Signal invoked when the COMMAND_DISTANCE_MEASURE mode is active and user clicks
    //! on a slice geometry in any OSG window.
    //! - Connected signal handlers receive the measured length [mm].
	typedef vpl::mod::CSignal<void, double> tSigDistanceMeasure;

    //! Signal invoked by drawing handler after user finishes any drawing on a slice
    //! geometry. One of drawing modes must be active.
	//! - Signal handlers receive an array of points (recomputed to the volume space),
    //!   used drawing mode, and special flags.
	typedef vpl::mod::CSignal<void, const osg::Vec3Array *, int, int> tSigDrawingDone;

    //! Signal to modify measuring tool parameters
    //! Firs parameter selects tool (density = 0, distance = 1), second is tool parameter.
    typedef vpl::mod::CSignal< void, int, int > tSigMeasuringParameters;

    //! Signal invoked when the MODE_SLICE_MOVE mode is active and user clicks with right
    //! button on a dragger in any OSG window
    //! - All connected signal handlers receive dragger pointer and additional flags
    //!   return true if the signal
    typedef vpl::mod::CSignal<void, osgManipulator::Dragger*, int> tSigDraggerHit;

public:
    //! Returns the current mode.
    tMode get() const { return m_Mode; }

    //! Sets the mode.
    void set(tMode Mode)
    {
        if( m_Mode != Mode )
        {
            m_Mode = Mode;
            m_sigModeChanged.invoke(m_Mode);
        }
    }

    //! Saves the current mode and sets a new one.
    void storeAndSet(tMode Mode) 
    {
        if( m_Mode != Mode )
        {
            storeCurrentMode();
            m_Mode = Mode;
	        m_sigModeChanged.invoke(m_Mode);
        }
    }

    //! Sets the mode and stores the new value.
    void setAndStore(tMode Mode) 
    {
        if( m_Mode != Mode )
        {
            m_Mode = Mode;
            storeCurrentMode();
	        m_sigModeChanged.invoke(m_Mode);
        }
    }

    //! Returns true if a given mode is enabled.
    bool check(tMode Mode) { return (m_Mode == Mode); }

    //! Sets the default mode.
    void setDefault()
    {
        m_StoredMode = m_Mode = DEFAULT_MODE;
	    m_sigModeChanged.invoke(m_Mode);
    }

    //! Restores the previously saved mode.
    //! - Returns false on failure (= no change).
    bool restore()
    {
        if( m_Mode == m_StoredMode )
        {
            return false;
        }

        m_Mode = m_StoredMode;
        m_sigModeChanged.invoke(m_Mode);
        return true;
    }

    //! Get stored mode
    int getStoredMode() const { return m_StoredMode; }

    //! Is temp mode
    bool isTempMode() const { return m_Mode != m_StoredMode; }

    //! Get mode changed signal.
    tSigModeChanged& getModeChangedSignal()
    {
	    return m_sigModeChanged;
    }

    //! Returns reference to the "scene hit" signal.
    tSigSceneHit& getSceneHitSignal()
    {
	    return m_sigSceneHit;
    }

    //! Returns reference to the "density measure" signal.
    tSigDensityMeasure& getDensityMeasureSignal()
    {
	    return m_sigDensityMeasure;
    }

    //! Returns reference to the "continuous density measure" signal.
    tSigDensityMeasure& getContinuousDensityMeasureSignal()
    {
        return m_sigContinuousDensityMeasure;
    }

    //! Returns reference to the "distance measure" signal.
    tSigDistanceMeasure& getDistanceMeasureSignal()
    {
	    return m_sigDistanceMeasure;
    }

    //! Returns reference to the "measuring parameters" signal
    tSigMeasuringParameters & getMeasuringParametersSignal()
    {
        return m_sigMeasuringParameters;
    }

    //! Returns reference to the "scene hit" signal.
    tSigDraggerHit& getDraggerHitSignal()
    {
        return m_sigDraggerHit;
    }

    //! Is drawing handler connected?
    bool areDrawingHandlers() 
    {
        return m_sigDrawingDone.getNumOfConnections() > 0;
    }

    //! Is scene hit connected?
    bool areSceneHitHandlers() 
    {
        return m_sigSceneHit.getNumOfConnections() > 0;
    }

    //! Unregisters all existing signal handlers
    void disconnectAllSceneHitHandlers()
    {
        if( m_sigSceneHit.getNumOfConnections() > 0 )
        {
            // disconnect old handler
            m_sigSceneHit.disconnectAll();
        }
    }

    //! Registers a new signal handler.
    //! - Returns unique connection identifier.
    //! - Parameter F is a function type.
    template <typename F>
    vpl::mod::tSignalConnection connectDrawingHandler(F Func)
    {
        if( m_sigDrawingDone.getNumOfConnections() > 0 )
        {
            // Set focus lost
            m_sigDrawingDone.invoke( NULL, data::CDrawingOptions::FOCUS_LOST, 0 );

            // disconnect old handler
            m_sigDrawingDone.disconnectAll();
        }

        // Connect new handler
        vpl::mod::tSignalConnection con( m_sigDrawingDone.connect( Func ) );

        // Send focus on signal
        m_sigDrawingDone.invoke( NULL, data::CDrawingOptions::FOCUS_ON, 0 );

        return con;
    }

    //! Constructor creates functor encapsulating pointer to member function.
    //! - Parameter O is pointer to an object.
    //! - M is pointer to the object member function.
    template <class O, typename F>
    vpl::mod::tSignalConnection connectDrawingHandler(const O& pObject, F pMemFunc)
    {
        if( m_sigDrawingDone.getNumOfConnections() > 0 )
        {
            // Set focus lost
            m_sigDrawingDone.invoke( NULL, data::CDrawingOptions::FOCUS_LOST, 0 );

            // disconnect old handler
            m_sigDrawingDone.disconnectAll();

        }

        // Connect new handler
        vpl::mod::tSignalConnection con( m_sigDrawingDone.connect( pObject, pMemFunc ));

        // Send focus on signal
        m_sigDrawingDone.invoke( NULL, data::CDrawingOptions::FOCUS_ON, 0 );

        return con;
    }

    //! Ivoke drawing done signal - invoked by drawing handlers
    void invokeDrawingDone( const osg::Vec3Array * buffer, int p1, int p2 )
    {
        m_sigDrawingDone.invoke( buffer, p1, p2 );
    }

    //! Unregisters an existing signal handler.
    void disconnectDrawingHandler(vpl::mod::tSignalConnection& sc)
    {
        m_sigDrawingDone.disconnect( sc );
    }

    //! Unregisters all existing signal handlers
    void disconnectAllDrawingHandlers()
    {
        if( m_sigDrawingDone.getNumOfConnections() > 0 )
        {
            // Set focus lost
            m_sigDrawingDone.invoke( NULL, data::CDrawingOptions::FOCUS_LOST, 0 );

            // disconnect old handler
            m_sigDrawingDone.disconnectAll();
        }
    }

    //! Enable drawing handler DrawingDone during drawing
    void enableContinuousDrawing(bool bEnable)
    {
        m_bContinuousDrawing = bEnable;
    }

    //! Is enabled drawing handler DrawingDone calling during drawing
    bool isContinuousDrawingEnabled() const
    {
        return m_bContinuousDrawing;
    }

    //! Enable/disable highlighting
    void enableHighlight( bool enabled ){ m_bHighlightEnabled = enabled; }

    //! Is highlighting enabled
    bool isHighlightEnabled(){ return m_bHighlightEnabled; }

protected:
    //! Current application mode.
    tMode m_Mode;

    //! Previous/stored mode.
    tMode m_StoredMode;

    //! Is highlighting enabled
    bool m_bHighlightEnabled;

    //! Is continous drawing enabled
    bool m_bContinuousDrawing;

	//! Mode changed signal
	tSigModeChanged m_sigModeChanged;

    //! Scene hit signal.
    tSigSceneHit m_sigSceneHit;

    //! Density measured signal.
    tSigDensityMeasure m_sigDensityMeasure;

    //! Continuous density measured signal.
    tSigDensityMeasure m_sigContinuousDensityMeasure;

    //! Distance measured signal.
    tSigDistanceMeasure m_sigDistanceMeasure;

    //! Measuring tool parameters signal
    tSigMeasuringParameters m_sigMeasuringParameters;

	//! Drawing is done signal
	tSigDrawingDone m_sigDrawingDone;

    //! Dragger hit signal.
    tSigDraggerHit m_sigDraggerHit;

protected:
    //! Stores the current mouse mode.
    void storeCurrentMode()
    {
        // Cannot store a command mode...
        if( (m_Mode & COMMAND_MODE) == 0 )
        {
            m_StoredMode = m_Mode;
        }
    }

private:
    //! Private default constructor.
    CAppMode() : 
		m_Mode(DEFAULT_MODE), 
		m_StoredMode(DEFAULT_MODE), 
		m_bHighlightEnabled(true),
		m_bContinuousDrawing(false)
	{
	}

    //! Allow instantiation via singleton holder only.
    VPL_PRIVATE_SINGLETON(CAppMode);
};


//! Macro helpful accessing the application mode singleton.
#define APP_MODE VPL_SINGLETON(scene::CAppMode)


} // namespace scene

#endif // CAppMode_H
