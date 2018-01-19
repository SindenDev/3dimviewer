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

///////////////////////////////////////////////////////////////////////////////
// include files

#include <osg/CHitProspector.h>
#include <algorithm>

using namespace osg;

//////////////////////////////////////////////////////////////////////////
// Constructor
CHitProspector::CHitProspector()
    : m_useObstacles(false)
    , m_useDesired(false)
{

}

//////////////////////////////////////////////////////////////////////////
// Destructor
CHitProspector::~CHitProspector()
{
	// Delete all obstacle rules
	std::for_each(m_obstacleRules.begin(), m_obstacleRules.end(), &deleteElement<CHitObstacle>);

	// Delete all desired rules
	std::for_each(m_desiredRules.begin(), m_desiredRules.end(), &deleteElement<CHitDesired>);
}


///////////////////////////////////////////////////////////////////////////////
// Prospect hit list and find relevant hits (in front of obstacles, with relevant nodes).
bool CHitProspector::prospect(const tHitList & input_hitlist, tHitList & output_hitlist, bool bClearOutput /*= true*/)
{
	if(bClearOutput)
		output_hitlist.clear();

	tHitList::const_iterator hit;
	std::vector<CHitObstacle *>::iterator obstacle;
	std::vector<CHitDesired *>::iterator desired;

	osg::NodePath::iterator foundNode;

	// iterate trough all hits in list
	for(hit = input_hitlist.begin(); hit != input_hitlist.end(); hit++)
	{
		if(m_useObstacles)
		{
			// try to find obstacle
			for(obstacle = m_obstacleRules.begin(); obstacle != m_obstacleRules.end(); obstacle++)
				if((*obstacle)->isObstacle(*hit))
				{
					// This hit is an obstacle, so don't use other hits and return
					return output_hitlist.size() > 0;
				} 

		} // if m_useObstacles

		if(m_useDesired)
		{
			// Try to find desired object
			for(desired = m_desiredRules.begin(); desired != m_desiredRules.end(); desired++)
				if((*desired)->isDesired(*hit))
				{
					// object is desired, so add hit to the output list
					output_hitlist.push_back(*hit);
				}

		}else{

			// just copy hit to the output
			output_hitlist.push_back(*hit);
		}
	} // for all hits in input hit list

	return output_hitlist.size() > 0;
}

///////////////////////////////////////////////////////////////////////////////
//
void CHitProspector::addObstacleRule(CHitObstacle * obstacle)
{
	if(obstacle == NULL)
		return;

	m_obstacleRules.push_back(obstacle);
}

//////////////////////////////////////////////////////////////////////////
// Add desired hit rule
void CHitProspector::addDesiredRule(CHitDesired * desired)
{
	if(desired == NULL)
		return;

	m_desiredRules.push_back(desired);
}

//////////////////////////////////////////////////////////////////////////
// Obstacle specializations
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Obstacle - by vector of nodes - adding node
void CNodeListHitObstacle::addNode(ref_ptr<Node> node)
{
	m_nodesVector.push_back(node);
}

//////////////////////////////////////////////////////////////////////////
// Obstacle - by vector of nodes - hit test
bool CNodeListHitObstacle::isObstacle(const osgUtil::Hit & hit)
{
	// node path iterator
	osg::NodePath::const_iterator foundNode, b;

	// obstacles iterator
	std::vector<ref_ptr<Node> >::iterator obstacle;

	for(obstacle = m_nodesVector.begin(); obstacle != m_nodesVector.end(); obstacle++)
	{
		foundNode =  std::find(hit.getNodePath().begin(), hit.getNodePath().end(), obstacle->get());

		b = hit.getNodePath().end();

		
		// if node is obstacle, return true - obstacle found...
		if(foundNode != hit.getNodePath().end())
			return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Desired hits specializations
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Desired hits - by vector of nodes - adding node
void CNodeListHitDesired::addNode(ref_ptr<Node> node)
{
	m_nodesVector.push_back(node);
}

//////////////////////////////////////////////////////////////////////////
// Desired hits - by vector of nodes - hit test
bool CNodeListHitDesired::isDesired(const osgUtil::Hit & hit)
{
	// node path iterator
	osg::NodePath::const_iterator foundNode, b;

	// obstacles iterator
	std::vector<ref_ptr<Node> >::iterator node;

	for(node = m_nodesVector.begin(); node != m_nodesVector.end(); node++)
	{
		foundNode =  std::find(hit.getNodePath().begin(), hit.getNodePath().end(), node->get());

		b = hit.getNodePath().end();


		// if node is obstacle, return true - obstacle found...
		if(foundNode != hit.getNodePath().end())
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
