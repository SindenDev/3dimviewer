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

////////////////////////////////////////////////////////////
// Includes

#include <alg/CSmoothing.h>

#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>

////////////////////////////////////////////////////////////
//

bool CSmoothing::Smooth(geometry::CMesh &mesh, int loops)
{
/*
    setProgressMax(loops + 1);
    beginProgress();
    progress();

    OpenMesh::Smoother::JacobiLaplaceSmootherT<geometry::CMesh> smoother(mesh);
    OpenMesh::Smoother::SmootherT<geometry::CMesh>::Component component = OpenMesh::Smoother::SmootherT<geometry::CMesh>::Tangential_and_Normal;
    OpenMesh::Smoother::SmootherT<geometry::CMesh>::Continuity continuity = OpenMesh::Smoother::SmootherT<geometry::CMesh>::C1;
    smoother.initialize(component, continuity);

    // smoothing cycle
    for (int smooth_iter = 0; smooth_iter < loops; ++smooth_iter)
    {
        smoother.smooth(1);

        // set progress value and test function termination
        if (!progress())
        {
            return false;
        }
    }

    // finish function progress
    endProgress();

    // OK
    return true;
*/

    // smoothing progress setting
    setProgressMax(loops + 1);
    beginProgress();
    progress();

    // add mesh custom property - new vertex position
    OpenMesh::VPropHandleT<geometry::CMesh::Point>      new_positions_prop;
    mesh.add_property(new_positions_prop);

    geometry::CMesh::VertexIter                         v_it, v_end(mesh.vertices_end());               // mesh vertex iterator and end iterator
    geometry::CMesh::VertexVertexIter                   vv_it;                                          // vertex circulator                       
    geometry::CMesh::Point                              vertex_vector;                                  // vector from actual (center) point to actual adjacent vertex    
    geometry::CMesh::Point                              centr_point;                                    // position of actual (center) point

    double                                              smooth_factor;                                  // used smoothing factor
    bool                                                result = true;

    // smoothing iterations cycle
    for (int i = 0; i < loops; ++i)
    {
        // set first Taubin smoothing parameter
        smooth_factor = m_taubins_lambda;

        // Taubin smoothing cycle
        for (int ts = 0; ts < 2; ++ts)
        {
            // mesh vertices cycle
            for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it)
            {      
                mesh.property(new_positions_prop, v_it).vectorize(0.0f);
                // vertex normalisation weight, sum of adjacent vertices weight
                double normalisation_weight = 0.0;
                // position of actual (center) point
                centr_point = mesh.point(v_it);
      
                // ring of vertices around actual vertex cycle
                for (vv_it = mesh.vv_iter(v_it); vv_it; ++vv_it)
                {
                    // adjacent vertex vector calculation
                    vertex_vector = mesh.point(vv_it) - centr_point;
                    // squared distance of actual vertex and adjacent actual vertex
                    double vertex_vector_length_2 = vertex_vector.sqrnorm(); 
                    // distance weight calculation
                    double distance_weight = exp((vertex_vector_length_2) / (-2.0 * m_squared_smooth_factor));
                    if (distance_weight != distance_weight)
                    {
                        // nan
                    }
                    else
                    {
                        normalisation_weight += distance_weight;
                        // adjacent vertex position cumulation
                        mesh.property(new_positions_prop, v_it) += (vertex_vector * distance_weight);
                    }
                }

                // actual vertex new position weighting
                if (fabs(normalisation_weight)>0.000001) // very low numbers produce nan
                {
                    mesh.property(new_positions_prop, v_it) *= (smooth_factor / normalisation_weight);
                    mesh.property(new_positions_prop, v_it) += centr_point;
                }
                else
                    mesh.property(new_positions_prop, v_it) = centr_point;
            }
    
            // mesh vertices cycle to realise smoothing step
            for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it)
            {
                // boundary vertex test
                if ( !mesh.is_boundary(v_it) )
                    // set new position for actual vertex
                    mesh.set_point( v_it, mesh.property(new_positions_prop,v_it) );
            }

            // set second Taubin smoothing parameter
            smooth_factor = m_taubins_mu;
        }

        // set progress value and test function termination
        if (!progress())
        {
            result = false;
            break;
        }
    }

    // free mesh custom property - new vertex position
    mesh.remove_property(new_positions_prop);

    // finish function progress
    endProgress();

    // OK
    return result;
}
