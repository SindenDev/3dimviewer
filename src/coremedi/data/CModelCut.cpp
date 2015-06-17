///////////////////////////////////////////////////////////////////////////////
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2015 3Dim Laboratory s.r.o.
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

#include "data/CModelCut.h"
#include "data/COrthoSlice.h"
#include <app/Signals.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//
CModelCut::CModelCut()
    : vpl::base::CObject()
{
    m_vertices = new osg::Vec3Array;
    m_indices = new osg::DrawElementsUInt(osg::DrawElements::LINES);
    m_modelId = data::Storage::BonesModel::Id;
    m_color = data::CColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

//
CModelCut::CModelCut(const CModelCut &modelCut)
    : vpl::base::CObject()
{
    m_vertices->clear();
    for (std::size_t i = 0; i < modelCut.m_vertices->size(); ++i)
    {
        m_vertices->push_back(modelCut.m_vertices->operator[](i));
    }
    
    m_indices->clear();
    for (std::size_t i = 0; i < modelCut.m_indices->size(); ++i)
    {
        m_indices->push_back(modelCut.m_indices->operator[](i));
    }
    m_modelId = modelCut.m_modelId;
    m_color = modelCut.m_color;
}

//
CModelCut::~CModelCut()
{ }

//
void CModelCut::update(const CChangedEntries &changedEntries)
{ }

//
void CModelCut::init()
{
    clear();
}

//
void CModelCut::clear()
{
    m_vertices->clear();
    m_indices->clear();
}

///////////////////////////////////////////////////////////////////////////////
//
void CModelCutSliceXY::update(const CChangedEntries &changedEntries)
{
    data::CObjectPtr<data::COrthoSliceXY> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXY::Id));
    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(m_modelId));

    double dZ = 1.0;

    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId != CUSTOM_DATA)
    {
        data::CObjectPtr<data::CDensityData> spDensityData(APP_STORAGE.getEntry(datasetId));
        dZ = spDensityData->getDZ();
    }

    m_color = spModel->getColor();

    geometry::CMesh *mesh = spModel->getMesh();
    float planePosition = spSlice->getPosition() * dZ;

    if (changedEntries.hasChanged(m_modelId))
    {
		data::CChangedEntries::tFilter filter;
		filter.insert(m_modelId);
		if (!changedEntries.checkExactFlagsAll(data::CModel::MESH_NOT_CHANGED,data::CModel::MESH_NOT_CHANGED,filter))
			mesh->updateOctree(APP_STORAGE.getEntry(m_modelId).get()->getLatestVersion());
    }

    clear();
    const osg::Matrix& matrix = spModel->getTransformationMatrix(); // check correction matrix of the model
    m_transformMatrix = matrix;
    if (matrix.isIdentity())
    {
        geometry::CMesh::cutByZPlane(mesh, m_vertices, m_indices, planePosition);
    }
    else
    {
        osg::Plane plane( osg::Vec3(0.0, 0.0, 1.0), -planePosition );
        geometry::CMesh::cutByPlane(mesh, m_vertices, m_indices, plane, matrix);
    }
}


///////////////////////////////////////////////////////////////////////////////
//
void CModelCutSliceXZ::update(const CChangedEntries &changedEntries)
{
    data::CObjectPtr<data::COrthoSliceXZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id));
    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(m_modelId));

    double dY = 1.0;

    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId != CUSTOM_DATA)
    {
        data::CObjectPtr<data::CDensityData> spDensityData(APP_STORAGE.getEntry(datasetId));
        dY = spDensityData->getDY();
    }

    m_color = spModel->getColor();

    geometry::CMesh *mesh = spModel->getMesh();
    float planePosition = spSlice->getPosition() * dY;

    if (changedEntries.hasChanged(m_modelId))
    {
		data::CChangedEntries::tFilter filter;
		filter.insert(m_modelId);
		if (!changedEntries.checkExactFlagsAll(data::CModel::MESH_NOT_CHANGED,data::CModel::MESH_NOT_CHANGED,filter))
			mesh->updateOctree(APP_STORAGE.getEntry(m_modelId).get()->getLatestVersion());
    }

    clear();
    const osg::Matrix& matrix = spModel->getTransformationMatrix(); // check correction matrix of the model
    m_transformMatrix = matrix;
    if (matrix.isIdentity())
    {
        geometry::CMesh::cutByYPlane(mesh, m_vertices, m_indices, planePosition);
    }
    else
    {
        osg::Plane plane( osg::Vec3(0.0, 1.0, 0.0), -planePosition );
        geometry::CMesh::cutByPlane(mesh, m_vertices, m_indices, plane, matrix);
    }
}


///////////////////////////////////////////////////////////////////////////////
//
void CModelCutSliceYZ::update(const CChangedEntries &changedEntries)
{
    data::CObjectPtr<data::COrthoSliceYZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id));
    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(m_modelId));

    double dX = 1.0;

    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId != CUSTOM_DATA)
    {
        data::CObjectPtr<data::CDensityData> spDensityData(APP_STORAGE.getEntry(datasetId));
        dX = spDensityData->getDX();
    }

    m_color = spModel->getColor();

    geometry::CMesh *mesh = spModel->getMesh();
    float planePosition = spSlice->getPosition() * dX;

    if (changedEntries.hasChanged(m_modelId))
    {
		data::CChangedEntries::tFilter filter;
		filter.insert(m_modelId);
		if (!changedEntries.checkExactFlagsAll(data::CModel::MESH_NOT_CHANGED,data::CModel::MESH_NOT_CHANGED,filter))
			mesh->updateOctree(APP_STORAGE.getEntry(m_modelId).get()->getLatestVersion());
    }

    clear();
    const osg::Matrix& matrix = spModel->getTransformationMatrix(); // check correction matrix of the model
    m_transformMatrix = matrix;
    if (matrix.isIdentity())
    {
        geometry::CMesh::cutByXPlane(mesh, m_vertices, m_indices, planePosition);
    }
    else
    {
        osg::Plane plane( osg::Vec3(1.0, 0.0, 0.0), -planePosition );
        geometry::CMesh::cutByPlane(mesh, m_vertices, m_indices, plane, matrix);
    }
}

} // namespace data
