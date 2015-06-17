///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2015 3Dim Laboratory s.r.o.
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

#ifndef CVolumeTransformation_H
#define CVolumeTransformation_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/SharedPtr.h>
#include <osg/Matrix>

#include <data/CObjectHolder.h>

// Serialization
#include <VPL/Module/Serializable.h>
#include <data/CSerializableData.h>
#include <data/CStorageInterface.h>
#include <sstream>
#include <data/storage_ids_core.h>

///////////////////////////////////////////////////////////////////////////////
// type definitions

namespace data
{

///////////////////////////////////////////////////////////////////////////////
// Storage entries

///////////////////////////////////////////////////////////////////////////////
//! Volume transformation.

class CVolumeTransformation
    : public vpl::base::CObject
    , public vpl::mod::CSerializable
{
public:
    //! Smart pointer.
    VPL_SHAREDPTR(CVolumeTransformation);


    //! Default class name.
    VPL_ENTITY_NAME("VolumeTransformation");

    //! Default compression method.
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

public:
    //! Default constructor.
    CVolumeTransformation();

    //! Copy constructor.
    CVolumeTransformation(const CVolumeTransformation &volumeTransformation);

    //! Destructor
    ~CVolumeTransformation();

    //! Sets transformation
    void setTransformation(osg::Matrix transformation);

    //! Gets transformation
    osg::Matrix getTransformation() const;

    //! Does object contain relevant data?
    virtual bool hasData()
    { 
        return true;
    }

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries &Changes);

    //! Initializes the object to its default state.
    void init();

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry *pParent) 
    {
        return true;
    }

    //! Copy implant
    CVolumeTransformation &operator=(const CVolumeTransformation &volumeTransformation);

    //! Serialize
    virtual void serialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Writer);

    //! Deserialize
    virtual void deserialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Reader);

protected:
    osg::Matrix m_transformation;
};

typedef CVolumeTransformation::tSmartPtr CVolumeTransformationPtr;

//! Serialization class for the volume transformation
DECLARE_SERIALIZATION_WRAPPER(CVolumeTransformation)

namespace Storage
{
	DECLARE_OBJECT(VolumeTransformation, data::CVolumeTransformation, CORE_STORAGE_VOLUME_TRANSFORMATION_ID);
}

} // namespace data

#endif // CVolumeTransformation_H
