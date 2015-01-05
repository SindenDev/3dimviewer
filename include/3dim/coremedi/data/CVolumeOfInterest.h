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

#ifndef CVolumeOfInterest_H
#define CVolumeOfInterest_H

#include <VPL/Math/Base.h>
#include <data/CSerializableData.h>

#include "data/CObjectHolder.h"

#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

////////////////////////////////////////////////////////////
//! a Class

struct SVolumeOfInterest
{
    //! Minimal and maximal volume coordinates.
    vpl::tSize m_MinX, m_MaxX, m_MinY, m_MaxY, m_MinZ, m_MaxZ;

    //! Default constructor.
    SVolumeOfInterest() : m_MinX(0) , m_MaxX(0) , m_MinY(0) , m_MaxY(0) , m_MinZ(0) , m_MaxZ(0) {}

    //! Normalization...
    void normalize()
    {
        if( m_MinX > m_MaxX )
        {
            vpl::math::swap2(m_MinX, m_MaxX);
        }
        if( m_MinY > m_MaxY )
        {
            vpl::math::swap2(m_MinY, m_MaxY);
        }
        if( m_MinZ > m_MaxZ )
        {
            vpl::math::swap2(m_MinZ, m_MaxZ);
        }
    }
};


////////////////////////////////////////////////////////////
//! a Class

class CVolumeOfInterest : public vpl::base::CObject, public vpl::mod::CSerializable
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CVolumeOfInterest);

	//! Default class name.
    VPL_ENTITY_NAME("CVolumeOfInterest");

    //! Default compression method.
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

public:
    //! Default constructor.
    CVolumeOfInterest() {}

    //! Destructor.
    ~CVolumeOfInterest() {}

    //! Returns limits.
    SVolumeOfInterest& get() { return m_Limits; }
    const SVolumeOfInterest& get() const { return m_Limits; }

    //! Sets all limits at once.
    CVolumeOfInterest& set(const SVolumeOfInterest& Limits)
    {
        m_Limits = Limits;
        return *this;
    }

    //! Returns one of the limits.
    vpl::tSize getMinX() const { return m_Limits.m_MinX; }
    vpl::tSize getMinY() const { return m_Limits.m_MinY; }
    vpl::tSize getMinZ() const { return m_Limits.m_MinZ; }
    vpl::tSize getMaxX() const { return m_Limits.m_MaxX; }
    vpl::tSize getMaxY() const { return m_Limits.m_MaxY; }
    vpl::tSize getMaxZ() const { return m_Limits.m_MaxZ; }

    //! Normalization...
    CVolumeOfInterest& normalize()
    {
        m_Limits.normalize();
        return *this;
    }


    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& Changes);

    //! Re-initializes the object.
    void init();

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    //! Does object contain relevant data?
    virtual bool hasData(){ return false; }

	//! Serialize 
	template< class tpSerializer >
	void serialize( vpl::mod::CChannelSerializer<tpSerializer> & Writer )
	{
		Writer.beginWrite( *this );

        WRITEINT32( 1 ); // version

		Writer.write( (vpl::sys::tInt32)m_Limits.m_MinX );
		Writer.write( (vpl::sys::tInt32)m_Limits.m_MaxX );
		Writer.write( (vpl::sys::tInt32)m_Limits.m_MinY );
		Writer.write( (vpl::sys::tInt32)m_Limits.m_MaxY );
		Writer.write( (vpl::sys::tInt32)m_Limits.m_MinZ );
		Writer.write( (vpl::sys::tInt32)m_Limits.m_MaxZ );

		Writer.endWrite( *this );
	}

	//! Deserialize 
	template< class tpSerializer >
	void deserialize( vpl::mod::CChannelSerializer<tpSerializer> & Reader )
	{
		Reader.beginRead( *this ); 

        int version = 0;
        READINT32( version );

        READINT32(m_Limits.m_MinX);
        READINT32(m_Limits.m_MaxX);
        READINT32(m_Limits.m_MinY);
        READINT32(m_Limits.m_MaxY);
        READINT32(m_Limits.m_MinZ);
        READINT32(m_Limits.m_MaxZ);

		Reader.endRead( *this ); 
	}

protected:
    //! Current limits.
    SVolumeOfInterest m_Limits;

};

//! CSerializableData serialization
template<>
class CSerializableData< CVolumeOfInterest >
{
public:
	//! Default class name.
    VPL_ENTITY_NAME("CVolumeOfInterest");

	//! Serialize 
	template< class tpSerializer >
	static void serialize( CVolumeOfInterest * v, vpl::mod::CChannelSerializer<tpSerializer> & Writer )
	{ v->serialize< tpSerializer >( Writer ); }

	//! Deserialize
	template< class tpSerializer >
	static void deserialize( CVolumeOfInterest * v, vpl::mod::CChannelSerializer<tpSerializer> & Reader )
	{ v->deserialize< tpSerializer >( Reader ); }
};

namespace Storage
{
	//! Volume of interest.
	DECLARE_OBJECT(PatientVOI, CVolumeOfInterest, PATIENT_DATA + 2);
	DECLARE_OBJECT(AuxVOI, CVolumeOfInterest, AUX_DATA + 2);
}

} // namespace data

#endif // CVolumeOfInterest_H
