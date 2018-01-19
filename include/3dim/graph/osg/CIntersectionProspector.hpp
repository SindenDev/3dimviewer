///////////////////////////////////////////////////////////////////////////////
// $Id: CIntersectionProspector.hpp 1289 2011-05-15 00:08:39Z spanel $
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

#ifndef CIntersectionProspector_HPP
#define CIntersectionProspector_HPP

namespace osg
{

    //////////////////////////////////////////////////////////////////////////
    // DEFINITIONS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // Constructor

    template <class tpElementType, class tpIntersectionList>
    CIntersectionProspector<tpElementType, tpIntersectionList>::CIntersectionProspector()
        : m_useObstacles(true)
        , m_useDesired(true)
    { }

    //////////////////////////////////////////////////////////////////////////
    // Destructor

    template <class tpElementType, class tpIntersectionList>
    CIntersectionProspector<tpElementType, tpIntersectionList>::~CIntersectionProspector()
    {
        // Delete all obstacle rules
        std::for_each(m_obstacleRules.begin(), m_obstacleRules.end(), &deleteElement<CIntersectionObstacle<tpElementType> >);

        // Delete all desired rules
        std::for_each(m_desiredRules.begin(), m_desiredRules.end(), &deleteElement<CIntersectionDesired<tpElementType> >);
    }


    ///////////////////////////////////////////////////////////////////////////////
    // Prospect Intersection list and find relevant Intersections (in front of obstacles, with relevant nodes).

    template <class tpElementType, class tpIntersectionList>
    bool CIntersectionProspector<tpElementType, tpIntersectionList>::prospect(const tpIntersectionList & input_Intersectionlist, tpIntersectionList & output_Intersectionlist, bool bClearOutput /*= true*/)
    {
        if (bClearOutput)
        {
            output_Intersectionlist.clear();
        }

        typename tpIntersectionList::const_iterator Intersection;
        typename std::vector<CIntersectionObstacle<tpElementType> *>::iterator obstacle;
        typename std::vector<CIntersectionDesired<tpElementType> *>::iterator desired;

        osg::NodePath::iterator foundNode;

        // iterate trough all Intersections in list
        for (Intersection = input_Intersectionlist.begin(); Intersection != input_Intersectionlist.end(); Intersection++)
        {
            if (m_useObstacles)
            {
                // try to find obstacle
                for (obstacle = m_obstacleRules.begin(); obstacle != m_obstacleRules.end(); ++obstacle)
                {
                    if ((*obstacle)->isObstacle(*Intersection))
                    {
                        // This Intersection is an obstacle, so don't use other Intersections and return
                        return output_Intersectionlist.size() > 0;
                    }
                }
            } // if m_useObstacles

            if (m_useDesired)
            {
                // Try to find desired object
                for (desired = m_desiredRules.begin(); desired != m_desiredRules.end(); ++desired)
                {
                    if ((*desired)->isDesired(*Intersection))
                    {
                        // object is desired, so add Intersection to the output list
                        output_Intersectionlist.insert(*Intersection);
                    }
                }
            }
            else
            {
                // just copy Intersection to the output
                output_Intersectionlist.insert(*Intersection);
            }
        } // for all Intersections in input Intersection list

        return output_Intersectionlist.size() > 0;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //

    template <class tpElementType, class tpIntersectionList>
    void CIntersectionProspector<tpElementType, tpIntersectionList>::addObstacleRule(CIntersectionObstacle<tpElementType> * obstacle)
    {
        if (obstacle == NULL)
        {
            return;
        }

        m_obstacleRules.push_back(obstacle);
    }

    //////////////////////////////////////////////////////////////////////////
    // Add desired Intersection rule

    template <class tpElementType, class tpIntersectionList>
    void CIntersectionProspector<tpElementType, tpIntersectionList>::addDesiredRule(CIntersectionDesired<tpElementType> * desired)
    {
        if (desired == NULL)
        {
            return;
        }

        m_desiredRules.push_back(desired);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //

    template <class tpElementType, class tpIntersectionList>
    void CIntersectionProspector<tpElementType, tpIntersectionList>::removeObstacleRule(CIntersectionObstacle<tpElementType> * obstacle)
    {
        if (obstacle == NULL)
        {
            return;
        }

        for (auto it = m_obstacleRules.begin(); it != m_obstacleRules.end(); ++it)
        {
            if (*it == obstacle)
            {
                m_obstacleRules.erase(it);
                return;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Add desired Intersection rule

    template <class tpElementType, class tpIntersectionList>
    void CIntersectionProspector<tpElementType, tpIntersectionList>::removeDesiredRule(CIntersectionDesired<tpElementType> * desired)
    {
        if (desired == NULL)
        {
            return;
        }

        for (auto it = m_desiredRules.begin(); it != m_desiredRules.end(); ++it)
        {
            if (*it == desired)
            {
                m_desiredRules.erase(it);
                return;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Obstacle specializations
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // Obstacle - by vector of nodes - adding node

    template <class tpElementType>
    void CNodeListIntersectionObstacle<tpElementType>::addNode(ref_ptr<Node> node)
    {
        m_nodesVector.push_back(node);
    }

    //////////////////////////////////////////////////////////////////////////
    // Obstacle - by vector of nodes - Intersection test

    template <class tpElementType>
    bool CNodeListIntersectionObstacle<tpElementType>::isObstacle(const tpElementType & Intersection)
    {
        // node path iterator
        osg::NodePath::const_iterator foundNode;

        // obstacles iterator
        typename std::vector<ref_ptr<Node> >::iterator obstacle;

        for (obstacle = m_nodesVector.begin(); obstacle != m_nodesVector.end(); ++obstacle)
        {
            foundNode = std::find(Intersection.nodePath.begin(), Intersection.nodePath.end(), obstacle->get());

            // if node is obstacle, return true - obstacle found...
            if (foundNode != Intersection.nodePath.end())
            {
                return true;
            }
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    // Desired Intersections specializations
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // Desired Intersections - by vector of nodes - adding node

    template <class tpElementType>
    void CNodeListIntersectionDesired<tpElementType>::addNode(Node *node)
    {
        if (node != 0)
        {
            m_nodesVector.push_back(node);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Desired Intersections - by vector of nodes - Intersection test

    template <class tpElementType>
    bool CNodeListIntersectionDesired<tpElementType>::isDesired(const tpElementType & Intersection)
    {
        // node path iterator
        osg::NodePath::const_iterator foundNode;

        // obstacles iterator
        typename tNodesVector::const_iterator node;

        for (node = m_nodesVector.begin(); node != m_nodesVector.end(); ++node)
        {
            foundNode = std::find(Intersection.nodePath.begin(), Intersection.nodePath.end(), *node);

            // if node is obstacle, return true - obstacle found...
            if (foundNode != Intersection.nodePath.end())
            {
                return true;
            }
        }

        return false;
    }

}

#endif // CIntersectionProspector_HPP

