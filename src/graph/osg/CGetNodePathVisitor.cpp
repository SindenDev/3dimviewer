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

#include "3dim/graph/osg/CGetNodePathVisitor.h"
#include <osg/Transform>
#include <osg/Camera>

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn scene:::::CGetNodePathVisitor(void)
//
//\brief ! Constructor. 
////////////////////////////////////////////////////////////////////////////////////////////////////

scene::CGetNodePathVisitor::CGetNodePathVisitor(void)
   : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_PARENTS )
   , m_done( false )
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void scene:::::apply(osg::Node &node)
//
//\brief ! Override apply on camera - store model view matrix. 
//
//\param [in,out] node  The node. 
////////////////////////////////////////////////////////////////////////////////////////////////////

void scene::CGetNodePathVisitor::apply(osg::Node &node)
{
	if (!m_done)
         {
            if ( 0 == node.getNumParents() ) // no parents
            {
               m_lw.set( osg::computeLocalToWorld(this->getNodePath()) );
               m_wl.set( osg::computeWorldToLocal( this->getNodePath() ) );
               m_el.set( osg::computeEyeToLocal( m_modelView, this->getNodePath() ) );
               m_le.set( osg::computeLocalToEye( m_modelView, this->getNodePath() ) );
               m_done = true;
            }
            traverse(node);
         }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn void scene:::::apply(osg::Camera &node)
//
//\brief ! Override apply on camera - store model view matrix. 
//
//\param [in,out] node  The node. 
////////////////////////////////////////////////////////////////////////////////////////////////////

void scene::CGetNodePathVisitor::apply(osg::Camera &node)
{
   m_modelView = node.getViewMatrix();	

 //  if (!m_done)
   {
      if ( 0 == node.getNumParents() ) // no parents
      {
         m_modelView.makeIdentity();
         m_lw.set( osg::computeLocalToWorld(this->getNodePath() ) );
         m_wl.set( osg::computeWorldToLocal( this->getNodePath() ) );
         m_el.set( osg::computeEyeToLocal( m_modelView, this->getNodePath(), false ) );
         m_le.set( osg::computeLocalToEye( m_modelView, this->getNodePath(), false ) );
         m_done = true;
      }
      traverse(node);
   }
   
}

