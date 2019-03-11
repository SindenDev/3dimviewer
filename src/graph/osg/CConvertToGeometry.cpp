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

#include "osg/CConvertToGeometry.h"

#include <geometry/base/CMesh.h>


osg::Geometry* osg::CConvertToGeometry::convert(geometry::CMesh& mesh, MeshConversionParams params)
{
    osg::Geometry* geometry = new osg::Geometry();

    if (!mesh.has_vertex_normals())
    {
        mesh.request_vertex_normals();
    }
    if (!mesh.has_face_normals())
    {
        mesh.request_face_normals();
    }

    mesh.update_normals();

    long numvert(mesh.n_vertices());
    long numtris(mesh.n_faces());

    // Copy vertices and add bufferIndex property to each vertex of mesh for easy face-vertex indexing later
    OpenMesh::VPropHandleT<int> vProp_bufferIndex;

    // THIS PROPERTY TIES OSG AND OPENMESH TOGETHER. IT IS USED IN ANOTHER PARTS OF BSP. DO NOT REMOVE IT. NEVER!
    if (!mesh.get_property_handle(vProp_bufferIndex, BUFFER_INDEX_PROPERTY))
    {
        mesh.add_property(vProp_bufferIndex, BUFFER_INDEX_PROPERTY);
    }

    if (params.sharedVertices)
    {
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(numvert);
        osg::ref_ptr<osg::DrawElementsUInt> primitives = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, numtris * 3);
        osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(params.useNormals ? numvert : 0);
        osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(params.useVertexColors ? numvert : 0);
        osg::ref_ptr<osg::Vec2Array> texCoords = new osg::Vec2Array(params.useTexCoords ? numvert : 0);

        long index = 0;
        for (auto vh : mesh.vertices())
        {
            mesh.property(vProp_bufferIndex, vh) = index;

            (*vertices)[index] = geometry::convert3<osg::Vec3>(mesh.point(vh));

            if (params.useNormals)
            {
                (*normals)[index] = geometry::convert3<osg::Vec3>(mesh.normal(vh));
            }

            if (params.useTexCoords)
            {
                (*texCoords)[index] = osg::Vec2(mesh.texcoord2D(vh)[0], mesh.texcoord2D(vh)[1]);
            }

            if (params.useVertexColors)
            {
                (*colors)[index] = osg::Vec4(mesh.color(vh)[0] / 255.0, mesh.color(vh)[1] / 255.0, mesh.color(vh)[2] / 255.0, 1.0);
            }

            index++;
        }

        index = 0;
        for (auto fh : mesh.faces())
        {
            for (auto vit = mesh.cfv_ccwbegin(fh); vit != mesh.cfv_ccwend(fh); ++vit)
            {
                (*primitives)[index++] = mesh.property(vProp_bufferIndex, *vit);
            }
        }

        geometry->setVertexArray(vertices);
        geometry->addPrimitiveSet(primitives);

        if (params.useNormals)
        {
            geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
        }

        if (params.useVertexColors)
        {
            geometry->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
        }

        if (params.useTexCoords)
        {
            geometry->setTexCoordArray(0, texCoords, osg::Array::BIND_PER_VERTEX);
            geometry->setTexCoordArray(1, texCoords, osg::Array::BIND_PER_VERTEX);
        }
    }
    else
    {
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(numtris * 3);
        osg::ref_ptr<osg::DrawArrays> primitives = new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, numtris * 3);
        osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(params.useNormals ? numtris * 3 : 0);

        long index = 0;
        for (auto vh : mesh.vertices())
        {
            mesh.property(vProp_bufferIndex, vh) = index++;
        }

        index = 0;
        for (auto fh : mesh.faces())
        {
            osg::Vec3 normal = geometry::convert3<osg::Vec3>(mesh.normal(fh));

            for (auto vit = mesh.cfv_ccwbegin(fh); vit != mesh.cfv_ccwend(fh); ++vit)
            {
                (*vertices)[index] = geometry::convert3<osg::Vec3>(mesh.point(*vit));

                if (params.useNormals)
                {
                    (*normals)[index] = normal;
                }

                index++;
            }
        }

        geometry->setVertexArray(vertices);
        geometry->addPrimitiveSet(primitives);

        if (params.useNormals)
        {
            geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
        }
    }

    if (params.useColor)
    {
        osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
        (*colors)[0] = params.color;

        geometry->setColorArray(colors, osg::Array::BIND_OVERALL);
    }

    // remove no longer needed bufferIndex property
    //mesh.remove_property(vProp_bufferIndex);

    return geometry;
}

osg::Texture2D* osg::CConvertToGeometry::convert(const vpl::img::CRGBAImage::tSmartPtr& image)
{
    osg::ref_ptr<osg::Image> osgImage = new osg::Image();
    osgImage->setImage(image->getXSize(), image->getYSize(), 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)image->getPtr(0, 0), osg::Image::NO_DELETE);

    osg::Texture2D *osgTexture = new osg::Texture2D();
    osgTexture->setImage(osgImage);
    osgTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    osgTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    osgTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    osgTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
    osgTexture->setUseHardwareMipMapGeneration(false);
    osgTexture->setMaxAnisotropy(1.0f);
    osgTexture->setResizeNonPowerOfTwoHint(false);

    return osgTexture;
}

osg::MeshConversionParams osg::MeshConversionParams::SimpleWhite()
{
    return MeshConversionParams();
}

osg::MeshConversionParams osg::MeshConversionParams::FlatShading()
{
    MeshConversionParams params;

    params.sharedVertices = false;

    return params;
}

osg::MeshConversionParams osg::MeshConversionParams::JustVertices()
{
    MeshConversionParams params;

    params.useNormals = false;
    params.useColor = false;

    return params;
}

osg::MeshConversionParams osg::MeshConversionParams::WithoutColor()
{
    MeshConversionParams params;

    params.useColor = false;

    return params;
}

osg::MeshConversionParams osg::MeshConversionParams::WithTexture()
{
    MeshConversionParams params;

    params.useTexCoords = true;

    return params;
}
