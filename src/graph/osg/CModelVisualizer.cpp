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

///////////////////////////////////////////////////////////////////////////////
// include files

#include "osg/CModelVisualizer.h"

#include <osg/CullFace>
#include <osg/Depth>
#include <osg/LightModel>


//////////////////////////////////////////////////////////////////////////
//

//bool osg::ModelVisualizer::setupModelStateSet(osg::Node *pMesh)
bool osg::ModelVisualizer::setupModelStateSet(osg::Geode *pMesh)
{
    if( !pMesh )
    {
        return false;
    }

    osg::StateSet * stateSet = pMesh->getOrCreateStateSet();

    // Enable depth test so that an opaque polygon will occlude a transparent one behind it.
    stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    // Rescale normals
    stateSet->setMode( GL_RESCALE_NORMAL, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    // Setup the light model
    osg::ref_ptr< osg::LightModel > pLightModel = new osg::LightModel();
    pLightModel->setTwoSided( true );
    pLightModel->setAmbientIntensity( Vec4(0.1, 0.1, 0.1, 1.0) );
    stateSet->setAttributeAndModes( pLightModel.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    // Enable lighting
//    stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON );

    // Culling
    osg::CullFace * cull = new osg::CullFace();
    cull->setMode( osg::CullFace::BACK );
    stateSet->setAttributeAndModes( cull, osg::StateAttribute::OFF );
//    stateSet->setAttributeAndModes( cull, osg::StateAttribute::ON );
//    stateSet->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

#if(0)
    osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
    stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
#endif

    pMesh->setStateSet( stateSet );

    return true;
}
