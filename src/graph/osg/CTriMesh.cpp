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
#include <osg/LightModel>
#include <osg/CullFace>
#include <osg/Version>

#define BUFFER_INDEX_PROPERTY "bufferIndex"

///////////////////////////////////////////////////////////////////////////////
//

osg::CTriMesh::CTriMesh()
    : m_bKDTreeUsed(false)
    , m_bUseVertexColors(false)
{
    setName("CTriMesh");
    pGeometries.push_back(new osg::Geometry);
    pVertices = new osg::Vec3Array;
    pVertexColors = new osg::Vec4Array;
    pPrimitiveSets.push_back(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES));

    pGeometries.back()->setVertexArray(pVertices);
    pGeometries.back()->addPrimitiveSet(pPrimitiveSets.back());

    pDefaultMaterial = pDefaultMaterialBackup = new osg::CPseudoMaterial;
    pDefaultMaterial->uniform("Shininess")->set(85.0f);
    pDefaultMaterial->uniform("Specularity")->set(1.0f);
    pMaterials[0] = pDefaultMaterial;
    pDefaultMaterial->apply(pGeometries.back());

    pColors = new osg::Vec4Array(1);
    (*pColors)[0] = osg::Vec4(1.0, 1.0, 1.0, 1.0);
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    pGeometries.back()->setColorArray(pColors, osg::Array::BIND_OVERALL);
#else
    pGeometries.back()->setColorArray(pColors);
#endif
    pGeometries.back()->setColorBinding(osg::Geometry::BIND_OVERALL);

    this->addDrawable(pGeometries.back());
}

///////////////////////////////////////////////////////////////////////////////
//

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
        removeDrawable(pGeometries.back());
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
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
        pGeometries.back()->setColorArray(pColors, osg::Array::BIND_OVERALL);
#else
        pGeometries.back()->setColorArray(pColors);
#endif
        pGeometries.back()->setColorBinding(osg::Geometry::BIND_OVERALL);
        addDrawable(pGeometries.back());
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

    for (int i = 0; i < pGeometries.size(); ++i)
    {
        pGeometries[i]->dirtyDisplayList();
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


///////////////////////////////////////////////////////////////////////////////
//

void osg::CTriMesh::useVertexColors(bool value, bool force)
{
    if (force || value != m_bUseVertexColors)
    {
        m_bUseVertexColors = value;

        for (int i = 0; i < pGeometries.size(); ++i)
        {
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
            pGeometries[i]->setColorArray(m_bUseVertexColors ? pVertexColors : pColors, osg::Array::BIND_OVERALL);
#else
            pGeometries[i]->setColorArray(m_bUseVertexColors ? pVertexColors : pColors);
#endif
            pGeometries[i]->setColorBinding(m_bUseVertexColors ? osg::Geometry::BIND_PER_VERTEX : osg::Geometry::BIND_OVERALL);
            pGeometries[i]->dirtyDisplayList();
        }
    }
}

void osg::CTriMesh::setColor(float r, float g, float b, float a)
{
    auto& color = (*pColors)[0];
    const auto newColor = osg::Vec4(r, g, b, a);

    if (color != newColor)
    {
        color = newColor;

        for (int i = 0; i < pGeometries.size(); ++i)
        {
            pGeometries[i]->dirtyDisplayList();
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
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
            pGeometries[i]->setNormalArray(pNormals.get(), osg::Array::BIND_PER_VERTEX);
#else
            pGeometries[i]->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
            pGeometries[i]->setNormalArray(pNormals.get());
#endif
            break;

        case ENU_NONE:
        default:
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
            pGeometries[i]->setNormalArray(pNoNormals.get(), osg::Array::BIND_OVERALL);
#else
            pGeometries[i]->setNormalBinding(osg::Geometry::BIND_OVERALL);
            pGeometries[i]->setNormalArray(pNoNormals.get());
#endif
            break;
        }

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
        pGeometries[i]->dirtyDisplayList();
#endif
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
        pGeometries[i]->dirtyDisplayList();
        pGeometries[i]->dirtyBound();
    }
}

void osg::CTriMesh::setUseDisplayList(bool bUse)
{
    for (int i = 0; i < pGeometries.size(); ++i)
    {
        pGeometries[i]->setUseDisplayList(bUse);
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
            pMaterials[materialIndex]->apply(pGeometries[i]);
        }
    }
    else
    {
        for (int i = 0; i < pGeometries.size(); ++i)
        {
            pDefaultMaterial->apply(pGeometries[i]);
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
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    osg::Vec3Array *pNormals = new osg::Vec3Array(vertexNormals ? numvert : numtris * 3);
    osg::Vec3Array *pVertices = new osg::Vec3Array(vertexNormals ? numvert : numtris * 3);
#else
    osg::Vec3Array *pNormals = new osg::Vec3Array(vertexNormals ? numvert : numtris);
    osg::Vec3Array *pVertices = new osg::Vec3Array(numvert);
#endif
    osg::DrawElementsUInt *pPrimitives = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, numtris * 3);
    osg::StateSet *pState = geometry->getOrCreateStateSet();
    geometry->setNormalArray(pNormals);
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
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    if (vertexNormals)
    {
#endif
        index = 0;
        for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
        {
            for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
            {
                (*pPrimitives)[index++] = mesh->property(vProp_bufferIndex, fvit);
            }
        }
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
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
#endif

    // Copy normals
    if (vertexNormals)
    {
        index = 0;
        for (geometry::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
        {
            (*pNormals)[index] = osg::Vec3(mesh->normal(vit)[0], mesh->normal(vit)[1], mesh->normal(vit)[2]);
            ++index;
        }

        osg::ShadeModel *pShadeModel = new osg::ShadeModel(osg::ShadeModel::SMOOTH);
        pState->setAttributeAndModes(pShadeModel, osg::StateAttribute::ON);
        geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    else
    {
        osg::ShadeModel *pShadeModel = new osg::ShadeModel(osg::ShadeModel::FLAT);
        pState->setAttributeAndModes(pShadeModel, osg::StateAttribute::ON);
        geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }
#else
    else
    {
        triindex = 0;
        for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
        {
            (*pNormals)[triindex] = osg::Vec3(mesh->normal(fit)[0], mesh->normal(fit)[1], mesh->normal(fit)[2]);
            ++triindex;
        }

        osg::ShadeModel *pShadeModel = new osg::ShadeModel(osg::ShadeModel::FLAT);
        pState->setAttributeAndModes(pShadeModel, osg::StateAttribute::ON);
        geometry->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
    }
#endif

    // remove no longer needed bufferIndex property
    mesh->remove_property(vProp_bufferIndex);

    geometry->dirtyDisplayList();
    geometry->dirtyBound();

    return geometry;
}


osg::Geometry *osg::convertOpenMesh2OSGGeometry(geometry::CMesh *mesh, const osg::Vec4& color)
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
    osg::Vec3Array *pNormals = new osg::Vec3Array(numvert);
    osg::Vec3Array *pVertices = new osg::Vec3Array(numvert);

    osg::DrawElementsUInt *pPrimitives = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, numtris * 3);
    osg::StateSet *pStateSet = geometry->getOrCreateStateSet();

    // Enable depth test so that an opaque polygon will occlude a transparent one behind it.
    pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    // Rescale normals
    pStateSet->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    // Culling
    osg::CullFace *pCull = new osg::CullFace();
    pCull->setMode(osg::CullFace::BACK);
    pStateSet->setAttributeAndModes(pCull, osg::StateAttribute::OFF);

    // Color
    osg::Vec4Array *pColors = new osg::Vec4Array(1);
    (*pColors)[0] = color;

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    geometry->setColorArray(pColors, osg::Array::BIND_OVERALL);
#else
    geometry->setColorArray(pColors);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
#endif

    // prepare osg::Geometry
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    geometry->setNormalArray(pNormals, osg::Array::BIND_PER_VERTEX);
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
#else
    geometry->setNormalArray(pNormals);
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
#endif    
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

    geometry->dirtyDisplayList();
    geometry->dirtyBound();

    return geometry;
}
