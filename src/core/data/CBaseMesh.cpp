////////////////////////////////////////////////////////////
// $Id$
////////////////////////////////////////////////////////////

#ifdef _OPENMP
    #include <omp.h> // has to be included first, don't ask me why
#endif

#include <data/CBaseMesh.h>

namespace data
{

////////////////////////////////////////////////////////////
/*!
 * Triangular mesh
 */
//! Ctor
CBaseMesh::CBaseMesh()
{ }

//! Copy constructor
CBaseMesh::CBaseMesh(const CBaseMesh &mesh) : OMMesh(mesh)
{
}

CBaseMesh::CBaseMesh(const data::OMMesh &mesh)  : OMMesh(mesh)
{
}

//! Assignment operator
CBaseMesh &CBaseMesh::operator=(const CBaseMesh &mesh)
{
    if (&mesh == this)
    {
        return *this;
    }

    OMMesh::operator=(mesh);
    
    return *this;
}

//! Dtor
CBaseMesh::~CBaseMesh()
{
}


int CBaseMesh::boundaryCount() 
{  
    int nBounds = 0;

    OpenMesh::HPropHandleT< bool > visited;
    add_property(visited);

    // set to false for all half edges
    for (HalfedgeIter he_it = halfedges_begin(); he_it != halfedges_end() ; ++he_it ) 
        property(visited,he_it) = false;

    // go through all half edges
    for (HalfedgeIter he_it = halfedges_begin(); he_it != halfedges_end() ; ++he_it ) 
    {
        if ( property(visited,he_it) )
            continue;
        if( !is_boundary(he_it ) )
            continue;
        property(visited,he_it) = true;

        // go along boundary half edges
        HalfedgeHandle he = next_halfedge_handle(he_it);    
        while( is_boundary(he) && ! property(visited,he) ) 
        {
            property(visited,he) = true;
            he = next_halfedge_handle(he);
        }    
        nBounds++;
    }

    remove_property(visited);  

    return nBounds;
}

int CBaseMesh::componentCount() 
{  
    int nComponents = 0;  

    OpenMesh::VPropHandleT< bool > visited;
    add_property(visited);

    VertexIter v_it, v_end = vertices_end();

    // iterate over all vertices
    for (v_it = vertices_begin(); v_it != v_end; ++v_it)
        property(visited, v_it) = false;
    
    VertexIter current_pos = vertices_begin();
    while( true )
    {
        // find an unvisited vertex
        VertexHandle vh;
        for (v_it = current_pos ; v_it != v_end; ++v_it)
        {
            if ( !property(visited, v_it) )
            {
                property(visited, v_it) = true;
                current_pos = v_it;
                vh = v_it.handle();                
                break;
            }
        }

        // none was found -> done
        if (!vh.is_valid()) break;

        nComponents++;

        std::vector<VertexHandle> handles;
        handles.push_back( vh );

        // go from found vertex
        while( handles.size() > 0 )
        {
            VertexHandle current = handles.back();
            handles.pop_back();

            for (VertexVertexIter vv_it = vv_iter( current ); vv_it; ++vv_it)
            {
                if ( !property(visited, vv_it) )
                {
                    property(visited, vv_it) = true;
                    handles.push_back( vv_it.handle() );
                }
            }
        }
    }  
    remove_property(visited);  
    return nComponents;
}

bool CBaseMesh::isClosed()
{
    CBaseMesh::ConstEdgeIter e_it, e_end = edges_end();
    for (e_it = edges_begin(); e_it != e_end; ++e_it)
    {
        if (!status(e_it).deleted())
        {
            if (is_boundary(e_it.handle()))
                return false;
        }
    }
    return true;
}

bool CBaseMesh::isTwoManifold()
{
    // check whether is 2-manifold
    CBaseMesh::ConstVertexIter ve_it = vertices_end();
    for (CBaseMesh::ConstVertexIter v_it = vertices_begin(); v_it!=ve_it; ++v_it)
    {
        if (!is_manifold(v_it))
            return false;
    }
    return true;
}

double CBaseMesh::meshVolume()
{
    double volume = 0;
    FaceIter f_end = faces_end();
    std::vector<FaceHandle> facesList;
    for (FaceIter f_it = faces_begin(); f_it != faces_end(); ++f_it)
        facesList.push_back(f_it.handle());

    const int nFaces = facesList.size();
#pragma omp parallel for
    for(int i=0; i<nFaces; i++)
    {
        FaceHandle f_it = facesList[i];
        std::vector<CBaseMesh::Point> pts;                    
        for (CBaseMesh::FaceVertexIter fvit = fv_begin(f_it); fvit != fv_end(f_it); ++fvit)
            pts.push_back(point(fvit.handle()));
        double v = (1.0/6)*(
                    - pts[2][0]*pts[1][1]*pts[0][2] 
                    + pts[1][0]*pts[2][1]*pts[0][2] 
                    + pts[2][0]*pts[0][1]*pts[1][2] 
                    - pts[0][0]*pts[2][1]*pts[1][2] 
                    - pts[1][0]*pts[0][1]*pts[2][2] 
                    + pts[0][0]*pts[1][1]*pts[2][2] 
            );
#pragma omp critical
        volume += v;
    }    
    return volume;
}

void CBaseMesh::invertNormals()
{
    // get all faces, remove them and reinsert with reverse vertex order
    std::vector< std::vector<VertexHandle> > facesList;
    std::vector<FaceHandle> removed;
    for (FaceIter f_it = faces_begin(); f_it != faces_end(); ++f_it)
    {
        std::vector<VertexHandle> pts;                    
        for (CBaseMesh::FaceVertexIter fvit = fv_begin(f_it); fvit != fv_end(f_it); ++fvit)
            pts.push_back(fvit.handle());
        facesList.push_back(pts);        
        removed.push_back(f_it.handle());
    }
    int nFaces = removed.size();
    for(int i=0; i<nFaces; i++)
        delete_face(removed[i],false);
    garbage_collection();
    for(int i=0; i<nFaces; i++)
        add_face(facesList[i][0],facesList[i][2],facesList[i][1]);
    garbage_collection();
}

int CBaseMesh::fixOpenComponents()
{
    int nDropped = 0;

    std::vector<std::vector<CBaseMesh::Point> > badTriangleList;

    // extract all open triangles
    if (componentCount()>0)
    {        
        OpenMesh::VPropHandleT< bool > visited;
        add_property(visited);

        VertexIter v_it, v_end = vertices_end();

        //iterate over all vertices
        for (v_it = vertices_begin(); v_it != v_end; ++v_it)
            property(visited, v_it) = false;
    
        VertexIter current_pos = vertices_begin();
        while( true )
        {
            //find an unvisited vertex
            VertexHandle vh;
            for (v_it = current_pos ; v_it != v_end; ++v_it)
            {
                if ( !property(visited, v_it) )
                {
                    property(visited, v_it) = true;
                    current_pos = v_it;
                    vh = v_it.handle();                
                    break;
                }
            }

            //if none was found -> finished
            if (!vh.is_valid()) break;

            std::vector<VertexHandle> handles;
            handles.push_back( vh );

            std::vector<VertexHandle> handlesX;
            handlesX.push_back( vh );
            bool bIsOpen = false;

            //grow from found vertex
            while( handles.size() > 0 )
            {
                VertexHandle current = handles.back();
                handles.pop_back();

                for (VertexVertexIter vv_it = vv_iter( current ); vv_it; ++vv_it)
                {
                    if ( !property(visited, vv_it) )
                    {
                        property(visited, vv_it) = true;
                        handles.push_back( vv_it.handle() );
                        handlesX.push_back( vv_it.handle() );

                        if (is_boundary(vv_it.handle()))
                            bIsOpen = true;
                    }
                }
            }

            if (bIsOpen)
            {
                if (handlesX.size()<=3) // a lost triangle
                {
                    nDropped++;
                    std::vector<CBaseMesh::Point> ptslst;                    
                    // delete existing vertexes but store them in a list of lost triangles
                    for(std::vector<VertexHandle>::iterator it = handlesX.begin(); it != handlesX.end(); ++it)
                    {
                        ptslst.push_back(this->point(*it));
                        this->delete_vertex(*it);
                    }                    
                    if (3==ptslst.size())
                        badTriangleList.push_back(ptslst);
                }
            }            
        }  
        remove_property(visited);  
    }

    garbage_collection();    

    // for every lost triangle search for an open position in the model
    if (badTriangleList.size()>0)
    {
        omerr().disable();

        std::vector<VertexHandle> boundaryVertices;
        for (VertexIter v_it = vertices_begin(); v_it != vertices_end(); ++v_it)
        {
            if (status(v_it).deleted()) continue;
            if (is_boundary(v_it.handle()))
                boundaryVertices.push_back(v_it.handle());
        }

        int nBadTriangles;
        do // as long as we are successfully adding triangles to model
        {
            std::cout << ".";
            nBadTriangles = badTriangleList.size();
            for(int i=0;i<badTriangleList.size();i++)
            {
                std::vector<CBaseMesh::Point> &ptslst = badTriangleList[i];
                assert(ptslst.size()==3);
                std::vector<VertexHandle> ptshndls;
                ptshndls.push_back(VertexHandle());
                ptshndls.push_back(VertexHandle());
                ptshndls.push_back(VertexHandle());
                int nValid = 0;
                // for every triangle point find the nearest one
#pragma omp parallel for
                for(int j=0;j<3;j++)
                {
                    const Point pts = ptslst[j];
                    VertexHandle hBest;
                    double dist = DBL_MAX;                    
                    //for (VertexIter v_it = vertices_begin(); v_it != vertices_end(); ++v_it)
                    const int nCandidateVertices = boundaryVertices.size();
                    for(int vi = 0; vi < nCandidateVertices; vi++)
                    {
                        VertexHandle vih = boundaryVertices[vi];
                        if (status(vih).deleted()) continue;
                        if (is_boundary(vih))
                        {
                            double d = (point(vih)-pts).length();
                            if (d<dist)
                            {
                                dist = d;
                                hBest = vih;
                            }
                        }
                    }
                    if (hBest.is_valid() && dist<0.001)
                    {
                        ptshndls[j]=hBest;
#pragma omp critical
                        nValid++;
                    }
                }

                std::vector<VertexHandle> newhandles;
                if (2==nValid) // if only one vertex doesn't fit in, add it
                {
                    for(int m=0;m<3;m++)
                    {
                        if (!ptshndls[m].is_valid())
                        {
                            ptshndls[m]=add_vertex(ptslst[m]);
                            newhandles.push_back(ptshndls[m]);
                            nValid++;
                        }
                    }
                }
                bool bFaceAdded = false;
                if (3==nValid)
                {
                    // add face
                    if (ptshndls[0]==ptshndls[1] || ptshndls[1]==ptshndls[2] || ptshndls[0]==ptshndls[2])
                    {
                        //std::cout << "bad luck";
                    }
                    else
                    {
                        FaceHandle fh = this->add_face(ptshndls[2],ptshndls[1],ptshndls[0]);
                        if (!fh.is_valid())
                        {
                            fh = this->add_face(ptshndls[0],ptshndls[1],ptshndls[2]);
                        }
                        bFaceAdded = fh.is_valid();
                    }
                }
                if (bFaceAdded)
                {
                    badTriangleList.erase(badTriangleList.begin() + i);
                    i--;

                    for(int k = 0; k < newhandles.size(); k++)
                        if (is_boundary(newhandles[k]))
                            boundaryVertices.push_back(newhandles[k]);
                }
                else
                {
                    // we failed - remove newly created vertices
                    for(int k = 0; k < newhandles.size(); k++)
                        delete_vertex(newhandles[k]);
                    // call garbage_collection(); or check status(v_it).deleted() on every vertex access
                }
            }    
        } while (nBadTriangles != badTriangleList.size());

        // perform cleanup
        garbage_collection();

        // reinsert all bad triangles that we were not able to connect with other parts
        nBadTriangles = badTriangleList.size();
        for(int i = 0 ; i < nBadTriangles; i++)
        {
            std::vector<CBaseMesh::Point> &ptslst = badTriangleList[i];
            if (3==ptslst.size())
            {
#if(0)
                VertexHandle vh0 = add_vertex(ptslst[0]);
                VertexHandle vh1 = add_vertex(ptslst[1]);
                VertexHandle vh2 = add_vertex(ptslst[2]);
                FaceHandle fh = this->add_face(vh0,vh1,vh2);
                assert(fh.is_valid());
#else
                std::vector<VertexHandle> ptshndls;
                ptshndls.push_back(VertexHandle());
                ptshndls.push_back(VertexHandle());
                ptshndls.push_back(VertexHandle());
                int nValid = 0;
                // for every triangle point find the nearest one
#pragma omp parallel for
                for(int j=0;j<3;j++)
                {
                    const Point pts = ptslst[j];
                    VertexHandle hBest;
                    double dist = DBL_MAX;         

                    const int nCandidateVertices = boundaryVertices.size();
                    for(int vi = 0; vi < nCandidateVertices; vi++)
                    {
                        VertexHandle vih = boundaryVertices[vi];
                        if (vih.idx()>=n_vertices())
                            continue;
                        if (status(vih).deleted()) continue;
                        if (is_boundary(vih))
                        {
                            double d = (point(vih)-pts).length();
                            if (d<dist)
                            {
                                dist = d;
                                hBest = vih;
                            }
                        }
                    }
                    if (hBest.is_valid() && dist<0.001)
                    {
                        ptshndls[j]=hBest;
#pragma omp critical
                        nValid++;
                    }
                }

                if (ptshndls[0]==ptshndls[1] || ptshndls[1]==ptshndls[2] || ptshndls[0]==ptshndls[2])
                {
                    ptshndls.clear();
                    ptshndls.push_back(VertexHandle());
                    ptshndls.push_back(VertexHandle());
                    ptshndls.push_back(VertexHandle());
                }

                std::vector<VertexHandle> newhandles;
                // replace missing vertices with new ones
                for(int m=0;m<3;m++)
                {
                    if (!ptshndls[m].is_valid())
                    {
                        ptshndls[m]=add_vertex(ptslst[m]);
                        newhandles.push_back(ptshndls[m]);
                        nValid++;
                    }
                }
                bool bFaceAdded = false;
                if (3==nValid)
                {
                    FaceHandle fh = this->add_face(ptshndls[0],ptshndls[1],ptshndls[2]);
                    if (!fh.is_valid())
                    {
                        fh = this->add_face(ptshndls[2],ptshndls[1],ptshndls[0]);
                    }
                    bFaceAdded = fh.is_valid();
                }                
                if (!bFaceAdded)
                {
                    // we failed - remove newly created vertices
                    for(int k = 0; k < newhandles.size(); k++)
                        delete_vertex(newhandles[k]);
                    VertexHandle vh0 = add_vertex(ptslst[0]);
                    VertexHandle vh1 = add_vertex(ptslst[1]);
                    VertexHandle vh2 = add_vertex(ptslst[2]);
                    FaceHandle fh = this->add_face(vh0,vh1,vh2);
                    assert(fh.is_valid());
                    
                    boundaryVertices.push_back(vh0);
                    boundaryVertices.push_back(vh1);
                    boundaryVertices.push_back(vh2);
                }
                else
                {
                    for(int k = 0; k < newhandles.size(); k++)
                        if (is_boundary(newhandles[k]))
                            boundaryVertices.push_back(newhandles[k]);
                }
#endif
            }
        }

        // perform cleanup
        garbage_collection();

        omerr().enable();
    }

// add faces to boundaries that consist of three vertices    
    /* 
    int cm=0;
    bool bAdded;
    do
    {
        bAdded = false;
        for (HalfedgeIter he_it = halfedges_begin(); he_it != halfedges_end() ; ++he_it ) 
        {
            if (is_boundary(he_it.handle()))
            {                   
                HalfedgeHandle hnext = next_halfedge_handle(next_halfedge_handle(next_halfedge_handle(he_it)));
                assert(hnext.is_valid());
                if (he_it.handle()==hnext)
                {
                    HalfedgeHandle hnext = next_halfedge_handle(he_it);
                    VertexHandle v0=from_vertex_handle(he_it);
                    VertexHandle v1=to_vertex_handle(he_it);
                    VertexHandle v2=to_vertex_handle(hnext);
                    assert(v0!=v1 && v1!=v2 && v2!=v0);
                    assert(is_boundary(v0) && is_boundary(v1) && is_boundary(v2));
                    FaceHandle fh = add_face(v0,v1,v2);
                    if (!fh.is_valid())
                    {
                        fh = add_face(v1,v0,v2);
                        if (fh.is_valid())
                        {
                            bAdded = true;
                            cm++;
                        }
                    }               
                }
            }
        }
    } while (bAdded);
    std::cout << cm; */

 /*   m_nComponents = -1;
    m_nBoundaries = -1;
    boundaryCount();
    componentCount();*/

    return nDropped;
}

} // namespace data

