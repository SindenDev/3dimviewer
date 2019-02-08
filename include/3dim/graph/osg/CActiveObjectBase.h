///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// This file comes from 3DimViewer software and was modified for 
// 
// BlueSkyPlan version 3.x
// Diagnostic and implant planning software for dentistry.
//
// The original 3DimViewer license can be found below.
//
// Changes are Copyright 2012 Blue Sky Bio, LLC
// All rights reserved 
//
// Changelog:
//    [2012/mm/dd] - ...
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
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

#ifndef CActiveObjectInterface_H_Included
#define CActiveObjectInterface_H_Included

#include <string>
#include <VPL/Module/Signal.h>

namespace osgGA
{
    class GUIEventAdapter;
    class GUIActionAdapter;
}

namespace osg
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //!\brief	Active object interface. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class CActiveObjectBase
    {
    public:
        //! "Classes" of the active objects
        enum EObjectClass
        {
            AO_DEFAULT,
            AO_SCENE,
            AO_OVERLAY
        };

        typedef vpl::mod::CSignal<void, CActiveObjectBase *> tOnDestroySignal;

    public:
        /// Constructor
        CActiveObjectBase( const std::string & name = "ActiveObject", int priority = 0, EObjectClass oc = AO_DEFAULT );

        //! Destructor
        virtual ~CActiveObjectBase();

        //! On mouse enter callback
        virtual void onMouseEnter(const osgGA::GUIEventAdapter& ea, bool command_mode);

        //! On mouse exit callback
        virtual void onMouseExit(const osgGA::GUIEventAdapter& ea, bool command_mode);

        //! On mouse down callback
        virtual bool onMouseDown(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        //! On mouse release callback
        virtual bool onMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        //! On mouse scroll
        virtual bool onMouseScroll(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        //! On key down
        virtual bool onKeyDown(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        //! Get object name
        std::string getName() const { return m_name; }

        //! Set object priority
        void setPriority(int priority) { m_priority = priority; }

        //! Get object priority
        int getPriority() const { return m_priority; }

        //! Get object class
        EObjectClass getClass() { return m_class; }

        //! Set object class
        void setClass( EObjectClass c ) { m_class = c; }

        //! Enable/disable active object handling
        void setActive(bool active) { m_active = active;}

        //! Is object enabled
        bool isActive() const {return m_active;}

        //! Get on destroy signal
        tOnDestroySignal &getDestroySignal() { return m_destroySignal; }

    protected:
        //! Name of the object
        std::string m_name;

        //! Object priority
        int m_priority;

        //! Object class
        EObjectClass m_class;

        //! Is object active?
        bool m_active;

        tOnDestroySignal m_destroySignal;

    }; // class CActiveObjectInterface
}


// CActiveObjectInterface_H_Included
#endif