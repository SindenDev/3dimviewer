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

#ifndef CDRAGGERBASECOMPOSITE_H_included
#define CDRAGGERBASECOMPOSITE_H_included

#include <osgManipulator/Dragger>


namespace osgManipulator
{

class CDraggerBaseComposite: public CompositeDragger
{
public:
    CDraggerBaseComposite();

    bool addDragger(Dragger* dragger) override;

    void lock(bool lockUnlock);
    void setChildValue(const Node* child, bool value);

    bool handle(const osgManipulator::PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;

private:
    bool m_locked;
    osg::ref_ptr<osg::Switch> m_draggersSwitch;
};


} //namespace osgManipulator

#endif // CDRAGGERBASECOMPOSITE_H_included