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

#ifndef CObjectObserverOSG_H
#define CObjectObserverOSG_H

#include <data/CObjectObserver.h>
#include <osg/OSGCanvas.h>

#include <osg/Node>
#include <osg/Drawable>
#include <osg/Version>

namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! Update callback modified to check state of a storage entry.
//! - Class T must be derived from the CObjectObserverOSG<> class.

template <class T>
class CUpdateFromStorageCallback : public osg::NodeCallback
{
public:
    virtual void operator()(osg::Node * node, osg::NodeVisitor * nv)
    {
        T * pObserver = dynamic_cast<T *>(node);

        if( pObserver && pObserver->hasChanged() )
        {
            pObserver->updateFromStorage();
            pObserver->clearChanges();
        }

        traverse(node,nv);
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////
//! Update from storage drawable callback. 
//! - Class T must be derived from the CObjectObserverOSG<> class.

template <class T>
class CUpdateFromStorageDrawableCallback : public osg::Drawable::UpdateCallback
{
public:
    virtual void update(osg::NodeVisitor * nv, osg::Drawable * d)
    {
        T * pObserver = dynamic_cast< T * >( d );

        if( pObserver && pObserver->hasChanged() )
        {
            pObserver->updateFromStorage();
            pObserver->clearChanges();
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
//! Base class for all observers of data entries.
//! - Specialized for OSG nodes containing a geometry.
//! - Only for objects encapsulated in CObjectHolder<T> class!

template <class T, class C = OSGCanvas>
class CObjectObserverOSG : public data::CObjectObserver<T>
{
public:
    //! Object type.
    typedef T tObject;

    //! OSG update callback - node version.
    typedef CUpdateFromStorageCallback<CObjectObserverOSG> tNodeCallback;

    //! OSG update callback - drawable version
    typedef CUpdateFromStorageDrawableCallback<CObjectObserverOSG> tDrawableCallback;

    //! User data.
    typedef C tCanvas;

public:
    //! Default constructor.
    CObjectObserverOSG(tCanvas *pCanvas = NULL) : m_pCanvas(pCanvas) {}

    //! Virtual destructor.
	virtual ~CObjectObserverOSG() {}

    //! Sets the OpenGL canvas.
    CObjectObserverOSG& setCanvas(tCanvas *pCanvas)
    {
        m_pCanvas = pCanvas;
        return *this;
    }

    //! Returns pointer to the OpenGL canvas.
    tCanvas *getCanvas() { return m_pCanvas; }

    //! Virtual method called on any change of the entry
    //! by the data storage.
    virtual void objectChanged(tObject *pObject)
    {
        // Invalidate OpenGL canvas
        if( m_pCanvas )
        {
            m_pCanvas->Refresh(false);
        }
    }

    //! Method called during the OSG update callback.
    virtual void updateFromStorage() = 0;

    //! Modifies the update callback of a given Node.
    void setupObserver(osg::Node * node)
    {
        if( node )
        {
            node->addUpdateCallback(new tNodeCallback);
        }
    }

    //! Removes the update callback of a given Node.
    void freeObserver(osg::Node * node)
    {
        if( node )
        {
#if OSG_VERSION_GREATER_OR_EQUAL(3,2,0)
            osg::Callback* pCB = node->getUpdateCallback();
#else
			osg::NodeCallback* pCB = node->getUpdateCallback();
#endif
            if( pCB )
            {
                node->removeUpdateCallback(pCB);
            }
        }
    }

    //! Modifies the drawable update callback
/*    void setupObserver( osg::Drawable * drawable )
    {
        if( drawable )
        {
            drawable->addUpdateCallback( new tDrawableCallback );
        }
    }*/

protected:
    //! Pointer to the OpenGL canvas.
    tCanvas *m_pCanvas;
};


} // namespace scene

#endif // CObjectObserverOSG_H
