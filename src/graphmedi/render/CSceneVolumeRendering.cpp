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

#include <render/CSceneVolumeRendering.h>
#include <render/PSVRosg.h>
#include <render/PSVRrenderer.h>

///////////////////////////////////////////////////////////////////////////////
//
scene::CSceneVolumeRendering::CSceneVolumeRendering(OSGCanvas *pCanvas)
    : scene::CSceneBase(pCanvas)
{
    setName("CSceneVolumeRendering");
    // VR drawable and geode
    m_vrDrawable = new PSVR::osgPSVolumeRendering;
    //m_vrDrawable->getOrCreateStateSet()->setRenderBinDetails(LAYER_VOLUME_RENDERING, "RenderBin");
    PSVR::osgPSVolumeRenderingGeode *pGeode = new PSVR::osgPSVolumeRenderingGeode;
    pGeode->setName("osgPSVolumeRenderingGeode");
    pGeode->setCanvas(pCanvas);
    m_vrGeode = pGeode;
    m_vrGeode->addDrawable(m_vrDrawable);

    // Create and set rendered MT
    m_renderedGroup = new osg::MatrixTransform;
    m_renderedGroup->setName("Rendered Group");

    m_renderedSS = new osg::StateSet;
    m_renderedSS->setRenderBinDetails(LAYER_VOLUME_RENDERING, "RenderBin");
    //m_renderedSS->setRenderBinMode(osg::StateSet::INHERIT_RENDERBIN_DETAILS);
    //m_renderedSS->setRenderBinMode(osg::StateSet::USE_RENDERBIN_DETAILS);
    
    //m_renderedSS->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    //->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    //->setMode(GL_BLEND, osg::StateAttribute::ON);

    m_renderedGroup->setStateSet(m_renderedSS.get());

    // Add renderer gizmo to the scene
    m_renderedGroup->addChild(m_vrGeode.get());

    // Set anchored groups render order
    //p_AnchorGroup->getOrCreateStateSet()->setRenderBinDetails(LAYER_SCENE, "RenderBin");
    //p_AnchorAndCenterGroup->getOrCreateStateSet()->setRenderBinDetails(LAYER_SCENE, "RenderBin");

    m_orthoTransformMatrix = osg::Matrix::identity();

    /*// Preview 
    osg::Geode * gcube = new osg::Geode;
    osg::Box* unitCube = new osg::Box( osg::Vec3(0,0,0), 1.0f);
    osg::ShapeDrawable* unitCubeDrawable = new osg::ShapeDrawable(unitCube);

    gcube->addDrawable(unitCubeDrawable);
    this->addChild(gcube);*/

    // Add rendered group to the scene
    this->addChild(m_renderedGroup.get());

    /*// Clipping node
    m_clipNode = new osg::ClipNode;

    // Clipping plane
    m_clipPlane = new osg::ClipPlane(0, 1, 0, 0, 0);

    // Add clipping plane
    m_clipNode->addClipPlane(m_clipPlane.get());

    this->addChild( m_clipNode.get() );
    m_clipNode->addChild(m_renderedGroup.get());*/
}

///////////////////////////////////////////////////////////////////////////////
//
void scene::CSceneVolumeRendering::setRenderer(PSVR::PSVolumeRendering *pRenderer)
{
    PSVR::osgPSVolumeRendering *pDrawable = dynamic_cast<PSVR::osgPSVolumeRendering *>(m_vrDrawable.get());
    if (!pDrawable)
    {
        return;
    }

    pDrawable->setRenderer(pRenderer);
}

///////////////////////////////////////////////////////////////////////////////
//
void scene::CSceneVolumeRendering::updateFromStorage()
{
    // Get the active dataset
    int datasetId = VPL_SIGNAL(SigGetActiveDataSet).invoke2();
    if (datasetId == data::CUSTOM_DATA)
    {
        return;
    }
    data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(datasetId));

    // Get sizes
    double sx = spData->getXSize() * spData->getDX() * 0.5;
    double sy = spData->getYSize() * spData->getDY() * 0.5;
    double sz = spData->getZSize() * spData->getDZ() * 0.5;

    // Translate rendered group 
    m_renderedGroup->setMatrix( osg::Matrix::translate( sx, sy, sz ) );

    // Data loaded - recenter scene
    // TODO: tady je asi problem (volume rendering jeste neni nastaveny, aby vratil rozumny box)
    if (m_pCanvas)
    {
        m_pCanvas->centerAndScale();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void scene::CSceneVolumeRendering::enableRenderer(bool enable)
{
    PSVR::osgPSVolumeRendering *pDrawable = dynamic_cast<PSVR::osgPSVolumeRendering *>(m_vrDrawable.get());
    if (!pDrawable || !pDrawable->getRenderer())
    {
        return;
    }

    pDrawable->getRenderer()->enable(enable);
}

///////////////////////////////////////////////////////////////////////////////
//
osg::Geode *scene::CSceneVolumeRendering::getVRGeode()
{
    return m_vrGeode.get();
}
