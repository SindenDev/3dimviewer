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

#ifndef CForceCullCallback_H_included
#define CForceCullCallback_H_included

#include <osg/Drawable>

namespace osg
{
    /**
     * \class   ForceCullNodeCallback
     *
     * \brief   Force cull callback - used to make nodes invisible.
     */
    class CForceCullNodeCallback : public osg::NodeCallback
    {
    public:
        //! Constructor 
        CForceCullNodeCallback( bool bVisible = true ) : m_bVisible( bVisible ) {}

        //! Set visible/hidden
        void show( bool bShow ) { m_bVisible = bShow; }

        //! Culling callback
        virtual void operator()(Node *node, NodeVisitor *nv)
        {
            if (m_bVisible)
            {
                node->traverse(*nv);
            }
        }

        //! Is object visible
        bool isVisible(){ return m_bVisible; }

    protected:
        //! Is this object visible?
        bool m_bVisible;
    };

    /**
     * \class   ForceCullCallback
     *
     * \brief   Force cull callback - used to make drawable invisible.
     */
    class CForceCullCallback : public osg::Drawable::CullCallback
    {
    public:
        //! Constructor 
        CForceCullCallback( bool bVisible = true ) : m_bVisible( bVisible ) {}

        //! Set visible/hidden
        void show( bool bShow ) { m_bVisible = true; }

        //! Culling callback
        virtual bool cull(osg::NodeVisitor*, osg::Drawable*, osg::State*) const
        {
            return m_bVisible;
        }

        //! Is object visible
        bool isVisible(){ return m_bVisible; }

    protected:
        //! Is this object visible?
        bool m_bVisible;
    };

    class CInvisibleObjectInterface 
    {
    public:
        //! Constructor
        CInvisibleObjectInterface( bool bVisible = true ){ m_fcCallback = new CForceCullCallback( bVisible ); }

        //! Is this object visible
        bool isIObjectVisible() { return m_fcCallback->isVisible(); }

    protected:
        //! Add callback to the drawable
        template< class tpDrawable >
        void setForceCullCallback( tpDrawable * drawable ){ if( drawable != 0 ) drawable->setCullCallback( m_fcCallback ); }

        //! Set object visible/invisible
        void setIObjectVisible( bool bVisible ) { m_fcCallback->show( bVisible ); }

    protected:
        //! Culling callback
        osg::ref_ptr< CForceCullCallback > m_fcCallback;
    };
}

// CForceCullCallback_H_included
#endif



