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

#ifndef CTriMesh_H
#define CTriMesh_H

///////////////////////////////////////////////////////////////////////////////
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Texture2D>
#include <VPL/Image/Image.h>

namespace geometry
{
    class CMesh;
}

namespace osg
{
    class CPseudoMaterial;

    ///////////////////////////////////////////////////////////////////////////////
    //! OSG geode representing triangular surface mesh.
    class CTriMesh : public osg::MatrixTransform
    {
    public:
        enum ENormalsUsage
        {
            ENU_NONE = 0,
            ENU_VERTEX,
        };

        struct SPositionNormal
        {
            osg::Vec3 position;
            osg::Vec3 normal;
        };

    public:
        //! Default constructor.
        CTriMesh();

        //! Initialization of the OSG geode based on a given surface mesh.
        void createMesh(geometry::CMesh& mesh, const std::map<std::string, vpl::img::CRGBAImage::tSmartPtr>& textures, bool createNormals = true);

        //! Changes surface color.
        void setColor(const osg::Vec4& value);

        void useVertexColors(bool value = true, bool force = false);

        void updateVertexColors(const geometry::CMesh& mesh, float alpha);

        //! Switches between face/vertex normals
        void useNormals(ENormalsUsage normalsUsage);

        //! Update only part of model
        void updatePartOfMesh(const std::vector<std::pair<long, SPositionNormal>>& ip);

        //! Sets use of multiple materials at once
        void setUseMultipleMaterials(bool value);

        //! Returns material with specified id, -1 is default material
        osg::CPseudoMaterial* getMaterial(int id = -1);

        //! Sets and applies (if multiple materials are enabled) material with specified id, -1 is default material
        void setMaterial(osg::CPseudoMaterial* material, int id = -1);

        const std::map<int, osg::ref_ptr<osg::CPseudoMaterial>>& getMaterials() const;

        void buildKDTree();

        void applyMaterials();

        osg::Geode* getMeshGeode();

    protected:
        void dirtyGeometry();

    protected:
        osg::ref_ptr<osg::Geode> m_geode;

        std::map<int, osg::ref_ptr<osg::Geometry>> m_geometries;
        std::map<int, osg::ref_ptr<osg::DrawElementsUInt>> m_primitiveSets;
        std::map<int, osg::ref_ptr<osg::CPseudoMaterial>> m_materials;

        osg::ref_ptr<osg::CPseudoMaterial> m_defaultMaterial;
        osg::ref_ptr<osg::CPseudoMaterial> m_defaultMaterialBackup;

        bool m_useMultipleMaterials;

        osg::ref_ptr<osg::Vec3Array> m_vertices;
        osg::ref_ptr<osg::Vec2Array> m_texCoords;
        osg::ref_ptr<osg::Vec3Array> m_noNormals;
        osg::ref_ptr<osg::Vec3Array> m_normals;
        osg::ref_ptr<osg::Vec4Array> m_colors;
        osg::ref_ptr<osg::Vec4Array> m_vertexColors;
        osg::ref_ptr<osg::Vec4Array> m_vertexGroupIndices;
        osg::ref_ptr<osg::Vec4Array> m_vertexGroupWeights;

        std::map<std::string, osg::ref_ptr<osg::Texture2D>> m_textureMap;

        //! Is KD-tree build?
        bool m_kdtreeUsed;

        //! Is vertex coloring used?
        bool m_useVertexColors;
    };

}// namespace osg

#endif // CTriMesh_H
