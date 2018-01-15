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

#ifndef CBoundingBoxVisitor_H
#define CBoundingBoxVisitor_H

#include <osg/ComputeBoundsVisitor>


namespace osg
{

///////////////////////////////////////////////////////////////////////////////
//! Visitor calculates bounding box of the node.
//
class  CBoundingBoxVisitor : public osg::ComputeBoundsVisitor
{
public:
	//! Constructor
	CBoundingBoxVisitor();
       
	//! Destructor
	virtual ~CBoundingBoxVisitor();

    //! Apply for cameras
    virtual void apply(osg::Camera &camera) override;

	//! Set matrix used when computing bounding box.
	void setInitialMatrix(const osg::Matrix & m)
	{
        // From unknown reasons takes pushMatrix non const matrix only.
        osg::Matrix _m(m);
		pushMatrix(_m);
	}

protected:
    // The overall resultant bounding box.
	osg::BoundingBox m_boundingBox;

    // Skip camera nodes? Camera node probably means subtree with totally different scene settings.
    bool m_bSkipCameraNode;
};


} // namespace osg

#endif // CBoundingBoxVisitor_H

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
