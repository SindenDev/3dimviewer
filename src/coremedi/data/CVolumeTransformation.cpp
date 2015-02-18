///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2008-2015 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////


#include <data/CVolumeTransformation.h>
#include <data/CSerializableOSG.h>

using namespace data;

CVolumeTransformation::CVolumeTransformation()
    : vpl::base::CObject()
    , m_transformation(osg::Matrix::identity())
{ }

CVolumeTransformation::CVolumeTransformation(const CVolumeTransformation &volumeTransformation)
    : vpl::base::CObject()
    , m_transformation(volumeTransformation.m_transformation)
{ }

CVolumeTransformation::~CVolumeTransformation()
{ }

void CVolumeTransformation::setTransformation(osg::Matrix transformation)
{
    m_transformation = transformation;
}

osg::Matrix CVolumeTransformation::getTransformation() const
{
    return m_transformation;
}

void CVolumeTransformation::update(const CChangedEntries &Changes)
{
    if (Changes.checkFlagAll(data::Storage::STORAGE_RESET))
    {
        m_transformation = osg::Matrix::identity();
    }
}

void CVolumeTransformation::init()
{
    m_transformation = osg::Matrix::identity();
}

CVolumeTransformation &CVolumeTransformation::operator=(const CVolumeTransformation &volumeTransformation)
{
    if (this == &volumeTransformation)
    {
        return *this;
    }

    m_transformation = volumeTransformation.m_transformation;

    return *this;
}

//! Serialize
void CVolumeTransformation::serialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Writer)
{
    // Begin of data serialization block
    Writer.beginWrite(*this);

    int blockVersion = 0;
    WRITEINT32( blockVersion );

    //! Position serializer
    CSerializableData< osg::Matrix >::serialize(&m_transformation, Writer);

    // End of the block
    Writer.endWrite(*this);
}

//! Deserialize
void CVolumeTransformation::deserialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Reader)
{
    // Begin of data serialization block
    Reader.beginRead(*this);

    int blockVersion = 0;
    READINT32(blockVersion);

    //! Position serializer
    CSerializableData< osg::Matrix >::deserialize(&m_transformation, Reader);

    // End of the block
    Reader.endRead(*this);
}
