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

#include <data/CDicom.h>

// STL
#include <fstream>

//=================================================================================================
data::CDicom::CDicom( const std::string & file  )
    : m_bOk( false )
    , m_lSize( -1 )
    , m_sFileName(file)
    , m_Compression( RAW )
{   
}

//=================================================================================================
data::CDicom::CDicom()
    : m_bOk( false )
    , m_lSize( -1 )
    , m_sFileName( "" )
    , m_Compression( RAW )
{
}

//=================================================================================================
data::CDicom::~CDicom()
{    
}

//=================================================================================================
long data::CDicom::getFileSize( const std::string & file )
{
    std::ifstream inFile( file.c_str(), std::ios::in|std::ios::binary );

    if ( inFile.bad() )
    {
        return -1;
    }
    else
    {
        std::ifstream::pos_type beg, end;
        inFile.seekg(0, std::ios::beg);
        beg = inFile.tellg();
        inFile.seekg(0, std::ios::end);
        end = inFile.tellg();

        inFile.close();

        return long(end - beg);
    }
}

//==============================================================================================
int data::CDicom::convPos2Id(double a, double b, double c)
{
    static int MaxId = 0x3FFFFFFF;

    double dValue = a * a + b * b + c * c;
    assert(dValue >= 0);

    int sign = 1, res = 0;
    if (a<0) sign = -sign;
    if (b<0) sign = -sign;
    if (c<0) sign = -sign;
    if (int(dValue * 1000.0)<MaxId)
        res = int(dValue * 1000.0);
    else
        res = int(dValue) | 0x40000000;
    res *= sign;

    //char ss[256]={};
    //sprintf(ss,"%.3f %.3f %.3f   %.3f %d\n",a,b,c,dValue,res);
    //OutputDebugStringA(ss);
    return res;
}


