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

#ifndef CHitProspector_H
#define CHitProspector_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osgUtil/IntersectVisitor>
#include <vector>

namespace osg
{

///////////////////////////////////////////////////////////////////////////////
//! Class is used to test obstacle intersections.

class CHitObstacle
{
public:
    //! Empty virtual destructor
    virtual ~CHitObstacle() {}

    //! Test if it is obstacle 
    virtual bool isObstacle(const osgUtil::Hit & hit)
    {
#pragma unused(hit)
        return false;
    }
};


//////////////////////////////////////////////////////////////////////////
//! Class is used to test if desired geometry is hited.

class CHitDesired
{
public:
    //! Empty virtual destructor
    virtual ~CHitDesired() {}

    //! Test if it is desired geometry
    virtual bool isDesired(const osgUtil::Hit & hit)
    {
        return true;
    }
};


//////////////////////////////////////////////////////////////////////////
//! Prospector takes hit list and finds relevant hits (in front of obstacles, with relevant nodes).

class CHitProspector 
{
public:
    //! List of all found intersections.
    typedef osgUtil::IntersectVisitor::HitList tHitList;

    //! Constructor
    CHitProspector();

    //! Destructor
    ~CHitProspector();

    //! Prospect input hitlist, if contains desirable hits. If it is true, they are stored in the output list.
    bool prospect(const tHitList & input_hitlist, tHitList & output_hitlist, bool bClearOutput = true);

    //! Adds obstacle rule 
    void addObstacleRule(CHitObstacle * obstacle);

    //! Add desired rule
    void addDesiredRule(CHitDesired * desired);

    //! Should I use obstacles (implicit is false).
    void useObstacles(bool use)
    {
	    m_useObstacles = use;
    }

    //! Should I use desired rules.
    void useDesired(bool use)
    {
	    m_useDesired = use;
    }

protected:
    //! Obstacles
    std::vector<CHitObstacle *> m_obstacleRules; 

    //! Desired nodes/hits/etc...
    std::vector<CHitDesired *> m_desiredRules;

    //! Use obstacles?
    bool m_useObstacles;

    //! Use desired?
    bool m_useDesired;
};


//! Template used to delete vector element 
template <class tpElementType>
void deleteElement(tpElementType * element)
{
    if( element )
    {
	    delete element;
        element = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//! Obstacle - by vector of nodes

class CNodeListHitObstacle : public CHitObstacle
{
public:
    //! Add tested node
    void addNode(ref_ptr<Node> node);

    //! Test if hit is with some of the stored obstacles 
    virtual bool isObstacle(const osgUtil::Hit & hit);

protected:
    //! Nodes vector
    std::vector<ref_ptr<Node> > m_nodesVector;
};


//////////////////////////////////////////////////////////////////////////
//! Obstacle - by node type

template<class tpNodeType>
class CNodeTypeHitObstacle : public CHitObstacle
{
    //! Test if hit is with some of the stored obstacles 
    virtual bool isObstacle(const osgUtil::Hit & hit)
    {
	    const tpNodeType * node = dynamic_cast<const tpNodeType *>(*hit.getNodePath().end());
	    return (node != NULL);
    }
};


//////////////////////////////////////////////////////////////////////////
//! Desired hits by node list

class CNodeListHitDesired : public CHitDesired
{
public:
    //! Add tested node
    void addNode(ref_ptr<Node> node);

    //! Test if hit is with some of the stored obstacles 
    virtual bool isDesired(const osgUtil::Hit & hit);

protected:
    //! Nodes vector
    std::vector<ref_ptr<Node> > m_nodesVector;
};


//////////////////////////////////////////////////////////////////////////
//! Desired hits - by node type.

template <class tpNodeType>
class CNodeTypeHitDesired : public CHitDesired
{
    //! Test if hit is with some of the stored obstacles 
    virtual bool isDesired(const osgUtil::Hit & hit)
    {
	    const tpNodeType * node = dynamic_cast<const tpNodeType *>(*hit.getNodePath().end() );
	    return (node != NULL);
    }
};


} // namespace osg

#endif // CHitProspector_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
