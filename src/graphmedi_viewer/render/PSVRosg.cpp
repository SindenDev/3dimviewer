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

#include <configure.h>

// Check predefined type of volume rendering algorithm
#ifdef USE_PSVR

///////////////////////////////////////////////////////////////////////////////
// includes
#include <render/PSVRosg.h>
#include <render/PSVRrenderer.h>
#include <osg/CGetNodePathVisitor.h>
#include <osg/osgcompat.h>
#include <data/CDensityData.h>

namespace PSVR
{

///////////////////////////////////////////////////////////////////////////////
//
osgPSVolumeRendering::osgPSVolumeRendering()
    : m_pRenderer(NULL)
{
    this->setUseDisplayList(false);
    this->setSupportsDisplayList(false);

    // Add update callback
    this->setUpdateCallback(new PSVR::CPSVPlaneUpdateCallback);
}

///////////////////////////////////////////////////////////////////////////////
//
osgPSVolumeRendering::osgPSVolumeRendering(const osgPSVolumeRendering& volume, const osg::CopyOp& copyop)
    : osg::Drawable(volume, copyop)
{
    this->setUseDisplayList(false);
    this->setSupportsDisplayList(false);

    m_pRenderer = volume.m_pRenderer;

    // Add update callback
    this->setUpdateCallback(new PSVR::CPSVPlaneUpdateCallback);
}

///////////////////////////////////////////////////////////////////////////////
//
osgPSVolumeRendering::~osgPSVolumeRendering()
{ }

///////////////////////////////////////////////////////////////////////////////
//
void osgPSVolumeRendering::drawImplementation(osg::RenderInfo& ri) const
{
    this->setBound(OSGCOMPUTEBOUND(this));

    if (m_pRenderer)
    {
        m_pRenderer->renderVolume();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
#if OSG_VERSION_GREATER_OR_EQUAL(3,1,10)
    osg::BoundingBox osgPSVolumeRendering::computeBoundingBox() const
#else
    osg::BoundingBox osgPSVolumeRendering::computeBound() const
#endif
{
    static const float MultCoeff = 1.0f;

    float sizex = 1.0, sizey = 1.0, sizez = 1.0;

    if (m_pRenderer)
    {
        sizex = MultCoeff * m_pRenderer->getRealXSize() * 0.5f;
        sizey = MultCoeff * m_pRenderer->getRealYSize() * 0.5f;
        sizez = MultCoeff * m_pRenderer->getRealZSize() * 0.5f;
    }

    osg::BoundingBox bbox(-sizex, -sizey, -sizez, sizex, sizey, sizez);

    return bbox;
}

///////////////////////////////////////////////////////////////////////////////
//
osgPSVolumeRenderingGeode::~osgPSVolumeRenderingGeode()
{ 
    scene::CGeneralObjectObserverOSG<osgPSVolumeRenderingGeode>::disconnect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id).get());
}

///////////////////////////////////////////////////////////////////////////////
//
osgPSVolumeRenderingGeode::osgPSVolumeRenderingGeode()
//    : m_pRenderer(NULL)
{
    // Set the update callback
    scene::CGeneralObjectObserverOSG<osgPSVolumeRenderingGeode>::connect(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id).get());
    this->setupObserver(this);

    this->setCullingActive(false);
}

///////////////////////////////////////////////////////////////////////////////
//
void osgPSVolumeRenderingGeode::updateFromStorage()
{
    // Get the drawable
    osgPSVolumeRendering *pVR = dynamic_cast<osgPSVolumeRendering *>(getDrawable(0));
    if (!pVR)
    {
        return;
    }

    // Get the active dataset
    data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(VPL_SIGNAL(SigGetActiveDataSet).invoke2()));

    // Update the data
    pVR->getRenderer()->uploadData(spData.get());
}

} // namespace PSVR

#endif // USE_PSVR

////////////////////////////////////////////////////////////////////////////////////////////////////
//\fn voidPSVR:::::void(void)
//
//\brief ! Update operator. 
////////////////////////////////////////////////////////////////////////////////////////////////////
void PSVR::CPSVPlaneUpdateCallback::update(osg::NodeVisitor* nv, osg::Drawable * node)
{
    osgPSVolumeRendering * vr = dynamic_cast<osgPSVolumeRendering *>(node);

    if (vr)
    {
        // Get node path and computational matrix
        scene::CGetNodePathVisitor visitor;
        vr->getParent(0)->accept(visitor);

        // Get eye to local matrix
        osg::Matrix elMatrix(visitor.getEyeToLocal());

        // Transform view vector to the local coordinates
        osg::Vec3 vector(osg::Vec3(0.0, 0.0, 1.0) * elMatrix);

        // Invert normal
        vector = -vector;
        vector.normalize();

        // Compute d parameter. Center point should be 0.5, 0.5, 0.5
        float d = -(vector.x() * 0.5 + vector.y() * 0.5 + vector.z() * 0.5);

        // Set new plane orientation and position
        vr->getRenderer()->setCuttingPlane(vector.x(), vector.y(), vector.z(), d);
    }

    osg::Drawable::UpdateCallback::update(nv, node);
}

