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

#include "CRegion3DPreviewVisualizer.h"

#include <osg/CullFace>
#include <osg/PolygonMode>
#include <osg/CPseudoMaterial.h>
#include <graph/osg/NodeMasks.h>

osg::CRegion3DPreviewVisualizer::CRegion3DPreviewVisualizer()
{
    setName("CRegion3DPreviewVisualizer");

    m_pTransform = new osg::Geode();
    m_trianglesGeometry = new osg::Geometry;

    m_geomVertices = new osg::Vec3Array;
    m_geomNormals = new osg::Vec3Array;
    m_geomIndices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
    m_geomVertexColors = new osg::Vec4Array(1);

    (*m_geomVertexColors)[0] = osg::Vec4(0.0, 0.0, 0.0, 0.0);

    m_trianglesGeometry->setVertexArray(m_geomVertices);
    m_trianglesGeometry->setNormalArray(m_geomNormals, osg::Array::BIND_PER_VERTEX);
    m_trianglesGeometry->addPrimitiveSet(m_geomIndices);
    m_trianglesGeometry->setColorArray(m_geomVertexColors, osg::Array::BIND_OVERALL);

    m_pTransform->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    m_pTransform->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::OFF);

    m_pTransform->addDrawable(m_trianglesGeometry);
    addChild(m_pTransform);

    m_materialRegular = new osg::CPseudoMaterial();
    m_materialRegular->uniform("Shininess")->set(20.0f);
    m_materialRegular->uniform("Specularity")->set(0.5f);
    m_materialRegular->applySingleLightSetup();

    m_materialRegular->apply(this);

    setVisibility(false);

    setNodeMask(MASK_REGION_PREVIEW);
}

osg::CRegion3DPreviewVisualizer::~CRegion3DPreviewVisualizer()
{
}

void osg::CRegion3DPreviewVisualizer::setVisibility(bool visible)
{
    setOnOffState(visible);
}

void osg::CRegion3DPreviewVisualizer::update(const osg::Vec4& color)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_geomVertices->clear();
    m_geomIndices->clear();
    m_geomNormals->clear();

    std::vector<int> normalsCnt;

    size_t verticesCnt = m_vertices.size();
    size_t indiciesCnt = m_indicies.size();

    for (size_t i = 0; i < verticesCnt; ++i)
    {
        m_geomVertices->push_back(geometry::convert3<osg::Vec3, geometry::Vec3>(m_vertices.at(i)));
        m_geomNormals->push_back(osg::Vec3f(0.0, 0.0, 0.0));
        normalsCnt.push_back(0);
    }

    for (size_t i = 0; i < indiciesCnt; i += 3)
    {
        int index1 = m_indicies[i];
        int index2 = m_indicies[i + 1];
        int index3 = m_indicies[i + 2];

        m_geomIndices->push_back(index1);
        m_geomIndices->push_back(index2);
        m_geomIndices->push_back(index3);

        osg::Vec3 p1 = m_geomVertices->at(index1);
        osg::Vec3 p2 = m_geomVertices->at(index2);
        osg::Vec3 p3 = m_geomVertices->at(index3);

        osg::Vec3 normal = (p2 - p1) ^ (p3 - p1);
        normal.normalize();

        osg::Vec3 n1 = m_geomNormals->at(index1);
        osg::Vec3 n2 = m_geomNormals->at(index2);
        osg::Vec3 n3 = m_geomNormals->at(index3);

        m_geomNormals->at(index1) = (normal + n1);
        m_geomNormals->at(index2) = (normal + n2);
        m_geomNormals->at(index3) = (normal + n3);

        normalsCnt[index1]++;
        normalsCnt[index2]++;
        normalsCnt[index3]++;
    }

    for (size_t i = 0; i < verticesCnt; ++i)
    {
        assert(normalsCnt[i] > 0);
        m_geomNormals->at(i) /= normalsCnt[i];
    }

    (*m_geomVertexColors)[0] = color;

    m_geomVertices->dirty();
    m_geomIndices->dirty();
    m_geomNormals->dirty();
    m_geomVertexColors->dirty();

    m_trianglesGeometry->dirtyGLObjects();
    m_trianglesGeometry->dirtyBound();
}

void osg::CRegion3DPreviewVisualizer::setData(const std::vector<geometry::Vec3>& vertices, const std::vector<int>& indicies)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_vertices.clear();
    m_vertices.resize(0);
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        m_vertices.push_back(vertices.at(i));
    }

    m_indicies.clear();
    m_indicies.resize(0);
    for (size_t i = 0; i < indicies.size(); ++i)
    {
        m_indicies.push_back(indicies.at(i));
    }
}
