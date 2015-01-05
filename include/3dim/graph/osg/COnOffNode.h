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

#ifndef COnOffNode_H
#define COnOffNode_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osg/Switch>


namespace osg
{

///////////////////////////////////////////////////////////////////////////////
//! class description

class COnOffNode : public Switch
{
public:
    //! Constructor
    COnOffNode() : m_bIsOn(true) {}

    //! Accept node visitor
    virtual void traverse(NodeVisitor &nv);

    //! Set state
    void setOnOffState(bool newState); 

    //! Returns true if the node is visible.
    bool isShown() const
    {
        return m_bIsOn;
    }

    //! Hide node
    void hide()
    {
        setOnOffState( false );
    }

    //! Show node
    void show()
    {
        setOnOffState( true );
    }
    
protected:
    //! Current state
    bool m_bIsOn;
};


} // namespace osg

#endif // COnOffNode_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
