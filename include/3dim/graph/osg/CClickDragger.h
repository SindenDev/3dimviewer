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

#ifndef CClickDragger_H
#define CClickDragger_H

#include <osgManipulator/Dragger>

namespace osgManipulator
{
    class CClickDragger : public Dragger
    {
    public:
        class CClickDraggerCallback : public osg::Object
        {
        public:
            CClickDraggerCallback() { }
            CClickDraggerCallback(const CClickDraggerCallback &callback, const osg::CopyOp &copyop = osg::CopyOp::SHALLOW_COPY) { }
            virtual Object *cloneType() const { return new CClickDraggerCallback(); }
            virtual Object *clone(const osg::CopyOp &copyop) const { return new CClickDraggerCallback(*this, copyop); }
            virtual bool isSameKindAs(const Object *obj) const { return dynamic_cast<const CClickDraggerCallback *>(obj) != NULL; }
            virtual const char *libraryName() const { return "osgManipulator"; }
            virtual const char *className() const { return "CClickDraggerCallback"; }
            virtual bool operator()(const PointerInfo &pointer, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) { return false; }

        protected:
            virtual ~CClickDraggerCallback() { }
        };

    protected:
        bool m_bIsDown;
        osg::ref_ptr<CClickDraggerCallback> m_clickCallback;
        osg::ref_ptr<CClickDraggerCallback> m_releaseCallback;

    public:
        CClickDragger(osg::Node *child, CClickDraggerCallback *clickCallback, CClickDraggerCallback *releaseCallback)
            : Dragger()
            , m_bIsDown(false)
            , m_clickCallback(clickCallback)
            , m_releaseCallback(releaseCallback)
        {
            addChild(child);
        }

        virtual ~CClickDragger()
        { }

        virtual bool handle(const PointerInfo &pointer, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
        {
            if (!pointer.contains(this))
            {
                return false;
            }

            if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
            {
                m_bIsDown = true;
                if (m_clickCallback == NULL)
                {
                    if (onClick(pointer, ea, aa))
                    {
                        aa.requestRedraw();
                        return true;
                    }
                }
                else
                {
                    if (m_clickCallback->operator()(pointer, ea, aa))
                    {
                        aa.requestRedraw();
                        return true;
                    }
                }
            }

            if ((ea.getEventType() == osgGA::GUIEventAdapter::RELEASE) && (m_bIsDown))
            {
                m_bIsDown = false;
                if (m_releaseCallback == NULL)
                {
                    if (onRelease(pointer, ea, aa))
                    {
                        aa.requestRedraw();
                        return true;
                    }
                }
                else
                {
                    if (m_releaseCallback->operator()(pointer, ea, aa))
                    {
                        aa.requestRedraw();
                        return true;
                    }
                }
            }

            return false;
        }

    protected:
        virtual bool onClick(const PointerInfo &pointer, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
        {
            return false;
        }

        virtual bool onRelease(const PointerInfo &pointer, const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
        {
            return false;
        }
    }; // CClickDragger

} // namespace osgManipulator

#endif // CClickDragger_H
