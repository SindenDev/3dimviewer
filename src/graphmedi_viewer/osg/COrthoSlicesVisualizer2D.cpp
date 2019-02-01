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

#include <osg/COrthoSlicesVisualizer2D.h>
#include <geometry/base/types.h>
#include <geometry/base/functions.h>
#include <graph/osg/NodeMasks.h>
#include <data/CActiveDataSet.h>
#include <data/COrthoSlice.h>
#include <osg/CThickLineMaterial.h>
#include <data/CArbitrarySlice.h>
#include <graph/osg/CGeometryGenerator.h>


osg::COrthoSlicesVisualizer2D::COrthoSlicesVisualizer2D(OSGCanvas *pCanvas, int id, ESceneType sceneType)
    : CActiveObjectBase("OrthoSlicesVisualizer2D")
    , osg::CDraggableGeometry(osg::Matrix::identity(), id)
    , m_sceneType(sceneType)
    , m_slicePlaneNormal(1.0, 0.0, 0.0)
    , m_slicePlanePoint(0.0, 0.0, 0.0)
{
    setCanvas(pCanvas);

    m_geometryCallback->setFinishSignalEnabled(true);

    m_sliceGeometry = new osg::Geode;
    m_transDraggerGeometry = new osg::Geode;

    this->addChild(m_sliceGeometry);

    this->setupObserver(this);
    this->dirtyBound();

    m_prevMatrix = osg::Matrix::identity();

    createDraggers();

    // App mode changed signal connection
    m_conAppModeChanged = APP_MODE.getModeChangedSignal().connect(this, &osg::COrthoSlicesVisualizer2D::onModeChanged);

    setNodeMask(MASK_ORTHO_2D_DRAGGER);
}

osg::COrthoSlicesVisualizer2D::~COrthoSlicesVisualizer2D()
{
    freeObserver(this);
    APP_MODE.getModeChangedSignal().disconnect(m_conAppModeChanged);
}

void osg::COrthoSlicesVisualizer2D::createDraggers()
{
    osg::Vec3 normal;

    if (m_sceneType == EST_XY)
    {
        normal = osg::Vec3(0.0, 0.0, 1.0);
    }
    else if(m_sceneType == EST_XZ)
    {
        normal = osg::Vec3(0.0, 1.0, 0.0);
    }
    else
    {
        normal = osg::Vec3(1.0, 0.0, 0.0);
    }

    m_translatePlaneDragger = new osgManipulator::CDraggerPlane(m_transDraggerGeometry, osg::Plane(normal, osg::Vec3(0.0, 0.0, 0.0)));

    //final composite dragger
    m_compositeDragger = new osgManipulator::CDraggerBaseComposite();
    m_compositeDragger->setName("2D ortho dragger");
    m_compositeDragger->addDragger(m_translatePlaneDragger);

    CDraggableGeometry::addDragger(m_compositeDragger);

    prepareDraggersMaterials();

    showDraggers(false);

    // Set geometry node mask
    setNodeMask(MASK_VISIBLE_OBJECT);

    getSignal().connect(this, &COrthoSlicesVisualizer2D::sigCommandFromDG);

    updateGeometry();
}

void osg::COrthoSlicesVisualizer2D::updateDraggerSize()
{

}

void osg::COrthoSlicesVisualizer2D::updateDraggers()
{
    this->relocate(osg::Matrix::identity());
    updateDraggerSize();
}

void osg::COrthoSlicesVisualizer2D::updateGeometry()
{
    updateDraggers();

    m_sliceGeometry->removeChildren(0, m_sliceGeometry->getNumChildren());
    m_transDraggerGeometry->removeChildren(0, m_transDraggerGeometry->getNumChildren());

    data::CObjectPtr<data::CArbitrarySlice> spSliceArb(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id, data::Storage::NO_UPDATE));

    osg::Vec3 arbSliceNormal(spSliceArb->getPlaneNormal());
    osg::Vec3 arbSliceRight(spSliceArb->getPlaneRight());
    osg::Vec3 arbSliceCenter(spSliceArb->getPlaneCenter());

    geometry::Vec3 orthoSliceNormal;
    geometry::Vec3 orthoSlicePoint;

    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));

    if (m_sceneType == EST_XY)
    {
        orthoSliceNormal = geometry::Vec3(0.0, 0.0, 1.0);
        data::CObjectPtr<data::COrthoSliceXY> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXY::Id, data::Storage::NO_UPDATE));
        orthoSlicePoint = geometry::Vec3(spVolume->getXSize() * spVolume->getDX() + 0.5, spVolume->getYSize() * spVolume->getDY() + 0.5, spSlice->getPosition() * spVolume->getDZ());
    }
    else if (m_sceneType == EST_XZ)
    {
        orthoSliceNormal = geometry::Vec3(0.0, 1.0, 0.0);
        data::CObjectPtr<data::COrthoSliceXZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id, data::Storage::NO_UPDATE));
        orthoSlicePoint = geometry::Vec3(spVolume->getXSize() * spVolume->getDX() + 0.5, spSlice->getPosition() * spVolume->getDY(), spVolume->getZSize() * spVolume->getDZ() + 0.5);
    }
    else
    {
        orthoSliceNormal = geometry::Vec3(1.0, 0.0, 0.0);
        data::CObjectPtr<data::COrthoSliceYZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id, data::Storage::NO_UPDATE));
        orthoSlicePoint = geometry::Vec3(spSlice->getPosition() * spVolume->getDX(), spVolume->getYSize() * spVolume->getDY() + 0.5, spVolume->getZSize() * spVolume->getDZ() + 0.5);
    }

    geometry::CPlane arbSlicePlane(geometry::convert3<geometry::Vec3, osg::Vec3>(arbSliceNormal), geometry::convert3<geometry::Vec3, osg::Vec3>(arbSliceCenter));
    geometry::CPlane orthoSlicePlane(orthoSliceNormal, orthoSlicePoint);

    arbSlicePlane.makeUnitLength();
    orthoSlicePlane.makeUnitLength();

    geometry::Vec3 intersectLineDir = orthoSlicePlane.getNormal() ^ arbSlicePlane.getNormal();

    double det = intersectLineDir.length2();

    if (det != 0)
    {
        geometry::Vec3 pointOnIntersectLine = (((intersectLineDir ^ arbSlicePlane.getNormal()) * vpl::CScalar<double>(orthoSlicePlane.getPlaneParameters()[3])) +
            ((orthoSlicePlane.getNormal() ^ intersectLineDir) * vpl::CScalar<double>(arbSlicePlane.getPlaneParameters()[3]))).operator/(vpl::CScalar<double>(det));

        osg::Vec3 p1, p2;
        getGeometryPoints(geometry::convert3<osg::Vec3, geometry::Vec3>(pointOnIntersectLine), geometry::convert3<osg::Vec3, geometry::Vec3>(intersectLineDir), p1, p2);

        osg::Vec3 up, right, forward;

        up = arbSliceRight ^ arbSliceNormal;
        right = arbSliceRight;
        forward = up ^ right;

        up.normalize();
        right.normalize();
        forward.normalize();

        osg::Matrix transformation(osg::Matrix(
            up[0], up[1], up[2], 0,
            right[0], right[1], right[2], 0,
            forward[0], forward[1], forward[2], 0,
            arbSliceCenter[0], arbSliceCenter[1], arbSliceCenter[2], 1
        ));

        p1 = p1 * osg::Matrix::inverse(transformation);
        p2 = p2 * osg::Matrix::inverse(transformation);

        osg::Vec3Array *vertices = new osg::Vec3Array(2);

        vertices->operator[](0) = p1;
        vertices->operator[](1) = p2;

        osg::Vec3Array *colors = new osg::Vec3Array(1);

        osg::Vec3d eye(0.0, 0.0, 1.0);
        osg::Vec3 t1 = osg::Vec3(0.0, 0.0, 0.0);
        osg::Vec3 t2;

        if (m_sceneType == EST_XY)
        {
            colors->operator[](0) = osg::Vec3(0.0, 0.0, 1.0);
            t2 = osg::Vec3(0.0, 0.0, spVolume->getZSize() * spVolume->getDZ());
        }
        else if (m_sceneType == EST_XZ)
        {
            colors->operator[](0) = osg::Vec3(0.0, 1.0, 0.0);
            t2 = osg::Vec3(0.0, spVolume->getYSize() * spVolume->getDY(), 0.0);
        }
        else
        {
            colors->operator[](0) = osg::Vec3(1.0, 0.0, 0.0);
            t2 = osg::Vec3(spVolume->getXSize() * spVolume->getDX(), 0.0, 0.0);
        }

        t1 = t1 * osg::Matrix::inverse(transformation);
        t2 = t2 * osg::Matrix::inverse(transformation);
        osg::Vec3 normal = t2 - t1;
        normal.normalize();

        m_slicePlaneNormal = geometry::convert3<geometry::Vec3, osg::Vec3>(normal);
        m_slicePlanePoint = geometry::convert4x4T<geometry::Matrix, osg::Matrix>(osg::Matrix::inverse(transformation)) * pointOnIntersectLine;

        osg::Vec3 orthoToLine = p2 - p1;
        orthoToLine.normalize();
        orthoToLine = orthoToLine ^ eye;
        orthoToLine.normalize();

        osg::Vec3 voxelSize = spSliceArb->getSliceVoxelSize();
        double scalar = std::fabs(osg::Vec3(1.0, 0.0, 0.0) * orthoToLine);

        if (scalar < 0.5)
        {
            double a = voxelSize[0];
            double b = voxelSize[1] * 2 * scalar;
        }
        else
        {
            double a = voxelSize[1];
            double b = voxelSize[0] * 2 * scalar;
        }

        // Create line geometry
        osg::Geometry *sliceGeometry = new osg::Geometry;
        sliceGeometry->setVertexArray(vertices);
        sliceGeometry->setColorArray(colors);
        sliceGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        sliceGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, vertices->size()));
        osg::CMaterialLines* material = new osg::CMaterialLines(getCanvas()->getView()->getCamera(), 2.0);
        material->apply(sliceGeometry);
        m_sliceGeometry->addDrawable(sliceGeometry);
        m_sliceGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

        osg::Vec3Array *draggerVertices = new osg::Vec3Array(4);
        draggerVertices->operator[](0) = p1 - (orthoToLine * vpl::CScalar<double>(2.0));
        draggerVertices->operator[](1) = p1 + (orthoToLine * vpl::CScalar<double>(2.0));
        draggerVertices->operator[](2) = p2 - (orthoToLine * vpl::CScalar<double>(2.0));
        draggerVertices->operator[](3) = p2 + (orthoToLine * vpl::CScalar<double>(2.0));

        // Create dragger line geometry
        osg::Geometry *draggerSliceGeometry = new osg::Geometry;
        draggerSliceGeometry->setVertexArray(draggerVertices);
        draggerSliceGeometry->setColorArray(colors);
        draggerSliceGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        osg::DrawElementsUInt* plane_ps = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 6);

        (*plane_ps)[0] = 0;
        (*plane_ps)[1] = 3;
        (*plane_ps)[2] = 1;

        (*plane_ps)[3] = 0;
        (*plane_ps)[4] = 2;
        (*plane_ps)[5] = 3;

        draggerSliceGeometry->addPrimitiveSet(plane_ps);

        osgManipulator::setDrawableToAlwaysCull(*draggerSliceGeometry);

        m_transDraggerGeometry->addDrawable(draggerSliceGeometry);
        /*osg::CPseudoMaterial* materialDragger = new osg::CPseudoMaterial();
        materialDragger->uniform("Diffuse")->set(osg::Vec3(1.0, 1.0, 1.0));
        materialDragger->uniform("Specularity")->set(0.0f);
        materialDragger->apply(draggerSliceGeometry);*/
        m_transDraggerGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    }
}

void osg::COrthoSlicesVisualizer2D::getGeometryPoints(const osg::Vec3& inPoint, const osg::Vec3& direction, osg::Vec3& outPoint1, osg::Vec3& outPoint2)
{
    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));

    vpl::tSize xSize = spVolume->getXSize();
    vpl::tSize ySize = spVolume->getYSize();
    vpl::tSize zSize = spVolume->getZSize();

    double dx = spVolume->getDX();
    double dy = spVolume->getDY();
    double dz = spVolume->getDZ();

    osg::Vec3 p1 = inPoint - (direction * 10000.0);

    std::vector<osg::Vec3> planeNormals;
    std::vector<osg::Vec3> planePoints;

    planeNormals.push_back(osg::Vec3(0, 0, 1));
    planeNormals.push_back(osg::Vec3(0, 1, 0));
    planeNormals.push_back(osg::Vec3(1, 0, 0));
    planeNormals.push_back(osg::Vec3(0, 0, -1));
    planeNormals.push_back(osg::Vec3(0, -1, 0));
    planeNormals.push_back(osg::Vec3(-1, 0, 0));

    planePoints.push_back(osg::Vec3(0, 0, 0));
    planePoints.push_back(osg::Vec3(0, 0, 0));
    planePoints.push_back(osg::Vec3(0, 0, 0));
    planePoints.push_back(osg::Vec3(xSize * dx, ySize * dy, zSize * dz));
    planePoints.push_back(osg::Vec3(xSize * dx, ySize * dy, zSize * dz));
    planePoints.push_back(osg::Vec3(xSize * dx, ySize * dy, zSize * dz));

    std::vector<osg::Vec3> intersections;

    for (int i = 0; i < planeNormals.size(); ++i)
    {
        float d = planeNormals[i] * planePoints[i];

        if (planeNormals[i] * direction != 0)
        {
            // Compute the X value for the directed line ray intersecting the plane
            float x = (d - (planeNormals[i] * p1)) / (planeNormals[i] * direction);

            // intersection
            osg::Vec3 intersection = p1 + (direction * x);
            if (intersection[0] + 0.001 >= 0 && intersection[0] - 0.001 <= xSize * dx && intersection[1] + 0.001 >= 0 && intersection[1] - 0.001 <= ySize * dy && intersection[2] + 0.001 >= 0 && intersection[2] - 0.001 <= zSize * dz)
            {
                if (std::find(intersections.begin(), intersections.end(), intersection) == intersections.end())
                {
                    intersections.push_back(intersection);
                }
            }
        }
    }

    outPoint1 = osg::Vec3(0.0, 0.0, 0.0);
    outPoint2 = osg::Vec3(0.0, 0.0, 0.0);

    if (intersections.size() == 2)
    {
        outPoint1 = intersections[0];
        outPoint2 = intersections[1];
    }
}

bool osg::COrthoSlicesVisualizer2D::sigCommandFromDG(const osg::Matrix & matrix, const osgManipulator::MotionCommand &command, int command_type, long dg_id)
{
    if (command.getStage() == osgManipulator::MotionCommand::START)
    {
        data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));

        m_slicePlane = geometry::CPlane(m_slicePlaneNormal, m_slicePlanePoint);

        if (m_sceneType == EST_XY)
        {
            data::CObjectPtr<data::COrthoSliceXY> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXY::Id));
            m_startPosition = spSlice->getPosition();
        }
        else if (m_sceneType == EST_XZ)
        {
            data::CObjectPtr<data::COrthoSliceXZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id));
            m_startPosition = spSlice->getPosition();
        }
        else
        {
            data::CObjectPtr<data::COrthoSliceYZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id));
            m_startPosition = spSlice->getPosition();
        }

        return true;
    }

    if (command.getStage() == osgManipulator::MotionCommand::FINISH)
    {
        updateGeometry();
        return true;
    }

    if (command.getStage() != osgManipulator::MotionCommand::MOVE)
    {
        return true;
    }

    if (command_type == osgManipulator::DraggerTransformCallback::HANDLE_TRANSLATE_IN_PLANE)
    {
        geometry::Quat rotation;
        geometry::Vec3 translation, scale;
        geometry::convert4x4T<geometry::Matrix, osg::Matrix>(matrix).decompose(translation, rotation, scale);

        data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));

        if (m_sceneType == EST_XY)
        {
            data::CObjectPtr<data::COrthoSliceXY> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXY::Id));
            geometry::Vec3 newCenter = m_slicePlane.getCenter() + translation;
            double shift = m_slicePlane.distance(newCenter) / spVolume->getDZ();
            double newPos = m_startPosition + shift;

            if (newPos < 0)
            {
                spSlice->setPosition(0);
                updateGeometry();
                APP_STORAGE.invalidate(spSlice.getEntryPtr());
                return true;
            }

            if (newPos > spVolume->getZSize())
            {
                spSlice->setPosition(spVolume->getZSize());
                updateGeometry();
                APP_STORAGE.invalidate(spSlice.getEntryPtr());
                return true;
            }

            m_prevMatrix = matrix;
            spSlice->setPosition(newPos);
            APP_STORAGE.invalidate(spSlice.getEntryPtr());
            updateGeometry();
            return true;
        }
        else if (m_sceneType == EST_XZ)
        {
            data::CObjectPtr<data::COrthoSliceXZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id));
            geometry::Vec3 newCenter = m_slicePlane.getCenter() + translation;
            double shift = m_slicePlane.distance(newCenter) / spVolume->getDY();
            double newPos = m_startPosition + shift;

            if (newPos < 0)
            {
                spSlice->setPosition(0);
                updateGeometry();
                APP_STORAGE.invalidate(spSlice.getEntryPtr());
                return true;
            }

            if (newPos > spVolume->getYSize())
            {
                spSlice->setPosition(spVolume->getYSize());
                updateGeometry();
                APP_STORAGE.invalidate(spSlice.getEntryPtr());
                return true;
            }

            m_prevMatrix = matrix;
            spSlice->setPosition(newPos);
            APP_STORAGE.invalidate(spSlice.getEntryPtr());
            updateGeometry();
            return true;
        }
        else
        {
            data::CObjectPtr<data::COrthoSliceYZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id));
            geometry::Vec3 newCenter = m_slicePlane.getCenter() + translation;
            double shift = m_slicePlane.distance(newCenter) / spVolume->getDX();
            double newPos = m_startPosition + shift;

            if (newPos < 0)
            {
                spSlice->setPosition(0);
                updateGeometry();
                APP_STORAGE.invalidate(spSlice.getEntryPtr());
                return true;
            }

            if (newPos > spVolume->getXSize())
            {
                spSlice->setPosition(spVolume->getXSize());
                updateGeometry();
                APP_STORAGE.invalidate(spSlice.getEntryPtr());
                return true;
            }

            m_prevMatrix = matrix;
            spSlice->setPosition(newPos);
            APP_STORAGE.invalidate(spSlice.getEntryPtr());
            updateGeometry();
            return true;
        }
    }

    return true;
}

void osg::COrthoSlicesVisualizer2D::prepareDraggersMaterials()
{
    osg::Vec3 colorPicked(1.0f, 1.0f, 0.6f);
    osg::Vec3 colorX(1.0f, 0.0f, 0.0f);

    if (m_translatePlaneDragger)
        m_translatePlaneDragger->setColors(colorX, colorPicked);
}

//=====================================================================================================================
void osg::COrthoSlicesVisualizer2D::updateFromStorage()
{
    updateGeometry();
}

void osg::COrthoSlicesVisualizer2D::onNewDensityData(data::CStorageEntry* entry, const data::CChangedEntries& changes)
{
    updateGeometry();
}

void osg::COrthoSlicesVisualizer2D::onArbSliceChanged(data::CStorageEntry* entry, const data::CChangedEntries& changes)
{

    if (changes.hasChanged(data::Storage::ArbitrarySlice::Id))
    {
        updateGeometry();
    }
}

void osg::COrthoSlicesVisualizer2D::onModeChanged(scene::CAppMode::tMode mode)
{
    if (APP_MODE.check(scene::CAppMode::MODE_SLICE_MOVE))
    {
        showDraggers(true);
    }
    else
    {
        showDraggers(false);
    }

    this->dirtyBound();
}