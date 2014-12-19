///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2014 3Dim Laboratory s.r.o.
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


#ifndef _CMODELVISUALIZER_H
#define _CMODELVISUALIZER_H

#include <osg/COnOffNode.h>
#include <osg/CTriMesh.h>
#include <osg/LightModel>
#include <data/CMesh.h>
#include <osg/CObjectObserverOSG.h>
#include <data/CModel.h>

///////////////////////////////////////////////////////////////////////////////
// Model visualizer

namespace osg
{
    class CModelVisualizerEx : public osg::CModelVisualizer
    {
    public:
        enum EModelVisualization
        {
            EMV_FLAT = 0,
            EMV_SMOOTH,
            EMV_WIRE,
            EMV_COUNT,
        };
        //! Constructor
        CModelVisualizerEx(int id) : osg::CModelVisualizer(id) 
        {
            m_modelVisualization = EMV_SMOOTH;
        }
        //! Sets type of visualization
        void setModelVisualization(EModelVisualization modelVisualization)
        {
            m_modelVisualization = modelVisualization;
            osg::Geode* pMesh = m_pMesh.get();
            if (NULL==pMesh) return;

            osg::StateSet * stateSet = pMesh->getOrCreateStateSet();
            /*osg::StateAttribute *sa = stateSet->getAttribute ( osg::StateAttribute::LIGHTMODEL );
            if (sa)
            {
                osg::ref_ptr<osg::LightModel> lm = dynamic_cast <  osg::LightModel * > ( sa );
                {
                    lm->setTwoSided(false);
                    stateSet->setAttributeAndModes ( lm.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
                }
            }*/
	        osg::PolygonMode *polyModeObj = dynamic_cast< osg::PolygonMode* >( stateSet->getAttribute( osg::StateAttribute::POLYGONMODE ));
            if ( !polyModeObj ) 
            {
		        polyModeObj = new osg::PolygonMode;
		        stateSet->setAttribute( polyModeObj );                    
	        }
            stateSet->setAttributeAndModes(polyModeObj, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
            switch (m_modelVisualization)
            {
            case EMV_SMOOTH:
                {
                    m_pMesh->useNormals(osg::CTriMesh::ENU_VERTEX);
                    //osg::ref_ptr<osg::ShadeModel> shadeModel = new osg::ShadeModel(osg::ShadeModel::SMOOTH);
                    //stateSet->setAttributeAndModes(shadeModel, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
                    polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
                    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
                }
                break;
            case EMV_FLAT:
                {
                    m_pMesh->useNormals(osg::CTriMesh::ENU_FACE);
                    //osg::ref_ptr<osg::ShadeModel> shadeModel = new osg::ShadeModel(osg::ShadeModel::FLAT);
                    //stateSet->setAttributeAndModes(shadeModel, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
                    polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
                    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
                }
                break;
            case EMV_WIRE:
                {
                    m_pMesh->useNormals(osg::CTriMesh::ENU_NONE);
                    polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
                    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
                }
                break;
            }
        }
    protected:
        //! Current visualization mode
        EModelVisualization m_modelVisualization;
    };
}
#endif

