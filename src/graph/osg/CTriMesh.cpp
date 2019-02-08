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

#include "osg/CTriMesh.h"

#include <osg/KdTree>
#include <osg/CullFace>

#include "osg/CConvertToGeometry.h"
#include <osg/CPseudoMaterial.h>
#include <geometry/base/CMesh.h>


osg::CTriMesh::CTriMesh()
    : m_kdtreeUsed(false)
    , m_useVertexColors(false)
    , m_useMultipleMaterials(false)
{
    setName("CTriMesh");

    m_geode = new osg::Geode();
    m_vertices = new osg::Vec3Array();
    m_texCoords = new osg::Vec2Array();
    m_normals = new Vec3Array();
    m_noNormals = new Vec3Array(1);
    m_colors = new osg::Vec4Array(1);
    m_vertexColors = new osg::Vec4Array();
    m_vertexGroupIndices = new osg::Vec4Array();
    m_vertexGroupWeights = new osg::Vec4Array();

    (*m_noNormals)[0] = osg::Vec3(0.0, 0.0, 0.0);
    (*m_colors)[0] = osg::Vec4(1.0, 1.0, 1.0, 1.0);

    m_defaultMaterial = m_defaultMaterialBackup = new osg::CPseudoMaterial();
    m_defaultMaterial->uniform("Shininess")->set(85.0f);
    m_defaultMaterial->uniform("Specularity")->set(1.0f);
    m_materials[0] = m_defaultMaterial;

    addChild(m_geode);
}

void osg::CTriMesh::createMesh(geometry::CMesh& mesh, const std::map<std::string, vpl::img::CRGBAImage::tSmartPtr>& textures, bool createNormals)
{
    // KD tree is not used
    m_kdtreeUsed = false;

    mesh.garbage_collection();
    mesh.update_face_normals();
    mesh.update_vertex_normals();

    auto numvert = mesh.n_vertices();
    auto numtris = mesh.n_faces();

    std::set<int> usedMaterials;

    OpenMesh::FPropHandleT<int> fPropHandle_material;
    bool hasMaterialProperty = mesh.get_property_handle(fPropHandle_material, MATERIAL_PROPERTY_NAME);

    // Analyze material property
    if (!hasMaterialProperty)
    {
        mesh.add_property(fPropHandle_material, MATERIAL_PROPERTY_NAME);

        for (geometry::CMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
        {
            mesh.property(fPropHandle_material, fit.handle()) = 0;
        }

        usedMaterials.insert(0);
    }
    else
    {
        for (geometry::CMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
        {
            int materialIndex = mesh.property(fPropHandle_material, fit.handle());

            usedMaterials.insert(materialIndex);
        }
    }

    auto materials = m_materials;
    m_materials.clear();

    // Resolve materials
    for (auto matIdx : usedMaterials)
    {
        if (materials.find(matIdx) == materials.end())
        {
            m_materials[matIdx] = m_defaultMaterial;
        }
        else
        {
            m_materials[matIdx] = materials[matIdx];
        }
    }

    m_geometries.clear();
    m_primitiveSets.clear();

    m_geode->removeDrawables(0, m_geode->getNumDrawables());

    std::map<int, long> indices;
    std::map<int, long> primitiveSetsSizes;

    for (auto matIdx : usedMaterials)
    {
        indices[matIdx] = 0;
        primitiveSetsSizes[matIdx] = 0;

        m_primitiveSets[matIdx] = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
        m_geometries[matIdx] = new osg::Geometry();

        m_geometries[matIdx]->addPrimitiveSet(m_primitiveSets[matIdx]);
        m_geometries[matIdx]->setVertexArray(m_vertices);
        m_geometries[matIdx]->setColorArray(m_colors, osg::Array::BIND_OVERALL);

        m_geode->addDrawable(m_geometries[matIdx]);

        m_geometries[matIdx]->setCullingActive(false);
    }

    // Create normals
    m_normals->resize(numvert);
    m_vertexColors->resize(numvert);

    useNormals(createNormals ? ENU_VERTEX : ENU_NONE);
    useVertexColors(m_useVertexColors, true);

    // Allocate memory for vertices
    m_vertices->resize(numvert);
    m_texCoords->resize(numvert);

    // Copy vertices and add bufferIndex property to each vertex of mesh for easy face-vertex indexing later
    OpenMesh::VPropHandleT<int> vProp_bufferIndex;

    // Test if property exist and if not, add it
    if (!mesh.get_property_handle(vProp_bufferIndex, BUFFER_INDEX_PROPERTY))
    {
        mesh.add_property(vProp_bufferIndex, BUFFER_INDEX_PROPERTY);
    }

    long index = 0;

    for (geometry::CMesh::VertexIter vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit, ++index)
    {
        mesh.property(vProp_bufferIndex, vit) = index;

        (*m_vertices)[index] = osg::Vec3(mesh.point(vit)[0], mesh.point(vit)[1], mesh.point(vit)[2]);
        (*m_normals)[index] = osg::Vec3(mesh.normal(vit)[0], mesh.normal(vit)[1], mesh.normal(vit)[2]);
        (*m_vertexColors)[index] = osg::Vec4(mesh.color(vit)[0] / 255.0, mesh.color(vit)[1] / 255.0, mesh.color(vit)[2] / 255.0, 1.0);
        (*m_texCoords)[index] = osg::Vec2(mesh.texcoord2D(vit)[0], mesh.texcoord2D(vit)[1]);
    }
    m_vertices->dirty();
    m_normals->dirty();
    m_vertexColors->dirty();
    m_texCoords->dirty();

    // Calculate sizes of primitive sets
    for (geometry::CMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        int materialIndex = mesh.property(fPropHandle_material, fit.handle());

        primitiveSetsSizes[materialIndex]++;
    }

    // Allocate memory for indexing
    for (auto matIdx : usedMaterials)
    {
        m_primitiveSets[matIdx]->resize(3 * primitiveSetsSizes[matIdx]);
    }

    // Copy triangle vertex indexing
    for (geometry::CMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        int materialIndex = mesh.property(fPropHandle_material, fit.handle());

        for (geometry::CMesh::FaceVertexIter fvit = mesh.fv_begin(fit); fvit != mesh.fv_end(fit); ++fvit)
        {
            (*m_primitiveSets[materialIndex])[indices[materialIndex]++] = mesh.property(vProp_bufferIndex, fvit);
        }
    }

    OpenMesh::VPropHandleT<geometry::CMesh::CVertexGroups> vProp_vertexGroups;
    if (mesh.get_property_handle(vProp_vertexGroups, VERTEX_GROUPS_PROPERTY_NAME))
    {
        m_vertexGroupIndices->resize(numvert);
        m_vertexGroupWeights->resize(numvert);

        index = 0;
        for (geometry::CMesh::VertexIter vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit, ++index)
        {
            geometry::CMesh::CVertexGroups vertexGroup = mesh.property(vProp_vertexGroups, vit);

            for (int i = 0; i < 4; ++i)
            {
                vertexGroup.indices[i] = std::max(vertexGroup.indices[i], 0);
            }

            (*m_vertexGroupIndices)[index] = osg::Vec4(vertexGroup.indices[0], vertexGroup.indices[1], vertexGroup.indices[2], vertexGroup.indices[3]);
            (*m_vertexGroupWeights)[index] = osg::Vec4(vertexGroup.weights[0], vertexGroup.weights[1], vertexGroup.weights[2], vertexGroup.weights[3]);
        }

        for (auto geometry : m_geometries)
        {
            geometry.second->setVertexAttribArray(6, m_vertexGroupIndices, osg::Array::BIND_PER_VERTEX);
            geometry.second->setVertexAttribArray(7, m_vertexGroupWeights, osg::Array::BIND_PER_VERTEX);
        }

        m_vertexGroupIndices->setNormalize(false);
        m_vertexGroupWeights->setNormalize(false);

        m_vertexGroupWeights->dirty();
        m_vertexGroupIndices->dirty();
    }

    for (auto geometry : m_geometries)
    {
        geometry.second->setTexCoordArray(0, m_texCoords, osg::Array::BIND_PER_VERTEX);
        geometry.second->setTexCoordArray(1, m_texCoords, osg::Array::BIND_PER_VERTEX);
    }

    // clean up
    if (!hasMaterialProperty)
    {
        mesh.remove_property(fPropHandle_material);
    }

    m_textureMap.clear();

    // apply new textures
    for (auto& texture : textures)
    {
        m_textureMap[texture.first] = CConvertToGeometry::convert(texture.second);
    }

    // apply materials
    applyMaterials();
}

void osg::CTriMesh::useVertexColors(bool value, bool force)
{
    if (force || value != m_useVertexColors)
    {
        m_useVertexColors = value;

        for (auto geometry : m_geometries)
        {
            if (m_useVertexColors)
            {
                geometry.second->setColorArray(m_vertexColors, osg::Array::BIND_PER_VERTEX);
            }
            else
            {
                geometry.second->setColorArray(m_colors, osg::Array::BIND_OVERALL);
            }

            geometry.second->dirtyGLObjects();
        }
    }
}

void osg::CTriMesh::updateVertexColors(const geometry::CMesh& mesh, float alpha)
{
    long index = 0;

    for (geometry::CMesh::ConstVertexIter vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit)
    {
        const auto& color = mesh.color(vit);

        (*m_vertexColors)[index++] = osg::Vec4(color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, alpha);
    }

    m_vertexColors->dirty();

    dirtyGeometry();
}

void osg::CTriMesh::dirtyGeometry()
{
    for (auto geometry : m_geometries)
    {
        geometry.second->dirtyGLObjects();
    }
}

void osg::CTriMesh::setColor(const osg::Vec4& value)
{
    (*m_colors)[0] = value;

    m_colors->dirty();

    dirtyGeometry();
}

void osg::CTriMesh::useNormals(ENormalsUsage normalsUsage)
{
    for (auto geometry : m_geometries)
    {
        switch (normalsUsage)
        {
            case ENU_VERTEX:
                geometry.second->setNormalArray(m_normals, osg::Array::BIND_PER_VERTEX);
                m_normals->dirty();
                break;

            case ENU_NONE:
            default:
                geometry.second->setNormalArray(m_noNormals, osg::Array::BIND_OVERALL);
                m_noNormals->dirty();
                break;
        }

        geometry.second->dirtyGLObjects();
    }
}

void osg::CTriMesh::updatePartOfMesh(const std::vector<std::pair<long, SPositionNormal>>& ip)
{
    // If no vertices, do nothing...
    if (ip.size() == 0)
        return;

    m_kdtreeUsed = false;

    // Get vertex array size
    long vsize(m_vertices->size());

    // For all given points
    for (auto ith : ip)
    {
        assert(ith.first < vsize);

        // Move point
        (*m_vertices)[ith.first] = ith.second.position;
        (*m_normals)[ith.first] = ith.second.normal;
    }

    m_normals->dirty();
    m_vertices->dirty();

    for (auto geometry : m_geometries)
    {
        geometry.second->dirtyGLObjects();
        geometry.second->dirtyBound();
    }
}

//! Sets use of multiple materials at once
void osg::CTriMesh::setUseMultipleMaterials(bool value)
{
    m_useMultipleMaterials = value;

    applyMaterials();
}

void osg::CTriMesh::applyMaterials()
{
    if (m_useMultipleMaterials)
    {
        for (auto geometry : m_geometries)
        {
            m_materials[geometry.first]->setTextures(m_textureMap);
            m_materials[geometry.first]->apply(geometry.second, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        }
    }
    else
    {
        for (auto geometry : m_geometries)
        {
            m_defaultMaterial->setTextures(m_textureMap);
            m_defaultMaterial->apply(geometry.second, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        }
    }
}

//! Sets and applies material with specified id, -1 is default material
void osg::CTriMesh::setMaterial(osg::CPseudoMaterial* material, int id)
{
    if (id == -1)
    {
        if (material == nullptr)
        {
            material = m_defaultMaterialBackup;
        }

        for (auto &mat : m_materials)
        {
            if (mat.second == m_defaultMaterial)
            {
                mat.second = material;
            }
        }

        material->copyInternals(m_defaultMaterial);
        m_defaultMaterial = material;
    }
    else
    {
        if (material == nullptr)
        {
            material = m_defaultMaterial;
        }

        if (m_materials[id] != nullptr)
        {
            material->copyInternals(m_materials[id]);
        }

        m_materials[id] = material;
    }

    applyMaterials();
}

//! Returns material with specified id, -1 is default material
osg::CPseudoMaterial *osg::CTriMesh::getMaterial(int id)
{
    if (id == -1)
    {
        return m_defaultMaterial;
    }
    else
    {
        auto found = m_materials.find(id);

        return (found == m_materials.end() ? nullptr : found->second);
    }
}

const std::map<int, osg::ref_ptr<osg::CPseudoMaterial>>& osg::CTriMesh::getMaterials() const
{
    return m_materials;
}

void osg::CTriMesh::buildKDTree()
{
    if (m_kdtreeUsed)
    {
        return;
    }

    for (auto geometry : m_geometries)
    {
        auto currentGeometry = geometry.second;

        // Update the KDTree
        osg::KdTree::BuildOptions kdTreeBuildOptions;
        osg::ref_ptr<osg::KdTree> kdTree = new osg::KdTree();

        if (kdTree->build(kdTreeBuildOptions, currentGeometry))
        {
            currentGeometry->setShape(kdTree);
            m_kdtreeUsed = true;
        }
    }
}

osg::Geode * osg::CTriMesh::getMeshGeode()
{
    return m_geode;
}
