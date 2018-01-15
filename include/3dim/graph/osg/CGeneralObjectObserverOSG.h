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

#ifndef CGeneralObjectObserverOSG_H
#define CGeneralObjectObserverOSG_H

#include <data/CGeneralObjectObserver.h>

#include <osg/Node>
#include <osg/Drawable>
#include <osg/OSGCanvas.h>

namespace scene
{
    template <class T>
    class CGeneralObjectObserverOSG;
    
    ///////////////////////////////////////////////////////////////////////////////
    //! Update callback modified to check state of a storage entry.
    template <class T>
    class CUpdateFromStorageGeneralCallback : public osg::NodeCallback
    {
    public:
        virtual void operator()(osg::Node *node, osg::NodeVisitor *nv)
        {
            CGeneralObjectObserverOSG<T> *pObserver = dynamic_cast<CGeneralObjectObserverOSG<T> *>(node);

            if (pObserver && pObserver->hasChanged())
            {
                pObserver->updateFromStorage();
                pObserver->changesHandled();
            }

            traverse(node, nv);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    //! Update callback modified to check state of a storage entry.
    template <class T>
    class CUpdateFromStorageDrawableGeneralCallback : public osg::Drawable::UpdateCallback
    {
    public:
        virtual void update(osg::NodeVisitor *nv, osg::Drawable * d)
        {
            CGeneralObjectObserverOSG<T> *pObserver = dynamic_cast<CGeneralObjectObserverOSG<T> *>(d);

            if (pObserver && pObserver->hasChanged())
            {
                pObserver->updateFromStorage();
                pObserver->changesHandled();
            }
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    //! Base class for all observers of data entries.
    //! - Specialized for OSG nodes containing a geometry.
    //! - Only for objects encapsulated in CObjectHolder<T> class!
    template <class T>
    class CGeneralObjectObserverOSG : public data::CGeneralObjectObserver<T>
    {
    public:
        //! OSG update callback.
        typedef CUpdateFromStorageGeneralCallback<T> tNodeCallback;

        //! OSG update callback - Drawables
        typedef CUpdateFromStorageDrawableGeneralCallback<T> tDrawableCallback;

        //! User data.
        typedef OSGCanvas tCanvas;

        //! Base class.
        typedef data::CGeneralObjectObserver<T> tBase;

    public:
        //! Default constructor.
        CGeneralObjectObserverOSG(tCanvas *pCanvas = NULL)
            : m_pCanvas(pCanvas)
        {
            data::CGeneralObjectObserver<T>::m_changesHandledAutoInvoke = false;
        }

        //! Virtual destructor.
        virtual ~CGeneralObjectObserverOSG()
        { }

        //! Sets the OpenGL canvas.
        CGeneralObjectObserverOSG &setCanvas(tCanvas *pCanvas)
        {
            m_pCanvas = pCanvas;
            return *this;
        }

        //! Returns pointer to the OpenGL canvas.
        tCanvas *getCanvas()
        {
            return m_pCanvas;
        }

        //! Virtual method called on any change of the entry by the data storage.
        virtual void objectChanged(data::CStorageEntry *pEntry, const data::CChangedEntries &changes)
        {
            // Invalidate OpenGL canvas
            if (m_pCanvas)
            {
                m_pCanvas->Refresh(false);
            }
        }

        //! Method called during the OSG update callback.
        virtual void updateFromStorage() = 0;

        //! Modifies the update callback of a given Node.
        void setupObserver(osg::Node *node)
        {
            if (node)
            {
                m_callback = new tNodeCallback;
                node->addUpdateCallback(m_callback);
            }
        }

        //! Removes the update callback of a given Node.
        void freeObserver(osg::Node *node)
        {
            if (node)
            {
                if (m_callback)
                {
                    node->removeUpdateCallback(m_callback);
                }
            }
        }

    protected:
        //! Pointer to the OpenGL canvas.
        tCanvas *m_pCanvas;

        //! Node callback
        osg::ref_ptr<tNodeCallback> m_callback;
    };
} // namespace scene

#endif // CGeneralObjectObserverOSG_H
