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

#ifndef CBoundingBoxVisitor_H
#define CBoundingBoxVisitor_H

#include <osg/NodeVisitor>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>
#include <osg/MatrixTransform>
#include <osg/Billboard>


namespace osg
{

///////////////////////////////////////////////////////////////////////////////
//! Visitor calculates bounding box of the node.
//
class  CBoundingBoxVisitor : public osg::NodeVisitor
{
public:
	//! Constructor
	CBoundingBoxVisitor();
       
	//! Destructor
	virtual ~CBoundingBoxVisitor();


	//! Apply for geodes - compute bb.
	virtual void apply(osg::Geode & geode);

	//! Apply for matrix transform nodes - compute transform;
	virtual void apply(osg::MatrixTransform & node);
        
	//! Apply for billboards.
	virtual void apply(osg::Billboard & node); 

	virtual void reset();

	//! Get computed bounding box
	osg::BoundingBox & getBoundBox();

	//! Set matrix used when computing bounding box.
	void setInitialMatrix(const osg::Matrix & m)
	{
		m_transformMatrix = m;
	}

protected:
    // The overall resultant bounding box.
	osg::BoundingBox m_boundingBox;

    // The current transform matrix.
	osg::Matrix m_transformMatrix;
};


} // namespace osg

#endif // CBoundingBoxVisitor_H

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
