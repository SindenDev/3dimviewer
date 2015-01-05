////////////////////////////////////////////////////////////
// $Id$
////////////////////////////////////////////////////////////

#ifdef _OPENMP
    #include <omp.h> // has to be included first, don't ask me why
#endif

#include "data/CMesh.h"
#include <cmath>
// Debugging 
//#include <bluedent/osg/dbout.h> // this dependency is not allowed
#include <VPL/System/Stopwatch.h>

// ati drivers (13.251.9001.1001 2014-07-04) end up with heap corruption when drawing extremely short lines, therefore drop them
#define OUTPUT_PRECISION 0.001

namespace data
{

////////////////////////////////////////////////////////////
/*!
 * Octree node for subdivison of mesh
 */
CMeshOctreeNode::CMeshOctreeNode()
{
    faceCount = 0;
    for (int i = 0; i < 8; i++)
    {
        nodes[i] = -1;
    }
}

//
CMeshOctreeNode::CMeshOctreeNode(const CMeshOctreeNode &node)
{
    for (int i = 0; i < 8; i++)
    {
        nodes[i] = node.nodes[i];
    }

    boundingBox = node.boundingBox;
    faceCount = node.faceCount;
    faces = node.faces;
}

//
CMeshOctreeNode &CMeshOctreeNode::operator=(const CMeshOctreeNode &node)
{
    if (&node == this)
    {
        return *this;
    }

    for (int i = 0; i < 8; i++)
    {
        nodes[i] = node.nodes[i];
    }

    boundingBox = node.boundingBox;
    faceCount = node.faceCount;
    faces = node.faces;

    return *this;
}

//
CMeshOctreeNode::~CMeshOctreeNode()
{ }

////////////////////////////////////////////////////////////
/*!
 * Octree subdivison of mesh
 */
CMeshOctree::CMeshOctree()
    : m_initialized(false)
{ }

//
CMeshOctree::CMeshOctree(const CMeshOctree &octree)
{
    m_initialized = octree.m_initialized;
    m_nodes = octree.m_nodes;
    m_nextIndex = octree.m_nextIndex;
}

//
CMeshOctree &CMeshOctree::operator=(const CMeshOctree &octree)
{
    if (&octree == this)
    {
        return *this;
    }

    m_initialized = octree.m_initialized;
    m_nodes = octree.m_nodes;
    m_nextIndex = octree.m_nextIndex;

    return *this;
}

//
bool CMeshOctree::initialize(int levels)
{
    m_initialized = false;

    m_nextIndex = 1;
    int nodeCount = calculateNodeCount(levels);
    m_nodes.resize(nodeCount);

    // resolve links for (levels-1) number of levels (last level's nodes are leaves and have no children)
    int countToResolve = calculateNodeCount(levels - 1);
    for (int i = 0; i < nodeCount; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            m_nodes[i].nodes[j] = i < countToResolve ? m_nextIndex++ : -1;
        }
    }

    m_initialized = true;

    return m_initialized;
}

//
void CMeshOctree::update(CMesh *mesh, osg::BoundingBox boundingBox)
{
    if (!m_initialized)
    {
        return;
    }

    initializeNode(m_nodes[0], boundingBox);
    fillFaceLists(mesh);
    fillVertexLists(mesh);
}

//
//std::vector<CMeshOctreeNode *> CMeshOctree::getIntersectedNodes(osg::Plane plane)
const std::vector<CMeshOctreeNode *>& CMeshOctree::getIntersectedNodes(osg::Plane plane)
{
    m_intersectedNodes.clear();
    intersect(m_nodes[0], plane);
    return m_intersectedNodes;
}

//
const std::vector<CMeshOctreeNode *>& CMeshOctree::getIntersectedNodes(osg::BoundingBox boundingBox)
{
    m_intersectedNodes.clear();
    intersect(m_nodes[0], boundingBox);
    return m_intersectedNodes;
}

//
void CMeshOctree::intersect(CMeshOctreeNode &node, osg::Plane plane)
{
    // skip if node is not intersected by a plane
    if (plane.intersect(node.boundingBox) != 0)
    {
        return;
    }

    // skip if node is intersected but has no faces
    if ((node.faceCount == 0) && (node.vertexCount == 0))
    {
        return;
    }

    // add node if it has assigned faces in it's own container
    if ((node.faces.size() > 0) || (node.vertices.size() > 0))
    {
        m_intersectedNodes.push_back(&node);
    }

    // add children if node is not leaf
    if (node.nodes[0] != -1)
    {
        for (int i = 0; i < 8; ++i)
        {
            intersect(m_nodes[node.nodes[i]], plane);
        }
    }
}

//
void CMeshOctree::intersect(CMeshOctreeNode &node, osg::BoundingBox boundingBox)
{
    // skip if node is not intersected by a plane
    if (!boundingBox.intersects(node.boundingBox))
    {
        return;
    }

    // skip if node is intersected but has no faces
    if ((node.faceCount == 0) && (node.vertexCount == 0))
    {
        return;
    }

    // add node if it has assigned faces in it's own container
    if ((node.faces.size() > 0) || (node.vertices.size() > 0))
    {
        m_intersectedNodes.push_back(&node);
    }

    // add children if node is not leaf
    if (node.nodes[0] != -1)
    {
        for (int i = 0; i < 8; ++i)
        {
            intersect(m_nodes[node.nodes[i]], boundingBox);
        }
    }
}

//
CMeshOctree::~CMeshOctree()
{ }

//
void CMeshOctree::initializeNode(CMeshOctreeNode &node, osg::BoundingBox boundingBox)
{
    node.boundingBox = boundingBox;
    node.faces.clear();
    node.vertices.clear();

    if (node.nodes[0] == -1)
    {
        return;
    }

    osg::Vec3 start = osg::Vec3(boundingBox.xMin(), boundingBox.yMin(), boundingBox.zMin());
    osg::Vec3 half = boundingBox.center() - start;

    for (int z = 0; z < 2; z++)
    {
        for (int y = 0; y < 2; y++)
        {
            for (int x = 0; x < 2; x++)
            {
                osg::Vec3 min = osg::Vec3(start.x() + x * half.x(), start.y() + y * half.y(), start.z() + z * half.z());
                osg::Vec3 max = osg::Vec3(start.x() + (x + 1) * half.x(), start.y() + (y + 1) * half.y(), start.z() + (z + 1) * half.z());
                initializeNode(m_nodes[node.nodes[z * 4 + y * 2 + x]], osg::BoundingBox(min, max));
            }
        }
    }
}

//
void CMeshOctree::fillFaceLists(data::CMesh *mesh)
{
    for (data::CMesh::FaceIter fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
    {
        data::CMesh::FaceHandle face = fit.handle();
        if (!face.is_valid() || mesh->status(face).deleted())
        {
            continue;
        }

        osg::BoundingBox faceBoundingBox;
        for (data::CMesh::FaceVertexIter fvit = mesh->fv_begin(face); fvit != mesh->fv_end(face); ++fvit)
        {
            data::CMesh::Point point = mesh->point(fvit.handle());
            faceBoundingBox.expandBy(osg::Vec3(point[0], point[1], point[2]));
        }

        assignFace(m_nodes[0], 0, face, faceBoundingBox);
    }
}

//
void CMeshOctree::fillVertexLists(data::CMesh *mesh)
{
    for (data::CMesh::VertexIter vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
    {
        data::CMesh::VertexHandle vertex = vit.handle();
        data::CMesh::Point point = mesh->point(vertex);
        assignVertex(m_nodes[0], 0, vertex, osg::Vec3(point[0], point[1], point[2]));
    }
}

//
bool CMeshOctree::assignFace(CMeshOctreeNode &node, int level, data::CMesh::FaceHandle face, osg::BoundingBox faceBoundingBox)
{
    // test if triangle is entirely inside node's bounding box
    bool contains = false;
    if ((node.boundingBox.xMin() <= faceBoundingBox.xMin()) && (node.boundingBox.xMax() >= faceBoundingBox.xMax()) &&
        (node.boundingBox.yMin() <= faceBoundingBox.yMin()) && (node.boundingBox.yMax() >= faceBoundingBox.yMax()) &&
        (node.boundingBox.zMin() <= faceBoundingBox.zMin()) && (node.boundingBox.zMax() >= faceBoundingBox.zMax()))
    {
        contains = true;
    }

    // if not (and node is not root), tell parent that face cannot be assigned
    if ((!contains) && (level != 0))
    {
        return false;
    }

    // increment face count as the face will be assigned
    node.faceCount++;

    // if node is leaf, assign face
    if (node.nodes[0] == -1)
    {
        node.faces.push_back(face);
        return true;
    }

    // try to assign face to one of the children
    for (int i = 0; i < 8; ++i)
    {
        if (assignFace(m_nodes[node.nodes[i]], level + 1, face, faceBoundingBox))
        {
            return true;
        }
    }

    // if face was not assigned, it either intersects children or is out of them and face has to be assigned into this node
    node.faces.push_back(face);
    return true;
}

//
bool CMeshOctree::assignVertex(CMeshOctreeNode &node, int level, data::CMesh::VertexHandle vertex, osg::Vec3 coordinates)
{
    // test if vertex is inside node's bounding box
    bool contains = node.boundingBox.contains(coordinates);

    // if not (and node is not root), tell parent that vertex cannot be assigned
    if ((!contains) && (level != 0))
    {
        return false;
    }

    // increment face count as the vertex will be assigned
    node.vertexCount++;

    // if node is leaf, assign face
    if (node.nodes[0] == -1)
    {
        node.vertices.push_back(vertex);
        return true;
    }

    // try to assign vertex to one of the children
    for (int i = 0; i < 8; ++i)
    {
        if (assignVertex(m_nodes[node.nodes[i]], level + 1, vertex, coordinates))
        {
            return true;
        }
    }

    // if vertex was not assigned, it either intersects children or is out of them and vertex has to be assigned into this node
    node.vertices.push_back(vertex);
    return true;
}

//
int CMeshOctree::calculateNodeCount(int numOfLevels)
{
    return (static_cast<int>(pow(8.0f, numOfLevels)) - 1) / (8 - 1);
}

////////////////////////////////////////////////////////////
/*!
 * data::CMesh and osg::Geometry cutter
 */
CMeshCutter::CMeshCutter()
{
    m_initialized = false;
    m_initializedOrtho = false;
}

bool CMeshCutter::initialize(osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, osg::Plane plane, osg::Matrix worldMatrix)
{
    if ((vertices == NULL) || (indices == NULL) || (!plane.valid()))
    {
        return m_initialized;
    }
    
    m_vertices = vertices;
    m_indices = indices;
    m_plane = plane;
    m_worldMatrix = worldMatrix;

    m_invWorldMatrix = osg::Matrix::inverse(m_worldMatrix);
    m_transformedPlane = m_plane;
    m_transformedPlane.transform(m_invWorldMatrix);
    m_transformedPlane.makeUnitLength();

    m_initialized = true;
    
    return m_initialized;
}

bool CMeshCutter::initialize(osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition)
{
    if ((vertices == NULL) || (indices == NULL))
    {
        return m_initializedOrtho;
    }
    
    m_vertices = vertices;
    m_indices = indices;
    m_planePosition = planePosition;

    m_initializedOrtho = true;
    
    return m_initializedOrtho;
}

//
CMeshCutter::~CMeshCutter()
{ }

//
//void CMeshCutter::calculateDistancesX(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes)
void CMeshCutter::calculateDistancesX(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes)
{
    OpenMesh::VPropHandleT<float> vProp_distance;
    source->get_property_handle(vProp_distance, "distance");

    if (intersectedNodes == NULL)
    {
        for (data::CMesh::VertexIter vit = source->vertices_begin(); vit != source->vertices_end(); ++vit)
        {
            data::CMesh::Point point = source->point(vit.handle());
            source->property(vProp_distance, vit) = m_planePosition - point[0];
        }
    }
    else
    {
        for (unsigned int i = 0; i < intersectedNodes->size(); ++i)
        {
            for (unsigned int f = 0; f < (*intersectedNodes)[i]->faces.size(); ++f)
            {
//                data::CMesh::FaceHandle face = (*intersectedNodes)[i]->faces[f];
                const data::CMesh::FaceHandle face = (*intersectedNodes)[i]->faces[f];
                for (data::CMesh::FaceVertexIter fvit = source->fv_begin(face); fvit != source->fv_end(face); ++fvit)
                {
                    data::CMesh::Point point = source->point(fvit.handle());
                    source->property(vProp_distance, fvit) = m_planePosition - point[0];
                }
            }
        }
    }
}

//
//void CMeshCutter::calculateDistancesY(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes)
void CMeshCutter::calculateDistancesY(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes)
{
    OpenMesh::VPropHandleT<float> vProp_distance;
    source->get_property_handle(vProp_distance, "distance");
    
    if (intersectedNodes == NULL)
    {
        for (data::CMesh::VertexIter vit = source->vertices_begin(); vit != source->vertices_end(); ++vit)
        {
            data::CMesh::Point point = source->point(vit.handle());
            source->property(vProp_distance, vit) = m_planePosition - point[1];
        }
    }
    else
    {
        for (unsigned int i = 0; i < intersectedNodes->size(); ++i)
        {
            for (unsigned int f = 0; f < (*intersectedNodes)[i]->faces.size(); ++f)
            {
//                data::CMesh::FaceHandle face = (*intersectedNodes)[i]->faces[f];
                const data::CMesh::FaceHandle face = (*intersectedNodes)[i]->faces[f];
                for (data::CMesh::FaceVertexIter fvit = source->fv_begin(face); fvit != source->fv_end(face); ++fvit)
                {
                    data::CMesh::Point point = source->point(fvit.handle());
                    source->property(vProp_distance, fvit) = m_planePosition - point[1];
                }
            }
        }
    }
}

//
//void CMeshCutter::calculateDistancesZ(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes)
void CMeshCutter::calculateDistancesZ(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes)
{
    OpenMesh::VPropHandleT<float> vProp_distance;
    source->get_property_handle(vProp_distance, "distance");
    
    if (intersectedNodes == NULL)
    {
        for (data::CMesh::VertexIter vit = source->vertices_begin(); vit != source->vertices_end(); ++vit)
        {
            data::CMesh::Point point = source->point(vit.handle());
            source->property(vProp_distance, vit) = m_planePosition - point[2];
        }
    }
    else
    {
        for (unsigned int i = 0; i < intersectedNodes->size(); ++i)
        {
            for (unsigned int f = 0; f < (*intersectedNodes)[i]->faces.size(); ++f)
            {
//                data::CMesh::FaceHandle face = (*intersectedNodes)[i]->faces[f];
                const data::CMesh::FaceHandle face = (*intersectedNodes)[i]->faces[f];
                for (data::CMesh::FaceVertexIter fvit = source->fv_begin(face); fvit != source->fv_end(face); ++fvit)
                {
                    data::CMesh::Point point = source->point(fvit.handle());
                    source->property(vProp_distance, fvit) = m_planePosition - point[2];
                }
            }
        }
    }
}

//
//void CMeshCutter::calculateDistances(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes)
void CMeshCutter::calculateDistances(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes)
{
    OpenMesh::VPropHandleT<float> vProp_distance;
    source->get_property_handle(vProp_distance, "distance");
    
    if (intersectedNodes == NULL)
    {
        for (data::CMesh::VertexIter vit = source->vertices_begin(); vit != source->vertices_end(); ++vit)
        {
            data::CMesh::Point point = source->point(vit.handle());
            source->property(vProp_distance, vit) = m_transformedPlane.distance(osg::Vec3(point[0], point[1], point[2]));
        }
    }
    else
    {
        for (unsigned int i = 0; i < intersectedNodes->size(); ++i)
        {
            for (unsigned int f = 0; f < (*intersectedNodes)[i]->faces.size(); ++f)
            {
//                data::CMesh::FaceHandle face = (*intersectedNodes)[i]->faces[f];
                const data::CMesh::FaceHandle face = (*intersectedNodes)[i]->faces[f];
                for (data::CMesh::FaceVertexIter fvit = source->fv_begin(face); fvit != source->fv_end(face); ++fvit)
                {
                    data::CMesh::Point point = source->point(fvit.handle());
                    source->property(vProp_distance, fvit) = m_transformedPlane.distance(osg::Vec3(point[0], point[1], point[2]));
                }
            }
        }
    }
}

//
void CMeshCutter::cutX(data::CMesh *source)
{
    if (!m_initializedOrtho)
    {
        return;
    }

    vpl::sys::CStopwatch sw;
    sw.start();

    if (source->m_octree != NULL)
    {
        m_plane = osg::Plane(1.0f, 0.0f, 0.0f, -m_planePosition);
        m_transformedPlane = m_plane;
        m_transformedPlane.transform(m_invWorldMatrix);
        m_transformedPlane.makeUnitLength();

//        std::vector<CMeshOctreeNode *> intersectedNodes = source->m_octree->getIntersectedNodes(m_transformedPlane);
        const std::vector<CMeshOctreeNode *>& intersectedNodes = source->m_octree->getIntersectedNodes(m_transformedPlane);
        if (intersectedNodes.size() == 0)
        {
            return;
        }

        OpenMesh::VPropHandleT<float> vProp_distance;
        source->add_property(vProp_distance, "distance");

        calculateDistancesX(source, &intersectedNodes);
        performCut(source, &intersectedNodes);

        source->remove_property(vProp_distance);
    }
    else
    {
        OpenMesh::VPropHandleT<float> vProp_distance;
        source->add_property(vProp_distance, "distance");

        calculateDistancesX(source);
        performCut(source);

        source->remove_property(vProp_distance);
    }

//    unsigned long duration = sw.getDuration();
//    DBOUT("Cut X duration >>> " << duration);
}

//
void CMeshCutter::cutY(data::CMesh *source)
{
    if (!m_initializedOrtho)
    {
        return;
    }

    vpl::sys::CStopwatch sw;
    sw.start();

    if (source->m_octree != NULL)
    {
        m_plane = osg::Plane(0.0f, 1.0f, 0.0f, -m_planePosition);
        m_transformedPlane = m_plane;
        m_transformedPlane.transform(m_invWorldMatrix);
        m_transformedPlane.makeUnitLength();

//        std::vector<CMeshOctreeNode *> intersectedNodes = source->m_octree->getIntersectedNodes(m_transformedPlane);
        const std::vector<CMeshOctreeNode *>& intersectedNodes = source->m_octree->getIntersectedNodes(m_transformedPlane);
        if (intersectedNodes.size() == 0)
        {
            return;
        }

        OpenMesh::VPropHandleT<float> vProp_distance;
        source->add_property(vProp_distance, "distance");

        calculateDistancesY(source, &intersectedNodes);
        performCut(source, &intersectedNodes);

        source->remove_property(vProp_distance);
    }
    else
    {
        OpenMesh::VPropHandleT<float> vProp_distance;
        source->add_property(vProp_distance, "distance");

        calculateDistancesY(source);
        performCut(source);

        source->remove_property(vProp_distance);
    }

//    unsigned long duration = sw.getDuration();
//    DBOUT("Cut Y duration >>> " << duration);
}

//
void CMeshCutter::cutZ(data::CMesh *source)
{
    if (!m_initializedOrtho)
    {
        return;
    }

    vpl::sys::CStopwatch sw;
    sw.start();

    if (source->m_octree != NULL)
    {
        m_plane = osg::Plane(0.0f, 0.0f, 1.0f, -m_planePosition);
        m_transformedPlane = m_plane;
        m_transformedPlane.transform(m_invWorldMatrix);
        m_transformedPlane.makeUnitLength();

//        std::vector<CMeshOctreeNode *> intersectedNodes = source->m_octree->getIntersectedNodes(m_transformedPlane);
        const std::vector<CMeshOctreeNode *>& intersectedNodes = source->m_octree->getIntersectedNodes(m_transformedPlane);
        if (intersectedNodes.size() == 0)
        {
            return;
        }

        OpenMesh::VPropHandleT<float> vProp_distance;
        source->add_property(vProp_distance, "distance");

        calculateDistancesZ(source, &intersectedNodes);
        performCut(source, &intersectedNodes);

        source->remove_property(vProp_distance);
    }
    else
    {
        OpenMesh::VPropHandleT<float> vProp_distance;
        source->add_property(vProp_distance, "distance");

        calculateDistancesZ(source);
        performCut(source);

        source->remove_property(vProp_distance);
    }

//    unsigned long duration = sw.getDuration();
//    DBOUT("Cut Z duration >>> " << duration);
}

//
void CMeshCutter::cut(data::CMesh *source)
{
    if (!m_initialized)
    {
        return;
    }

    vpl::sys::CStopwatch sw;
    sw.start();

    if (source->m_octree != NULL)
    {
//        std::vector<CMeshOctreeNode *> intersectedNodes = source->m_octree->getIntersectedNodes(m_transformedPlane);
        const std::vector<CMeshOctreeNode *>& intersectedNodes = source->m_octree->getIntersectedNodes(m_transformedPlane);
        if (intersectedNodes.size() == 0)
        {
            return;
        }

        OpenMesh::VPropHandleT<float> vProp_distance;
        source->add_property(vProp_distance, "distance");

        calculateDistances(source, &intersectedNodes);
        performCut(source, &intersectedNodes);

        source->remove_property(vProp_distance);
    }
    else
    {
        OpenMesh::VPropHandleT<float> vProp_distance;
        source->add_property(vProp_distance, "distance");

        calculateDistances(source);
        performCut(source);

        source->remove_property(vProp_distance);
    }

//    unsigned long duration = sw.getDuration();
//    DBOUT("Cut duration >>> " << duration);
}

//#define omp_parallel
//void CMeshCutter::performCut(data::CMesh *source, std::vector<CMeshOctreeNode *> *intersectedNodes)
void CMeshCutter::performCut(data::CMesh *source, const std::vector<CMeshOctreeNode *> *intersectedNodes)
{
    OpenMesh::VPropHandleT<float> vProp_distance;
    source->get_property_handle(vProp_distance, "distance");

    if (intersectedNodes == NULL)
    {
//        #ifdef omp_parallel
//        #ifdef _OPENMP
        #if 0
        std::vector<CMesh::FaceIter> start;
        int parts = source->n_faces() / 4096 + 1;
        int partCount = source->n_faces() / parts;
        int index = 0;
        for (data::CMesh::FaceIter fit = source->faces_begin(); fit != source->faces_end(); ++fit)
        {
            if (index % partCount == 0)
            {
                start.push_back(fit);
            }
            index++;
        }
        start.push_back(source->faces_end());

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < start.size() - 1; ++i)
        {
            data::CMesh::FaceIter fitstart = start[i];
            data::CMesh::FaceIter fitend = start[i + 1];

            for (data::CMesh::FaceIter fit = fitstart; fit != fitend; ++fit)
            {
                data::CMesh::FaceHandle face = fit.handle();
                std::vector<osg::Vec3> vertices;
                std::vector<float> distances;

                for (data::CMesh::FaceVertexIter fvit = source->fv_begin(face); fvit != source->fv_end(face); ++fvit)
                {
                    data::CMesh::Point vertex = source->point(fvit.handle());
                    vertices.push_back(osg::Vec3(vertex[0], vertex[1], vertex[2]));
                    distances.push_back(source->property(vProp_distance, fvit.handle()));
                }

                if (vertices.size() != 3)
                {
                    continue;
                }

#pragma omp critical
                this->operator()(vertices[0], vertices[1], vertices[2], distances[0], distances[1], distances[2]);
            }
        }
        #else
        for (data::CMesh::FaceIter fit = source->faces_begin(); fit != source->faces_end(); ++fit)
        {
            data::CMesh::FaceHandle face = fit.handle();
            std::vector<osg::Vec3> vertices;
            std::vector<float> distances;

            for (data::CMesh::FaceVertexIter fvit = source->fv_begin(face); fvit != source->fv_end(face); ++fvit)
            {
                data::CMesh::Point vertex = source->point(fvit.handle());
                vertices.push_back(osg::Vec3(vertex[0], vertex[1], vertex[2]));
                distances.push_back(source->property(vProp_distance, fvit.handle()));
            }

            if (vertices.size() != 3)
            {
                continue;
            }

            this->operator()(vertices[0], vertices[1], vertices[2], distances[0], distances[1], distances[2]);
        }
        #endif
    }
    else
    {   
        //int clk = clock();

#ifdef _OPENMP        

        // limit to 8 threads
        int nThreads=omp_get_max_threads( ); 
        if (nThreads>8)
            nThreads=8;
        
        std::vector<std::vector<osg::Vec3> > vLists[8]; // list of lists for every thread
                
        #pragma omp parallel for schedule(dynamic) num_threads(nThreads)
#endif
        for( int i = 0; i < int(intersectedNodes->size()); ++i )
        {
            for( int f = 0; f < int((*intersectedNodes)[i]->faces.size()); ++f)
            {
                data::CMesh::FaceHandle face = (*intersectedNodes)[i]->faces[f];
                std::vector<osg::Vec3> vertices;
                std::vector<float> distances;

                for (data::CMesh::FaceVertexIter fvit = source->fv_begin(face); fvit != source->fv_end(face); ++fvit)
                {
                    data::CMesh::Point vertex = source->point(fvit.handle());
                    vertices.push_back(osg::Vec3(vertex[0], vertex[1], vertex[2]));
                    distances.push_back(source->property(vProp_distance, fvit.handle()));
                }

                if (vertices.size() != 3)
                {
                    continue;
                }

#ifndef _OPENMP
                // This acesses a shared resource so it cannot run in parallel!!!
                this->operator()(vertices[0], vertices[1], vertices[2], distances[0], distances[1], distances[2]);
#else
                int iThread = omp_get_thread_num(); // index of thread

                const osg::Vec3 &v1 = vertices[0]; 
                const osg::Vec3 &v2 = vertices[1]; 
                const osg::Vec3 &v3 = vertices[2];
                float d1 = distances[0];
                float d2 = distances[1];
                float d3 = distances[2];
                // completely above or completely below plane
                if ((sign(d1) == sign(d2)) && (sign(d1) == sign(d3)) && (sign(d1) != 0))
                {
                    continue;
                }

                std::vector<osg::Vec3> newVertices;

                // find intersection between v1 and v2
                if (sign(d1) != sign(d2))
                {
                    float a1 = fabs(d1);
                    float a2 = fabs(d2);
                    float distance = a1 / (a1 + a2);
                    newVertices.push_back(v1 + (v2 - v1) * distance);
                }

                // find intersection between v2 and v3
                if (sign(d2) != sign(d3))
                {
                    float a2 = fabs(d2);
                    float a3 = fabs(d3);
                    float distance = a2 / (a2 + a3);
                    newVertices.push_back(v2 + (v3 - v2) * distance);
                }

                // find intersection between v1 and v3
                if (sign(d1) != sign(d3))
                {
                    float a1 = fabs(d1);
                    float a3 = fabs(d3);
                    float distance = a1 / (a1 + a3);
                    newVertices.push_back(v1 + (v3 - v1) * distance);
                }

                vLists[iThread].push_back(newVertices);
#endif
            }
        }
#ifdef _OPENMP
        // add new vertices to geometry
        for(int t = 0 ; t<nThreads; t++) // for every thread
        {
            int cnt=vLists[t].size();
            for(int l=0;l<cnt;l++) // for every list
            {
                const std::vector<osg::Vec3>&  newVertices = vLists[t].at(l);
                // input list can contain >2 vertices that are to be connected
                for (unsigned int i = 0; i < std::min(2,(int)newVertices.size()); i++)
                {
                    // ati drivers (13.251.9001.1001 2014-07-04) end up with heap corruption when drawing extremely short lines, therefore drop them
                    if (i>0 && (newVertices[i]-m_vertices->back()).length()<OUTPUT_PRECISION)
                    {
                        m_vertices->pop_back();
                    }
                    else
                    {
                        m_vertices->push_back(newVertices[i]);
                    }
                }
            }
        }
        assert(m_vertices->size()%2==0);
        for(int i = 0; i < m_vertices->size(); i++)
        {
            m_indices->push_back(i);
        }
#endif

#if(0)
        clk = clock() - clk;
        char sss[32];
        sprintf(sss,"cut: %d",clk);
        VPL_LOG_INFO(sss);
        OutputDebugStringA(sss);
#endif
    }
}

//
void CMeshCutter::operator()(const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, float d1, float d2, float d3)
{ 
    VPL_ASSERT(m_initialized || m_initializedOrtho);
    
    // completely above or completely below plane
    if ((sign(d1) == sign(d2)) && (sign(d1) == sign(d3)) && (sign(d1) != 0))
    {
        return;
    }

    std::vector<osg::Vec3> newVertices;

    // find intersection between v1 and v2
    if (sign(d1) != sign(d2))
    {
        float a1 = fabs(d1);
        float a2 = fabs(d2);
        float distance = a1 / (a1 + a2);
        newVertices.push_back(v1 + (v2 - v1) * distance);
    }

    // find intersection between v2 and v3
    if (sign(d2) != sign(d3))
    {
        float a2 = fabs(d2);
        float a3 = fabs(d3);
        float distance = a2 / (a2 + a3);
        newVertices.push_back(v2 + (v3 - v2) * distance);
    }

    // find intersection between v1 and v3
    if (sign(d1) != sign(d3))
    {
        float a1 = fabs(d1);
        float a3 = fabs(d3);
        float distance = a1 / (a1 + a3);
        newVertices.push_back(v1 + (v3 - v1) * distance);
    }

    // add new vertices to geometry
    const int startIndex = m_vertices->size();
    // input list can contain >2 vertices to be connected
    for (unsigned int i = 0; i < std::min(2,(int)newVertices.size()); i++)
    {
        // ati drivers (13.251.9001.1001 2014-07-04) end up with heap corruption when drawing extremely short lines, therefore drop them
        if (i>0 && (newVertices[i]-m_vertices->back()).length()<OUTPUT_PRECISION)
        {
            m_vertices->pop_back();
        }
        else
        {
            m_vertices->push_back(newVertices[i]);
        }
    }
    for(int i = startIndex; i < m_vertices->size(); i++)
    {
        m_indices->push_back(i);
    }
}

void CMeshCutter::operator()(const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, bool treatVertexDataAsTemporary)
{ 
    VPL_ASSERT(m_initialized);
    
    // calculate distances
    float d1 = m_transformedPlane.distance(osg::Vec3(v1[0], v1[1], v1[2]));
    float d2 = m_transformedPlane.distance(osg::Vec3(v2[0], v2[1], v2[2]));
    float d3 = m_transformedPlane.distance(osg::Vec3(v3[0], v3[1], v3[2]));

    // completely above or completely below plane
    if ((sign(d1) == sign(d2)) && (sign(d1) == sign(d3)) && (sign(d1) != 0))
    {
        return;
    }

    std::vector<osg::Vec3> newVertices;

    // find intersection between v1 and v2
    if (sign(d1) != sign(d2))
    {
        float a1 = fabs(d1);
        float a2 = fabs(d2);
        float distance = a1 / (a1 + a2);
        newVertices.push_back(v1 + (v2 - v1) * distance);
    }

    // find intersection between v2 and v3
    if (sign(d2) != sign(d3))
    {
        float a2 = fabs(d2);
        float a3 = fabs(d3);
        float distance = a2 / (a2 + a3);
        newVertices.push_back(v2 + (v3 - v2) * distance);
    }

    // find intersection between v1 and v3
    if (sign(d1) != sign(d3))
    {
        float a1 = fabs(d1);
        float a3 = fabs(d3);
        float distance = a1 / (a1 + a3);
        newVertices.push_back(v1 + (v3 - v1) * distance);
    }

    // add new vertices to geometry
    const int startIndex = m_vertices->size();
    // input list can contain >2 vertices to be connected
    for (unsigned int i = 0; i < std::min(2,(int)newVertices.size()); i++)
    {
        // ati drivers (13.251.9001.1001 2014-07-04) end up with heap corruption when drawing extremely short lines, therefore drop them
        if (i>0 && (newVertices[i]-m_vertices->back()).length()<OUTPUT_PRECISION)
        {
            m_vertices->pop_back();
        }
        else
        {
            m_vertices->push_back(newVertices[i]);
        }
    }
    for(int i = startIndex; i < m_vertices->size(); i++)
    {
        m_indices->push_back(i);
    }
}

////////////////////////////////////////////////////////////
/*!
 * Triangular mesh
 */
//! Ctor
CMesh::CMesh()
    : m_octree(NULL)
{
    m_pp.resize(PPT_VERTEX+1);
}

//! Copy constructor
CMesh::CMesh(const CMesh &mesh) : vpl::base::CObject(), CBaseMesh(mesh), m_octree(NULL)
{
    if (mesh.m_octree != NULL)
    {
        m_octree = new CMeshOctree();
        *m_octree = *mesh.m_octree;
    }

    request_face_normals();
}

//! Assignment operator
CMesh &CMesh::operator=(const CMesh &mesh)
{
    if (&mesh == this)
    {
        return *this;
    }

    CBaseMesh::operator=(mesh);
    
    if( m_octree )
    {
        delete m_octree;
        m_octree = NULL;
    }

    if (mesh.m_octree != NULL)
    {
        m_octree = new CMeshOctree();
        *m_octree = *mesh.m_octree;
    }

    return *this;

    m_pp.clear();
    m_pp.insert(m_pp.end(), mesh.m_pp.begin(), mesh.m_pp.end());

    request_face_normals();
}

//! Dtor
CMesh::~CMesh()
{
    if( m_octree )
    {
        delete m_octree;
    }
}

//! Updates octree of mesh
void CMesh::updateOctree()
{
    if (m_octree == NULL)
    {
        m_octree = new CMeshOctree;
    }

    CMesh::Point min, max;
    calc_bounding_box(min, max);
    
    int faceCount = this->n_faces();
    int numOfLevels = std::pow(faceCount / 1024.0, 1.0 / 3.0);
    vpl::math::limit<int>(numOfLevels, 3, 6);

    m_octree->initialize(numOfLevels);
    m_octree->update(this, osg::BoundingBox(osg::Vec3(min[0], min[1], min[2]), osg::Vec3(max[0], max[1], max[2])));
}

//! Cutting by plane
bool CMesh::cutByXPlane(data::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition)
{
    if ((source == NULL) || (vertices == NULL) || (indices == NULL))
    {
        return false;
    }

    CMeshCutter meshCutter;
    meshCutter.initialize(vertices, indices, planePosition);
    meshCutter.cutX(source);

    return true;
}

//! Cutting by plane
bool CMesh::cutByYPlane(data::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition)
{
    if ((source == NULL) || (vertices == NULL) || (indices == NULL))
    {
        return false;
    }

    CMeshCutter meshCutter;
    meshCutter.initialize(vertices, indices, planePosition);
    meshCutter.cutY(source);

    return true;
}

//! Cutting by plane
bool CMesh::cutByZPlane(data::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, float planePosition)
{
    if ((source == NULL) || (vertices == NULL) || (indices == NULL))
    {
        return false;
    }

    CMeshCutter meshCutter;
    meshCutter.initialize(vertices, indices, planePosition);
    meshCutter.cutZ(source);

    return true;
}

//! Cutting by plane
bool CMesh::cutByPlane(data::CMesh *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, osg::Plane plane, osg::Matrix worldMatrix)
{
    if ((source == NULL) || (vertices == NULL) || (indices == NULL) || (!plane.valid()))
    {
        return false;
    }

    CMeshCutter meshCutter;
    meshCutter.initialize(vertices, indices, plane, worldMatrix);
    meshCutter.cut(source);

    return true;
}

//! Cutting by plane
bool CMesh::cutByPlane(osg::Geometry *source, osg::Vec3Array *vertices, osg::DrawElementsUInt *indices, osg::Plane plane, osg::Matrix worldMatrix)
{
    if ((source == NULL) || (vertices == NULL) || (indices == NULL))
    {
        return false;
    }

    vpl::sys::CStopwatch sw;
    sw.start();

    osg::TriangleFunctor<CMeshCutter> meshCutter;
    meshCutter.initialize(vertices, indices, plane, worldMatrix);
    source->accept(meshCutter);

//    unsigned long duration = sw.getDuration();
//    DBOUT("Cut duration >>> " << duration);

    return true;
}

void CMesh::getVerticesInRange(std::vector<data::CMesh::VertexHandle> &vertices, std::vector<double> &distances, data::CMesh::Point referencePoint, double distance)
{
    vertices.clear();
    distances.clear();

    if (m_octree != NULL)
    {
        osg::Vec3 min = osg::Vec3(referencePoint[0] - distance, referencePoint[1] - distance, referencePoint[2] - distance);
        osg::Vec3 max = osg::Vec3(referencePoint[0] + distance, referencePoint[1] + distance, referencePoint[2] + distance);
        const std::vector<CMeshOctreeNode*>& intersectedNodes = m_octree->getIntersectedNodes(osg::BoundingBox(min, max));
        for (int i = 0; i < intersectedNodes.size(); ++i)
        {
            for (std::vector<data::CMesh::VertexHandle>::iterator vit = intersectedNodes[i]->vertices.begin(); vit != intersectedNodes[i]->vertices.end(); ++vit)
            {
                double currDistance = (referencePoint - point(*vit)).length();
                if (currDistance < distance)
                {
                    vertices.push_back(*vit);
                    distances.push_back(currDistance);
                }
            }
        }
    }
    else
    {
        for (data::CMesh::VertexIter vit = vertices_begin(); vit != vertices_end(); ++vit)
        {
            double currDistance = (referencePoint - point(*vit)).length();
            if (currDistance < distance)
            {
                vertices.push_back(vit.handle());
                distances.push_back(currDistance);
            }
        }
    }
}

//! calculates quality of face
double CMesh::quality(data::CMesh::FaceHandle fh)
{
    double per = perimeter(fh);
    double max = max_edge_length(fh);
    return (per == 0.0 ? 0.0 : (per - 2.0 * max) / max);

    //double lengths[3];
    //int i = 0;
    //for (CMesh::FaceEdgeIter feit = this->fe_begin(fh); feit != this->fe_end(fh); ++feit)
    //{
    //    lengths[i] = this->calc_edge_length(feit.handle());
    //    i++;
    //}
    //double max = vpl::math::getMax<double>(lengths[0], lengths[1], lengths[2]);
    //double min = vpl::math::getMin<double>(lengths[0], lengths[1], lengths[2]);
    //return min / max;

    //return (12.0 * area(fh)) / (SQRT_3 * max_edge_length(fh) * perimeter(fh));
}

//double CMesh::quality(data::CMesh::Point p0, data::CMesh::Point p1, data::CMesh::Point p2)
/*double CMesh::quality(const data::CMesh::Point& p0, const data::CMesh::Point& p1, const data::CMesh::Point& p2)
{
    double per = perimeter(p0, p1, p2);
    double max = max_edge_length(p0, p1, p2);
    return (per - 2.0 * max) / max;
    
    //double max = vpl::math::getMax<double>((p0 - p1).length(), (p1 - p2).length(), (p2 - p0).length());
    //double min = vpl::math::getMin<double>((p0 - p1).length(), (p1 - p2).length(), (p2 - p0).length());
    //return min / max;
    
    //return (12.0 * area(p0, p1, p2)) / (SQRT_3 * max_edge_length(p0, p1, p2) * perimeter(p0, p1, p2));
}*/

//! calculates area that is covered by face
double CMesh::area(data::CMesh::FaceHandle fh)
{
    CMesh::Point vertices[3];
    int i = 0;
    for (CMesh::FaceVertexIter fvit = this->fv_begin(fh); fvit != this->fv_end(fh); ++fvit)
    {
        vertices[i] = this->point(fvit);
        i++;
    }
        
    OpenMesh::Vec3d vec0(vertices[1] - vertices[0]);
    OpenMesh::Vec3d vec1(vertices[2] - vertices[0]);        

    // area = 1/2 * cross_product(vec(u0, u1), vec2(u0, u2))
    return (vec0 % vec1).length() / 2;
}

//double CMesh::area(data::CMesh::Point p0, data::CMesh::Point p1, data::CMesh::Point p2)
/*double CMesh::area(const data::CMesh::Point& p0, const data::CMesh::Point& p1, const data::CMesh::Point& p2)
{
    // area = 1/2 * cross_product(vec(u0, u1), vec2(u0, u2))
    return ((p1 - p0) % (p2 - p0)).length() / 2;
}*/

//! calculates perimeter of face
double CMesh::perimeter(data::CMesh::FaceHandle fh)
{
    double result = 0.0;

    for (CMesh::FaceEdgeIter feit = this->fe_begin(fh); feit != this->fe_end(fh); ++feit)
    {
        result += this->calc_edge_length(feit.handle());
    }
        
    return result;
}

//! calculates length of longest edge
double CMesh::min_edge_length(data::CMesh::FaceHandle fh)
{
    double lengths[3];
    int i = 0;
    for (CMesh::FaceEdgeIter feit = this->fe_begin(fh); feit != this->fe_end(fh); ++feit)
    {
        lengths[i] = this->calc_edge_length(feit.handle());
        i++;
    }

    return vpl::math::getMin<double>(lengths[0], lengths[1], lengths[2]);
}

//! calculates length of longest edge
double CMesh::max_edge_length(data::CMesh::FaceHandle fh)
{
    double lengths[3];
    int i = 0;
    for (CMesh::FaceEdgeIter feit = this->fe_begin(fh); feit != this->fe_end(fh); ++feit)
    {
        lengths[i] = this->calc_edge_length(feit.handle());
        i++;
    }

    return vpl::math::getMax<double>(lengths[0], lengths[1], lengths[2]);
}

//! return shortest edge
data::CMesh::EdgeHandle CMesh::min_edge(data::CMesh::FaceHandle fh)
{
    data::CMesh::EdgeHandle minEdge;
    data::CMesh::EdgeHandle currEdge;
    double minLength = -1.0;
    double currLength;

    for (CMesh::FaceEdgeIter feit = this->fe_begin(fh); feit != this->fe_end(fh); ++feit)
    {
        currEdge = feit.handle();
        currLength = this->calc_edge_length(currEdge);

        if (currLength != -1 && currLength < minLength)
        {
            minLength = currLength;
            minEdge = currEdge;
        }
    }

    return minEdge;
}

//! return longest edge
data::CMesh::EdgeHandle CMesh::max_edge(data::CMesh::FaceHandle fh)
{
    data::CMesh::EdgeHandle maxEdge;
    data::CMesh::EdgeHandle currEdge;
    double maxLength = -1.0;
    double currLength;

    for (CMesh::FaceEdgeIter feit = this->fe_begin(fh); feit != this->fe_end(fh); ++feit)
    {
        currEdge = feit.handle();
        currLength = this->calc_edge_length(currEdge);

        if (currLength > maxLength)
        {
            maxLength = currLength;
            maxEdge = currEdge;
        }
    }

    return maxEdge;
}

//! returns face that shares specified edge
data::CMesh::FaceHandle CMesh::neighbour(data::CMesh::FaceHandle fh, data::CMesh::EdgeHandle eh)
{
    data::CMesh::FaceHandle retNeighbour;
    retNeighbour.invalidate();

    for (data::CMesh::FaceFaceIter ffit = this->ff_begin(fh); ffit != this->ff_end(fh); ++ffit)
    {
        data::CMesh::FaceHandle curr = ffit.handle();
        if (curr.is_valid())
        {
            for (data::CMesh::FaceEdgeIter feit = this->fe_begin(curr); feit != this->fe_end(curr); ++feit)
            {
                if (feit.handle() == eh)
                {
                    retNeighbour = curr;
                    break;
                }
            }
        }
    }

    return retNeighbour;
}

//! returns the remaining vertex of face
data::CMesh::VertexHandle CMesh::rest_vertex(data::CMesh::FaceHandle fh, data::CMesh::VertexHandle vh0, data::CMesh::VertexHandle vh1)
{
    data::CMesh::VertexHandle vertex;
    vertex.invalidate();
        
    for (data::CMesh::FaceVertexIter fvit = this->fv_begin(fh); fvit != this->fv_end(fh); ++fvit)
    {
        data::CMesh::VertexHandle curr = fvit.handle();
        if ((curr != vh0) && (curr != vh1))
        {
            vertex = curr;
            break;
        }
    }

    return vertex;
}

//! calculates normal of face
//data::CMesh::Normal CMesh::calc_face_normal(data::CMesh::Point p0, data::CMesh::Point p1, data::CMesh::Point p2)
/*data::CMesh::Normal CMesh::calc_face_normal(const data::CMesh::Point& p0, const data::CMesh::Point& p1, const data::CMesh::Point& p2)
{
    return PolyMesh::calc_face_normal(p0, p1, p2);
}*/

data::CMesh::Normal CMesh::calc_face_normal(data::CMesh::FaceHandle fh)
{
    return PolyMesh::calc_face_normal(fh);
}

bool CMesh::calc_bounding_box(data::CMesh::Point &min, data::CMesh::Point &max)
{
    bool first = true;

    for (CMesh::VertexIter vit = this->vertices_begin(); vit != this->vertices_end(); ++vit)
    {
        if(this->status(vit).deleted())
            continue;

        CMesh::Point point = this->point(vit.handle());
        
        if (first)
        {
            first = false;
            min = max = point;
        }
        else
        {
            min[0] = vpl::math::getMin<float>(min[0], point[0]);
            min[1] = vpl::math::getMin<float>(min[1], point[1]);
            min[2] = vpl::math::getMin<float>(min[2], point[2]);

            max[0] = vpl::math::getMax<float>(max[0], point[0]);
            max[1] = vpl::math::getMax<float>(max[1], point[1]);
            max[2] = vpl::math::getMax<float>(max[2], point[2]);
        }
    }

    return (!first);
}

bool CMesh::calc_average_vertex(data::CMesh::Point &average)
{
    if (this->n_vertices() == 0)
    {
        return false;
    }

    average = data::CMesh::Point(0.0, 0.0, 0.0);
    for (CMesh::VertexIter vit = this->vertices_begin(); vit != this->vertices_end(); ++vit)
    {
        average += this->point(vit.handle()) * (1.0 / this->n_vertices());
    }

    return true;
}

/**
 * Translates.
 *
 * \param   x   The x coordinate.
 * \param   y   The y coordinate.
 * \param   z   The z coordinate.
**/
void CMesh::translate( float x, float y, float z )
{
    CMesh::VertexIter v_it, v_end(vertices_end());
    for (v_it=vertices_begin(); v_it!=v_end; ++v_it)
    {
        Point p(point(v_it));
        p[0] += x; p[1] += y; p[2] += z;
        set_point(v_it, p);
    }
}

/**
 * Transforms.
 *
 * \param   tm  The transform matrix.
**/
void CMesh::transform( const osg::Matrix &tm )
{
    CMesh::VertexIter v_it, v_end(vertices_end());
    for (v_it=vertices_begin(); v_it!=v_end; ++v_it)
    {
        Point p(point(v_it));

        // Transform to osg
        osg::Vec3 osg_point(p[0], p[1], p[2]);

        // Do transformation
        osg_point = osg_point * tm;

        // Transform back to openmesh
        p[0] = osg_point[0];
        p[1] = osg_point[1];
        p[2] = osg_point[2];

        set_point(v_it, p);
    }
}

} // namespace data
