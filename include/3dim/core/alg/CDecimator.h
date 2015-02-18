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

#ifndef CDecimator_H
#define CDecimator_H

////////////////////////////////////////////////////////////
// Includes

#include <data/CMesh.h>
#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>

// VPL
#include <VPL/Module/Serializable.h>
#include <VPL/Module/Progress.h>

// STL
#include <vector>
#include <map>

// forward declaration
class CDecimator;

////////////////////////////////////////////////////////////
//! Tri mesh decimating algorithm implementation class.
//! Implements Volume error metrics polygonal surface simplification.
class CDecimator : public vpl::mod::CProgress
{
private:
    int m_step;
    int m_progress;
    bool m_success;

public:
    //! Decimating class constructor.
    CDecimator() {}

    //! Decimating class destructor.
    ~CDecimator() {}

    //! Reduce input tri mesh using Volume error metrics polygonal surface simplification.
    bool Reduce(data::CMesh &mesh, int final_vert_number, int final_tri_number);

    bool makeProgress(int step);
};

template <typename tDecimater>
class CDecimatorProgressModule : public OpenMesh::Decimater::ModBaseT<tDecimater>
{
private:
    typedef OpenMesh::Decimater::ModBaseT<tDecimater> tBase;
    CDecimator *m_decimator;
    int m_lastCount;
    bool m_cancel;

public:
    DECIMATING_MODULE(CDecimatorProgressModule, tDecimater, CDecimatorProgressModule);

    CDecimatorProgressModule(tDecimater &decimater)
        : tBase(decimater, true)
    {
        m_decimator = NULL;
		m_lastCount = 0;
		m_cancel = false;
    }
    
    ~CDecimatorProgressModule()
    {
        
    }

    virtual void initialize() override
    {
        m_decimator = NULL;
        m_cancel = false;
        m_lastCount = this->mesh().n_faces();
    }
	virtual void setDecimator(CDecimator *decimater)
	{
		m_decimator = decimater;
	}

    virtual float collapse_priority(const OpenMesh::Decimater::CollapseInfoT<data::CMesh>& collapseInfo) override
    {
        if (m_cancel)
        {
            return this->ILLEGAL_COLLAPSE;
        }
        else
        {
            return this->LEGAL_COLLAPSE;
        }
    }

    virtual void postprocess_collapse(const OpenMesh::Decimater::CollapseInfoT<data::CMesh> &collapseInfo) override
    {
        if (!m_cancel && NULL!=m_decimator && !m_decimator->makeProgress(2))
        {
            m_cancel = true;
        }
    }
};

#endif // CDecimator_H

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
