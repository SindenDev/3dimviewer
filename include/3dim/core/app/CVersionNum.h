////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////

#ifndef CVersionNum_H
#define CVersionNum_H

#include <VPL/Base/Setup.h>


namespace app
{

///////////////////////////////////////////////////////////////////////////////
//! Class representing an application version number.
//! - Major and minor version number.
//! - Two products are compatible if their major number is equal.

class CVersionNum
{
public:
    //! Default constructor.
    CVersionNum(int MajorNum = 1, int MinorNum = 0, int BuildNum = 0)
        : m_MajorNum(MajorNum)
        , m_MinorNum(MinorNum)
        , m_BuildNum(BuildNum)
    {}

    //! Copy constructor.
    CVersionNum(const CVersionNum& v)
        : m_MajorNum(v.m_MajorNum)
        , m_MinorNum(v.m_MinorNum)
        , m_BuildNum(v.m_BuildNum)
    {}
    
    //! Returns the major version number.
    int getMajorNum() const { return m_MajorNum; }
    
    //! Returns the minor version number.
    int getMinorNum() const { return m_MinorNum; }

    //! Returns the minor version number.
    int getBuildNum() const { return m_BuildNum; }
    
    //! Returns true if two version numbers are identical.
    bool isIdentical(const CVersionNum& v) const
    {
        return (v.m_MajorNum == m_MajorNum && v.m_MinorNum == m_MinorNum && v.m_BuildNum == m_BuildNum);
    }

    //! Returns true if the major number is equal.
    bool isCompatibleWith(const CVersionNum& v) const
    {
        return (m_MajorNum == v.m_MajorNum);
    }

    //! Returns true if the major number is equal.
    bool isCompatibleWith(int MajorNum) const
    {
        return (m_MajorNum == MajorNum);
    }

    //! Sets the major number.
    CVersionNum& setMajorNum(int v)
    {
        m_MajorNum = v;
        return *this;
    }

    //! Sets the minor number.
    CVersionNum& setMinorNum(int v)
    {
        m_MinorNum = v;
        return *this;
    }

    //! Sets the build number.
    CVersionNum& setBuildNum(int v)
    {
        m_BuildNum = v;
        return *this;
    }

protected:
    //! Major and minor version number.
    int m_MajorNum, m_MinorNum, m_BuildNum;
};


} // namespace app

#endif // CVersionNum_H

