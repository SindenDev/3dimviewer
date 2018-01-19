#include "osg/Node"
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

#ifndef CTools_H_included
#define CTools_H_included

namespace osg
{

// Compute node path to the root
//void computeNodePathToRoot(osg::Node& node, osg::NodePath& np);

// Compute world to local matrix
osg::Matrix getWorldToLocalMatrix( osg::Node & node );

// Compute local to world matrix
osg::Matrix getLocalToWorldMatrix( osg::Node & node );
} // namespace osg

// CTools_H_included
#endif