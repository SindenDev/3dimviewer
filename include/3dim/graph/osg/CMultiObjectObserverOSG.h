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

#ifndef CMultiObjectObserverOSG_H
#define CMultiObjectObserverOSG_H

#include <data/CMultiObjectObserver.h>
#include <osg/OSGCanvas.h>

#include <osg/Node>
#include <osg/Drawable>


namespace scene
{

///////////////////////////////////////////////////////////////////////////////
//! Update callback modified to check state of a storage entry.
//! - Class T must be derived from the CMultiObjectObserverOSG<> class.

template <class T>
class CUpdateFromStorageMultiCallback : public osg::NodeCallback
{
public:
    virtual void operator()(osg::Node * node, osg::NodeVisitor * nv)
    {
        T * pObserver = dynamic_cast<T *>(node);

        if( pObserver )
        {
            int Changes = pObserver->hasChanged();
            if( Changes != 0 )
            {
                pObserver->updateFromStorage(Changes);
                pObserver->clearChanges(Changes);
            }
        }

        traverse(node,nv);
    }
};


///////////////////////////////////////////////////////////////////////////////
//! Update callback modified to check state of a storage entry.
//! - Class T must be derived from the CMultiObjectObserverOSG<> class.

template <class T>
class CUpdateFromStorageDrawableMultiCallback : public osg::Drawable::UpdateCallback
{
public:
    virtual void update(osg::NodeVisitor * nv, osg::Drawable * d)
    {
        T * pObserver = dynamic_cast< T * >( d );

        if( pObserver )
        {
            int Changes = pObserver->hasChanged();
            if( Changes != 0 )
            {
                pObserver->updateFromStorage(Changes);
                pObserver->clearChanges(Changes);
            }
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
//! Base class for all observers of data entries.
//! - Specialized for OSG nodes containing a geometry.
//! - Only for objects encapsulated in CObjectHolder<T> class!

template <class T1, class T2, class C = OSGCanvas>
class CMultiObjectObserverOSG : public data::CMultiObjectObserver<T1, T2>
{
public:
    //! Object type.
    typedef T1 tObject1;
    typedef T2 tObject2;

    //! OSG update callback.
    typedef CUpdateFromStorageMultiCallback<CMultiObjectObserverOSG> tNodeCallback;

    //! OSG update callback - Drawables
    typedef CUpdateFromStorageDrawableMultiCallback< CMultiObjectObserverOSG > tDrawableCallback;

    //! User data.
    typedef C tCanvas;

    //! Base class.
    typedef data::CMultiObjectObserver<T1, T2> tBase;

public:
    //! Default constructor.
    CMultiObjectObserverOSG(tCanvas *pCanvas = NULL) : m_pCanvas(pCanvas) {}

    //! Virtual destructor.
	virtual ~CMultiObjectObserverOSG() {}

    //! Sets the OpenGL canvas.
    CMultiObjectObserverOSG& setCanvas(tCanvas *pCanvas)
    {
        m_pCanvas = pCanvas;
        return *this;
    }

    //! Returns pointer to the OpenGL canvas.
    tCanvas *getCanvas() { return m_pCanvas; }

    //! Virtual method called on any change of the entry by the data storage.
    virtual void objectChanged(tObject1 *pObject)
    {
        // Invalidate OpenGL canvas
        if( m_pCanvas )
        {
            m_pCanvas->Refresh(false);
        }
    }

    //! Virtual method called on any change of the entry by the data storage.
    virtual void objectChanged(tObject2 *pObject)
    {
        // Invalidate OpenGL canvas
        if( m_pCanvas )
        {
            m_pCanvas->Refresh(false);
        }
    }

    //! Method called during the OSG update callback.
    virtual void updateFromStorage(int ChangedEntries) = 0;

    //! Modifies the update callback of a given Node.
    void setupObserver(osg::Node * node)
    {
        if( node )
        {
    	    node->addUpdateCallback(new tNodeCallback);
        }
    }

    //! Modifies the update callback of a given Drawable.
/*    void setupObserver(osg::Drawable * drawable)
    {
        if( drawable )
        {
    	    drawable->addUpdateCallback(new tDrawableCallback);
        }
    }*/

protected:
    //! Pointer to the OpenGL canvas.
    tCanvas *m_pCanvas;
};


} // namespace scene

#endif // CMultiObjectObserverOSG_H
