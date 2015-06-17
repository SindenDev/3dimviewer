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

#ifndef CGetNodePathVisitor_H_included
#define CGetNodePathVisitor_H_included

#include <osg/NodeVisitor>
#include <osg/Node>

namespace scene
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//\class CGetNodePathVisitor
//
//\brief Get node path visitor. 
////////////////////////////////////////////////////////////////////////////////////////////////////

class CGetNodePathVisitor : public osg::NodeVisitor
{
public:
   //! Constructor
   CGetNodePathVisitor();

   //! Get world to local matrix
   osg::Matrix getWorldToLocal() { return m_wl; }

   //! Get local to world matrix
   osg::Matrix getLocalToWorld() { return m_lw; }

   //! Get eye to local matrix
   osg::Matrix getEyeToLocal() { return m_el; }

   //! Get local to eye matrix
   osg::Matrix getLocalToEye() { return m_le; }

   //! Override apply on any node - if it is last parent, compute matrices.
   virtual void apply (osg::Node &node);

   //! Override apply on camera - store model view matrix.
   virtual void apply( osg::Camera & node );
   

protected:
   //! Stored model-view matrix
   osg::Matrix m_modelView;

   //! Other matrices
   osg::Matrix m_wl, m_el, m_lw, m_le;

   //! Computation is done
   bool m_done;

}; // class CGetNodePathVisitor

} // namespace scene


// CGetNodePathVisitor_H_included
#endif

