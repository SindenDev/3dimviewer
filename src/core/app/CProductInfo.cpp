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

#include <app/CProductInfo.h>

#include <sstream>


namespace app
{

///////////////////////////////////////////////////////////////////////////////
//

std::string CProductInfo::getProductId() const
{
    std::stringstream Stream;
    Stream << m_ssName << ' ' << m_Version.getMajorNum() << '.' << m_Version.getMinorNum();
    return Stream.rdbuf()->str();
}


///////////////////////////////////////////////////////////////////////////////
//

std::string CProductInfo::getProductIdWithNote() const
{
//    return getProductId() + m_ssNote;
    std::stringstream Stream;
    Stream << m_ssName << ' ' << m_Version.getMajorNum() << '.' << m_Version.getMinorNum() << '.' << m_Version.getBuildNum() << m_ssNote;
    return Stream.rdbuf()->str();
}


///////////////////////////////////////////////////////////////////////////////
//

bool CProductInfo::setProductId(const std::string& Id)
{
    std::string::size_type Dash = Id.rfind(' ');
    if( Dash == std::string::npos )
    {
        return false;
    }

    int Major, Minor = -1, Build = -1;

    std::stringstream Stream(Id.substr(Dash + 1));
    Stream >> Major;
    if( Stream.peek() != '.' )
    {
        return false;
    }
    Stream.ignore();
    Stream >> Minor;
    if( Stream.fail() || Major < 0 || Minor < 0 )
    {
        return false;
    }
    Stream.ignore();
    Stream >> Build;
    if (Build<0) 
        Build = 0;

    // Product info
    m_ssName = Id.substr(0, Dash);
    m_Version.setMajorNum(Major).setMinorNum(Minor).setBuildNum(Build);
    m_ssNote = std::string("");

    // O.K.
    return true;
}


///////////////////////////////////////////////////////////////////////////////
//

bool CProductInfo::normalizeProductId(std::string& Id)
{
    if( Id.empty() )
    {
        return false;
    }

    // Find the dot
    std::string::size_type pos = Id.find('.');
    if( pos == std::string::npos || (pos + 1) >= Id.size() )
    {
        return false;
    }

    // Clear the minor version number
    Id[pos + 1] = 'x';

    return true;
}


} // namespace app

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
