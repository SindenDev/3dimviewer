///////////////////////////////////////////////////////////////////////////////
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

#ifndef CMESH_H
#define CMESH_H

////////////////////////////////////////////////////////////
// include
#include <map>

#include <VPL/Module/Serializer.h>
#include <VPL/Math/Base.h>
#include <VPL/Math/Matrix.h>

#include <data/CSerializableData.h>
#include <data/CSerializationManager.h>

#include <osg/Geometry>
#include <osg/TriangleFunctor>
//#include <osg/dbout.h>

#ifndef _USE_MATH_DEFINES
  #define _USE_MATH_DEFINES
#endif

#ifndef OM_STATIC_BUILD
#define OM_STATIC_BUILD
#endif

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMeshT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#include <geometry/base/OMMesh.h>
#include <geometry/base/CBaseMesh.h>
#include <geometry/base/types.h>

#define SQRT_3 1.73205080756887729353

namespace geometry
{
class CMesh;
class CMeshCutter;
class CMeshOctree;
class CMeshOctreeNode;

// define our OM mesh
/*
struct OMTraits : public OpenMesh::DefaultTraits
{
    VertexAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal);
    FaceAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal);
    EdgeAttributes(OpenMesh::Attributes::Status);
};

typedef OpenMesh::TriMesh_ArrayKernelT<OMTraits> OMMesh;*/

typedef osg::TriangleFunctor<CMeshCutter> CCuttingFunctor;

////////////////////////////////////////////////////////////
/*!
 * geometry::CMesh and osg::Geometry cutter
 */
class CMeshCutter
{
private:
    bool m_initialized;
    bool m_initializedOrtho;
    float m_planePosition;
    osg::Plane m_plane;
    osg::Plane m_transformedPlane;
    osg::Matrix m_worldMatrix;
    osg::Matrix m_invWorldMatrix;
    osg::Vec3Array *m_vertices;
    osg::DrawElementsUInt *m_indices;

public:
    CMeshCutter();
    ~CMeshCutter();
    
    void operator()(const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, float d1, float d2, float d3);
    void operator()(const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, bool treatVertexDataAsTemporary);

    bool initialize(osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, osg::Plane plane, osg::Matrix worldMatrix);
    bool initialize(osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition);

    void cutX(geometry::CMesh *source);
    void cutY(geometry::CMesh *source);
    void cutZ(geometry::CMesh *source);
    void cut(geometry::CMesh *source);
    
private:
    void calculateDistancesX(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistancesY(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistancesZ(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistances(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void performCut(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);

    inline int sign(float value)
    {
        //return value < 0.0f ? -1 : (value > 0.0f ? 1 : 0);
        return (0.0f < value) - (value < 0.0f);
    }
};

////////////////////////////////////////////////////////////
// Helper classes
////////////////////////////////////////////////////////////

#define MATERIAL_PROPERTY_NAME "material"
#define MAX_GROUPS_PER_VERTEX 4
#define VERTEX_GROUPS_PROPERTY_NAME "vertexGroups"

////////////////////////////////////////////////////////////
/*!
 * Triangular mesh
 */
class CMesh : public vpl::base::CObject, public CBaseMesh, public vpl::mod::CSerializable
{
public:
    template <int C = MAX_GROUPS_PER_VERTEX>
    class CVertexGroupsBasic
    {
    public:
        int indices[C];
        float weights[C];

        enum { GROUP_COUNT = C };

    public:
        CVertexGroupsBasic()
        {
            for (int i = 1; i < C; ++i)
            {
                indices[i] = -1;
                weights[i] = 0.0f;
            }
            indices[0] = 0;
            weights[0] = 1.0;
        }

        ~CVertexGroupsBasic()
        { }

        float sum() const
        {
            float s = 0.0;
            for (int i = 0; i < C; ++i)
            {
                s += (indices[i] != -1 ? weights[i] : 0.0);
            }
            return s;
        }

        void normalize()
        {
            float s = sum();
            for (int i = 0; i < C; ++i)
            {
                weights[i] /= (indices[i] != -1 ? s : 1.0);
            }
        }

        static CVertexGroupsBasic<C> interpolate(const CVertexGroupsBasic<C> &group0, const CVertexGroupsBasic<C> &group1, float weight)
        {
            CVertexGroupsBasic<C> retVal;
            std::map<int, std::pair<float, float> > allValues;
            std::map<int, float> newValues;
            for (int i = 0; i < C; ++i)
            {
                int g0i = group0.indices[i];
                float g0w = group0.weights[i];
                if (allValues.find(g0i) == allValues.end())
                {
                    allValues.insert(std::pair<int, std::pair<float, float> >(g0i, std::pair<float, float>(g0w, 0.0)));
                }
                else
                {
                    allValues[g0i].first = g0w;
                }

                int g1i = group1.indices[i];
                float g1w = group1.weights[i];
                if (allValues.find(g1i) == allValues.end())
                {
                    allValues.insert(std::pair<int, std::pair<float, float> >(g1i, std::pair<float, float>(0.0, g1w)));
                }
                else
                {
                    allValues[g1i].second = g1w;
                }
            }

            for (std::map<int, std::pair<float, float> >::iterator it = allValues.begin(); it != allValues.end(); ++it)
            {
                newValues.insert(std::pair<int, float>(it->first, it->second.first + weight * (it->second.second - it->second.first)));
            }

            int i = 0;
            for (std::map<int, float>::iterator it = newValues.begin(); it != newValues.end(); ++it)
            {
                if (it->first == -1)
                {
                    continue;
                }

                retVal.indices[i] = it->first;
                retVal.weights[i] = it->second;
                ++i;
                if (i == C)
                {
                    break;
                }
            }

            retVal.normalize();

            return retVal;
        }
    };

    typedef CVertexGroupsBasic<MAX_GROUPS_PER_VERTEX> CVertexGroups;

public:
    //! Standard method getEntityName().
    VPL_ENTITY_NAME("CMesh");

    //! Standard method getEntityCompression().
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    // VPL_SHAREDPTR(CMesh);   !!! incompatible with EIGEN_MAKE_ALIGNED_OPERATOR_NEW !!!

    friend class CMeshCutter;

    //! Persistent property type
    enum EPPType
    {
        PPT_EDGE = 0,
        PPT_FACE,
        PPT_HEDGE,
        PPT_VERTEX,
        PPT_LAST
    };

    //! Persistent property value type
    enum EPPValueType
    {
        PPV_INT = 0,
        PPV_FLOAT,
        PPV_DOUBLE,
        PPV_VERTEX_GROUP,
        PPV_LAST
    };

    //! Type of persistnig properties names set
    typedef std::map<std::string, EPPValueType> tPPNameSet;

private:
    CMeshOctree *m_octree;

    int m_octreeVersion;

    //! Persisting properties sets
    std::vector<tPPNameSet> m_pp;

public:
    //! Default constructor.
    CMesh();

    //! Copy constructor.
    CMesh(const CMesh &mesh);

    //! Assignment operator
    CMesh &operator=(const CMesh &mesh);

    //! Destructor.
    ~CMesh();

    //! updates octree of mesh
    void updateOctree();

    //! updates octree of mesh
    void updateOctree(int version);

    //! Gets octree
    CMeshOctree *getOctree()
    {
        return m_octree;
    }

    //! Cutting OM-mesh by planes
    static bool cutByXPlane(geometry::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition);
    static bool cutByYPlane(geometry::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition);
    static bool cutByZPlane(geometry::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition);
    static bool cutByPlane(geometry::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, osg::Plane plane, osg::Matrix worldMatrix);

    //! Cutting OSG geometry by planes
    static bool cutByPlane(osg::Geometry *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, osg::Plane plane, osg::Matrix worldMatrix);

    void getVerticesInRange(std::vector<geometry::CMesh::VertexHandle> &vertices, std::vector<double> &distances, geometry::CMesh::Point point, double distance);

    //! Set model mesh property serializable
    void setSerializedProperty(const std::string &property_name, EPPType type, EPPValueType value_type)
    {
        m_pp[type][property_name] = value_type;
    }

    void removeSerializedProperty(const std::string &property_name)
    {
        for (std::vector<std::map<std::string, EPPValueType> >::iterator it = m_pp.begin(); it != m_pp.end(); ++it)
        {
            std::map<std::string, EPPValueType>::iterator found = (*it).find(property_name);

            if (found != (*it).end())
            {
                (*it).erase(found);
                break;
            }
        }
    }

#if _MSC_VER >= 1700
#pragma optimize( "", off )
#endif

    //! Serializes the triangular mesh.
    template <class S>
    void serialize(vpl::mod::CChannelSerializer<S>& Writer)
    {
        // Begin of data serialization block
        Writer.beginWrite(*this);

        serializeNested(Writer);


        // End of the block
        Writer.endWrite(*this);
    }

    /*!
     * \fn  void serializeNested(vpl::mod::CChannelSerializer<S> &Writer)
     *
     * \brief   Serialize - nested version. This method must be used where beginWrite was already called before.
     *
     * \param [in,out]  Writer  The writer.
     */
    template <class S>
    void serializeNested(vpl::mod::CChannelSerializer<S> &Writer)
    {
        WRITEINT32(3); // version

        // add property containing vertex indices
        OpenMesh::VPropHandleT<vpl::sys::tUInt32> vProp_bufferIndex;
        if (get_property_handle(vProp_bufferIndex, "bufferIndex"))
        {
            remove_property(vProp_bufferIndex);
        }
        add_property<vpl::sys::tUInt32>(vProp_bufferIndex, "bufferIndex");

        Writer.write((vpl::sys::tUInt32)this->n_vertices());
        Writer.write((vpl::sys::tUInt32)this->n_faces());

        // write vertices
        int vIndex = 0;
        for (typename geometry::CMesh::VertexIter vit = this->vertices_begin(); vit != this->vertices_end(); ++vit)
        {
            typename geometry::CMesh::VertexHandle vertex = vit.handle();
            typename geometry::CMesh::Point point = this->point(vertex);
            this->property<vpl::sys::tUInt32>(vProp_bufferIndex, vertex) = vIndex;
            Writer.write(point[0]);
            Writer.write(point[1]);
            Writer.write(point[2]);

            typename geometry::CMesh::Color color = this->color(vertex);
            Writer.write(color[0]);
            Writer.write(color[1]);
            Writer.write(color[2]);

            vIndex++;
        }

        // write faces
        for (geometry::CMesh::FaceIter fit = this->faces_begin(); fit != this->faces_end(); ++fit)
        {
            geometry::CMesh::FaceHandle face = fit.handle();
            for (geometry::CMesh::FaceVertexIter fvit = this->fv_begin(face); fvit != this->fv_end(face); ++fvit)
            {
                Writer.write(this->property<vpl::sys::tUInt32>(vProp_bufferIndex, fvit.handle()));
            }
        }

        int counter(0);
        // Write all persistent properties
        for (int i = PPT_EDGE; i < PPT_LAST; ++i)
        {
            // Store number of persistent properties of given type
            if (i < m_pp.size())
            {
                // Check if properties can be serialized - mesh can have declared serialized properties but they are not present
                int propertyCount = 0;
                tPPNameSet::const_iterator itn(m_pp[i].begin()), itnEnd(m_pp[i].end());
                for (; itn != itnEnd; ++itn)
                {
                    if (checkProperty(static_cast<EPPType>(i), itn->second, itn->first))
                    {
                        propertyCount++;
                    }
                    else
                    {
                        VPL_LOG_WARN("Property \"" << itn->first << "\"marked for serialization is not present.");
                    }
                }

                Writer.write((vpl::sys::tUInt32)propertyCount);

                if (m_pp[i].size() > 0)
                {
                    // For all persistent properties
                    tPPNameSet::const_iterator itname(m_pp[i].begin()), itnameEnd(m_pp[i].end());
                    for (; itname != itnameEnd; ++itname)
                    {
                        if (checkProperty(static_cast<EPPType>(i), itname->second, itname->first))
                        {
                            serializeProperty(Writer, static_cast<EPPType>(i), itname->second, itname->first);
                            ++counter;
                        }
                    }
                }
            }
            else
            {
                Writer.write((vpl::sys::tUInt32)0);
            }

            Writer.write((vpl::sys::tUInt32)i);
        }
    }



    //! Deserializes the triangular mesh.
    template <class S>
    void deserialize(vpl::mod::CChannelSerializer<S>& Reader)
    {
        // Begin of data deserialization block
        Reader.beginRead(*this);

        deserializeNested(Reader);

        // End of the block
        Reader.endRead(*this);
    }

    /*!
     * \fn  void deserializeNested(vpl::mod::CChannelSerializer<S> &Reader)
     *
     * \brief   Deserialize - nested version. This method must be used where beginRead was already used before.
     *
     * \param [in,out]  Reader  The reader.
     */
    template <class S>
    void deserializeNested(vpl::mod::CChannelSerializer<S> &Reader)
    {
        int version = 0;
        READINT32(version);

        this->clear();
        this->garbage_collection();

        // read counts
        vpl::sys::tUInt32 vertexCount, faceCount;
        Reader.read(vertexCount);
        Reader.read(faceCount);

        // read vertices
        std::vector<geometry::CMesh::VertexHandle> vertexHandles;
        for (unsigned int v = 0; v < vertexCount; ++v)
        {
            geometry::CMesh::Point::value_type data[3];
            Reader.read(data, 3);
            vertexHandles.push_back(this->add_vertex(geometry::CMesh::Point(data)));

            if (version > 2)
            {
                geometry::CMesh::Color::value_type colorData[3];
                Reader.read(colorData, 3);
                this->set_color(vertexHandles.back(), geometry::CMesh::Color(colorData));
            }
        }

        // read faces
        for (unsigned int f = 0; f < faceCount; ++f)
        {
            vpl::sys::tUInt32 data[3];
            Reader.read(data, 3);
            this->add_face(vertexHandles[data[0]], vertexHandles[data[1]], vertexHandles[data[2]]);
        }

        if (version > 1)
        {
            int counter(0);

            // Write all persistent properties
            for (int i = PPT_EDGE; i < PPT_LAST; ++i)
            {
                // Read number of properties of given type
                vpl::sys::tUInt32 num(0);
                Reader.read(num);
                // For all stored properties
                for (vpl::sys::tUInt32 j = 0; j < num; ++j)
                {
                    deserializeProperty(Reader);
                    ++counter;
                }

                Reader.read(num);
            }

        }
    }

#if _MSC_VER >= 1700
#pragma optimize( "", on )
#endif

///////////////////////////////////////////////////////////////////////////////
// funcionality enhancement
// - variants with points are faster and do not need existing entities

    //! calculates quality of face
    double quality(geometry::CMesh::FaceHandle fh);
    double quality(const geometry::CMesh::Point& p0, const geometry::CMesh::Point& p1, const geometry::CMesh::Point& p2)
    {
        double per = perimeter(p0, p1, p2);
        double max = max_edge_length(p0, p1, p2);
        return (per - 2.0 * max) / max;
    }


    //! calculates area that is covered by face
    double area(geometry::CMesh::FaceHandle fh);
    double area(const geometry::CMesh::Point& p0, const geometry::CMesh::Point& p1, const geometry::CMesh::Point& p2)
    {
        return ((p1 - p0) % (p2 - p0)).length() / 2;
    }

    //! calculates perimeter of face
    double perimeter(geometry::CMesh::FaceHandle fh);
    double perimeter(const geometry::CMesh::Point& p0, const geometry::CMesh::Point& p1, const geometry::CMesh::Point& p2)
    {
        return (p0 - p1).length() + (p1 - p2).length() + (p2 - p0).length();
    }

    //! calculates length of shortest/longest edge
    double min_edge_length(geometry::CMesh::FaceHandle fh);
    double max_edge_length(geometry::CMesh::FaceHandle fh);
    double min_edge_length(const geometry::CMesh::Point& p0, const geometry::CMesh::Point& p1, const geometry::CMesh::Point& p2)
    {
        return vpl::math::getMin<double>((p0 - p1).length(), (p1 - p2).length(), (p2 - p0).length());
    }
    double max_edge_length(const geometry::CMesh::Point& p0, const geometry::CMesh::Point& p1, const geometry::CMesh::Point& p2)
    {
        return vpl::math::getMax<double>((p0 - p1).length(), (p1 - p2).length(), (p2 - p0).length());
    }

    double triangle_height(geometry::CMesh::FaceHandle fh, geometry::CMesh::EdgeHandle eh);
    double triangle_height(const geometry::CMesh::Point& p0, const geometry::CMesh::Point& p1, const geometry::CMesh::Point& p2)
    {
        return 2.0 * area(p0, p1, p2) / (p0 - p1).length();
    }

    //! return shortest/longest edge
    geometry::CMesh::EdgeHandle min_edge(geometry::CMesh::FaceHandle fh);
    geometry::CMesh::EdgeHandle max_edge(geometry::CMesh::FaceHandle fh);

    //! returns face that shares specified edge
    geometry::CMesh::FaceHandle neighbour(geometry::CMesh::FaceHandle fh, geometry::CMesh::EdgeHandle eh);

    //! returns the remaining vertex of face
    geometry::CMesh::VertexHandle rest_vertex(geometry::CMesh::FaceHandle fh, geometry::CMesh::VertexHandle vh0, geometry::CMesh::VertexHandle vh1);

    //! calculates normal of face
    geometry::CMesh::Normal calc_face_normal(const geometry::CMesh::Point& p0, const geometry::CMesh::Point& p1, const geometry::CMesh::Point& p2)
    {
        return PolyMesh::calc_face_normal(p0, p1, p2);
    }
    geometry::CMesh::Normal calc_face_normal(geometry::CMesh::FaceHandle fh);

    //! calculates axis aligned bounding box of mesh
    bool calc_bounding_box(geometry::CMesh::Point &min, geometry::CMesh::Point &max) const;

	//! Calculate oriented bounding box
	bool calc_oriented_bounding_box(Matrix &tm, Vec3 &extent);

    //! Calculate oriented bounding box from triangles
    bool calc_oriented_bb_triangles(Matrix &tm, Vec3 &extent);

    //! calculates average vertex of model
    bool calc_average_vertex(geometry::CMesh::Point &average);

	//! Transform mesh vertices
	void translate(float x, float y, float z);

	//! Do complete transform by matrix
	void transform(const Matrix &tm);

    //! Finds edge by two vertices
    geometry::CMesh::EdgeHandle find_edge(const geometry::CMesh::VertexHandle &vh0, const geometry::CMesh::VertexHandle &vh1);

	//! Add given mesh to the current mesh
	void addMesh(const CMesh &added_mesh);

protected:
	// Compute oriented bounding box from the covariance matrix
	bool calc_obb_from_cm(Matrix3x3 &cm, Matrix &tm, Vec3 &extent);

private:

    template <typename T>
    bool checkSingleProperty(EPPType property_type, const std::string &name)
    {
        bool retVal = false;

        switch (property_type)
        {
        case PPT_EDGE:
            {
                OpenMesh::EPropHandleT<T> propertyHandle;
                retVal = get_property_handle(propertyHandle, name);
            }
            break;

        case PPT_FACE:
            {
                OpenMesh::FPropHandleT<T> propertyHandle;
                retVal = get_property_handle(propertyHandle, name);
            }
            break;

        case PPT_HEDGE:
            {
                OpenMesh::HPropHandleT<T> propertyHandle;
                retVal = get_property_handle(propertyHandle, name);
            }
            break;

        case PPT_VERTEX:
            {
                OpenMesh::VPropHandleT<T> propertyHandle;
                retVal = get_property_handle(propertyHandle, name);
            }
            break;
        };

        return retVal;
    }

    //! Serialize property 
    bool checkProperty(EPPType property_type, EPPValueType value_type, const std::string &name)
    {
        bool retVal = false;

        switch (value_type)
        {
        case geometry::CMesh::PPV_INT:
            retVal = checkSingleProperty<int>(property_type, name);
            break;

        case geometry::CMesh::PPV_FLOAT:
            retVal = checkSingleProperty<float>(property_type, name);
            break;

        case geometry::CMesh::PPV_DOUBLE:
            retVal = checkSingleProperty<double>(property_type, name);
            break;

        case geometry::CMesh::PPV_VERTEX_GROUP:
            retVal = checkSingleProperty<geometry::CMesh::CVertexGroups>(property_type, name);
            break;
        }

        return retVal;
    }

    template <class S, typename T>
    void serializePropertyValue(vpl::mod::CChannelSerializer<S> &writer, const T &value)
    {
        VPL_LOG_INFO("serializePropertyValue(...) not implemented for some type");
        throw CSerializationFailure("serializePropertyValue(...) not implemented for some type");
    }

    template <class S>
    void serializePropertyValue(vpl::mod::CChannelSerializer<S> &writer, const int &value)
    {
        writer.write((vpl::sys::tInt32)value);
    }

    template <class S>
    void serializePropertyValue(vpl::mod::CChannelSerializer<S> &writer, const float &value)
    {
        writer.template write<float>(value);
    }

    template <class S>
    void serializePropertyValue(vpl::mod::CChannelSerializer<S> &writer, const double &value)
    {
        writer.template write<double>(value);
    }

    template <class S>
    void serializePropertyValue(vpl::mod::CChannelSerializer<S> &writer, const geometry::CMesh::CVertexGroups &value)
    {
        for (int i = 0; i < MAX_GROUPS_PER_VERTEX; ++i)
        {
            writer.write((vpl::sys::tInt32)value.indices[i]);
            writer.template write<float>(value.weights[i]);
        }
    }

    template <class S, typename T>
    void deserializePropertyValue(vpl::mod::CChannelSerializer<S> &reader, T &value)
    {
        VPL_LOG_INFO("deserializePropertyValue(...) not implemented for some type");
        throw CSerializationFailure("deserializePropertyValue(...) not implemented for some type");
    }

    template <class S>
    void deserializePropertyValue(vpl::mod::CChannelSerializer<S> &reader, int &value)
    {
        vpl::sys::tInt32 v;
        reader.read(v);
        value = v;
    }

    template <class S>
    void deserializePropertyValue(vpl::mod::CChannelSerializer<S> &reader, float &value)
    {
        reader.template read<float>(value);
    }

    template <class S>
    void deserializePropertyValue(vpl::mod::CChannelSerializer<S> &reader, double &value)
    {
        reader.template read<double>(value);
    }

    template <class S>
    void deserializePropertyValue(vpl::mod::CChannelSerializer<S> &reader, geometry::CMesh::CVertexGroups &value)
    {
        for (int i = 0; i < MAX_GROUPS_PER_VERTEX; ++i)
        {
            vpl::sys::tInt32 v;
            reader.read(v);
            value.indices[i] = v;

            reader.template read<float>(value.weights[i]);
        }
    }

    template <class S, typename T>
    void serializeTypedEdgeProperty(vpl::mod::CChannelSerializer<S> &writer, const std::string &name)
    {
        OpenMesh::EPropHandleT<T> propertyHandle;
        get_property_handle(propertyHandle, name);

        for (geometry::CMesh::EdgeIter it = edges_begin(); it != edges_end(); ++it)
        {
            T &value = property(propertyHandle, *it);
            serializePropertyValue(writer, value);
        }
    }

    template <class S, typename T>
    void serializeTypedFaceProperty(vpl::mod::CChannelSerializer<S> &writer, const std::string &name)
    {
        OpenMesh::FPropHandleT<T> propertyHandle;
        get_property_handle(propertyHandle, name);

        for (geometry::CMesh::FaceIter it = faces_begin(); it != faces_end(); ++it)
        {
            T &value = property(propertyHandle, *it);
            serializePropertyValue(writer, value);
        }
    }

    template <class S, typename T>
    void serializeTypedHalfedgeProperty(vpl::mod::CChannelSerializer<S> &writer, const std::string &name)
    {
        OpenMesh::HPropHandleT<T> propertyHandle;
        get_property_handle(propertyHandle, name);

        for (geometry::CMesh::HalfedgeIter it = halfedges_begin(); it != halfedges_end(); ++it)
        {
            T &value = property(propertyHandle, *it);
            serializePropertyValue(writer, value);
        }
    }

    template <class S, typename T>
    void serializeTypedVertexProperty(vpl::mod::CChannelSerializer<S> &writer, const std::string &name)
    {
        OpenMesh::VPropHandleT<T> propertyHandle;
        get_property_handle(propertyHandle, name);

        for (geometry::CMesh::VertexIter it = vertices_begin(); it != vertices_end(); ++it)
        {
            T &value = property(propertyHandle, *it);
            serializePropertyValue(writer, value);
        }
    }

    template <class S, typename T>
    void deserializeTypedEdgeProperty(vpl::mod::CChannelSerializer<S> &reader, const std::string &name)
    {
        OpenMesh::EPropHandleT<T> propertyHandle;
        add_property(propertyHandle, name);

        for (geometry::CMesh::EdgeIter it = edges_begin(); it != edges_end(); ++it)
        {
            T &value = property(propertyHandle, *it);
            deserializePropertyValue(reader, value);
        }
    }

    template <class S, typename T>
    void deserializeTypedFaceProperty(vpl::mod::CChannelSerializer<S> &reader, const std::string &name)
    {
        OpenMesh::FPropHandleT<T> propertyHandle;
        add_property(propertyHandle, name);

        for (geometry::CMesh::FaceIter it = faces_begin(); it != faces_end(); ++it)
        {
            T &value = property(propertyHandle, *it);
            deserializePropertyValue(reader, value);
        }
    }

    template <class S, typename T>
    void deserializeTypedHalfedgeProperty(vpl::mod::CChannelSerializer<S> &reader, const std::string &name)
    {
        OpenMesh::HPropHandleT<T> propertyHandle;
        add_property(propertyHandle, name);

        for (geometry::CMesh::HalfedgeIter it = halfedges_begin(); it != halfedges_end(); ++it)
        {
            T &value = property(propertyHandle, *it);
            deserializePropertyValue(reader, value);
        }
    }

    template <class S, typename T>
    void deserializeTypedVertexProperty(vpl::mod::CChannelSerializer<S> &reader, const std::string &name)
    {
        OpenMesh::VPropHandleT<T> propertyHandle;
        add_property(propertyHandle, name);

        for (geometry::CMesh::VertexIter it = vertices_begin(); it != vertices_end(); ++it)
        {
            T &value = property(propertyHandle, *it);
            deserializePropertyValue(reader, value);
        }
    }

    template <class S, typename T>
    void serializeTypedProperty(vpl::mod::CChannelSerializer<S> &writer, EPPType propertyType, const std::string &name)
    {
        // Store property values
        switch (propertyType)
        {
        case PPT_EDGE:
            serializeTypedEdgeProperty<S, T>(writer, name);
            break;

        case PPT_FACE:
            serializeTypedFaceProperty<S, T>(writer, name);
            break;

        case PPT_HEDGE:
            serializeTypedHalfedgeProperty<S, T>(writer, name);
            break;

        case PPT_VERTEX:
            serializeTypedVertexProperty<S, T>(writer, name);
            break;
        }
    }

    template <class S, typename T>
    void deserializeTypedProperty(vpl::mod::CChannelSerializer<S> &reader, EPPType propertyType, const std::string &name)
    {
        // Store property values
        switch (propertyType)
        {
        case PPT_EDGE:
            deserializeTypedEdgeProperty<S, T>(reader, name);
            break;

        case PPT_FACE:
            deserializeTypedFaceProperty<S, T>(reader, name);
            break;

        case PPT_HEDGE:
            deserializeTypedHalfedgeProperty<S, T>(reader, name);
            break;

        case PPT_VERTEX:
            deserializeTypedVertexProperty<S, T>(reader, name);
            break;
        }
    }

    //! Serialize property - without macros
    template <class S>
    void serializeProperty(vpl::mod::CChannelSerializer<S> &writer, EPPType propertyType, EPPValueType valueType, const std::string &name)
    {
        if (!checkProperty(propertyType, valueType, name))
        {
            return;
        }

        // Serialize property attributes
        writer.write((vpl::sys::tInt32)propertyType);
        writer.write((vpl::sys::tInt32)valueType);
        writer.write(name);

        switch (valueType)
        {
        case geometry::CMesh::PPV_INT:
            serializeTypedProperty<S, int>(writer, propertyType, name);
            break;

        case geometry::CMesh::PPV_FLOAT:
            serializeTypedProperty<S, float>(writer, propertyType, name);
            break;

        case geometry::CMesh::PPV_DOUBLE:
            serializeTypedProperty<S, double>(writer, propertyType, name);
            break;

        case geometry::CMesh::PPV_VERTEX_GROUP:
            serializeTypedProperty<S, geometry::CMesh::CVertexGroups>(writer, propertyType, name);
            break;
        }
    }

    //! Serialize property - without macros
    template <class S>
    void deserializeProperty(vpl::mod::CChannelSerializer<S> &reader)
    {
        EPPType propertyType;
        EPPValueType valueType;
        std::string name;

        // Read property type
        vpl::sys::tInt32 iPropertyType;
        reader.read(iPropertyType);
        assert(iPropertyType >= PPT_EDGE && iPropertyType < PPT_LAST);
        propertyType = static_cast<EPPType>(iPropertyType);

        // Read property value type
        vpl::sys::tInt32 iValueType;
        reader.read(iValueType);
        assert(iValueType >= PPV_INT && iValueType < PPV_LAST);
        valueType = static_cast<EPPValueType>(iValueType);

        // Read property name
        reader.read(name);
        assert(name.length() > 0);

        switch (valueType)
        {
        case geometry::CMesh::PPV_INT:
            deserializeTypedProperty<S, int>(reader, propertyType, name);
            break;

        case geometry::CMesh::PPV_FLOAT:
            deserializeTypedProperty<S, float>(reader, propertyType, name);
            break;

        case geometry::CMesh::PPV_DOUBLE:
            deserializeTypedProperty<S, double>(reader, propertyType, name);
            break;

        case geometry::CMesh::PPV_VERTEX_GROUP:
            deserializeTypedProperty<S, geometry::CMesh::CVertexGroups>(reader, propertyType, name);
            break;
        }

        setSerializedProperty(name, propertyType, valueType);
    }

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
}; // class CMesh

// typedef CMesh::tSmartPtr CMeshPtr;  !!! incompatible with EIGEN_MAKE_ALIGNED_OPERATOR_NEW !!!

////////////////////////////////////////////////////////////
/*!
 * Octree subdivision of mesh
 */
class CMeshOctreeNode
{
public:
    osg::BoundingBox boundingBox; //!  bounding box of node
    int nodes[8]; //! children indices of node
    std::vector<geometry::CMesh::FaceHandle> faces; //! list of faces
    std::vector<geometry::CMesh::VertexHandle> vertices; //! list of vertices
    int faceCount;
    int vertexCount;

public:
    //! Ctor
    CMeshOctreeNode();

    //! Copy Ctor
    CMeshOctreeNode(const CMeshOctreeNode &node);

    //! Assignment
    CMeshOctreeNode &operator=(const CMeshOctreeNode &node);

    //! Dtor
    ~CMeshOctreeNode();
};

class CMeshOctree
{
private:
    int m_nextIndex;
    std::vector<CMeshOctreeNode> m_nodes;
    bool m_initialized;
    std::vector<CMeshOctreeNode *> m_intersectedNodes;

public:
    //! Ctor
    CMeshOctree();

    //! Copy Ctor
    CMeshOctree(const CMeshOctree &octree);

    //! Assignment
    CMeshOctree &operator=(const CMeshOctree &octree);

    //! Dtor
    ~CMeshOctree();

    //! Initializes octree structure
    bool initialize(int levels);

    //! Updates octree
    void update(geometry::CMesh *mesh, osg::BoundingBox boundingBox);

    //! Gets list of intersected nodes
    //std::vector<CMeshOctreeNode *> getIntersectedNodes(osg::Plane plane);
    const std::vector<CMeshOctreeNode *>& getIntersectedNodes(osg::Plane plane);
    const std::vector<CMeshOctreeNode *>& getIntersectedNodes(osg::BoundingBox boundingBox);
    const std::vector<CMeshOctreeNode *>& getNearPoints(const osg::Vec3 &point, double maximal_distance);
    size_t getNearPointsHandles(const osg::Vec3 &point, double maximal_distance, std::vector<geometry::CMesh::VertexHandle> &handles, geometry::CMesh *mesh);

private:
    void intersect(CMeshOctreeNode &node, osg::Plane plane);
    void intersect(CMeshOctreeNode &node, osg::BoundingBox boundingBox);
    void initializeNode(CMeshOctreeNode &node, osg::BoundingBox boundingBox);
    void fillFaceLists(geometry::CMesh *mesh);
    void fillVertexLists(geometry::CMesh *mesh);
    bool assignFace(CMeshOctreeNode &node, int level, geometry::CMesh::FaceHandle face, osg::BoundingBox faceBoundingBox);
    bool assignVertex(CMeshOctreeNode &node, int level, geometry::CMesh::VertexHandle vertex, osg::Vec3 coordinates);
    static int calculateNodeCount(int numOfLevels);
};

} // namespace data

#endif
