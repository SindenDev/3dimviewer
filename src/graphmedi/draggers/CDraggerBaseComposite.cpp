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

#include <draggers/CDraggerBaseComposite.h>
#include <osg/Switch>

osgManipulator::CDraggerBaseComposite::CDraggerBaseComposite()
    : m_draggersSwitch(new osg::Switch())
    , m_locked(false)
{
    addChild(m_draggersSwitch);
}

bool osgManipulator::CDraggerBaseComposite::addDragger(Dragger * dragger)
{
    bool rv = CompositeDragger::addDragger(dragger);
    return rv && m_draggersSwitch->addChild(dragger);
}

void osgManipulator::CDraggerBaseComposite::lock(bool lockUnlock)
{
    m_locked = lockUnlock;
}

void osgManipulator::CDraggerBaseComposite::setChildValue(const Node * child, bool value)
{
    m_draggersSwitch->setChildValue(child, value);
}

bool osgManipulator::CDraggerBaseComposite::handle(const osgManipulator::PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    if (!m_locked)
    {
        return CompositeDragger::handle(pi, ea, aa);
    }
    else
    {
        return false;
    }
}
