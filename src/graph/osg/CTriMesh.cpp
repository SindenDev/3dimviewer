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
{
    pGeometry = new osg::Geometry;
    pVertices = new osg::Vec3Array;
	pVerticesFlat = new osg::Vec3Array;
    pPrimitives = new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLES );
	pPrimitivesFlat = new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLES );

    pGeometry->setVertexArray( pVertices.get() );
    pGeometry->addPrimitiveSet( pPrimitives.get() );

    osg::StateSet * pState = this->getOrCreateStateSet();

    pShadeModel = new osg::ShadeModel(osg::ShadeModel::FLAT);
    pState->setAttributeAndModes(pShadeModel.get(), osg::StateAttribute::ON);

    // Create white material
    m_material = new osg::Material();
    m_material->setDiffuse(Material::FRONT_AND_BACK,  Vec4(0.8, 0.8, 0.8, 1.0));
    m_material->setSpecular(Material::FRONT_AND_BACK, Vec4(0.5, 0.5, 0.5, 1.0));
    m_material->setAmbient(Material::FRONT_AND_BACK,  Vec4(0.5, 0.5, 0.5, 1.0));
    m_material->setEmission(Material::FRONT_AND_BACK, Vec4(0.0, 0.0, 0.0, 1.0));
    m_material->setShininess(Material::FRONT_AND_BACK, 25.0);
    m_material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
    pState->setAttribute( m_material.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    pColors = new osg::Vec4Array( 1 );
    (*pColors)[0] = osg::Vec4( 1.0, 1.0, 1.0, 1.0 );
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	pGeometry->setColorArray( pColors, osg::Array::BIND_OVERALL );
	pGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );
#else
    pGeometry->setColorArray( pColors );
	pGeometry.get()->setColorBinding( osg::Geometry::BIND_OVERALL );
#endif

    this->addDrawable( pGeometry.get() );
}


///////////////////////////////////////////////////////////////////////////////
//

void osg::CTriMesh::createMesh(geometry::CMesh *mesh, bool createNormals, bool smooth)
{
    if (!mesh)
    {
        return;
    }

	// KD tree is not used
	m_bKDTreeUsed = false;

    // remove entities marked for deletion
    mesh->garbage_collection();

    // get number of mesh vertices
    long numvert(mesh->n_vertices());

    // get number of mesh triangles
    long numtris(mesh->n_faces());

    // indexing counters
    long index = 0;
    long triindex = 0;

    // smoothing doesn't work on empty model
    if (0==numvert)
        smooth = false;

    // Create normals
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    pVertexNormals = new Vec3Array( std::max(1,(int)numvert) );
    pFaceNormals = new Vec3Array( std::max(1,(int)numtris*3) );
#else
    pVertexNormals = new Vec3Array( numvert );
    pFaceNormals = new Vec3Array( numtris );
#endif
    pNoNormals = new Vec3Array(1);
    pNoNormals->push_back(osg::Vec3(0.0, 0.0, 0.0));

    mesh->update_face_normals();
    mesh->update_vertex_normals();
    ENormalsUsage normalsUsage;
    if (createNormals)
    {
        if (smooth)
        {
            normalsUsage = ENU_VERTEX;
        }
        else
        {
            normalsUsage = ENU_FACE;
        }
    }
    else
    {
        normalsUsage = ENU_NONE;
    }
    useNormals(normalsUsage);

    // Allocate memory for vertices
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    pVertices->resize(std::max(1,(int)numvert));
	pVerticesFlat->resize(std::max(3,(int)numtris*3));
#else
	pVertices->resize( numvert );
#endif

    // Copy vertices and add bufferIndex property to each vertex of mesh for easy face-vertex indexing later
    index = 0;
    OpenMesh::VPropHandleT<int> vProp_bufferIndex;

	// Test if property exist and if not, add it
	if(!mesh->get_property_handle(vProp_bufferIndex, BUFFER_INDEX_PROPERTY))
		mesh->add_property(vProp_bufferIndex, BUFFER_INDEX_PROPERTY);

    for (geometry::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
    {
        mesh->property(vProp_bufferIndex, vit) = index;
        (*pVertices)[index] = osg::Vec3( mesh->point(vit)[0], mesh->point(vit)[1], mesh->point(vit)[2] );
        (*pVertexNormals)[index] = osg::Vec3( mesh->normal(vit)[0], mesh->normal(vit)[1], mesh->normal(vit)[2] );
        ++index;
    }
    
    // Allocate memory for indexing
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    pPrimitives->resize(std::max(3,(int)numtris * 3));
	pPrimitivesFlat->resize(std::max(3,(int)numtris * 3));
#else
	pPrimitives->resize( numtris * 3 );
#endif

    // Copy triangle vertex indexing
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    triindex = 0;
    index = 0;	
    for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
    {
		osg::Vec3 fn = osg::Vec3(mesh->normal(fit)[0], mesh->normal(fit)[1], mesh->normal(fit)[2]);
        for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
        {            
			(*pVerticesFlat)[index] = osg::Vec3( mesh->point(fvit)[0], mesh->point(fvit)[1], mesh->point(fvit)[2] );
			(*pFaceNormals)[index] = fn;
			(*pPrimitives)[index] = mesh->property(vProp_bufferIndex, fvit);
			(*pPrimitivesFlat)[index] = index;
			index++;
        }        
        ++triindex;
    }
#else
    triindex = 0;
    index = 0;
    for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
    {
        for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
        {            
			(*pPrimitives)[index++] = mesh->property(vProp_bufferIndex, fvit);
        }        
		(*pFaceNormals)[triindex] = osg::Vec3(mesh->normal(fit)[0], mesh->normal(fit)[1], mesh->normal(fit)[2]);
        ++triindex;
    }
#endif

    // Property is used in guide fabrication. This property is connection between visible osg model 
    // vertices (and faces) and openmesh model. Drawed guide limiting curve stores touched triangles numbers
    // and algortihm translates them to the (open)mesh triangle indexes. Do not remove commentary 
    // If the property must be removed for some reason, discuss it with me. Wik.
//    mesh->remove_property(vProp_bufferIndex);

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    //useNormals(ENU_VERTEX);
#endif

    pGeometry->dirtyDisplayList();
    pGeometry->dirtyBound();
}


///////////////////////////////////////////////////////////////////////////////
//

void osg::CTriMesh::setColor( float r, float g, float b, float a )
{
    osg::Vec4Array * colors = static_cast< osg::Vec4Array * >( pGeometry->getColorArray() );
    (*colors)[0] = osg::Vec4( r, g, b, a );

    m_material->setDiffuse(Material::FRONT_AND_BACK,  Vec4(0.8 * r, 0.8 * g, 0.8 * b, 1.0));
    m_material->setSpecular(Material::FRONT_AND_BACK, Vec4(0.5 * r, 0.5 * g, 0.5 * b, 1.0));
    m_material->setAmbient(Material::FRONT_AND_BACK,  Vec4(0.5 * r, 0.5 * g, 0.5 * b, 1.0));
//    m_material->setEmission( Material::FRONT_AND_BACK, Vec4(r, g, b, a) );
    m_material->setShininess(Material::FRONT_AND_BACK, 25.0);

    pGeometry->dirtyDisplayList();
}

osg::Material *osg::CTriMesh::getMaterial()
{
    return m_material;
}

void osg::CTriMesh::useNormals(ENormalsUsage normalsUsage)
{
    switch (normalsUsage)
    {
    case ENU_FACE:
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
        pGeometry.get()->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
        pShadeModel->setMode(osg::ShadeModel::FLAT);
		pGeometry->setVertexArray( pVerticesFlat.get() );
        pGeometry->setNormalArray( pFaceNormals.get(), osg::Array::BIND_PER_VERTEX  );		
		pGeometry->removePrimitiveSet(0,pGeometry->getNumPrimitiveSets());
		pGeometry->addPrimitiveSet( pPrimitivesFlat.get() );
#else
        pGeometry.get()->setNormalBinding( osg::Geometry::BIND_PER_PRIMITIVE );
        pShadeModel->setMode(osg::ShadeModel::FLAT);
        pGeometry->setNormalArray( pFaceNormals.get() );        
#endif
		break;
    case ENU_VERTEX:
        pGeometry.get()->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
        pShadeModel->setMode(osg::ShadeModel::SMOOTH);
		pGeometry->setVertexArray( pVertices.get() );        
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
        pGeometry->removePrimitiveSet(0,pGeometry->getNumPrimitiveSets());
        pGeometry->addPrimitiveSet( pPrimitives.get() );
		pGeometry->setNormalArray( pVertexNormals.get(), osg::Array::BIND_PER_VERTEX  );
#else
		pGeometry->setNormalArray( pVertexNormals.get() );
#endif
        break;

    case ENU_NONE:
    default:
        pShadeModel->setMode(osg::ShadeModel::FLAT);
		pGeometry->setVertexArray( pVertices.get() );
		pGeometry.get()->setNormalBinding( osg::Geometry::BIND_OVERALL );
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
        pGeometry->removePrimitiveSet(0,pGeometry->getNumPrimitiveSets());
        pGeometry->addPrimitiveSet( pPrimitives.get() );
        pGeometry->setNormalArray( pNoNormals.get(), osg::Array::BIND_OVERALL );		
#else
		pGeometry->setNormalArray( pNoNormals.get() );
#endif
        break;
    }
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	pGeometry->dirtyDisplayList();
#endif
}


void osg::CTriMesh::updatePartOfMesh(geometry::CMesh *mesh, const tIdPosVec &ip, bool createNormals /*= true*/, bool smooth /*= true*/ )
{
	// If no vertices, do nothing...
	if(ip.size() == 0)
		return;

	// Do not update kd-tree
//	if(m_bKDTreeUsed)
//		return;

/*
	// Normals settings
	ENormalsUsage normalsUsage;
	if (createNormals)
	{
		if (smooth)
		{
			normalsUsage = ENU_VERTEX;
		}
		else
		{
			normalsUsage = ENU_FACE;
		}
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
	for(; ith != ithEnd; ++ith)
	{
		assert(ith->first < vsize);

		// Move point
		(*pVertices)[ith->first] = ith->second.position;
		(*pVertexNormals)[ith->first] = ith->second.normal;
	}

#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	// update flat vertices - we assume that structure is the same, just that positions have changed
	// because pPrimitives and pPrimitivesFlat correspond, we can use them for flat vertex array mappings (but we perform full update)
	if (NULL!=mesh && NULL!=pVerticesFlat.get() && pVerticesFlat->size()>0)
	{
		assert(pVerticesFlat->size()==mesh->n_faces()*3);
		assert(pFaceNormals->size()==mesh->n_faces()*3);
		int triindex = 0;
		int index = 0;	
		for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
		{
			osg::Vec3 fn = osg::Vec3(mesh->normal(fit)[0], mesh->normal(fit)[1], mesh->normal(fit)[2]);
			for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
			{            
				(*pVerticesFlat)[index] = osg::Vec3( mesh->point(fvit)[0], mesh->point(fvit)[1], mesh->point(fvit)[2] );
				(*pFaceNormals)[index] = fn;
				index++;
			}        
			++triindex;
		}
		pVerticesFlat->dirty();
		pFaceNormals->dirty();
	}
#endif

	pVertexNormals->dirty();
	pVertices->dirty();
	pGeometry->dirtyDisplayList();
	pGeometry->dirtyBound();
}

void osg::CTriMesh::buildKDTree()
{
	if(m_bKDTreeUsed)
		return;

	// Update the KDTree
	osg::KdTree::BuildOptions kdTreeBuildOptions;
	osg::ref_ptr<osg::KdTree> kdTree = new osg::KdTree();

	if(kdTree->build(kdTreeBuildOptions, pGeometry))
	{
		pGeometry->setShape(kdTree.get());
		m_bKDTreeUsed = true;
	}
	else
	{
		//LOG_MSG(logERROR) << "osg::KdTree::build() unsuccessful.";
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
	if(!mesh->has_vertex_normals())
		mesh->request_vertex_normals();
	if(!mesh->has_face_normals())
		mesh->request_face_normals();
    mesh->update_normals();
    long numvert(mesh->n_vertices());
    long numtris(mesh->n_faces());

    // prepare osg::Geometry
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	osg::Vec3Array *pNormals = new osg::Vec3Array(vertexNormals ? numvert : numtris*3);
    osg::Vec3Array *pVertices = new osg::Vec3Array(vertexNormals ? numvert : numtris*3);
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


osg::Geometry *osg::convertOpenMesh2OSGGeometry(geometry::CMesh *mesh, const osg::Vec4& color )
{
    osg::Geometry *geometry = new osg::Geometry();
    
    if ((mesh == NULL) || (geometry == NULL))
    {
        return NULL;
    }

    // pre-process loaded mesh and get counts
    mesh->garbage_collection();
	if(!mesh->has_vertex_normals())
		mesh->request_vertex_normals();
	if(!mesh->has_face_normals())
		mesh->request_face_normals();
    mesh->update_normals();
	long numvert(mesh->n_vertices());
    long numtris(mesh->n_faces());

    // prepare osg::Geometry
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    osg::Vec3Array *pNormals = new osg::Vec3Array(numtris*3);
	osg::Vec3Array *pVertices = new osg::Vec3Array(numtris*3);
#else
	osg::Vec3Array *pNormals = new osg::Vec3Array(numtris);
	osg::Vec3Array *pVertices = new osg::Vec3Array(numvert);
#endif
    osg::DrawElementsUInt *pPrimitives = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, numtris * 3);
    osg::StateSet *pState = geometry->getOrCreateStateSet();

    // Enable depth test so that an opaque polygon will occlude a transparent one behind it.
    pState->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    // Rescale normals
    pState->setMode( GL_RESCALE_NORMAL, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    // Setup the light model
    osg::LightModel * pLightModel = new osg::LightModel();
    pLightModel->setTwoSided( true );
    pLightModel->setAmbientIntensity( osg::Vec4(0.1, 0.1, 0.1, 1.0) );
    pState->setAttributeAndModes( pLightModel, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
	
    // Enable lighting
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    osg::ShadeModel *pShadeModel = new osg::ShadeModel(osg::ShadeModel::FLAT);
	pState->setMode( GL_LIGHTING, osg::StateAttribute::ON );
#else
	osg::ShadeModel *pShadeModel = new osg::ShadeModel(osg::ShadeModel::SMOOTH);
	pState->setMode( GL_LIGHTING, osg::StateAttribute::ON );
#endif    
	pState->setAttributeAndModes(pShadeModel, osg::StateAttribute::ON );

    // Culling
    osg::CullFace *pCull = new osg::CullFace();
    pCull->setMode( osg::CullFace::BACK );
    pState->setAttributeAndModes( pCull, osg::StateAttribute::OFF );

    // Color
    osg::Vec4Array *pColors = new osg::Vec4Array( 1 );
    (*pColors)[0] = color;
    
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	geometry->setColorArray( pColors, osg::Array::BIND_OVERALL);
	geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
#else
	geometry->setColorArray( pColors );
	geometry->setColorBinding( osg::Geometry::BIND_OVERALL );
#endif
    
    // Material
    osg::Material * pMaterial = new osg::Material();
    pMaterial->setDiffuse(osg::Material::FRONT_AND_BACK,  osg::Vec4(0.8 * color.r(), 0.8 * color.g(), 0.8 * color.b(), 1.0));
    pMaterial->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5 * color.r(), 0.5 * color.g(), 0.5 * color.b(), 1.0));
    pMaterial->setAmbient(osg::Material::FRONT_AND_BACK,  osg::Vec4(0.5 * color.r(), 0.5 * color.g(), 0.5 * color.b(), 1.0));
    pMaterial->setShininess(osg::Material::FRONT_AND_BACK, 25.0);
    pMaterial->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0, 0.0, 0.0, 1.0));
    pMaterial->setColorMode(osg::Material::/*ColorMode::*/AMBIENT_AND_DIFFUSE);
    pState->setAttribute( pMaterial, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    // prepare osg::Geometry
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	geometry->setNormalArray(pNormals, osg::Array::BIND_PER_VERTEX );
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
#else
	geometry->setNormalArray(pNormals);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
#endif    
    geometry->setVertexArray(pVertices);
    geometry->addPrimitiveSet(pPrimitives);

    // indexing counters
    long index = 0;
    long triindex = 0;

    // Copy vertices and add bufferIndex property to each vertex of mesh for easy face-vertex indexing later
	index = 0;
    OpenMesh::VPropHandleT<int> vProp_bufferIndex;
    OpenMesh::VPropHandleT<int> vProp_flag;
    
    // THIS PROPERTY TIES OSG AND OPENMESH TOGETHER. IT IS USED IN ANOTHER PARTS OF BSP. DO NOT REMOVE IT. NEVER!
    mesh->add_property(vProp_bufferIndex, "bufferIndex");


#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	//
#else
    for (geometry::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end() && index < numvert; ++vit)
	{
        mesh->property(vProp_bufferIndex, vit) = index;
        (*pVertices)[index] = osg::Vec3(mesh->point(vit)[0], mesh->point(vit)[1], mesh->point(vit)[2]);
        ++index;
    }
#endif
    
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
	// Copy triangle vertex indexing
    triindex = 0;
    index = 0;
    for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end() && triindex < numtris; ++fit)
    {
		osg::Vec3 fn(mesh->normal(fit)[0], mesh->normal(fit)[1], mesh->normal(fit)[2]);
		for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
		{
			(*pVertices)[index] = osg::Vec3(mesh->point(fvit)[0], mesh->point(fvit)[1], mesh->point(fvit)[2]);				
			(*pNormals)[index] = fn;
			(*pPrimitives)[index] = index;
			index++;			
		}
		triindex++;
    }
#else
	// Copy triangle vertex indexing
    triindex = 0;
    index = 0;
    for (geometry::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end() && triindex < numtris; ++fit)
    {
        for (geometry::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit); fvit != mesh->fv_end(fit); ++fvit)
        {
            (*pPrimitives)[index++] = mesh->property(vProp_bufferIndex, fvit);
        }

        (*pNormals)[triindex] = osg::Vec3(mesh->normal(fit)[0], mesh->normal(fit)[1], mesh->normal(fit)[2]);
        ++triindex;
    }
#endif

    geometry->dirtyDisplayList();
    geometry->dirtyBound();

    return geometry;
}
