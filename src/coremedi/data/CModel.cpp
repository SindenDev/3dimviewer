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

#include <data/CModel.h>
#include <data/CDensityData.h>
#include <app/Signals.h>

///////////////////////////////////////////////////////////////////////////////
//

void data::CModel::init()
{
    hide();
    clear();
    m_transformationMatrix = osg::Matrix::identity();
	m_properties.clear();
}

///////////////////////////////////////////////////////////////////////////////
//

void data::CModel::update(const data::CChangedEntries& Changes)
{
    if( Changes.checkFlagAny(data::Storage::STORAGE_RESET) )
    {
        hide();
        clear();
        m_transformationMatrix = osg::Matrix::identity();
		m_properties.clear();
    }

/*
	// New density data loaded?
    if( Changes.hasChanged(data::Storage::PatientData::Id)
        && !Changes.checkFlagAll(data::CDensityData::DENSITY_MODIFIED) )
    {
        hide();
        clear();
    }
	*/
}

///////////////////////////////////////////////////////////////////////////////
//Copy model

data::CModel & data::CModel::operator = ( const data::CModel & model )
{
    m_Color = model.m_Color;
    m_transformationMatrix = model.m_transformationMatrix;
    m_bVisibility = model.m_bVisibility;
    m_label = model.m_label;
	m_properties = model.m_properties;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Restore state from the snapshot
void data::CModel::restore(CSnapshot *snapshot)
{
    assert( snapshot != NULL );
    data::CMeshSnapshot *meshSnapshot = dynamic_cast<data::CMeshSnapshot *>(snapshot);
    assert(meshSnapshot != NULL);

    // try to get model from the storage
    int storageId = getStorageId();
    if (storageId == 0)
    {
        return;
    }

    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(storageId));
    data::CMesh *mesh = getMesh();
    if (mesh == NULL)
    {
        mesh = new data::CMesh;
        setMesh(mesh);
    }
    mesh->clear();

    std::vector<data::CMesh::VertexHandle> vertices;
    for (int v = 0; v < meshSnapshot->vertexCount; ++v)
    {
        vertices.push_back(mesh->add_vertex(meshSnapshot->vertices[v]));
    }
    if (meshSnapshot->propertyVertexFlags != NULL)
    {
        OpenMesh::VPropHandleT<int> vProp_VertexFlag;
        if (!mesh->get_property_handle(vProp_VertexFlag, "property_vertex_flags"))
        {
            mesh->add_property(vProp_VertexFlag, "property_vertex_flags");
        }

        for (int v = 0; v < vertices.size(); ++v)
        {
            mesh->property(vProp_VertexFlag, vertices[v]) = meshSnapshot->propertyVertexFlags[v];
        }
    }
    for (int t = 0; t < meshSnapshot->indexCount / 3; ++t)
    {
        data::CMesh::VertexHandle i0 = vertices[meshSnapshot->indices[t * 3 + 0]];
        data::CMesh::VertexHandle i1 = vertices[meshSnapshot->indices[t * 3 + 1]];
        data::CMesh::VertexHandle i2 = vertices[meshSnapshot->indices[t * 3 + 2]];

        mesh->add_face(i0, i1, i2);
    }

    // invalidate
    APP_STORAGE.invalidate(spModel.getEntryPtr());
}

data::CSnapshot *data::CModel::getSnapshot(CSnapshot *snapshot)
{
    CMeshSnapshot *s = new CMeshSnapshot(this);

    data::CMesh *mesh = getMesh();
    if (mesh == NULL || mesh->n_vertices() == 0)
    {
        return s;
    }

    s->vertexCount = mesh->n_vertices();
    s->vertices = new data::CMesh::Point[s->vertexCount];
    std::map<data::CMesh::VertexHandle, int> vertexMap;
    OpenMesh::VPropHandleT<int> vProp_VertexFlag;
    if (mesh->get_property_handle(vProp_VertexFlag, "property_vertex_flags"))
    {
        s->propertyVertexFlags = new int[s->vertexCount];
    }
    int v = 0;
    for (data::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
    {
        vertexMap[vit.handle()] = v;
        s->vertices[v] = mesh->point(vit.handle());
        if (s->propertyVertexFlags != NULL)
        {
            s->propertyVertexFlags[v] = mesh->property(vProp_VertexFlag, vit.handle());
        }
        v++;
    }

    s->indexCount = mesh->n_faces() * 3;
    s->indices = new int[s->indexCount];
    int i = 0;
    for (data::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
    {
        for (data::CMesh::FaceVertexIter fvit = mesh->fv_begin(fit.handle()); fvit != mesh->fv_end(fit.handle()); ++fvit)
        {
            s->indices[i] = vertexMap[fvit.handle()];
            i++;
        }
    }

    return s;
}

/**
 * Gets an up down surface areas.
 *
 * \param [in,out]	up_area  	The up area.
 * \param [in,out]	down_area	The down area.
**/
void data::CModel::getUpDownSurfaceAreas( float &up_area, float &down_area )
{
	up_area = down_area = 0.0;

	m_spModel->request_face_normals();
	m_spModel->update_normals();

	// Compute local z axis orientation vector
	osg::Vec3 osg_zaxis(0.0, 0.0, 1.0);
	osg_zaxis = osg_zaxis * m_transformationMatrix;
	osg::Vec3 shift_vector(m_transformationMatrix.getTrans());
	osg_zaxis -= shift_vector;
	osg_zaxis.normalize();
	data::CMesh::Normal om_zaxis(osg_zaxis[0], osg_zaxis[1], osg_zaxis[2]);

	for (data::CMesh::FaceIter fit = m_spModel->faces_begin(); fit != m_spModel->faces_end(); ++fit)
	{
		// Calculate projection of the face normal to the reoriented z-axis
		data::CMesh::Normal normal(m_spModel->normal(fit.handle()));
		float dp(normal|om_zaxis);

		// Calculate face area
		float a(m_spModel->area(fit.handle()));

		if(dp > 0.0)
			up_area += a;
		else
			down_area += a;
	}

}

void data::CModel::createAndStoreSnapshot()
{
    VPL_SIGNAL(SigUndoSnapshot).invoke(this->getSnapshot(NULL));
}

data::CMeshSnapshot::CMeshSnapshot(CUndoProvider *provider)
    : CSnapshot(data::UNDO_MODELS, provider)
    , propertyVertexFlags(NULL)
    , vertices(NULL)
    , vertexCount(0)
    , indices(NULL)
    , indexCount(0)
{ }

data::CMeshSnapshot::~CMeshSnapshot()
{
    delete[] propertyVertexFlags;
    delete[] vertices;
    delete[] indices;
}

long data::CMeshSnapshot::getDataSize()
{
    return sizeof(CMeshSnapshot) + (sizeof(data::CMesh::Point) + sizeof(int)) * vertexCount + sizeof(int) * indexCount;
}

