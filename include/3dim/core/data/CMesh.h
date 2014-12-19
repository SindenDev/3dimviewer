////////////////////////////////////////////////////////////
// $Id$
////////////////////////////////////////////////////////////

#ifndef CMESH_H
#define CMESH_H


////////////////////////////////////////////////////////////
// include

#include <VPL/Module/Serializer.h>
#include <VPL/Math/Base.h>

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

#include <core/data/OMMesh.h>
#include <data/CBaseMesh.h>

#define SQRT_3 1.73205080756887729353

namespace data
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
 * data::CMesh and osg::Geometry cutter
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
    
    void cutX(data::CMesh *source);
    void cutY(data::CMesh *source);
    void cutZ(data::CMesh *source);
    void cut(data::CMesh *source);
    
private:
//    void calculateDistancesX(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
//    void calculateDistancesY(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
//    void calculateDistancesZ(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
//    void calculateDistances(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
//    void performCut(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistancesX(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistancesY(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistancesZ(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void calculateDistances(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);
    void performCut(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes = NULL);

    inline int sign(float value)
    {
//        return value < 0.0f ? -1 : (value > 0.0f ? 1 : 0);
        return (0.0f < value) - (value < 0.0f);
    }
};

////////////////////////////////////////////////////////////
// Helper classes
////////////////////////////////////////////////////////////



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

    //! Cutting OM-mesh by planes
    static bool cutByXPlane(data::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition);
    static bool cutByYPlane(data::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition);
    static bool cutByZPlane(data::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition);
    static bool cutByPlane(data::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, osg::Plane plane, osg::Matrix worldMatrix);

    //! Cutting OSG geometry by planes
    static bool cutByPlane(osg::Geometry *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, osg::Plane plane, osg::Matrix worldMatrix);

    void getVerticesInRange(std::vector<data::CMesh::VertexHandle> &vertices, std::vector<double> &distances, data::CMesh::Point point, double distance);

	//! Set model mesh property serializable
	void setSerializedProperty(const std::string &property_name, EPPType type, EPPValueType value_type) { m_pp[type][property_name] = value_type; }

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
        for (typename data::CMesh::VertexIter vit = this->vertices_begin(); vit != this->vertices_end(); ++vit)
        {
            typename data::CMesh::VertexHandle vertex = vit.handle();
            typename data::CMesh::Point point = this->point(vertex);
            this->property<vpl::sys::tUInt32>(vProp_bufferIndex, vertex) = vIndex;
            Writer.write(point[0]);
            Writer.write(point[1]);
            Writer.write(point[2]);
            vIndex++;
        }

        // write faces
        for (data::CMesh::FaceIter fit = this->faces_begin(); fit != this->faces_end(); ++fit)
        {
            data::CMesh::FaceHandle face = fit.handle();
            for (data::CMesh::FaceVertexIter fvit = this->fv_begin(face); fvit != this->fv_end(face); ++fvit)
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
        std::vector<data::CMesh::VertexHandle> vertexHandles;
        for (unsigned int v = 0; v < vertexCount; ++v)
        {
            data::CMesh::Point::value_type data[3];
            Reader.read(data, 3);
            vertexHandles.push_back(this->add_vertex(data::CMesh::Point(data)));            
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
				for( vpl::sys::tUInt32 i = 0; i < num; ++i )
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
    double quality(data::CMesh::FaceHandle fh);
    double quality(const data::CMesh::Point& p0, const data::CMesh::Point& p1, const data::CMesh::Point& p2)
    {
        double per = perimeter(p0, p1, p2);
        double max = max_edge_length(p0, p1, p2);
        return (per - 2.0 * max) / max;
    }


    //! calculates area that is covered by face
    double area(data::CMesh::FaceHandle fh);
    double area(const data::CMesh::Point& p0, const data::CMesh::Point& p1, const data::CMesh::Point& p2)
    {
        return ((p1 - p0) % (p2 - p0)).length() / 2;
    }

    //! calculates perimeter of face
    double perimeter(data::CMesh::FaceHandle fh);
    double perimeter(const data::CMesh::Point& p0, const data::CMesh::Point& p1, const data::CMesh::Point& p2)
    {
        return (p0 - p1).length() + (p1 - p2).length() + (p2 - p0).length();
    }

    //! calculates length of shortest/longest edge
    double min_edge_length(data::CMesh::FaceHandle fh);
    double max_edge_length(data::CMesh::FaceHandle fh);
    double min_edge_length(const data::CMesh::Point& p0, const data::CMesh::Point& p1, const data::CMesh::Point& p2)
    {
        return vpl::math::getMin<double>((p0 - p1).length(), (p1 - p2).length(), (p2 - p0).length());
    }
    double max_edge_length(const data::CMesh::Point& p0, const data::CMesh::Point& p1, const data::CMesh::Point& p2)
    {
        return vpl::math::getMax<double>((p0 - p1).length(), (p1 - p2).length(), (p2 - p0).length());
    }

    //! return shortest/longest edge
    data::CMesh::EdgeHandle min_edge(data::CMesh::FaceHandle fh);
    data::CMesh::EdgeHandle max_edge(data::CMesh::FaceHandle fh);

    //! returns face that shares specified edge
    data::CMesh::FaceHandle neighbour(data::CMesh::FaceHandle fh, data::CMesh::EdgeHandle eh);

    //! returns the remaining vertex of face
    data::CMesh::VertexHandle rest_vertex(data::CMesh::FaceHandle fh, data::CMesh::VertexHandle vh0, data::CMesh::VertexHandle vh1);

    //! calculates normal of face
    data::CMesh::Normal calc_face_normal(const data::CMesh::Point& p0, const data::CMesh::Point& p1, const data::CMesh::Point& p2)
    {
        return PolyMesh::calc_face_normal(p0, p1, p2);
    }
    data::CMesh::Normal calc_face_normal(data::CMesh::FaceHandle fh);

    //! calculates bounding box of mesh
    bool calc_bounding_box(data::CMesh::Point &min, data::CMesh::Point &max);

    //! calculates average vertex of model
    bool calc_average_vertex(data::CMesh::Point &average);

	//! Transform mesh vertices
	void translate(float x, float y, float z);

	//! Do complete transform by matrix
	void transform(const osg::Matrix &tm);

private:
	//! Serialize property 
	template <class S>
	void serializeProperty(vpl::mod::CChannelSerializer<S>& Writer, EPPType property_type, EPPValueType value_type, const std::string &name)
	{
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
			case data::CMesh::PPV_INT: \
				{ \
					handle_type<int> ph; \
					bool rv(this->get_property_handle(ph, name)); \
					assert(rv); \
					serializePropertyValuesInt(Writer, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			case data::CMesh::PPV_FLOAT: \
				{ \
					handle_type<float> ph; \
					bool rv(this->get_property_handle(ph, name)); \
					assert(rv); \
					serializePropertyValuesFloat(Writer, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			case data::CMesh::PPV_DOUBLE: \
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
			case data::CMesh::PPV_INT: \
				{ \
					handle_type<int> ph; \
					this->add_property(ph, name); \
					deserializePropertyValuesInt(Reader, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			case data::CMesh::PPV_FLOAT: \
				{ \
					handle_type<float> ph; \
					this->add_property(ph, name); \
					deserializePropertyValuesFloat(Reader, ph, this->it_begin(), this->it_end()); \
				} \
				break; \
			case data::CMesh::PPV_DOUBLE: \
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

};

////////////////////////////////////////////////////////////
/*!
 * Octree subdivision of mesh
 */
class CMeshOctreeNode
{
public:
    osg::BoundingBox boundingBox; //!  bounding box of node
    int nodes[8]; //! children indices of node
    std::vector<data::CMesh::FaceHandle> faces; //! list of faces
    std::vector<data::CMesh::VertexHandle> vertices; //! list of vertices
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
    void update(data::CMesh *mesh, osg::BoundingBox boundingBox);

    //! Gets list of intersected nodes
//    std::vector<CMeshOctreeNode *> getIntersectedNodes(osg::Plane plane);
    const std::vector<CMeshOctreeNode *>& getIntersectedNodes(osg::Plane plane);
    const std::vector<CMeshOctreeNode *>& getIntersectedNodes(osg::BoundingBox boundingBox);

private:
    void intersect(CMeshOctreeNode &node, osg::Plane plane);
    void intersect(CMeshOctreeNode &node, osg::BoundingBox boundingBox);
    void initializeNode(CMeshOctreeNode &node, osg::BoundingBox boundingBox);
    void fillFaceLists(data::CMesh *mesh);
    void fillVertexLists(data::CMesh *mesh);
    bool assignFace(CMeshOctreeNode &node, int level, data::CMesh::FaceHandle face, osg::BoundingBox faceBoundingBox);
    bool assignVertex(CMeshOctreeNode &node, int level, data::CMesh::VertexHandle vertex, osg::Vec3 coordinates);
    static int calculateNodeCount(int numOfLevels);
};

} // namespace data

#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
