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

#include <graph/osg/CTools.h>
#include <osg/Transform>

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Calculates the node path to root. 
//!
//!\param [in,out]	node	The node. 
//!\param [in,out]	np		The np. 
////////////////////////////////////////////////////////////////////////////////////////////////////

void computeNodePathToRoot( osg::Node& node, osg::NodePath& np )

{
	np.clear();

	osg::NodePathList nodePaths = node.getParentalNodePaths();

	if (!nodePaths.empty())
	{
		np = nodePaths.front();
		if (nodePaths.size()>1)
		{
			osg::notify(osg::NOTICE)<<"osgManipulator::computeNodePathToRoot(,) taking first parent path, ignoring others."<<std::endl;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Gets a world to local matrix. 
//!
//!\param [in,out]	node	The node. 
//!
//!\return	The world to local matrix. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osg::Matrix osg::getWorldToLocalMatrix( Node & node )
{
	osg::NodePath nodePathToRoot;
	computeNodePathToRoot(node,nodePathToRoot);
	osg::Matrix _localToWorld( osg::computeLocalToWorld(nodePathToRoot) );
	return osg::Matrix::inverse(_localToWorld);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//!\brief	Gets a local to world matrix. 
//!
//!\param [in,out]	node	The node. 
//!
//!\return	The local to world matrix. 
////////////////////////////////////////////////////////////////////////////////////////////////////
osg::Matrix osg::getLocalToWorldMatrix( Node & node )
{
	osg::NodePath nodePathToRoot;
	computeNodePathToRoot(node,nodePathToRoot);
	return( osg::computeLocalToWorld(nodePathToRoot) );
}
