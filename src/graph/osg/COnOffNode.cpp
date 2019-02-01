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

#include <osg/COnOffNode.h>

osg::COnOffNode::COnOffNode(bool isOn/* = true*/) : m_isOn(isOn)
{
    setName("COnOffNode");
}

void osg::COnOffNode::traverse(NodeVisitor& nv)
{
    if (m_isOn)
    {
        Switch::traverse(nv);
    }
}

osg::BoundingSphere osg::COnOffNode::computeBound() const
{
    if(m_isOn)
    {
        return Switch::computeBound();
    }
    else
    {
        return BoundingSphere();
    }
}

void osg::COnOffNode::setOnOffState(bool newState)
{
    if (m_isOn != newState)
    {
        m_isOn = newState;

        dirtyBound();
    }
}

bool osg::COnOffNode::isVisible() const
{
    return m_isOn;
}

void osg::COnOffNode::hide()
{
    setOnOffState(false);
}

void osg::COnOffNode::show()
{
    setOnOffState(true);
}

