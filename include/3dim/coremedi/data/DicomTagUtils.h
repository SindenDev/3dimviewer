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

#ifndef DicomTagUtils_H
#define DicomTagUtils_H

// VPL
#include <VPL/Base/Singleton.h>

// DCMTk
#include <dcmtk/dcmdata/dctag.h>

// STL
#include <set>

#include "DicomTags.h"


namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Function object compares group and element ID of two dicom tags.

class CDicomTagCompare
{
public:
    bool operator()( const DcmTag & t1, const DcmTag & t2 ) const
    {
        if ( t1.getGroup() < t2.getGroup() )
        {
            return true;
        }
        else if ( t1.getGroup() == t2.getGroup() ) 
        {
            return t1.getElement() < t2.getElement();
        }
        else 
        {
            return false;
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
//! A set of dicom tags.

class CDicomTagSet
{
public:
    //! Internal representation of the set of dicom tags.
    typedef std::set<DcmTag, CDicomTagCompare> tTagSet;

public:
    //! Default constructor
    CDicomTagSet() {}

    //! Virtual destructor.
    virtual ~CDicomTagSet() {}

    //! Returns true of this set contains a specified tag.
    bool contains( const DcmTag & tag ) const
    {
        return (m_Set.find(tag) != m_Set.end() );
    }

    //! Adds a new tag to the set.
    CDicomTagSet& add( const DcmTag & tag )
    {
        m_Set.insert( tag );
        return *this;
    }

protected:
    //! Set of dicom tags.
    tTagSet	m_Set;
};


//////////////////////////////////////////////////////////////////////////////
//! A set of required tags.

class CCompulsoryTags : vpl::base::CSingleton<vpl::base::SL_LONG>, public CDicomTagSet
{
private:
    //! Private default constructor.
    CCompulsoryTags() : CDicomTagSet()
    {
        this->add( TAG_PATIENTS_NAME );
        //this->add( TAG_PATIENTS_ID );
        this->add( TAG_STUDY_UID );
        //this->add( tags::STUDY_ID );
        //this->add( tags::STUDY_DATE );
        this->add( TAG_SERIES_UID );
        //this->add( tags::SERIES_DATE );
        this->add( TAG_IMAGE_POSITION );
    }

    //! Allows instantiation via singleton holder only.
    VPL_PRIVATE_SINGLETON(CCompulsoryTags);
};


//! Macro helpful accessing the compulsory tag set singleton.
#define COMPULSORY_TAGS VPL_SINGLETON(CCompulsoryTags)


} // namespace data

#endif // DicomTagUtils_H
