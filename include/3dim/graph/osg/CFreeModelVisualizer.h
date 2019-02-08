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

#ifndef CFreeModelVisualizer_H
#define CFreeModelVisualizer_H

#include <osg/COnOffNode.h>
#include <VPL/Image/Image.h>

namespace geometry
{
    class CMesh;
}

namespace osg
{
    class CTriMesh;

    //! Simple model visualiser with on-off support
    class CFreeModelVisualizer : public COnOffNode
    {
    public:
        //! Constructor
        CFreeModelVisualizer(geometry::CMesh* pMesh, const std::map<std::string, vpl::img::CRGBAImage::tSmartPtr> &textures, const osg::Vec4 &diffuse, const osg::Vec3 &emission);

        //! Show/hide the model
        void show(bool bShow);

        geometry::CMesh* getMesh();

        osg::observer_ptr<osg::CTriMesh> getTriMesh();

        void setMesh(geometry::CMesh* pMesh, const std::map<std::string, vpl::img::CRGBAImage::tSmartPtr> &textures);

    protected:
        //! Triangles...
        osg::ref_ptr<CTriMesh> m_pTriMesh;
        //! Original mesh
        std::unique_ptr<geometry::CMesh> m_pMesh;
        //! Color
        osg::Vec4 m_color;
        osg::Vec3 m_emission;
    };

} // osg

#endif // CFreeModelVisualizer_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
