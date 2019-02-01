////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////

#ifndef CRegion3DPreviewVisualizer_H
#define CRegion3DPreviewVisualizer_H

#include <osg/CTriMesh.h>
#include <osg/COnOffNode.h>
#include <osg/Geometry>
#include <geometry/base/types.h>

namespace osg
{
    //! Visualizer of region 3D preview.
    class CRegion3DPreviewVisualizer : public COnOffNode
    {

    public:
        CRegion3DPreviewVisualizer();
        ~CRegion3DPreviewVisualizer();

    public:
        //! Material
        osg::ref_ptr<osg::CPseudoMaterial> m_materialRegular;

        void setVisibility(bool visible);

        //! Updates the geometry.
        //! Sets the given color.
        void update(const osg::Vec4& color);

        //! Copies verticies and indicies from given vectors.
        void setData(const std::vector<geometry::Vec3>& vertices, const std::vector<int>& indicies);

    protected:
        std::vector<geometry::Vec3> m_vertices;
        std::vector<int> m_indicies;

        osg::ref_ptr<osg::Geometry> m_trianglesGeometry;
        osg::ref_ptr<osg::Geode> m_pTransform;
        osg::ref_ptr<osg::Vec3Array> m_geomVertices;
        osg::ref_ptr<osg::Vec3Array> m_geomNormals;
        osg::ref_ptr<osg::DrawElementsUInt> m_geomIndices;
        osg::ref_ptr<osg::Vec4Array> m_geomVertexColors;

        std::mutex m_mutex;
    };

} // namespace osg

#endif // CRegion3DPreviewVisualizer_H
