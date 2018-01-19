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
// include files
#include <vector>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PrimitiveSet>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/ShadeModel>
#include <osg/Shape>
#include <osg/Vec4>

#include <osg/CPseudoMaterial.h>
#include <geometry/base/CMesh.h>

namespace osg
{
    ///////////////////////////////////////////////////////////////////////////////
    //! OSG geode representing triangular surface mesh.
    class CTriMesh : public osg::Geode
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

        //! Vertex handles vector
        typedef std::vector<std::pair<long, SPositionNormal> > tIdPosVec;

    public:
        //! Default constructor.
        CTriMesh();

        //! Initialization of the OSG geode based on a given surface mesh.
        void createMesh(geometry::CMesh *mesh, bool createNormals = true);

        //! Changes surface color.
        void setColor(float r, float g, float b, float a = 1.0f);

        void useVertexColors(bool value = true, bool force = false);

        //! Switches between face/vertex normals
        void useNormals(ENormalsUsage normalsUsage);

        //! Update only part of model
        void updatePartOfMesh(geometry::CMesh *mesh, const tIdPosVec &ip, bool createNormals = true);

        //! Set use display lists
        void setUseDisplayList(bool bUse);

        //! Sets use of multiple materials at once
        void setUseMultipleMaterials(bool value);

        //! Sets and applies (if multiple materials are enabled) material with specified id, -1 is default material
        void setMaterial(osg::CPseudoMaterial *material, int id = -1);

        //! Gets number of materials
        int getNumMaterials();

        //! Returns material with specified id, -1 is default material
        osg::CPseudoMaterial *getMaterialById(int id = -1);

        //! Returns material at specified index, -1 is default material
        osg::CPseudoMaterial *getMaterialByIndex(int index = -1);

        //! Build kd tree
        void buildKDTree();

    protected:
        //! Applies materials
        void applyMaterials();

    protected:
        // OSG things...
        std::vector<osg::ref_ptr<osg::Geometry> > pGeometries;
        std::map<int, int> m_geometryIndexToMaterialIndex;
        std::vector<osg::ref_ptr<osg::DrawElementsUInt> > pPrimitiveSets;
        std::map<int, osg::ref_ptr<osg::CPseudoMaterial> > pMaterials;
        osg::ref_ptr<osg::CPseudoMaterial> pDefaultMaterial;
        osg::ref_ptr<osg::CPseudoMaterial> pDefaultMaterialBackup;
        bool bUseMultipleMaterials;
        osg::ref_ptr<osg::Vec3Array> pVertices;
        osg::ref_ptr<osg::Vec3Array> pNoNormals;
        osg::ref_ptr<osg::Vec3Array> pNormals;
        osg::ref_ptr<osg::Vec4Array> pColors;
        osg::ref_ptr<osg::Vec4Array> pVertexColors;
        osg::ref_ptr<osg::StateSet> pStateSet;

        //! Is KD-tree build?
        bool m_bKDTreeUsed;

        //! Is vertex coloring used?
        bool m_bUseVertexColors;
    };

    // Creates OSG geometry from loaded OpenMesh data structure
    osg::Geometry *convertOpenMesh2OSGGeometry(geometry::CMesh *mesh, bool vertexNormals);

    // Creates OSG geometry from loaded OpenMesh data structure
    osg::Geometry *convertOpenMesh2OSGGeometry(geometry::CMesh *mesh, const osg::Vec4 &color);

}// namespace osg

#endif // CTriMesh_H
