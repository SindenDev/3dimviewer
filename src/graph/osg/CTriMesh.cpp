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

#include <osgUtil/SmoothingVisitor>
#include <osg/KdTree>
#include <osg/CullFace>
#include <osg/Version>
#include <osg/CForceCullCallback.h>

#define BUFFER_INDEX_PROPERTY "bufferIndex"

void osg::CTriMeshDrawCallback::drawImplementation(osg::RenderInfo &ri, const osg::Drawable *d) const
{
    osg::Geometry *g = m_drawable->asGeometry();
    if (g != NULL)
    {
        if ((g->getVertexArray()->getDataSize() != 0) && (g->getNumPrimitiveSets() != 0) && (g->getPrimitiveSet(0)->getTotalDataSize() != 0))
        {
            // draw original geometry
            m_drawable->drawImplementation(ri);
        }
    }
}

osg::CTriMesh::CTriMesh()
    : m_bKDTreeUsed(false)
    , m_bUseVertexColors(false)
{
    setName("CTriMesh");
    pGeode = new osg::Geode;
    pGeometries.push_back(new osg::Geometry);
    pGeometriesVisitors.push_back(new osg::Geometry);
    pVertices = new osg::Vec3Array;
    pVertexColors = new osg::Vec4Array;
    pPrimitiveSets.push_back(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES));
    pVertexGroupIndices = new osg::Vec4Array;
    pVertexGroupWeights = new osg::Vec4Array;
    pVisitorsSubTree = new osg::MatrixTransform;
    setVisitorsSubTree(NULL);

    pGeometriesVisitors.back()->setCullingActive(false);  

    // set draw callback
    pGeometriesVisitors.back()->setDrawCallback(new CTriMeshDrawCallback(pGeometries.back()));

    this->addChild(pVisitorsSubTree);
    this->addChild(pGeode);

    pGeometries.back()->setVertexArray(pVertices);
    pGeometries.back()->addPrimitiveSet(pPrimitiveSets.back());

    pDefaultMaterial = pDefaultMaterialBackup = new osg::CPseudoMaterial;
    pDefaultMaterial->uniform("Shininess")->set(85.0f);
    pDefaultMaterial->uniform("Specularity")->set(1.0f);
    pMaterials[0] = pDefaultMaterial;
    pDefaultMaterial->apply(pGeometries.back());

    pColors = new osg::Vec4Array(1);
    (*pColors)[0] = osg::Vec4(1.0, 1.0, 1.0, 1.0);
    pGeometries.back()->setColorArray(pColors, osg::Array::BIND_OVERALL);
}

void osg::CTriMesh::setVisitorsSubTree(osg::MatrixTransform *visitorsSubTree)
{
    // set active node to geode - if visitorsSubTree is NULL, use original geometry, otherwise use visitors and set bones geometry to the visitors tree
    pVisitorsSubTree->removeChildren(0, pVisitorsSubTree->getNumChildren());
    if (visitorsSubTree != NULL)
    {
        pVisitorsSubTree->addChild(visitorsSubTree);
        pGeode->removeDrawables(0, pGeode->getNumDrawables());
        for (int i = 0; i < pGeometries.size(); ++i)
        {
            pGeode->addDrawable(pGeometriesVisitors[i]);
        }
    }
    else
    {
        pGeode->removeDrawables(0, pGeode->getNumDrawables());
        for (int i = 0; i < pGeometries.size(); ++i)
        {
            pGeode->addDrawable(pGeometries[i]);
        }
    }
}

void osg::CTriMesh::createMesh(geometry::CMesh *mesh, bool createNormals)
{
    if (!mesh)
    {
        return;
    }

    // KD tree is not used
    m_bKDTreeUsed = false;

    long numvert = mesh->n_vertices();
    long numtris = mesh->n_faces();

    // remove entities marked for deletion
    if (numvert + numtris > 0)
    {
        mesh->garbage_collection();
    }

    numvert = mesh->n_vertices();
    numtris = mesh->n_faces();

    // indexing counters
    long index = 0;

    // analyze material property
    std::map<int, int> materialIndexToGeometryIndex;
    int geometryIndex = 0;
    m_geometryIndexToMaterialIndex.clear();
    std::set<int> usedMaterials;
    OpenMesh::FPropHandleT<int> fPropHandle_material;
    bool tempProperty = false;
    if (!mesh->get_property_handle(fPropHandle_material, MATERIAL_PROPERTY_NAME))
    {
        usedMaterials.insert(0);
        tempProperty = true;
        m_geometryIndexToMaterialIndex[0] = 0;
        materialIndexToGeometryIndex[0] = 0;
        mesh->add_property(fPropHandle_material, MATERIAL_PROPERTY_NAME);

        for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
        {
            mesh->property(fPropHandle_material, fit.handle()) = 0;
        }
    }
    else
    {
        tempProperty = false;
        for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
        {
            int materialIndex = mesh->property(fPropHandle_material, fit.handle());

            if (materialIndexToGeometryIndex.find(materialIndex) == materialIndexToGeometryIndex.end())
            {
                m_geometryIndexToMaterialIndex[geometryIndex] = materialIndex;
                materialIndexToGeometryIndex[materialIndex] = geometryIndex;
                geometryIndex++;
            }

            usedMaterials.insert(materialIndex);
        }
    }

    // resolve materials
    std::map<int, osg::ref_ptr<osg::CPseudoMaterial> > tmpMaterials = pMaterials;
    pMaterials.clear();
    for (std::map<int, osg::ref_ptr<osg::CPseudoMaterial> >::iterator it = tmpMaterials.begin(); it != tmpMaterials.end(); ++it)
    {
        if (it->second != NULL)
        {
            pMaterials[it->first] = it->second;
        }
    }
    for (std::set<int>::iterator it = usedMaterials.begin(); it != usedMaterials.end(); ++it)
    {
        if (pMaterials.find(*it) == pMaterials.end())
        {
            pMaterials[*it] = pDefaultMaterial;
        }
    }

    // remove excess geometries and add new
    while (pGeometries.size() > usedMaterials.size())
    {
        pGeode->removeDrawable(pGeometries.back());
        pGeometries.back()->removePrimitiveSet(0, pGeometries.back()->getNumPrimitiveSets());
        pGeometries.pop_back();
        pPrimitiveSets.pop_back();
    }
    while (pGeometries.size() < usedMaterials.size())
    {
        pPrimitiveSets.push_back(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES));
        pGeometries.push_back(new osg::Geometry);
        pGeometries.back()->addPrimitiveSet(pPrimitiveSets.back());
        pGeometries.back()->setVertexArray(pVertices.get());
        pGeometries.back()->setColorArray(pColors, osg::Array::BIND_OVERALL);
        pGeode->addDrawable(pGeometries.back());
    }

    // Create normals
    pNormals = new Vec3Array(std::max(1, (int)numvert));
    pVertexColors = new Vec4Array(std::max(1, (int)numvert));
    pNoNormals = new Vec3Array();
    pNoNormals->push_back(osg::Vec3(0.0, 0.0, 0.0));

    mesh->update_face_normals();
    mesh->update_vertex_normals();
    ENormalsUsage normalsUsage;
    if (createNormals)
    {
        normalsUsage = ENU_VERTEX;
    }
    else
    {
        normalsUsage = ENU_NONE;
    }
    useNormals(normalsUsage);

    // resolve colors
    useVertexColors(m_bUseVertexColors, true);

    // Allocate memory for vertices
    pVertices->resize(std::max(1, (int)numvert));

    // Copy vertices and add bufferIndex property to each vertex of mesh for easy face-vertex indexing later
    index = 0;
    OpenMesh::VPropHandleT<int> vProp_bufferIndex;

    // Test if property exist and if not, add it
    if (!mesh->get_property_handle(vProp_bufferIndex, BUFFER_INDEX_PROPERTY))
    {
        mesh->add_property(vProp_bufferIndex, BUFFER_INDEX_PROPERTY);
    }

    for (geometry::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
    {
        mesh->property(vProp_bufferIndex, vit) = index;
        (*pVertices)[index] = osg::Vec3(mesh->point(vit)[0], mesh->point(vit)[1], mesh->point(vit)[2]);
        (*pNormals)[index] = osg::Vec3(mesh->normal(vit)[0], mesh->normal(vit)[1], mesh->normal(vit)[2]);
        (*pVertexColors)[index] = osg::Vec4(mesh->color(vit)[0] / 255.0, mesh->color(vit)[1] / 255.0, mesh->color(vit)[2] / 255.0, 1.0);
        ++index;
    }

    // Calculate sizes of primitive sets
    std::vector<int> primitiveSetsSizes(usedMaterials.size(), 0);
    for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
    {
        int materialIndex = mesh->property(fPropHandle_material, fit.handle());
        primitiveSetsSizes[materialIndexToGeometryIndex[materialIndex]]++;
    }

    // Allocate memory for indexing
    for (int i = 0; i < usedMaterials.size(); ++i)
    {
        pPrimitiveSets[i]->resize(std::max(3, (int)primitiveSetsSizes[i] * 3));

        pPrimitiveSets[i]->dirty();
    }

    // Copy triangle vertex indexing
    std::vector<int> indices(usedMaterials.size(), 0);
    for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
    {
        int materialIndex = mesh->property(fPropHandle_material, fit.handle());

        for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
        {
            (*pPrimitiveSets[materialIndexToGeometryIndex[materialIndex]])[indices[materialIndexToGeometryIndex[materialIndex]]++] = mesh->property(vProp_bufferIndex, fvit);
        }
    }

    // Property is used in guide fabrication. This property is connection between visible osg model 
    // vertices (and faces) and openmesh model. Drawed guide limiting curve stores touched triangles numbers
    // and algortihm translates them to the (open)mesh triangle indexes. Do not remove commentary 
    // If the property must be removed for some reason, discuss it with me. Wik.
    //    mesh->remove_property(vProp_bufferIndex);

    OpenMesh::VPropHandleT<geometry::CMesh::CVertexGroups> vProp_vertexGroups;
    if (mesh->get_property_handle(vProp_vertexGroups, VERTEX_GROUPS_PROPERTY_NAME))
    {
        pVertexGroupIndices->resize(numvert);
        pVertexGroupWeights->resize(numvert);

        index = 0;
        for (geometry::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
        {
            geometry::CMesh::CVertexGroups vertexGroup = mesh->property(vProp_vertexGroups, vit);
            for (int i = 0; i < 4; ++i)
            {
                vertexGroup.indices[i] = std::max(vertexGroup.indices[i], 0);
            }
            (*pVertexGroupIndices)[index] = osg::Vec4(vertexGroup.indices[0], vertexGroup.indices[1], vertexGroup.indices[2], vertexGroup.indices[3]);
            (*pVertexGroupWeights)[index] = osg::Vec4(vertexGroup.weights[0], vertexGroup.weights[1], vertexGroup.weights[2], vertexGroup.weights[3]);
            ++index;
        }

        for (int i = 0; i < pGeometries.size(); ++i)
        {
            pVertexGroupIndices->setNormalize(false);
            pGeometries[i]->setVertexAttribArray(6, pVertexGroupIndices, osg::Array::BIND_PER_VERTEX);

            pVertexGroupWeights->setNormalize(false);
            pGeometries[i]->setVertexAttribArray(7, pVertexGroupWeights, osg::Array::BIND_PER_VERTEX);

            pVertexGroupWeights->dirty();
            pVertexGroupIndices->dirty();
        }
    }
    else
    {
        for (int i = 0; i < pGeometries.size(); ++i)
        {
            pGeometries[i]->setVertexAttribArray(6, NULL);
            pGeometries[i]->setVertexAttribArray(7, NULL);
        }
    }

    for (int i = 0; i < pGeometries.size(); ++i)
    {
        pGeometries[i]->dirtyGLObjects();
        pGeometries[i]->dirtyBound();
    }

    // clean up
    if (tempProperty)
    {
        mesh->remove_property(fPropHandle_material);
    }

    // apply materials
    applyMaterials();
}

void osg::CTriMesh::useVertexColors(bool value, bool force)
{
    if (force || value != m_bUseVertexColors)
    {
        m_bUseVertexColors = value;

        for (int i = 0; i < pGeometries.size(); ++i)
        {
            pGeometries[i]->setColorArray(m_bUseVertexColors ? pVertexColors : pColors, m_bUseVertexColors ? osg::Array::BIND_PER_VERTEX : osg::Array::BIND_OVERALL);
            pGeometries[i]->dirtyGLObjects();
        }
    }
}

void osg::CTriMesh::updateVertexColors(osg::Vec4Array *vertexColors)
{
    pVertexColors = vertexColors;
    useVertexColors(m_bUseVertexColors, true);
}

void osg::CTriMesh::setColor(float r, float g, float b, float a)
{
    auto& color = (*pColors)[0];
    const auto newColor = osg::Vec4(r, g, b, a);

    if (color != newColor)
    {
        color = newColor;
        pColors->dirty();

        for (int i = 0; i < pGeometries.size(); ++i)
        {
            pGeometries[i]->dirtyGLObjects();
        }
    }
}

void osg::CTriMesh::useNormals(ENormalsUsage normalsUsage)
{
    for (int i = 0; i < pGeometries.size(); ++i)
    {
        switch (normalsUsage)
        {
        case ENU_VERTEX:
            pGeometries[i]->setNormalArray(pNormals.get(), osg::Array::BIND_PER_VERTEX);
            break;

        case ENU_NONE:
        default:
            pGeometries[i]->setNormalArray(pNoNormals.get(), osg::Array::BIND_OVERALL);
            break;
        }

        pGeometries[i]->dirtyGLObjects();
    }
}

void osg::CTriMesh::updatePartOfMesh(geometry::CMesh *mesh, const tIdPosVec &ip, bool createNormals)
{
    // If no vertices, do nothing...
    if (ip.size() == 0)
        return;

    // Do not update kd-tree
    //if (m_bKDTreeUsed)
    //    return;

    /*
    // Normals settings
    ENormalsUsage normalsUsage;
    if (createNormals)
    {
        normalsUsage = ENU_VERTEX;
    }
    else
    {
        normalsUsage = ENU_NONE;
    }
    useNormals(normalsUsage);
    */

    // Get vertex array size
    long vsize(pVertices->size());

    // For all given points
    tIdPosVec::const_iterator ith(ip.begin()), ithEnd(ip.end());
    for (; ith != ithEnd; ++ith)
    {
        assert(ith->first < vsize);

        // Move point
        (*pVertices)[ith->first] = ith->second.position;
        (*pNormals)[ith->first] = ith->second.normal;
    }

    pNormals->dirty();
    pVertices->dirty();

    for (int i = 0; i < pGeometries.size(); ++i)
    {
        pGeometries[i]->dirtyGLObjects();
        pGeometries[i]->dirtyBound();
    }
}

//! Sets use of multiple materials at once
void osg::CTriMesh::setUseMultipleMaterials(bool value)
{
    bUseMultipleMaterials = value;

    applyMaterials();
}

void osg::CTriMesh::applyMaterials()
{
    if (bUseMultipleMaterials)
    {
        for (int i = 0; i < pGeometries.size(); ++i)
        {
            int materialIndex = m_geometryIndexToMaterialIndex[i];
            pMaterials[materialIndex]->apply(pGeometries[i], osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        }
    }
    else
    {
        for (int i = 0; i < pGeometries.size(); ++i)
        {
            pDefaultMaterial->apply(pGeometries[i], osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
        }
    }
}

//! Sets and applies material with specified id, -1 is default material
void osg::CTriMesh::setMaterial(osg::CPseudoMaterial *material, int id)
{
    if (id == -1)
    {
        if (material == NULL)
        {
            material = pDefaultMaterialBackup;
        }

        for (std::map<int, osg::ref_ptr<osg::CPseudoMaterial> >::iterator it = pMaterials.begin(); it != pMaterials.end(); ++it)
        {
            if (it->second == pDefaultMaterial)
            {
                it->second = material;
            }
        }

        material->copyInternals(pDefaultMaterial);
        pDefaultMaterial = material;
    }
    else
    {
        if (material == NULL)
        {
            material = pDefaultMaterial;
        }

        material->copyInternals(pMaterials[id]);
        pMaterials[id] = material;
    }

    applyMaterials();
}

//! Gets number of materials
int osg::CTriMesh::getNumMaterials()
{
    return pMaterials.size();
}

//! Returns material with specified id, -1 is default material
osg::CPseudoMaterial *osg::CTriMesh::getMaterialById(int id)
{
    if (id == -1)
    {
        return pDefaultMaterial;
    }
    else
    {
        std::map<int, osg::ref_ptr<osg::CPseudoMaterial> >::iterator found = pMaterials.find(id);
        return (found == pMaterials.end() ? NULL : found->second);
    }
}

//! Returns material with specified index, -1 is default material
osg::CPseudoMaterial *osg::CTriMesh::getMaterialByIndex(int index)
{
    if ((index == -1) || (index >= pMaterials.size()))
    {
        return pDefaultMaterial;
    }
    else
    {
        std::vector<int> indices;
        for (std::map<int, osg::ref_ptr<osg::CPseudoMaterial> >::iterator it = pMaterials.begin(); it != pMaterials.end(); ++it)
        {
            indices.push_back(it->first);
        }
        std::sort(indices.begin(), indices.end());

        int id = indices[index];
        return getMaterialById(id);
    }
}

void osg::CTriMesh::buildKDTree()
{
    if (m_bKDTreeUsed)
    {
        return;
    }

    for (int i = 0; i < pGeometries.size(); ++i)
    {
        // Update the KDTree
        osg::KdTree::BuildOptions kdTreeBuildOptions;
        osg::ref_ptr<osg::KdTree> kdTree = new osg::KdTree();

        if (kdTree->build(kdTreeBuildOptions, pGeometries[i]))
        {
            pGeometries[i]->setShape(kdTree.get());
            m_bKDTreeUsed = true;
        }
        else
        {
            //LOG_MSG(logERROR) << "osg::KdTree::build() unsuccessful.";
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Creates OSG geometry from loaded OpenMesh data structure

osg::Geometry *osg::convertOpenMesh2OSGGeometry(geometry::CMesh *mesh, bool vertexNormals)
{
    osg::Geometry *geometry = new osg::Geometry();

    if ((mesh == NULL) || (geometry == NULL))
    {
        return NULL;
    }

    // pre-process loaded mesh and get counts
    mesh->garbage_collection();
    if (!mesh->has_vertex_normals())
    {
        mesh->request_vertex_normals();
    }
    if (!mesh->has_face_normals())
    {
        mesh->request_face_normals();
    }
    mesh->update_normals();
    long numvert(mesh->n_vertices());
    long numtris(mesh->n_faces());

    // prepare osg::Geometry
    osg::Vec3Array *pNormals = new osg::Vec3Array(vertexNormals ? numvert : numtris * 3);
    osg::Vec3Array *pVertices = new osg::Vec3Array(vertexNormals ? numvert : numtris * 3);

    osg::DrawElementsUInt *pPrimitives = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, numtris * 3);
    osg::StateSet *pState = geometry->getOrCreateStateSet();
    geometry->setNormalArray(pNormals, osg::Array::BIND_PER_VERTEX);
    geometry->setVertexArray(pVertices);
    geometry->addPrimitiveSet(pPrimitives);

    // indexing counters
    long index = 0;
    long triindex = 0;

    // Copy vertices and add bufferIndex property to each vertex of mesh for easy face-vertex indexing later
    index = 0;
    OpenMesh::VPropHandleT<int> vProp_bufferIndex;
    OpenMesh::VPropHandleT<int> vProp_flag;
    mesh->add_property(vProp_bufferIndex, "bufferIndex");

    for (geometry::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
    {
        mesh->property(vProp_bufferIndex, vit) = index;
        (*pVertices)[index] = osg::Vec3(mesh->point(vit)[0], mesh->point(vit)[1], mesh->point(vit)[2]);
        ++index;
    }

    // Copy triangle vertex indexing
    if (vertexNormals)
    {
        index = 0;
        for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
        {
            for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
            {
                (*pPrimitives)[index++] = mesh->property(vProp_bufferIndex, fvit);
            }
        }
    }
    else
    {
        index = 0;
        int vindex = 0;
        for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
        {
            osg::Vec3 fn(mesh->normal(fit)[0], mesh->normal(fit)[1], mesh->normal(fit)[2]);

            for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
            {
                (*pVertices)[vindex] = osg::Vec3(mesh->point(fvit)[0], mesh->point(fvit)[1], mesh->point(fvit)[2]);
                (*pNormals)[vindex] = fn;
                (*pPrimitives)[index] = vindex;
                index++;
                vindex++;
            }
        }
    }

    // Copy normals
    if (vertexNormals)
    {
        index = 0;
        for (geometry::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
        {
            (*pNormals)[index] = osg::Vec3(mesh->normal(vit)[0], mesh->normal(vit)[1], mesh->normal(vit)[2]);
            ++index;
        }
    }

    // remove no longer needed bufferIndex property
    mesh->remove_property(vProp_bufferIndex);

    geometry->dirtyGLObjects();
    geometry->dirtyBound();

    return geometry;
}


osg::Geometry *osg::convertOpenMesh2OSGGeometry(geometry::CMesh *mesh, const osg::Vec4& color)
{
    if (NULL==mesh)
        return NULL;
    osg::Geometry *geometry = new osg::Geometry();

    if (NULL==geometry)
        return NULL;

    // pre-process loaded mesh and get counts
    mesh->garbage_collection();
    if (!mesh->has_vertex_normals())
    {
        mesh->request_vertex_normals();
    }
    if (!mesh->has_face_normals())
    {
        mesh->request_face_normals();
    }
    mesh->update_normals();

    long numvert(mesh->n_vertices());
    long numtris(mesh->n_faces());

    // prepare osg::Geometry
    osg::Vec3Array *pNormals = new osg::Vec3Array(numvert);
    osg::Vec3Array *pVertices = new osg::Vec3Array(numvert);

    osg::DrawElementsUInt *pPrimitives = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, numtris * 3);
    osg::StateSet *pStateSet = geometry->getOrCreateStateSet();

    // Enable depth test so that an opaque polygon will occlude a transparent one behind it.
    pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    // Culling
    pStateSet->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::OFF);

    // Color
    osg::Vec4Array *pColors = new osg::Vec4Array(1);
    (*pColors)[0] = color;

    geometry->setColorArray(pColors, osg::Array::BIND_OVERALL);

    // prepare osg::Geometry
    geometry->setNormalArray(pNormals, osg::Array::BIND_PER_VERTEX);
    geometry->setVertexArray(pVertices);
    geometry->addPrimitiveSet(pPrimitives);

    // Copy vertices and add bufferIndex property to each vertex of mesh for easy face-vertex indexing later
    OpenMesh::VPropHandleT<int> vProp_bufferIndex;
    OpenMesh::VPropHandleT<int> vProp_flag;

    // THIS PROPERTY TIES OSG AND OPENMESH TOGETHER. IT IS USED IN ANOTHER PARTS OF BSP. DO NOT REMOVE IT. NEVER!
    mesh->add_property(vProp_bufferIndex, "bufferIndex");

    long index = 0;
    for (geometry::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end() && index < numvert; ++vit)
    {
        mesh->property(vProp_bufferIndex, vit) = index;
        (*pVertices)[index] = osg::Vec3(mesh->point(vit)[0], mesh->point(vit)[1], mesh->point(vit)[2]);
        (*pNormals)[index] = osg::Vec3(mesh->normal(vit)[0], mesh->normal(vit)[1], mesh->normal(vit)[2]);
        ++index;
    }

    // Copy triangle vertex indexing
    index = 0;
    for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
    {
        for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
        {
            (*pPrimitives)[index++] = mesh->property(vProp_bufferIndex, fvit);
        }
    }

    geometry->dirtyGLObjects();
    geometry->dirtyBound();

    return geometry;
}
