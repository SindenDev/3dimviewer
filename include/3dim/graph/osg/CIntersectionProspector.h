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

#ifndef CIntersectionProspector_H
#define CIntersectionProspector_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osgUtil/IntersectVisitor>
#include <osgUtil/LineSegmentIntersector>
#include <vector>
#include <algorithm>
#include <data/CDataStorage.h>
#include <osg/CModelVisualizer.h>


namespace osg
{

///////////////////////////////////////////////////////////////////////////////
//! Class is used to test obstacle intersections.
//
template <class tpIntersectionType>
class CIntersectionObstacle
{
public:
    //! Virtual destructor
    virtual ~CIntersectionObstacle() {}

    //! Test if it is obstacle 
    virtual bool isObstacle(const tpIntersectionType & Intersection) = 0;
/*    {
        return false;
    }*/
};


//////////////////////////////////////////////////////////////////////////
//! Class is used to test if desired geometry is Intersected.
//
template <class tpIntersectionType>
class CIntersectionDesired
{
public:
    //! Virtual destructor
    virtual ~CIntersectionDesired() {}

    //! Test if it is desired geometry
    virtual bool isDesired(const tpIntersectionType & Intersection) = 0;
/*    {
	    return true;
    }*/
};


//////////////////////////////////////////////////////////////////////////
//! Prospector takes Intersection list and finds relevant Intersections
//! (in front of obstacles, with relevant nodes).
//
template <class tpIntersectionType, class tpIntersectionList>
class CIntersectionProspector 
{
public:
    //! Constructor
    CIntersectionProspector();

    //! Destructor
    virtual ~CIntersectionProspector();

    //! Prospect input intersection list, if contains desirable Intersections.
    //! If it is true, they are stored in the output list.
    bool prospect(const tpIntersectionList & input_Intersectionlist,
                  tpIntersectionList & output_Intersectionlist,
                  bool bClearOutput = true
                  );

    //! Adds obstacle rule 
    void addObstacleRule(CIntersectionObstacle<tpIntersectionType> * obstacle);

    //! Add desired rule
    void addDesiredRule(CIntersectionDesired<tpIntersectionType> * desired);

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
    //! Template used to delete vector element 
    template <class tpElementType>
    static void deleteElement(tpElementType * element)
    {
	    if( element )
        {
		    delete element;
	        element = NULL;
        }
    }

    //! Obstacles
    std::vector<CIntersectionObstacle<tpIntersectionType> *> m_obstacleRules; 

    //! Desired nodes/Intersections/etc...
    std::vector<CIntersectionDesired<tpIntersectionType> *> m_desiredRules;

    //! Use obstacles?
    bool m_useObstacles;

    //! Use desired?
    bool m_useDesired;
};



//////////////////////////////////////////////////////////////////////////
//! Obstacle - by vector of nodes
//
template <class tpIntersectionType> 
class CNodeListIntersectionObstacle : public CIntersectionObstacle<tpIntersectionType>
{
public:
    //! Add tested node
    void addNode(ref_ptr<Node> node);

    //! Test if Intersection is with some of the stored obstacles 
    virtual bool isObstacle(const tpIntersectionType & Intersection);

protected:
    //! Nodes vector
    std::vector<ref_ptr<Node> > m_nodesVector;
};


//////////////////////////////////////////////////////////////////////////
//! Obstacle - by node type
//
template <class tpIntersectionType, class tpNodeType> 
class CNodeTypeIntersectionObstacle : public CIntersectionObstacle<tpIntersectionType>
{
public:
    //! Test if Intersection is with some of the stored obstacles 
    virtual bool isObstacle(const tpIntersectionType & Intersection)
    {
        osg::NodePath::const_iterator npIterator;
        const tpNodeType * node;

        for( npIterator = Intersection.nodePath.begin(); npIterator != Intersection.nodePath.end(); npIterator++ )
        {
            node = dynamic_cast<const tpNodeType *>(*npIterator );
            if( node )
            {
	            return true;
            }
        }

        return false;
    }
};

//////////////////////////////////////////////////////////////////////////
//! Obstacle - by CModel id in the storage
//
template< class tpIntersectionType >
class CStorageIdObstacle : public CIntersectionObstacle< tpIntersectionType >
{
protected:
    typedef std::vector< int > tIdsVector;

public:
    //! Test if intersection is with stored model
    virtual bool isObstacle( const tpIntersectionType & Intersection )
    {
        osg::NodePath::const_iterator npIterator;
        tIdsVector::iterator i;

        // For all stored ids
        for( i = m_ids.begin(); i != m_ids.end(); ++i )
        {
            for( npIterator = Intersection.nodePath.begin(); npIterator != Intersection.nodePath.end(); npIterator++ )
            {
                // Try to convert node to the model visualizer
                osg::CModelVisualizer * model = dynamic_cast< osg::CModelVisualizer * >( *npIterator );

                if( ! model )
                    continue;

                if( model->getId() == *i )
                {
	                return true;
                }
            }

        }

        return false;   
    }

    //! Add id
    void addId( int id ){ m_ids.push_back( id ); }

protected:
    // Stored id
    tIdsVector m_ids;
};


//////////////////////////////////////////////////////////////////////////
//! Obstacle - by CModel id in the storage
//
template< class tpIntersectionType >
class CStorageIdDesired : public CIntersectionDesired< tpIntersectionType >
{
protected:
    typedef std::vector< int > tIdsVector;

public:
    //! Test if intersection is with stored model
    virtual bool isDesired( const tpIntersectionType & Intersection )
    {
        osg::NodePath::const_iterator npIterator;
        tIdsVector::iterator i;

        // For all stored ids
        for( i = m_ids.begin(); i != m_ids.end(); ++i )
        {
            for( npIterator = Intersection.nodePath.begin(); npIterator != Intersection.nodePath.end(); npIterator++ )
            {
                // Try to convert node to the model visualizer
                osg::CModelVisualizer * model = dynamic_cast< osg::CModelVisualizer * >( *npIterator );

                if( ! model )
                    continue;

                if( model->getId() == *i )
                {
	                return true;
                }
            }

        }

        return false;   
    }

    //! Add id
    void addId( int id ){ m_ids.push_back( id ); }

    //! Remove all ids
    void clearIds() {m_ids.clear(); }

	//! Get ids
	const tIdsVector &getIds() {return m_ids; }

protected:
    // Stored id
    tIdsVector m_ids;
};


//////////////////////////////////////////////////////////////////////////
//! Desired Intersections by node list
//
template <class tpIntersectionType> 
class CNodeListIntersectionDesired : public CIntersectionDesired<tpIntersectionType>
{
public:
    typedef std::vector<Node *> tNodesVector;

public:
    //! Add tested node
    void addNode(Node *node);

    //! Test if Intersection is with some of the stored obstacles 
    virtual bool isDesired(const tpIntersectionType & Intersection);

    void clearNodes() {m_nodesVector.clear();}

    //! Get used nodes vector const access
    const tNodesVector & getNodesVector() { return m_nodesVector; }

    //! Set used nodes
    void setNodesVector(const tNodesVector &nodes) 
    { if(&nodes == &m_nodesVector)return; m_nodesVector.clear(); m_nodesVector.insert(m_nodesVector.end(), nodes.begin(), nodes.end());}

protected:
    //! Nodes vector
    tNodesVector m_nodesVector;
};


//////////////////////////////////////////////////////////////////////////
//! Desired Intersections - by node type
//
template <class tpIntersectionType, class tpNodeType> 
class CNodeTypeIntersectionDesired : public CIntersectionDesired<tpIntersectionType>
{
public:
    //! Test if Intersection is with some of the stored obstacles 
    virtual bool isDesired(const tpIntersectionType & Intersection)
    {
        osg::NodePath::const_iterator npIterator;
        const tpNodeType * node;

        for(npIterator = Intersection.nodePath.begin(); npIterator != Intersection.nodePath.end(); npIterator++)
        {
            node = dynamic_cast<const tpNodeType *>(*npIterator );
            if( node )
            {
                return true;
            }
        }

        return false;
    }
}; 


} // namespace osg

#endif // CIntersectionProspector_H

#include <osg/CIntersectionProspector.hpp>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
