///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// BlueSkyPlan version 3.x
// Diagnostic and implant planning software for dentistry.
//
// The original DentalViewer legal notice can be found below.
//
// Copyright 2012 Blue Sky Bio, LLC
// All rights reserved 
//
// Changelog:
//    [2012/mm/dd] - ...
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// include files

#include <data/CPivot.h>

namespace data
{

//! Default constructor.
CPivot::CPivot()
    : CUndoProvider(data::Storage::ModelPivot::Id, true)
    , m_position(vpl::img::CVector3d(0.0, 0.0, 0.0))
{ }

//! Constructor.
CPivot::CPivot(vpl::img::CPoint3d &position)
    : CUndoProvider(data::Storage::ModelPivot::Id, true)
    , m_position(position)
{ }

//! Copy constructor.
CPivot::CPivot(const CPivot &pivot)
    : CUndoProvider(data::Storage::ModelPivot::Id, true)
    , m_position(pivot.m_position)
{ }

//! Destructor
CPivot::~CPivot()
{ }

//! Set position
void CPivot::setPosition(const vpl::img::CVector3d &position)
{
    m_position = position;
}

//! Get position
vpl::img::CVector3d CPivot::getPosition() const
{
    return m_position;
}

//! Regenerates the object state according to any changes in the data storage.
void CPivot::update(const CChangedEntries &Changes)
{ }

//! Initializes the object to its default state.
void CPivot::init()
{
    m_position = vpl::img::CVector3d(0.0, 0.0, 0.0);
}

//! Copy implant
CPivot &CPivot::operator=(const CPivot &pivot)
{
    if (this == &pivot)
    {
        return *this;
    }

    m_position = pivot.m_position;

    return *this;
}

//! Serialize
void CPivot::serialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Writer)
{
    Writer.beginWrite(*this);
    WRITEINT32(0); // version
    CSerializableData<vpl::img::CVector3d>::serialize(&m_position, Writer);
    Writer.endWrite(*this);
}

//! Deserialize
void CPivot::deserialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Reader)
{
    Reader.beginRead(*this);
    int version = 0;
    READINT32(version);
    CSerializableData<vpl::img::CVector3d>::deserialize(&m_position, Reader);
    Reader.endRead(*this);
}

} // namespace data
