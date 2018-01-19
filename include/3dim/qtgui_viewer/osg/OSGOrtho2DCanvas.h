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

#ifndef OSGOrtho2DCanvas_H
#define OSGOrtho2DCanvas_H

#include <osg/OSGCanvas.h>

#include <osg/OrthoManipulator.h>
#include <osg/COnOffNode.h>

///////////////////////////////////////////////////////////////////////////////
// Forward declarations

namespace data { class CDensityData; }

///////////////////////////////////////////////////////////////////////////////
//! Class OSGOrtho2DCanvas is inherited from OSGCanvas.
//
class OSGOrtho2DCanvas : public OSGCanvas
{
    Q_OBJECT

public:
    //! Constructor
    OSGOrtho2DCanvas(QWidget *parent=NULL);

    //! Destructor
    virtual ~OSGOrtho2DCanvas() {}

    //! Get manipulator.
    osgGA::OrthoManipulator * getManipulator()
    {
        return m_sceneManipulator.get();
    }


    //! Compute sizes of pixel in field of view
    osg::Vec2 getPixelSize();

    //! Get scene data
    osg::Node * getSceneData()
    {
        return m_view->getSceneData();
    }

    virtual void centerAndScale();

    //! Center and scale - given bounding box
    virtual void centerAndScale( const osg::BoundingBox & box );

    //! Set camera manipulator
    void setManipulator(osgGA::OrthoManipulator * manipulator);

    //! Set scene data
    void setScene(osg::Node * node, bool bCenterView = true);

protected:
    //! Ortho manipulator
    osg::ref_ptr<osgGA::OrthoManipulator> m_sceneManipulator;
};


#endif
