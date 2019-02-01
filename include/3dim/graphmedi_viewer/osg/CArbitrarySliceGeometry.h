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

#ifndef CARBITRARYSLICEGEOMETRY_H_INCLUDED
#define CARBITRARYSLICEGEOMETRY_H_INCLUDED

#include <data/CArbitrarySlice.h>
#include <graph/osg/CThickLineMaterial.h>
#include <osg/MatrixTransform>

namespace osg
{
    class CArbitrarySliceGeometry : public osg::MatrixTransform
    {
    public:
        CArbitrarySliceGeometry();

        //! Destructor
        ~CArbitrarySliceGeometry();

        //! Scales the slice to specified dimensions
        void scale(double width, double height);

        //! Returns frame geode
        osg::Geode* getFrameGeode();

        //! Returns slice geode
        osg::Geode* getSliceGeode();

        //! Set frame geometry color
        void setFrameColor(float r, float g, float b, float a);

        //! Set frame geometry color as osg vector
        void setFrameColor(const osg::Vec4 & color);

        void update(data::CArbitrarySlice& slice);

        void setLineMaterial(osg::CMaterialLineStrip* material);

    protected:
        //! Pointer to the slice geometry
        osg::ref_ptr< osg::Geometry > p_SliceGeometry;

        //! Pointer to the slice geode
        osg::ref_ptr< osg::Geode > p_SliceGeode;

        //! Pointer to the slice state set
        osg::ref_ptr< osg::StateSet > p_SliceState;

        //! Pointer to the geometry of the frame ( outline )
        osg::ref_ptr< osg::Geometry > p_FrameGeometry;

        //! Pointer to the frame geode
        osg::ref_ptr< osg::Geode > p_FrameGeode;

        //! Pointer to the frame state set
        osg::ref_ptr< osg::StateSet > p_FrameState;
    };
}

#endif // CARBITRARYSLICEGEOMETRY_H_INCLUDED

