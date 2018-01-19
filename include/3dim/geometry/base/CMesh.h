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

#include <VPL/Module/Serializer.h>
#include <VPL/Math/Base.h>
#include <VPL/Math/Matrix.h>

#include <data/CSerializableData.h>

#include <osg/Geometry>
#include <osg/TriangleFunctor>
//#include <osg/dbout.h>

#define _USE_MATH_DEFINES

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
//    void calculateDistancesX(geometry::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
//    void calculateDistancesY(geometry::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
//    void calculateDistancesZ(geometry::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
//    void calculateDistances(geometry::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
//    void performCut(geometry::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistancesX(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistancesY(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistancesZ(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistances(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void performCut(geometry::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);

    inline int sign(float value)
    {
//        return value < 0.0f ? -1 : (value > 0.0f ? 1 : 0);
        return (0.0f < value) - (value < 0.0f);
    }
};

////////////////////////////////////////////////////////////
// Helper classes
////////////////////////////////////////////////////////////

#define MATERIAL_PROPERTY_NAME "material"

////////////////////////////////////////////////////////////
/*!
 * Triangular mesh
 */
class CMesh : public vpl::base::CObject, public CBaseMesh, public vpl::mod::CSerializable
{
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

#if _MSC_VER >= 1700
#pragma optimize( "", off )
#endif

    //! Serializes the triangular mesh.
    template <class S>
    void serialize(vpl::mod::CChannelSerializer<S>& Writer)
    {
        // Begin of data serialization block
        Writer.beginWrite(*this);

        WRITEINT32( 2 ); // version

        // add property containing vertex indices
        OpenMesh::VPropHandleT<vpl::sys::tUInt32> vProp_bufferIndex;
        this->add_property<vpl::sys::tUInt32>(vProp_bufferIndex, "bufferIndex");

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
		for(int i = PPT_EDGE; i < PPT_LAST; ++i)
		{
			// Store number of persistent properties of given type
			if (i<m_pp.size())
			{
				Writer.write((vpl::sys::tUInt32)m_pp[i].size());
				if(m_pp[i].size() > 0)
				{
					// For all persistent properties
					tPPNameSet::const_iterator itn(m_pp[i].begin()), itnEnd(m_pp[i].end());
					for(; itn != itnEnd; ++itn)
					{
						serializeProperty(Writer, static_cast<EPPType>(i), itn->second, itn->first);
 						++counter;
					}	
				}
			}
			else
				Writer.write((vpl::sys::tUInt32)0);

			Writer.write((vpl::sys::tUInt32)i);
		}

        // End of the block
        Writer.endWrite(*this);
    }



    //! Deserializes the triangular mesh.
    template <class S>
    void deserialize(vpl::mod::CChannelSerializer<S>& Reader)
    {
        // Begin of data deserialization block
        Reader.beginRead(*this);

        int version = 0;
        READINT32( version );

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
        }

        // read faces
        for (unsigned int f = 0; f < faceCount; ++f)
        {
            vpl::sys::tUInt32 data[3];
            Reader.read(data, 3);            
            this->add_face(vertexHandles[data[0]], vertexHandles[data[1]], vertexHandles[data[2]]);
        }


		if(version > 1)
		{
			int counter(0);

			// Write all persistent properties
			for(int i = PPT_EDGE; i < PPT_LAST; ++i)
			{
				// Read number of properties of given type
				vpl::sys::tUInt32 num(0);
				Reader.read(num);
				// For all stored properties
				for( vpl::sys::tUInt32 j = 0; j < num; ++j )
				{
					deserializeProperty(Reader);
 					++counter;
				}

				Reader.read(num);
			}

		}


        // End of the block
        Reader.endRead(*this);
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
    bool calc_bounding_box(geometry::CMesh::Point &min, geometry::CMesh::Point &max);

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

protected:
	// Compute oriented bounding box from the covariance matrix
	bool calc_obb_from_cm(Matrix3x3 &cm, Matrix &tm, Vec3 &extent);

private:

#define declare_property_test(handle_type, value_type, name) \
    { \
        switch(value_type) \
        { \
            case geometry::CMesh::PPV_INT: \
            { \
            handle_type<int> ph; \
            if(!this->get_property_handle(ph, name)) \
                return; \
        } \
        break; \
        case geometry::CMesh::PPV_FLOAT: \
        { \
            handle_type<float> ph; \
            if(!this->get_property_handle(ph, name)) \
                return; \
        } \
        break; \
        case geometry::CMesh::PPV_DOUBLE: \
        { \
            handle_type<double> ph; \
            if(!this->get_property_handle(ph, name)) \
                return; \
        } \
        break; \
        default: \
                 return; \
        } \
    }


    //! Serialize property 
	template <class S>
	void serializeProperty(vpl::mod::CChannelSerializer<S>& Writer, EPPType property_type, EPPValueType value_type, const std::string &name)
	{
        // Test if property exists
        switch (property_type)
        {
        case PPT_EDGE:
            declare_property_test(OpenMesh::EPropHandleT, value_type, name);
            break;

        case PPT_FACE:
            declare_property_test(OpenMesh::FPropHandleT, value_type, name);
            break;

        case PPT_HEDGE:
            declare_property_test(OpenMesh::HPropHandleT, value_type, name);
            break;

        case PPT_VERTEX:
            declare_property_test(OpenMesh::VPropHandleT, value_type, name);
            break;
        default:
            return; 
        }

		// Serialize property attributes
		WRITEINT32(property_type);
		WRITEINT32(value_type);
		Writer.write(name);

//		DBOUT("Serialized property: " << name.c_str() << ", type: " << (int)property_type << ", value type: " << (int)value_type);

		// Store property values
		switch (property_type)
		{
		case PPT_EDGE:
			valuesSerializationEdge(Writer, value_type, name);
			break;

		case PPT_FACE:
			valuesSerializationFace(Writer, value_type, name);
			break;

		case PPT_HEDGE:
			valuesSerializationHEdge(Writer, value_type, name);
			break;

		case PPT_VERTEX:
			valuesSerializationVertex(Writer, value_type, name);
			break;
		default:
			break;
		}
	}

	//! Deserialize property
	template <class S>
	void deserializeProperty(vpl::mod::CChannelSerializer<S>& Reader)
	{
		 EPPType property_type;
		 EPPValueType value_type;
		 std::string name;

		 // Read property type
		 int pt; READINT32(pt);
		 assert(pt >= PPT_EDGE && pt < PPT_LAST);
		 property_type = static_cast<EPPType>(pt);

		 // Read property value type
		 int pvt;
		 READINT32(pvt);
		 assert(pvt >= PPV_INT && pvt < PPV_LAST);
		 value_type = static_cast<EPPValueType>(pvt);

		 // Read property name
		 Reader.read(name);
		 assert(name.length() > 0);

//		 DBOUT("Deserialized property: " << name.c_str() << ", type: " << (int)property_type << ", value type: " << (int)value_type);
		// Store property values
		switch (property_type)
		{
		case PPT_EDGE:
			valuesDeserializationEdge(Reader, value_type, name);
			break;

		case PPT_FACE:
			valuesDeserializationFace(Reader, value_type, name);
			break;

		case PPT_HEDGE:
			valuesDeserializationHEdge(Reader, value_type, name);
			break;

		case PPT_VERTEX:
			valuesDeserializationVertex(Reader, value_type, name);
			break;
		default:
			break;
		}

        setSerializedProperty(name, property_type, value_type);
	}

/*
 *	Helper macro automatically declares property serialization wrapper
 */ 
#define declare_property_serialization( call_name, handle_type, it_begin, it_end ) \
	template <class S> \
	void call_name(vpl::mod::CChannelSerializer<S>& Writer,  EPPValueType value_type, const std::string &name) \
	{ \
		switch (value_type) \
		{ \
			case geometry::CMesh::PPV_INT: \
				{ \
					handle_type<int> ph; \
					bool rv(this->get_property_handle(ph, name)); \
					assert(rv); \
					serializePropertyValuesInt(Writer, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			case geometry::CMesh::PPV_FLOAT: \
				{ \
					handle_type<float> ph; \
					bool rv(this->get_property_handle(ph, name)); \
					assert(rv); \
					serializePropertyValuesFloat(Writer, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			case geometry::CMesh::PPV_DOUBLE: \
				{ \
					handle_type<double> ph; \
					bool rv(this->get_property_handle(ph, name)); \
					assert(rv); \
					serializePropertyValuesDouble(Writer, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			default: \
				break; \
		} \
	}; 

	// Declare serializers 
	declare_property_serialization( valuesSerializationEdge, OpenMesh::EPropHandleT, edges_begin, edges_end );	
	declare_property_serialization( valuesSerializationFace, OpenMesh::FPropHandleT, faces_begin, faces_end );	
	declare_property_serialization( valuesSerializationHEdge, OpenMesh::HPropHandleT, halfedges_begin, halfedges_end );	
	declare_property_serialization( valuesSerializationVertex, OpenMesh::VPropHandleT, vertices_begin, vertices_end );	

/*
 *	Helper macro automatically declares property deserialization wrapper
 */ 
#define declare_property_deserialization( call_name, handle_type, it_begin, it_end ) \
	template <class S> \
	void call_name(vpl::mod::CChannelSerializer<S>& Reader,  EPPValueType value_type, const std::string &name) \
	{ \
		switch (value_type) \
		{ \
			case geometry::CMesh::PPV_INT: \
				{ \
					handle_type<int> ph; \
					this->add_property(ph, name); \
					deserializePropertyValuesInt(Reader, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			case geometry::CMesh::PPV_FLOAT: \
				{ \
					handle_type<float> ph; \
					this->add_property(ph, name); \
					deserializePropertyValuesFloat(Reader, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			case geometry::CMesh::PPV_DOUBLE: \
				{ \
					handle_type<double> ph; \
					this->add_property(ph, name); \
					deserializePropertyValuesDouble(Reader, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			default: \
				break; \
		} \
	}; 

	// Declare deserializers
	declare_property_deserialization( valuesDeserializationEdge, OpenMesh::EPropHandleT, edges_begin, edges_end );	
	declare_property_deserialization( valuesDeserializationFace, OpenMesh::FPropHandleT, faces_begin, faces_end );	
	declare_property_deserialization( valuesDeserializationHEdge, OpenMesh::HPropHandleT, halfedges_begin, halfedges_end );	
	declare_property_deserialization( valuesDeserializationVertex, OpenMesh::VPropHandleT, vertices_begin, vertices_end );	

	//! Serialize signed int property values
	template <class S, typename tPH, typename tIt>
	void serializePropertyValuesInt(vpl::mod::CChannelSerializer<S>& Writer, tPH &ph, tIt itBegin, tIt itEnd)
	{
		int counter(0);
		for(tIt it = itBegin; it != itEnd; ++it)
		{
			WRITEINT32( this->property<int>(ph, it) );
			++counter;
		}

//		DBOUT("Ints serialized: " << counter);
	}

	//! Serialize float property values
	template <class S, typename tPH, typename tIt>
	void serializePropertyValuesFloat(vpl::mod::CChannelSerializer<S>& Writer, tPH &ph, tIt itBegin, tIt itEnd)
	{
		for(tIt it = itBegin; it != itEnd; ++it)
		{
			Writer.template write<float>(this->property<float>(ph, it));
		}
	}

	//! Serialize double property values
	template <class S, typename tPH, typename tIt>
	void serializePropertyValuesDouble(vpl::mod::CChannelSerializer<S>& Writer, tPH &ph, tIt itBegin, tIt itEnd)
	{
		for(tIt it = itBegin; it != itEnd; ++it)
		{
			Writer.template write<double>(this->property<double>(ph, it));
		}
	}

	//! Deserialize signed int property values
	template <class S, typename tPH, typename tIt>
	void deserializePropertyValuesInt(vpl::mod::CChannelSerializer<S>& Reader, tPH &ph, tIt itBegin, tIt itEnd)
	{
		int counter(0);

		for(tIt it = itBegin; it != itEnd; ++it)
		{
			READINT32( this->property<int>(ph, it) );
			++counter;
		}

//		DBOUT("Ints deserialized: " << counter);
	}

	//! Serialize float property values
	template <class S, typename tPH, typename tIt>
	void deserializePropertyValuesFloat(vpl::mod::CChannelSerializer<S>& Reader, tPH &ph, tIt itBegin, tIt itEnd)
	{
		for(tIt it = itBegin; it != itEnd; ++it)
		{
			Reader.template read<float>(this->property<float>(ph, it));
		}
	}

	//! Serialize double property values
	template <class S, typename tPH, typename tIt>
	void deserializePropertyValuesDouble(vpl::mod::CChannelSerializer<S>& Reader, tPH &ph, tIt itBegin, tIt itEnd)
	{
		for(tIt it = itBegin; it != itEnd; ++it)
		{
			Reader.template read<double>(this->property<double>(ph, it));
		}
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
