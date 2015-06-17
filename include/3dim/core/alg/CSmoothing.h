///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
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

#ifndef CSmoothing_H
#define CSmoothing_H

////////////////////////////////////////////////////////////
// Includes

#include <geometry/base/CMesh.h>

// VPL
#include <VPL/Image/DensityVolume.h>
#include <VPL/Module/Serializable.h>
#include <VPL/Module/Progress.h>

// STL
#include <vector>

////////////////////////////////////////////////////////////
//! CSmoothing algorithm implementation class.
//! Implements G. Taubin - Signal processing polygonal surface fairing.

class CSmoothing : public vpl::mod::CProgress
{
public:
    //! Smoothing class constructor.
    CSmoothing()                                                     { m_squared_smooth_factor = 4.0; m_taubins_lambda = 0.5; m_taubins_mu = -0.51; }

    //! Smoothing class destructor.
    ~CSmoothing() {}

    //! Smooth input mesh using G. Taubin - Signal processing polygonal surface fairing
    bool Smooth(geometry::CMesh &mesh, int loops);

    //! Set parameter of smoothing distance weighting, for Gauss weighting, squared variance.
    void SetWeightingSquaredVarance(double _set_value)               { m_squared_smooth_factor = _set_value; };
    //! Get parameter of smoothing distance weighting, for Gauss weighting, squared variance.
    double GetWeightingSquaredVarance()                              { return m_squared_smooth_factor; };

    //! Set Taubin smoothing parameter, positive factor, front move.
    void SetTaubinLambda(double _set_value)                          { m_taubins_lambda = _set_value; };
    //! Get Taubin smoothing parameter, positive factor, front move.
    double GetTaubinLambda()                                         { return m_taubins_lambda; };

    //! Set Taubin smoothing parameter, negative factor, back move.
    void SetTaubinMu(double _set_value)                              { m_taubins_mu = _set_value; };
    //! Get Taubin smoothing parameter, negative factor, back move.
    double GetTaubinMu()                                             { return m_taubins_mu; };

private:

    //! Parameter of smoothing distance weighting, for Gauss weighting, squared variance.
    double          m_squared_smooth_factor;

    //! Taubin smoothing parameter, positive factor, front move.
    double          m_taubins_lambda;
    //! Taubin smoothing parameter, negative factor, back move.
    double          m_taubins_mu;


};


#endif // MC_CSmoothing_H

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
