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


#ifndef _CMODELVISUALIZER_H
#define _CMODELVISUALIZER_H

#include "osg/CModelVisualizer.h"

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

        CModelVisualizerEx(int id) : osg::CModelVisualizer(id) , m_modelVisualization(EMV_SMOOTH)
        {
        }

        //! Sets type of visualization
        void setModelVisualization(EModelVisualization modelVisualization);

    protected:
        //! Current visualization mode
        EModelVisualization m_modelVisualization;
    };
}
#endif

