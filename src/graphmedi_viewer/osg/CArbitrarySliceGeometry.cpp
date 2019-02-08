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

#include <osg/CArbitrarySliceGeometry.h>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/NodeMasks.h>
#include <numeric>

osg::CArbitrarySliceGeometry::CArbitrarySliceGeometry()
    : osg::MatrixTransform()
    , p_SliceGeometry(new osg::Geometry)
    , p_SliceGeode(new osg::Geode)
    , p_SliceState(new osg::StateSet)
    , p_FrameGeometry(new osg::Geometry)
    , p_FrameGeode(new osg::Geode)
    , p_FrameState(new osg::StateSet)
{
    p_FrameGeode->setName("Frame Geode");

    // get the slice dimensions
    data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
    double	width = slice->getWidth();
    double	height = slice->getHeight();
    slice.release();

    // prepare shared vertices for the slice and the frame
    osg::Vec3Array	* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = osg::Vec3(-width / 2.0, -height / 2.0, 0.0);
    (*vertices)[1] = osg::Vec3(width / 2.0, -height / 2.0, 0.0);
    (*vertices)[2] = osg::Vec3(width / 2.0, height / 2.0, 0.0);
    (*vertices)[3] = osg::Vec3(-width / 2.0, height / 2.0, 0.0);

    p_SliceGeometry->setVertexArray(vertices);
    p_SliceGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, 4));

    if (1)
    {
        // create primitive set
        osg::DrawElementsUInt* frame_primitive_set = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_STRIP_ADJACENCY, 4);

        std::iota(frame_primitive_set->begin(), frame_primitive_set->end(), 0);

        // Insert auxiliary adjacent vertices
        frame_primitive_set->insert(frame_primitive_set->begin(), 3);
        frame_primitive_set->insert(frame_primitive_set->end(), 0);
        frame_primitive_set->insert(frame_primitive_set->end(), 1);

        p_FrameGeometry->setVertexArray(vertices);
        p_FrameGeometry->addPrimitiveSet(frame_primitive_set);

        // add polygon offset to the slice so that the outline of the slice is visible
        //osg::PolygonOffset * offset = new osg::PolygonOffset(-0.1f, -0.1f);
        //p_FrameState->setAttributeAndModes(offset, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::Vec4Array * frame_color = new osg::Vec4Array(1);
        (*frame_color)[0] = osg::Vec4(1.0, 1.0, 0.0, 1.0);

        p_FrameGeometry->setColorArray(frame_color, osg::Array::BIND_OVERALL);

        // finish up the frame geode
        p_FrameGeode->addDrawable(p_FrameGeometry.get());
        p_FrameGeode->setStateSet(p_FrameState.get());
    }

    // set initial slice color to black
    osg::Vec4Array * color = new osg::Vec4Array(1);
    (*color)[0] = osg::Vec4(0.0, 0.0, 0.0, 1.0);

    p_SliceGeometry->setColorArray(color, osg::Array::BIND_OVERALL);

    // prepare texture coordinates for the slice
    osg::Vec2Array * tex_coords = new osg::Vec2Array(4);
    (*tex_coords)[0] = osg::Vec2(0.0, 0.0);
    (*tex_coords)[1] = osg::Vec2(1.0, 0.0);
    (*tex_coords)[2] = osg::Vec2(1.0, 1.0);
    (*tex_coords)[3] = osg::Vec2(0.0, 1.0);
    p_SliceGeometry->setTexCoordArray(0, tex_coords);
    p_SliceGeode->addDrawable(p_SliceGeometry.get());

    p_SliceState = new osg::StateSet();
    p_SliceGeometry->setStateSet(p_SliceState.get());

    this->addChild(p_SliceGeode.get());
    this->addChild(p_FrameGeode.get());

    setNodeMask(~MASK_ORTHO_2D_DRAGGER);
}

//=====================================================================================================================
osg::CArbitrarySliceGeometry::~CArbitrarySliceGeometry()
{
}

osg::Geode *osg::CArbitrarySliceGeometry::getFrameGeode()
{
    return p_FrameGeode.get();
}

osg::Geode * osg::CArbitrarySliceGeometry::getSliceGeode()
{
    return p_SliceGeode.get();
}

void osg::CArbitrarySliceGeometry::setFrameColor(const osg::Vec4 & color)
{
    setFrameColor(color.r(), color.g(), color.b(), color.a());
}

//====================================================================================================================
void osg::CArbitrarySliceGeometry::setFrameColor(float r, float g, float b, float a)
{
    if (!p_FrameGeometry.get()) return;

    osg::Vec4Array * color = dynamic_cast<osg::Vec4Array*>(p_FrameGeometry->getColorArray());

    if (color == 0)
        return;

    (*color)[0][0] = r;
    (*color)[0][1] = g;
    (*color)[0][2] = b;
    (*color)[0][3] = a;

    color->dirty();

    p_FrameGeometry->dirtyGLObjects();
}

//=====================================================================================================================
void osg::CArbitrarySliceGeometry::scale(double width, double height)
{
    // set new values for slice and frame vertices
    osg::Vec3Array * vertices = static_cast<osg::Vec3Array*>(p_SliceGeometry->getVertexArray());
    (*vertices)[0] = osg::Vec3(-width / 2.0, -height / 2.0, 0.0);
    (*vertices)[1] = osg::Vec3(width / 2.0, -height / 2.0, 0.0);
    (*vertices)[2] = osg::Vec3(width / 2.0, height / 2.0, 0.0);
    (*vertices)[3] = osg::Vec3(-width / 2.0, height / 2.0, 0.0);

    vertices->dirty();

    // dirty everything
    p_SliceGeometry->dirtyGLObjects();
    p_SliceGeometry->dirtyBound();
    p_FrameGeometry->dirtyGLObjects();
    p_FrameGeometry->dirtyBound();
}


void osg::CArbitrarySliceGeometry::update(data::CArbitrarySlice& slice)
{
    // create new texture
    // Resize slice,
    osg::Vec3 voxelSize = slice.getSliceVoxelSize();
    scale(slice.getWidth() * voxelSize[0], slice.getHeight() * voxelSize[1]);

    // set the colour of the slice to white, so that the texture can be actually seen
    osg::Vec4Array * color = new osg::Vec4Array(1);
    (*color)[0] = osg::Vec4(1.0, 1.0, 1.0, 1.0);

    p_SliceGeometry->setColorArray(color, osg::Array::BIND_OVERALL);
    p_SliceGeometry->dirtyGLObjects();

    // get texture dimensions
    double w = slice.getTextureWidth();
    double h = slice.getTextureHeight();

    // set up new texture coordinates for the
    osg::Vec2Array * tex = static_cast<osg::Vec2Array*>(p_SliceGeometry->getTexCoordArray(0));

    (*tex)[0] = osg::Vec2(w, 0.0);
    (*tex)[1] = osg::Vec2(0.0, 0.0);
    (*tex)[2] = osg::Vec2(0.0, h);
    (*tex)[3] = osg::Vec2(w, h);

    tex->dirty();

    p_SliceState->setTextureAttributeAndModes(0, slice.getTexturePtr(), osg::StateAttribute::ON);
}

void osg::CArbitrarySliceGeometry::setLineMaterial(osg::CMaterialLineStrip* material)
{
    material->apply(p_FrameGeometry);
}
