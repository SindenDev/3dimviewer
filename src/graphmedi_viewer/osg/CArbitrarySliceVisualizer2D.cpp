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

#include <osg/CArbitrarySliceVisualizer2D.h>
#include <geometry/base/types.h>
#include <geometry/base/functions.h>
#include <graph/osg/NodeMasks.h>
#include <data/CActiveDataSet.h>
#include <data/COrthoSlice.h>
#include <osg/CThickLineMaterial.h>
#include <data/CArbitrarySlice.h>

osg::CArbitrarySliceVisualizer2D::CArbitrarySliceVisualizer2D(OSGCanvas *pCanvas, int id, ESceneType sceneType)
    : CActiveObjectBase("ArbSliceVisualizer2D")
    , osg::CDraggableGeometry(osg::Matrix::identity(), id)
    , m_sceneType(sceneType)
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
    m_conAppModeChanged = APP_MODE.getModeChangedSignal().connect(this, &osg::CArbitrarySliceVisualizer2D::onModeChanged);
}

osg::CArbitrarySliceVisualizer2D::~CArbitrarySliceVisualizer2D()
{
    freeObserver(this);
    APP_MODE.getModeChangedSignal().disconnect(m_conAppModeChanged);
}

void osg::CArbitrarySliceVisualizer2D::createDraggers()
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
    m_compositeDragger->setName("2D arb dragger");
    m_compositeDragger->addDragger(m_translatePlaneDragger);

    CDraggableGeometry::addDragger(m_compositeDragger);

    prepareDraggersMaterials();

    showDraggers(false);

    // Set geometry node mask
    setNodeMask(MASK_VISIBLE_OBJECT);

    getSignal().connect(this, &CArbitrarySliceVisualizer2D::sigCommandFromDG);

    updateGeometry();
}

void osg::CArbitrarySliceVisualizer2D::updateDraggerSize()
{

}

void osg::CArbitrarySliceVisualizer2D::updateDraggers()
{
    this->relocate(osg::Matrix::identity());
    updateDraggerSize();
}

void osg::CArbitrarySliceVisualizer2D::updateGeometry()
{
    updateDraggers();

    m_sliceGeometry->removeChildren(0, m_sliceGeometry->getNumChildren());
    m_transDraggerGeometry->removeChildren(0, m_transDraggerGeometry->getNumChildren());

    data::CObjectPtr<data::CArbitrarySlice> spSliceArb(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id, data::Storage::NO_UPDATE));

    geometry::Vec3 arbSliceNormal = geometry::convert3<geometry::Vec3, osg::Vec3>(spSliceArb->getPlaneNormal());
    geometry::Vec3 arbSlicePoint = geometry::convert3<geometry::Vec3, osg::Vec3>(spSliceArb->getPlaneCenter());

    geometry::Vec3 orthoSliceNormal;
    geometry::Vec3 orthoSlicePoint;

    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2(), data::Storage::NO_UPDATE));

    if (m_sceneType == EST_XY)
    {
        orthoSliceNormal = geometry::Vec3(0.0, 0.0, 1.0);
        data::CObjectPtr<data::COrthoSliceXY> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXY::Id, data::Storage::NO_UPDATE));
        orthoSlicePoint = geometry::Vec3(0.0, 0.0, 0.0) + orthoSliceNormal * vpl::CScalar<double>(spSlice->getPosition() * spVolume->getDZ());
    }
    else if (m_sceneType == EST_XZ)
    {
        orthoSliceNormal = geometry::Vec3(0.0, 1.0, 0.0);
        data::CObjectPtr<data::COrthoSliceXZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id, data::Storage::NO_UPDATE));
        orthoSlicePoint = geometry::Vec3(0.0, 0.0, 0.0) + orthoSliceNormal * vpl::CScalar<double>(spSlice->getPosition() * spVolume->getDY());
    }
    else
    {
        orthoSliceNormal = geometry::Vec3(1.0, 0.0, 0.0);
        data::CObjectPtr<data::COrthoSliceYZ> spSlice(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id, data::Storage::NO_UPDATE));
        orthoSlicePoint = geometry::Vec3(0.0, 0.0, 0.0) + orthoSliceNormal * vpl::CScalar<double>(spSlice->getPosition() * spVolume->getDX());
    }

    geometry::CPlane arbSlicePlane(arbSliceNormal, arbSlicePoint);
    geometry::CPlane orthoSlicePlane(orthoSliceNormal, orthoSlicePoint);

    arbSlicePlane.makeUnitLength();
    orthoSlicePlane.makeUnitLength();

    geometry::Vec3 intersectLineDir = arbSlicePlane.getNormal() ^ orthoSlicePlane.getNormal();
    double det = intersectLineDir.length2();

    if (det != 0)
    {
        vpl::tSize xSize = spVolume->getXSize();
        vpl::tSize ySize = spVolume->getYSize();
        vpl::tSize zSize = spVolume->getZSize();

        double dx = spVolume->getDX();
        double dy = spVolume->getDY();
        double dz = spVolume->getDZ();

        geometry::Vec3 volumeShift(-0.5 * xSize * dx,
            -0.5 * ySize * dy,
            -0.5 * zSize * dz);

        geometry::Vec3 pointOnIntersectLine = (((intersectLineDir ^ orthoSlicePlane.getNormal()) * vpl::CScalar<double>(arbSlicePlane.getPlaneParameters()[3])) +
            ((arbSlicePlane.getNormal() ^ intersectLineDir) * vpl::CScalar<double>(orthoSlicePlane.getPlaneParameters()[3]))).operator/(vpl::CScalar<double>(det));

        osg::Vec3 p1, p2;
        bool done = getIntersectionsWithVolume(geometry::convert3<osg::Vec3, geometry::Vec3>(pointOnIntersectLine), geometry::convert3<osg::Vec3, geometry::Vec3>(intersectLineDir), p1, p2);

        if (!done)
        {
            return;
        }

        osg::Vec3 center = (p1 + p2) * 0.5;

        getRealArbSliceGeometryPoints(center, geometry::convert3<osg::Vec3, geometry::Vec3>(intersectLineDir), p1, p2);

        osg::Vec3Array *vertices = new osg::Vec3Array(2);

        vertices->operator[](0) = p1 + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift);
        vertices->operator[](1) = p2 + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift);

        osg::Vec3Array *colors = new osg::Vec3Array(1);
        colors->operator[](0) = osg::Vec3(1.0, 1.0, 0.0);

        // Create line geometry
        osg::Geometry *sliceGeometry = new osg::Geometry;
        sliceGeometry->setVertexArray(vertices);
        sliceGeometry->setColorArray(colors);
        sliceGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        sliceGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, vertices->size()));

        m_sliceGeometry->addDrawable(sliceGeometry);
        osg::CMaterialLines* material = new osg::CMaterialLines(getCanvas()->getView()->getCamera(), 2.0);
        material->apply(sliceGeometry);
        m_sliceGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

        osg::Vec3Array *draggerVertices = new osg::Vec3Array(8);

        osg::Vec3 orthoToLine = geometry::convert3<osg::Vec3, geometry::Vec3>(orthoSliceNormal ^ intersectLineDir);
        orthoToLine.normalize();

        osg::Vec3 frontTop = p1;
        osg::Vec3 backTop = p1;
        osg::Vec3 frontBottom = p2;
        osg::Vec3 backBottom = p2;

        if (m_sceneType == EST_XY)
        {
            frontTop[2] = -1;
            backTop[2] = zSize;
            frontBottom[2] = -1;
            backBottom[2] = zSize;
        }
        else if (m_sceneType == EST_XZ)
        {
            frontTop[1] = -1;
            backTop[1] = ySize;
            frontBottom[1] = -1;
            backBottom[1] = ySize;
        }
        else
        {
            frontTop[0] = -1;
            backTop[0] = xSize;
            frontBottom[0] = -1;
            backBottom[0] = xSize;
        }

        draggerVertices->operator[](0) = frontTop + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift) - orthoToLine * vpl::CScalar<double>(2.0);
        draggerVertices->operator[](1) = frontTop + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift) + orthoToLine * vpl::CScalar<double>(2.0);
        draggerVertices->operator[](2) = backTop + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift) + orthoToLine * vpl::CScalar<double>(2.0);
        draggerVertices->operator[](3) = backTop + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift) - orthoToLine * vpl::CScalar<double>(2.0);
        draggerVertices->operator[](4) = frontBottom + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift) - orthoToLine * vpl::CScalar<double>(2.0);
        draggerVertices->operator[](5) = frontBottom + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift) + orthoToLine * vpl::CScalar<double>(2.0);
        draggerVertices->operator[](6) = backBottom + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift) + orthoToLine * vpl::CScalar<double>(2.0);
        draggerVertices->operator[](7) = backBottom + geometry::convert3<osg::Vec3, geometry::Vec3>(volumeShift) - orthoToLine * vpl::CScalar<double>(2.0);

        // Create dragger line geometry
        osg::Geometry *draggerSliceGeometry = new osg::Geometry;
        draggerSliceGeometry->setVertexArray(draggerVertices);
        draggerSliceGeometry->setColorArray(colors);
        draggerSliceGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        osg::DrawElementsUInt* plane_ps = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 36);

        (*plane_ps)[0] = 0;
        (*plane_ps)[1] = 2;
        (*plane_ps)[2] = 1;

        (*plane_ps)[3] = 0;
        (*plane_ps)[4] = 3;
        (*plane_ps)[5] = 2;

        (*plane_ps)[6] = 4;
        (*plane_ps)[7] = 6;
        (*plane_ps)[8] = 5;

        (*plane_ps)[9] = 4;
        (*plane_ps)[10] = 7;
        (*plane_ps)[11] = 6;

        (*plane_ps)[12] = 0;
        (*plane_ps)[13] = 1;
        (*plane_ps)[14] = 5;

        (*plane_ps)[15] = 0;
        (*plane_ps)[16] = 5;
        (*plane_ps)[17] = 4;

        (*plane_ps)[18] = 1;
        (*plane_ps)[19] = 2;
        (*plane_ps)[20] = 5;

        (*plane_ps)[21] = 2;
        (*plane_ps)[22] = 6;
        (*plane_ps)[23] = 5;

        (*plane_ps)[24] = 2;
        (*plane_ps)[25] = 3;
        (*plane_ps)[26] = 7;

        (*plane_ps)[27] = 2;
        (*plane_ps)[28] = 7;
        (*plane_ps)[29] = 6;

        (*plane_ps)[30] = 3;
        (*plane_ps)[31] = 0;
        (*plane_ps)[32] = 7;

        (*plane_ps)[33] = 0;
        (*plane_ps)[34] = 4;
        (*plane_ps)[35] = 7;

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

bool osg::CArbitrarySliceVisualizer2D::getIntersectionsWithVolume(const osg::Vec3& inPoint, const osg::Vec3& direction, osg::Vec3& outPoint1, osg::Vec3& outPoint2)
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

        return true;
    }

    return false;
}

void osg::CArbitrarySliceVisualizer2D::getRealArbSliceGeometryPoints(const osg::Vec3& inPoint, const osg::Vec3& direction, osg::Vec3& outPoint1, osg::Vec3& outPoint2)
{
    data::CObjectPtr<data::CArbitrarySlice> spSliceArb(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id, data::Storage::NO_UPDATE));

    double xSize = spSliceArb->getSliceHeight();
    double ySize = spSliceArb->getSliceWidth();

    osg::Vec3 voxelSize = spSliceArb->getSliceVoxelSize();
    osg::Vec3 center = spSliceArb->getPlaneCenter();
    osg::Vec3 normal = spSliceArb->getPlaneNormal();
    osg::Vec3 right = spSliceArb->getPlaneRight();
    osg::Vec3 left = normal ^ right;

    osg::Vec3 p1 = inPoint - (direction * 10000.0);

    std::vector<osg::Vec3> planeNormals;
    std::vector<osg::Vec3> planePoints;
    std::vector<geometry::CPlane> planes;

    planeNormals.push_back(right);
    planeNormals.push_back(-right);
    planeNormals.push_back(left);
    planeNormals.push_back(-left);

    planePoints.push_back(center - (right * (xSize * 0.5)));
    planePoints.push_back(center + (right * (xSize * 0.5)));
    planePoints.push_back(center - (left * (ySize * 0.5)));
    planePoints.push_back(center + (left * (ySize * 0.5)));

    planes.push_back(geometry::CPlane(geometry::convert3<geometry::Vec3, osg::Vec3>(planeNormals[0]), geometry::convert3<geometry::Vec3, osg::Vec3>(planePoints[0])));
    planes.push_back(geometry::CPlane(geometry::convert3<geometry::Vec3, osg::Vec3>(planeNormals[1]), geometry::convert3<geometry::Vec3, osg::Vec3>(planePoints[1])));
    planes.push_back(geometry::CPlane(geometry::convert3<geometry::Vec3, osg::Vec3>(planeNormals[2]), geometry::convert3<geometry::Vec3, osg::Vec3>(planePoints[2])));
    planes.push_back(geometry::CPlane(geometry::convert3<geometry::Vec3, osg::Vec3>(planeNormals[3]), geometry::convert3<geometry::Vec3, osg::Vec3>(planePoints[3])));

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

            double dist1 = planes[0].distance(geometry::convert3<geometry::Vec3, osg::Vec3>(intersection));
            double dist2 = planes[1].distance(geometry::convert3<geometry::Vec3, osg::Vec3>(intersection));
            double dist3 = planes[2].distance(geometry::convert3<geometry::Vec3, osg::Vec3>(intersection));
            double dist4 = planes[3].distance(geometry::convert3<geometry::Vec3, osg::Vec3>(intersection));

            if (dist1 > -0.1 && dist2 > -0.1 && dist3 > -0.1 && dist4 > -0.1)
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

void osg::CArbitrarySliceVisualizer2D::setSceneType(ESceneType sceneType)
{
    m_sceneType = sceneType;
}

bool osg::CArbitrarySliceVisualizer2D::sigCommandFromDG(const osg::Matrix & matrix, const osgManipulator::MotionCommand &command, int command_type, long dg_id)
{
    if (command.getStage() == osgManipulator::MotionCommand::START)
    {
        data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
        m_startSlicePosition = slice->getPosition();
        m_slicePlane = geometry::CPlane(geometry::convert3<geometry::Vec3, osg::Vec3>(slice->getPlaneNormal()), geometry::convert3<geometry::Vec3, osg::Vec3>(slice->getPlaneCenter()));
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
        data::CObjectPtr< data::CArbitrarySlice > slice(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));

        geometry::Quat rotation;
        geometry::Vec3 translation, scale;
        geometry::convert4x4T<geometry::Matrix, osg::Matrix>(matrix).decompose(translation, rotation, scale);
        geometry::Vec3 newCenter = m_slicePlane.getCenter() - translation;
        double shift = m_slicePlane.distance(newCenter) / slice->getSliceVoxelSize()[2];
        double newPos = m_startSlicePosition - shift;

        if (newPos < slice->getPositionMin())
        {
            slice->setPosition(slice->getPositionMin());
            this->relocate(m_prevMatrix);
            APP_STORAGE.invalidate(slice.getEntryPtr());
            return true;
        }

        if (newPos > slice->getPositionMax())
        {
            slice->setPosition(slice->getPositionMax());
            this->relocate(m_prevMatrix);
            APP_STORAGE.invalidate(slice.getEntryPtr());
            return true;
        }

        m_prevMatrix = matrix;

        slice->setPosition(newPos);
        APP_STORAGE.invalidate(slice.getEntryPtr());
    }

    return true;
}

void osg::CArbitrarySliceVisualizer2D::prepareDraggersMaterials()
{
    osg::Vec3 colorPicked(1.0f, 1.0f, 0.6f);
    osg::Vec3 colorX(1.0f, 0.0f, 0.0f);

    if (m_translatePlaneDragger)
        m_translatePlaneDragger->setColors(colorX, colorPicked);
}

//=====================================================================================================================
void osg::CArbitrarySliceVisualizer2D::updateFromStorage()
{
    updateGeometry();
}

void osg::CArbitrarySliceVisualizer2D::onNewDensityData(data::CStorageEntry* entry, const data::CChangedEntries& changes)
{
    updateGeometry();
}

void osg::CArbitrarySliceVisualizer2D::onArbSliceChanged(data::CStorageEntry* entry, const data::CChangedEntries& changes)
{

    if (changes.hasChanged(data::Storage::ArbitrarySlice::Id))
    {
        updateGeometry();
    }
}

void osg::CArbitrarySliceVisualizer2D::onModeChanged(scene::CAppMode::tMode mode)
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