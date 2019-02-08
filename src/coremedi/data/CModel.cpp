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

#include <data/CModel.h>
#include <data/CDensityData.h>
#include <coremedi/app/Signals.h>
#include <data/ESnapshotType.h>

#include <array>

///////////////////////////////////////////////////////////////////////////////
//

//! Default constructor.
data::CModel::CModel()
    : m_bVisibility(false)
    , m_Color(1.0f, 1.0f, 1.0f, 1.0f)
    , m_emission(0.0f, 0.0f, 0.0f)
    , m_bSelected(false)
    , m_bUseVertexColors(false)
    , m_regionId(-1)
    , m_bLinkedWithRegion(true)
    , m_bMirrored(false)
    , m_locked(false)
    , m_bExamined(false)
    , m_mesh_dirty(true)
{ }

//! Destructor.
data::CModel::~CModel()
{ }

//! Returns the model geometry.
geometry::CMesh *data::CModel::getMesh(bool bInvalidateKdTree)
{
    if (bInvalidateKdTree)
        m_mesh_dirty = true;
    return m_spModel.get();
}

//! Const version of the previous method
const geometry::CMesh *data::CModel::getMesh() const
{
    return m_spModel.get();
}

//! Returns the model geometry and clears Destroy flag in smart pointer
geometry::CMesh *data::CModel::releaseMesh()
{
    m_mesh_dirty = true;
    return m_spModel.release();
}

data::CModel::CModel(const data::CModel& model)
{
    m_Color = model.m_Color;
    m_emission = model.m_emission;
    m_transformationMatrix = model.m_transformationMatrix;
    m_bVisibility = model.m_bVisibility;
    m_bUseVertexColors = model.m_bUseVertexColors;
    m_label = model.m_label;
    m_properties = model.m_properties;
    m_spArmature = (model.m_spArmature.get() != NULL ? model.m_spArmature->clone() : NULL);
    m_segToBone = model.m_segToBone;
    m_bSelected = model.m_bSelected;
    m_regionId = model.m_regionId;
    m_bLinkedWithRegion = model.m_bLinkedWithRegion;
    m_bMirrored = model.m_bMirrored;
    m_locked = model.m_locked;
    m_bExamined = model.m_bExamined;
    m_textures = model.m_textures;

    m_transformationMatrixBeforeReposition = model.m_transformationMatrixBeforeReposition;
    m_transformationMatrixAfterReposition = model.m_transformationMatrixAfterReposition;

    m_mesh_dirty = true;
}

///////////////////////////////////////////////////////////////////////////////
//

void data::CModel::init()
{
    hide();
    clear();
    m_transformationMatrix = osg::Matrix::identity();
    m_properties.clear();
    m_bSelected = false;
    m_bUseVertexColors = false;
    m_spArmature = new geometry::CArmature;
    m_segToBone.clear();
    m_regionId = -1;
    m_bLinkedWithRegion = true;
    m_bMirrored = false;
    m_locked = false;
    m_bExamined = false;
    m_mesh_dirty = true;
    m_Color = CColor4f(1.0, 1.0, 1.0, 1.0);
    m_emission = geometry::Vec3(0.0, 0.0, 0.0);
    m_textures.clear();

    m_transformationMatrixBeforeReposition = osg::Matrix::identity();
    m_transformationMatrixAfterReposition = osg::Matrix::identity();
}

///////////////////////////////////////////////////////////////////////////////
//

void data::CModel::update(const data::CChangedEntries &Changes)
{
    if (Changes.checkFlagAny(data::Storage::STORAGE_RESET))
    {
        hide();
        clear();
        m_transformationMatrix = osg::Matrix::identity();
        m_properties.clear();
        m_bSelected = false;
        m_bUseVertexColors = false;
        m_spArmature = new geometry::CArmature;
        m_segToBone.clear();
        m_regionId = -1;
        m_bLinkedWithRegion = true;
        m_bMirrored = false;
        m_locked = false;
        m_bExamined = false;
        m_mesh_dirty = true;
        m_Color = CColor4f(1.0, 1.0, 1.0, 1.0);
        m_emission = geometry::Vec3(0.0, 0.0, 0.0);
        m_textures.clear();

        m_transformationMatrixBeforeReposition = osg::Matrix::identity();
        m_transformationMatrixAfterReposition = osg::Matrix::identity();
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

data::CModel &data::CModel::operator=(const data::CModel &model)
{
    if (this == &model)
    {
        return *this;
    }

    m_Color = model.m_Color;
    m_emission = model.m_emission;
    m_transformationMatrix = model.m_transformationMatrix;
    m_bVisibility = model.m_bVisibility;
    m_bUseVertexColors = model.m_bUseVertexColors;
    m_label = model.m_label;
    m_properties = model.m_properties;
    m_spArmature = (model.m_spArmature.get() != NULL ? model.m_spArmature->clone() : NULL);
    m_segToBone = model.m_segToBone;
    m_bSelected = model.m_bSelected;
    m_regionId = model.m_regionId;
    m_bLinkedWithRegion = model.m_bLinkedWithRegion;
    m_bMirrored = model.m_bMirrored;
    m_locked = model.m_locked;
    m_bExamined = model.m_bExamined;
    m_textures = model.m_textures;

    m_transformationMatrixBeforeReposition = model.m_transformationMatrixBeforeReposition;
    m_transformationMatrixAfterReposition = model.m_transformationMatrixAfterReposition;
    m_mesh_dirty = true;

    return *this;
}

data::CModel &data::CModel::operator=(data::CModel &model)
{
    if (this == &model)
    {
        return *this;
    }

    m_Color = model.m_Color;
    m_emission = model.m_emission;
    m_transformationMatrix = model.m_transformationMatrix;
    m_bVisibility = model.m_bVisibility;
    m_bUseVertexColors = model.m_bUseVertexColors;
    m_label = model.m_label;
    m_properties = model.m_properties;
    m_spArmature = (model.m_spArmature.get() != NULL ? model.m_spArmature->clone() : NULL);
    m_segToBone = model.m_segToBone;
    m_bSelected = model.m_bSelected;
    m_regionId = model.m_regionId;
    m_bLinkedWithRegion = model.m_bLinkedWithRegion;
    m_bMirrored = model.m_bMirrored;
    m_locked = model.m_locked;
    m_bExamined = model.m_bExamined;
    m_textures = model.m_textures;

    m_transformationMatrixBeforeReposition = model.m_transformationMatrixBeforeReposition;
    m_transformationMatrixAfterReposition = model.m_transformationMatrixAfterReposition;
    m_mesh_dirty = true;

    return *this;
}

void data::CModel::setMesh(geometry::CMesh *model)
{
    m_spModel = model;
    m_mesh_dirty = true;
}

//! This strictly MUST be called after any mesh change excluding setMesh method call (it updates dirty flag internally).
void data::CModel::setMeshDirty()
{
    m_mesh_dirty = true;
}

//! Returns true if the model is visible.
bool data::CModel::isVisible() const
{
    return m_bVisibility;
}

//! Turns on visibility of the model.
void data::CModel::show()
{
    m_bVisibility = true;
}

//! Turns off visibility of the model.
void data::CModel::hide()
{
    m_bVisibility = false;
}

//! Turns on/off visibility of the model. 
void data::CModel::setVisibility(bool visible)
{
    m_bVisibility = visible;
}

//! Turns on/off visibility of the model. 
void data::CModel::setUseVertexColors(bool value)
{
    m_bUseVertexColors = value;
}

bool data::CModel::getUseVertexColors() const
{
    return m_bUseVertexColors;
}

//! (De)selects model
void data::CModel::select(bool value)
{
    m_bSelected = value;
}

void data::CModel::deselect()
{
    select(false);
}

//! Returns true if model is selected
bool data::CModel::isSelected() const
{
    return m_bSelected;
}

//! Returns the model color.
void data::CModel::getColor(float& r, float& g, float& b, float& a) const
{
    m_Color.getColor(r, g, b, a);
}

//! Returns the model color.
const data::CColor4f &data::CModel::getColor() const
{
    return m_Color;
}

//! Sets the model color.
void data::CModel::setColor(float r, float g, float b, float a)
{
    m_Color.setColor(r, g, b, a);
}

//! Sets the model color.
void data::CModel::setColor(const CColor4f& Color)
{
    m_Color.setColor(Color);
}

//! Returns the model color.
const geometry::Vec3 &data::CModel::getEmission() const
{
    return m_emission;
}

//! Sets the model color.
void data::CModel::setEmission(const geometry::Vec3 &emission)
{
    m_emission = emission;
}

//! Sets the region id.
void data::CModel::setRegionId(int id)
{
    m_regionId = id;
}

//! Returns the region id.
int data::CModel::getRegionId()
{
    return m_regionId;
}

//! Sets if model mirrors region data.
void data::CModel::setLinkedWithRegion(bool linked)
{
    m_bLinkedWithRegion = linked;
}

//! Returns true if model mirrors region data.
bool data::CModel::isLinkedWithRegion()
{
    return m_bLinkedWithRegion;
}

//! Returns voluntary transformation matrix - !!! Remember that model visualizers are anchored to center
const osg::Matrix &data::CModel::getTransformationMatrix() const
{
    return m_transformationMatrix;
}

//! Sets transformation matrix for the model
void data::CModel::setTransformationMatrix(const osg::Matrix& matrix)
{
    m_transformationMatrix = matrix;
}

//! Get model name
const std::string &data::CModel::getLabel() const
{
    return m_label;
}

//! Set model name
void data::CModel::setLabel(const std::string &label)
{
    m_label = label;
}

void data::CModel::setReserved(bool value)
{
    m_reserved = value;
}

bool data::CModel::getReserved()
{
    return m_reserved;
}

//! Get model property
std::string data::CModel::getProperty(const std::string &prop) const
{
    auto it = m_properties.find(prop);
    if (it != m_properties.end())
    {
        return it->second;
    }
    return "";
}

//! Get floating point model property
double data::CModel::getFloatProperty(const std::string &prop) const
{
    std::string val = getProperty(prop);
    return strtod(val.c_str(), NULL);
}

//! Get int model property
long data::CModel::getIntProperty(const std::string &prop) const
{
    std::string val = getProperty(prop);
    return strtol(val.c_str(), NULL, 10);
}

//! Set model property
void data::CModel::setProperty(const std::string &prop, const std::string &value)
{
    m_properties[prop] = value;
}

//! Set int property
void data::CModel::setIntProperty(const std::string &prop, int value)
{
    std::stringstream ss;
    ss << value;
    m_properties[prop] = ss.str();
}

//! Set floating point property
void data::CModel::setFloatProperty(const std::string &prop, double value)
{
    std::stringstream ss;
    ss << value;
    m_properties[prop] = ss.str();
}

//! Clear all properties
void data::CModel::clearAllProperties()
{
    m_properties.clear();
}

//! Copy all properties
void data::CModel::copyAllProperties(const data::CModel & model)
{
    m_properties = model.m_properties;
}

//! Direct access to all properties
const std::map<std::string, std::string> &data::CModel::getAllProperties() const
{
    return m_properties;
}

//! Clear the model.
void data::CModel::clear()
{
    m_spModel = new geometry::CMesh;
    m_spArmature = new geometry::CArmature;
    m_mesh_dirty = true;
}

geometry::CArmature *data::CModel::getArmature()
{
    return m_spArmature.get();
}

void data::CModel::setArmature(geometry::CArmature *armature)
{
    m_spArmature = armature;
}

void data::CModel::buildNames(geometry::CBone *bone, int &name, std::map<geometry::CBone *, int> &names)
{
    names[bone] = name++;
    const std::vector<geometry::CBone *> &children = bone->getChildren();
    for (int i = 0; i < children.size(); ++i)
    {
        buildNames(children[i], name, names);
    }
}

void data::CModel::buildInverseNames(geometry::CBone *bone, int &name, std::map<int, geometry::CBone *> &names)
{
    names[name++] = bone;
    const std::vector<geometry::CBone *> &children = bone->getChildren();
    for (int i = 0; i < children.size(); ++i)
    {
        buildInverseNames(children[i], name, names);
    }
}

const osg::Matrix &data::CModel::getTransformationMatrixBeforeReposition()
{
    return m_transformationMatrixBeforeReposition;
}

//! Sets transformation matrix before reposition
void data::CModel::setTransformationMatrixBeforeReposition(const osg::Matrix& matrix)
{
    m_transformationMatrixBeforeReposition = matrix;
}

//! Returns transformation matrix after reposition
const osg::Matrix &data::CModel::getTransformationMatrixAfterReposition()
{
    return m_transformationMatrixAfterReposition;
}

//! Sets transformation matrix after reposition
void data::CModel::setTransformationMatrixAfterReposition(const osg::Matrix& matrix)
{
    m_transformationMatrixAfterReposition = matrix;
}

bool data::CModel::isMirrored()
{
    return m_bMirrored;
}

void data::CModel::setMirrored(bool mirrored)
{
    m_bMirrored = mirrored;
}

bool data::CModel::isExamined()
{
    return m_bExamined;
}

void data::CModel::setExamined(bool examined)
{
    m_bExamined = examined;
}

bool data::CModel::isLocked() const
{
    return m_locked;
}

void data::CModel::setLocked(bool locked)
{
    m_locked = locked;
}

void data::CModel::setTextures(const std::map<std::string, vpl::img::CRGBAImage::tSmartPtr> &textures)
{
    m_textures = textures;
}

const std::map<std::string, vpl::img::CRGBAImage::tSmartPtr> &data::CModel::getTextures() const
{
    return m_textures;
}

const bool data::CModel::hasTextures() const
{
    return !m_textures.empty();
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

    //if model was empty when snapshot was taken -> reset it
    //this caused problems when using mesh cut: in the middle of scene was 1 triangle left from this model
    if (meshSnapshot->getMesh().n_vertices() == 0)
    {
        this->init();
        APP_STORAGE.invalidate(spModel.getEntryPtr(), data::StorageEntry::UNDOREDO);
    }

    geometry::CMesh *mesh = m_spModel;
    if (mesh == NULL)
    {
        mesh = new geometry::CMesh;
        setMesh(mesh);
    }
    mesh->clear();

    *mesh = meshSnapshot->getMesh();

    //if model was hidden when the snapshot was taken, don't show it
    //this caused problems, when redo was applied and some models were hidden before -> redo made all models visible
    if (mesh->n_vertices() > 0 && spModel->isVisible())
    {
        spModel->setVisibility(true);
    }

    // m_transformationMatrix = meshSnapshot->transformMatrix; // NOTE: this shouldn't be here, snapshot of other model properties should be done via model manager

    m_mesh_dirty = true;

    // invalidate
    APP_STORAGE.invalidate(spModel.getEntryPtr(), data::StorageEntry::UNDOREDO);
}

data::CSnapshot *data::CModel::getSnapshot(CSnapshot *snapshot)
{
    CMeshSnapshot *s = new CMeshSnapshot(this);

    const geometry::CMesh *mesh = getMesh();
    if (mesh == NULL || mesh->n_vertices() == 0)
    {
        return s;
    }

    s->setMesh(*mesh);

    // s->transformMatrix = m_transformationMatrix; // NOTE: this shouldn't be here, snapshot of other model properties should be done via model manager
    return s;
}

/**
 * Gets an up down surface areas.
 *
 * \param [in,out]  up_area     The up area.
 * \param [in,out]  down_area   The down area.
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
    geometry::CMesh::Normal om_zaxis(osg_zaxis[0], osg_zaxis[1], osg_zaxis[2]);

    for (geometry::CMesh::FaceIter fit = m_spModel->faces_begin(); fit != m_spModel->faces_end(); ++fit)
    {
        // Calculate projection of the face normal to the reoriented z-axis
        geometry::CMesh::Normal normal(m_spModel->normal(fit.handle()));
        float dp(normal | om_zaxis);

        // Calculate face area
        float a(m_spModel->area(fit.handle()));

        if (dp > 0.0)
        {
            up_area += a;
        }
        else
        {
            down_area += a;
        }
    }
}

osg::Matrix data::CModel::calculateSurfaceAlignedPositionMatrix(const osg::Vec3 &world_coordinates_point, float surface_offset, const osg::Matrix &additional_to_model_to_world /*= osg::Matrix::identity()*/)
{
    // Test if model is valid
    if (m_spModel.get() == nullptr || m_spModel->n_faces() == 0)
    {
        return osg::Matrix::identity();
    }

    // Get mesh
    geometry::CMesh *mesh(m_spModel.get());

    // Update mesh normals if not done yet
    mesh->request_face_normals();
    mesh->update_normals();

    // Try to get octree
    mesh->updateOctree();
    geometry::CMeshOctree *octree = mesh->getOctree();
    if (octree == NULL)
    {
        mesh->updateOctree();
        octree = mesh->getOctree();
        if (NULL == octree)
        {
            return osg::Matrix::identity();
        }
    }

    // Recompute world matrices
    osg::Matrix model_to_world_matrix = m_transformationMatrix;

    osg::Matrix world_to_model_matrix = osg::Matrix::inverse(model_to_world_matrix);

    // Get near points
    osg::Vec3 local_point(world_coordinates_point * world_to_model_matrix);
    const std::vector<geometry::CMeshOctreeNode *> &nodes = octree->getNearPoints(local_point, 50.0);
    if (nodes.size() == 0)
    {
        return osg::Matrix::identity();
    }

    // Find the closest one
    geometry::CMesh::VHandle nearest_handle(geometry::CMesh::InvalidVertexHandle);
    double dist_nearestsq(std::numeric_limits<double>::max());

    // For all returned nodes
    std::vector<geometry::CMeshOctreeNode *>::const_iterator itn(nodes.begin()), itnEnd(nodes.end());
    for (; itn != itnEnd; ++itn)
    {
        // For all node points
        std::vector<geometry::CMesh::VertexHandle>::const_iterator itvh((*itn)->vertices.begin()), itvhEnd((*itn)->vertices.end());
        for (; itvh != itvhEnd; ++itvh)
        {
            // Get point coordinates
            geometry::CMesh::Point mp = mesh->point(*itvh);

            // Check distance
            double dx(local_point[0] - mp[0]), dy(local_point[1] - mp[1]), dz(local_point[2] - mp[2]);
            double dsq(dx*dx + dy*dy + dz*dz);

            // Compare current nearest point with current point distance
            if (dsq < dist_nearestsq)
            {
                // Store handle and squared distance
                nearest_handle = *itvh;
                dist_nearestsq = dsq;
            }
        }
    }

    if (nearest_handle == geometry::CMesh::InvalidVertexHandle)
    {
        return osg::Matrix::identity();
    }

    // Get mesh point
    osg::Vec3 nearest_point(geometry::convert3<osg::Vec3, geometry::CMesh::Normal>(mesh->point(nearest_handle)));

    // Get mesh normal in that place transformed to the 
    osg::Vec3 normal(calculateAverageVertexNormal(nearest_handle));
    normal.normalize();

    // World to model and model to world matrix modified with additional matrix - this thing is needed for orthodontics, where 
    // model is not aligned to ground sometimes and another matrix must be used.
    osg::Matrix modified_model_to_world_matrix(model_to_world_matrix * additional_to_model_to_world);
    osg::Matrix modified_world_to_model_matrix(osg::Matrix::inverse(modified_model_to_world_matrix));

    osg::Vec3 up = osg::Vec3(0.0, 0.0, 1.0) * modified_world_to_model_matrix - modified_world_to_model_matrix.getTrans();
    up.normalize();

    // Offset nearest point from the surface
    nearest_point += normal * surface_offset;

    // Test if normal is the same as up vector and modify it in that case
    if (fabs(normal * up) < std::numeric_limits<double>::min())
    {
        normal = osg::Vec3(0.0, 1.0, 0.0);
    }

    // Compute last needed vector
    osg::Vec3 right(normal ^ up);
    right.normalize();

    // Recompute up vector to be orthogonal on up and normal
    up = right ^ normal;
    up.normalize();

    // #NOTE This orients up axis always the same way. 
    if ((up * modified_model_to_world_matrix - modified_model_to_world_matrix.getTrans())[2] < 0.0)
    {
        up = -up;
    }
    
    // Finalize position matrix
    osg::Matrix position = 
        osg::Matrix(
            right[0], right[1], right[2], 0.0,
            normal[0], normal[1], normal[2], 0.0,
            up[0], up[1], up[2], 0.0,
            nearest_point[0], nearest_point[1], nearest_point[2], 1.0)
        * model_to_world_matrix;

    return position;
}

osg::Vec3 data::CModel::calculateAverageVertexNormal(geometry::CMesh::VHandle vertex_handle)
{
    // Test if model is valid
    if (m_spModel.get() == nullptr || m_spModel->n_faces() == 0)
    {
        return osg::Vec3(0.0, 0.0, 1.0);
    }

    // Get mesh
    geometry::CMesh *mesh(m_spModel.get());

    // Update mesh normals if not done yet
    mesh->request_vertex_normals();
    mesh->update_normals();

    geometry::CMesh::Normal normal = mesh->normal(vertex_handle);
    for (geometry::CMesh::ConstVertexVertexIter it = mesh->vv_iter(vertex_handle); it.is_valid(); ++it)
    {
        normal += mesh->normal(it);
    }

    normal.normalize();

    return geometry::convert3<osg::Vec3, geometry::CMesh::Normal>(normal);
}

/*!
* \fn  double getNearestModelPoint(const geometry::Vec3 &point, geometry::Vec3 &closest_point, bool bPrecise = true);
*
* \brief   Gets nearest model point
*
* \param           point           The query point.
* \param [in,out]  closest_point   The closest point.
* \param           bPrecise        (Optional) True if to calculate precise point on the surface or just closest vertex of the mesh.
*
* \return  The nearest model point distance. If no point found, returns std::numeric_limits<double>::max().
*
*/
double data::CModel::getNearestModelPoint(const geometry::Vec3 &point, geometry::Vec3 &closest_point, bool add_scene_shift_to_matrix, bool bPrecise /*= true*/)
{
    if (!updateKDTree())
    {
        return std::numeric_limits<double>::max();
    }

    SClosestData cd;
    getNearestModelPoint(point, cd, add_scene_shift_to_matrix);
    if (bPrecise)
    {
        closest_point = cd.precise_point;
        return cd.precise_distance;
    }

    closest_point = cd.point;
    return cd.distance;
}

/*!
* \fn  double getNearestModelPoint(const geometry::Vec3 &point, SClosestData &data, bool bPrecise = true);
*
* \brief   The same as above but SClosestData contains more information. Gets nearest model point - closest vertex or precise point on the surface.
*
* \param           point       The query point.
* \param [in,out]  data        The found point data.
* \param           add_scene_shift_to_matrix   True to add scene shift to the positioning matrix - without this the resulting coordinates will be in the world space.
* \param           bPrecise    (Optional) True if to calculate precise point on the surface or just closest vertex of the mesh.
*
* \return  The nearest model point. If no point found, returns std::numeric_limits<double>::max().
*/
double data::CModel::getNearestModelPoint(const geometry::Vec3 &point, SClosestData &data, bool add_scene_shift_to_matrix, bool bPrecise /*= true*/, bool localCoordsOnly/* = false*/)
{
    if (!updateKDTree())
    {
        return std::numeric_limits<double>::max();
    }

    // Create answer array
    geometry::CKDTreeOM::SIndexDistancePairs result(1);

    geometry::Matrix worldMatrix = geometry::Matrix::identity();

    if (!localCoordsOnly)
    {
        worldMatrix = geometry::convert4x4T<geometry::Matrix>(m_transformationMatrix);
    }

    if (add_scene_shift_to_matrix)
    {
        // Get recalculation object
        data::CCoordinatesConv CoordConv = VPL_SIGNAL(SigGetActiveConvObject).invoke2();
        osg::Vec3 scene_shift(CoordConv.getSceneShiftX(), CoordConv.getSceneShiftY(), CoordConv.getSceneShiftZ());

        // Recompute world matrices
        worldMatrix = geometry::convert4x4T<geometry::Matrix, osg::Matrix>(m_transformationMatrix * osg::Matrix::translate(scene_shift));
    }

    geometry::Matrix invWorldMatrix(geometry::Matrix::inverse(worldMatrix));

    // Compute point in the model space
    geometry::Vec3 model_point = invWorldMatrix * point;

    // Try to find closest point
    if (!m_kd_tree.getClosestPoints(geometry::convert3<geometry::CKDTreeOM::tVec, geometry::Vec3>(model_point), result))
    {
        return std::numeric_limits<double>::max();
    }

    // Store nearest vertex handle
    data.vhandle = m_kd_tree.getPointVH(result.indexes.front());

    // Test if handle is valid and element is not deleted...
    if (!m_spModel->is_valid_handle(data.vhandle) || m_spModel->status(data.vhandle).deleted())
        return std::numeric_limits<double>::max();

    if (bPrecise)
    {
        double best_distance(std::numeric_limits<double>::max());
        geometry::CMesh::FHandle best_handle;
        geometry::Vec3 best_point;

        // For all adjacent faces
        for (geometry::CMesh::ConstVertexFaceIter vfit(m_spModel->cvf_iter(data.vhandle)); vfit; ++vfit)
        {
            // For all (at least three) face vertices
            int i = 0;
            std::array<geometry::Vec3, 3> vertices;

            for (geometry::CMesh::ConstFaceVertexIter fvit = m_spModel->cfv_begin(vfit); fvit != m_spModel->cfv_end(vfit); ++fvit, ++i)
            {
                assert(i < 3);
                vertices[i] = geometry::convert3<geometry::Vec3, geometry::CMesh::Point>(m_spModel->point(fvit.handle()));
            }

            if (i < 3)
            {
                continue; // Just for sure
            }

            // Try to find closest triangle point
            geometry::Vec3 closest = worldMatrix * getClosestTrianglePoint(vertices[0], vertices[1], vertices[2], model_point);

            double closest_distance = (closest - point).length2();

            if (closest_distance < best_distance)
            {
                best_point = closest;
                best_handle = vfit.handle();
                best_distance = closest_distance;
            }
        }

        if (m_spModel->is_valid_handle(best_handle))
        {
            data.precise_point = best_point;
            data.precise_distance = best_distance;
            data.fhandle = best_handle;
            data.precise_normal = worldMatrix * geometry::convert3<geometry::Vec3, geometry::CMesh::Normal>(m_spModel->normal(data.fhandle)) - worldMatrix.getTrans<geometry::Vec3::tElement>();
        }
    }

    // Store result point
    data.point = geometry::Vec3(m_kd_tree.getPointVPL(result.indexes.front()));
    data.point = worldMatrix * data.point;

    data.normal = m_kd_tree.getNormalVPL(result.indexes.front());
    data.normal = worldMatrix * data.normal - worldMatrix.getTrans<geometry::Vec3::tElement>();

    data.distance = (point - data.point).length2();

    // Return its distance
    return data.distance;
}

bool data::CModel::updateKDTree()
{
    if(m_mesh_dirty)
    {
        if (m_spModel.get() != nullptr)
        {
            m_spModel->garbage_collection();

            // We will need face and vertex normals for precise point computation
            m_spModel->request_face_normals();
            m_spModel->request_vertex_normals();

            // let the mesh update the normals
            m_spModel->update_normals();

            m_kd_tree.init(*m_spModel);
        }
        else
        {
            m_kd_tree.clear();
        }

        m_mesh_dirty = false;
    }

    return m_kd_tree.hasData();
}

template <typename T>
T clamp(T in, T low, T high)
{
    return std::min(std::max(in, low), high);
}

/**
 * \fn  geometry::Vec3 data::CGuideCurve::getClosestTrianglePoint(const geometry::Vec3 &t0, const geometry::Vec3 &t1, const geometry::Vec3 &t2, const geometry::Vec3 &point) const
 *
 * \brief   Gets closest triangle point. Based on:
 *          David Eberly
 *          Geometric Tools, LLC
 *          http://www.geometrictools.com/
 *          Copyright
 *          c 1998-2016. All Rights Reserved.
 *
 * \param   t0      The first triangle point.
 * \param   t1      The second triangle point.
 * \param   t2      The third triangle point.
 * \param   point   The source point (point that we calculate nearest triangle point for).
 *
 * \return  The closest triangle point.
 */
geometry::Vec3 data::CModel::getClosestTrianglePoint(const geometry::Vec3 &t0, const geometry::Vec3 &t1, const geometry::Vec3 &t2, const geometry::Vec3 &point) const
{
    // Calculate geometry edge vectors
    geometry::Vec3 edge0 = t1 - t0;
    geometry::Vec3 edge1 = t2 - t0;

    // Vector from the first triangle point to the source point
    geometry::Vec3 v0 = t0 - point;

    // COefficients
    float a = edge0 * edge0;
    float b = edge0 * edge1;
    float c = edge1 * edge1;
    float d = edge0 * v0;
    float e = edge1 * v0;

    float det = a*c - b*b;
    float s = b*e - c*d;
    float t = b*d - a*e;

    if (s + t < det)
    {
        if (s < 0.f)
        {
            if (t < 0.f)
            {
                if (d < 0.f)
                {
                    s = clamp(-d / a, 0.f, 1.f);
                    t = 0.f;
                }
                else
                {
                    s = 0.f;
                    t = clamp(-e / c, 0.f, 1.f);
                }
            }
            else
            {
                s = 0.f;
                t = clamp(-e / c, 0.f, 1.f);
            }
        }
        else if (t < 0.f)
        {
            s = clamp(-d / a, 0.f, 1.f);
            t = 0.f;
        }
        else
        {
            float invDet = 1.f / det;
            s *= invDet;
            t *= invDet;
        }
    }
    else
    {
        if (s < 0.f)
        {
            float tmp0 = b + d;
            float tmp1 = c + e;
            if (tmp1 > tmp0)
            {
                float numer = tmp1 - tmp0;
                float denom = a - 2 * b + c;
                s = clamp(numer / denom, 0.f, 1.f);
                t = 1 - s;
            }
            else
            {
                t = clamp(-e / c, 0.f, 1.f);
                s = 0.f;
            }
        }
        else if (t < 0.f)
        {
            if (a + d > b + e)
            {
                float numer = c + e - b - d;
                float denom = a - 2 * b + c;
                s = clamp(numer / denom, 0.f, 1.f);
                t = 1 - s;
            }
            else
            {
                s = clamp(-e / c, 0.f, 1.f);
                t = 0.f;
            }
        }
        else
        {
            float numer = c + e - b - d;
            float denom = a - 2 * b + c;
            s = clamp(numer / denom, 0.f, 1.f);
            t = 1.f - s;
        }
    }

    return t0 + geometry::Scalar(s) * edge0 + geometry::Scalar(t) * edge1;
}

void data::CModel::createAndStoreSnapshot(CSnapshot *childSnapshot /*= NULL*/)
{
    if (childSnapshot == NULL)
    {
        VPL_SIGNAL(SigUndoSnapshot).invoke(this->getSnapshot(NULL));
    }
    else
    {
        CSnapshot *snapshot = this->getSnapshot(NULL);
        snapshot->addSnapshot(childSnapshot);
        VPL_SIGNAL(SigUndoSnapshot).invoke(snapshot);
    }
}

void data::CModel::getDeformedMesh(geometry::CMesh &mesh, bool keepProperties)
{
    geometry::CMesh &srcMesh = *m_spModel.get();

    mesh.clear();
    mesh.garbage_collection();

    std::map<geometry::CMesh::VertexHandle, geometry::CMesh::VertexHandle> vertexMap;

    if (keepProperties)
    {
        mesh = srcMesh;

        for (geometry::CMesh::VertexIter vit = srcMesh.vertices_begin(); vit != srcMesh.vertices_end(); ++vit)
        {
            vertexMap[vit.handle()] = vit.handle();
        }
    }
    else
    {
        for (geometry::CMesh::VertexIter vit = srcMesh.vertices_begin(); vit != srcMesh.vertices_end(); ++vit)
        {
            vertexMap[vit.handle()] = mesh.add_vertex(srcMesh.point(vit.handle()));
        }
        for (geometry::CMesh::FaceIter fit = srcMesh.faces_begin(); fit != srcMesh.faces_end(); ++fit)
        {
            geometry::CMesh::VertexHandle vertices[3];
            int i = 0;
            for (geometry::CMesh::FaceVertexIter fvit = srcMesh.fv_begin(fit.handle()); fvit != srcMesh.fv_end(fit.handle()); ++fvit)
            {
                vertices[i++] = fvit.handle();
            }
            mesh.add_face(vertexMap[vertices[0]], vertexMap[vertices[1]], vertexMap[vertices[2]]);
        }
    }

    std::vector<geometry::Matrix> matrices;
    geometry::CBone::gatherMatrices(matrices, m_spArmature);

    bool emptyArmature = std::find_if(matrices.begin(), matrices.end(), [](const geometry::Matrix& m) {return !m.asEigen().isIdentity(); }) == matrices.end();

    OpenMesh::VPropHandleT<geometry::CMesh::CVertexGroups> vProp_vertexGroups;
    if (!srcMesh.get_property_handle(vProp_vertexGroups, VERTEX_GROUPS_PROPERTY_NAME) || emptyArmature)
    {
        return;
    }

    for (geometry::CMesh::VertexIter vit = srcMesh.vertices_begin(); vit != srcMesh.vertices_end(); ++vit)
    {
        const geometry::CMesh::CVertexGroups &vertexGroups = srcMesh.property(vProp_vertexGroups, vit.handle());
        geometry::CMesh::Point &point = mesh.point(vertexMap[vit.handle()]);
        geometry::Vec4 vertex = geometry::Vec4(point[0], point[1], point[2], 1.0);

        geometry::Vec4 skinnedVertex = geometry::Vec4(0.0, 0.0, 0.0, 0.0);
        for (int i = 0; i < vertexGroups.GROUP_COUNT; ++i)
        {
            if (vertexGroups.indices[i] != -1)
            {
                skinnedVertex += (matrices[vertexGroups.indices[i]] * vertex) * geometry::Scalar(vertexGroups.weights[i]);
            }
        }
        skinnedVertex[3] = 1.0;

        point = geometry::CMesh::Point(skinnedVertex[0], skinnedVertex[1], skinnedVertex[2]);
    }
}

data::CMeshSnapshot::CMeshSnapshot(CUndoProvider *provider)
    : CSnapshot(data::UNDO_MODELS, provider)
{ }

data::CMeshSnapshot::~CMeshSnapshot()
{ }

long data::CMeshSnapshot::getDataSize()
{
    return sizeof(CMeshSnapshot) + 
        (sizeof(geometry::CMesh::Point) + sizeof(geometry::CMesh::VertexHandle)) * m_mesh.n_vertices() +
        (sizeof(int) * 3 + sizeof(geometry::CMesh::FaceHandle)) * m_mesh.n_faces();
}

