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

#ifndef CPivot_H_INCLUDED
#define CPivot_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/SharedPtr.h>
#include <VPL/Image/Vector3.h>

#include <data/CObjectHolder.h>

#include "data/CSnapshot.h"

// Serialization
#include <VPL/Module/Serializable.h>
#include <data/CSerializableData.h>
#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

///////////////////////////////////////////////////////////////////////////////
// type definitions

namespace data
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//\class CPivotSnapshot
//
//\brief Pivot snapshot.
////////////////////////////////////////////////////////////////////////////////////////////////////

class CPivotSnapshot : public CSnapshot
{
public:
    //! Constructor
    CPivotSnapshot(int type, CUndoProvider *provider = NULL)
        : CSnapshot(type, provider)
        , m_position(0.0, 0.0, 0.0)
    { }

    //! Destructor
    ~CPivotSnapshot()
    { }

    //! Each snapshot object must return its data size in bytes
    virtual long getDataSize()
    {
        return sizeof(vpl::img::CVector3d);
    }

protected:
    //! Pivot position
    vpl::img::CVector3d m_position;

    // Friend class
    friend class CPivot;
};


///////////////////////////////////////////////////////////////////////////////
//! Pivot point for synchronising draggers.

class CPivot
    : public vpl::base::CObject
    , public vpl::mod::CSerializable
    , public CUndoProvider
{
public:
    //! Smart pointer.
    VPL_SHAREDPTR(CPivot);

    //! Default class name.
    VPL_ENTITY_NAME("Pivot");

    //! Default compression method.
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

    //! Flags to determine what has changed
    enum EPivotFlags
    {
        CHANGED_POSITION = 1 << 3,
    };

public:
    //! Default constructor.
    CPivot();

    //! Constructor.
    CPivot(vpl::img::CPoint3d &position);

    //! Copy constructor.
    CPivot(const CPivot &pivot);

    //! Destructor
    ~CPivot();

    //! Set position
    void setPosition(const vpl::img::CVector3d &position);

    //! Get position
    vpl::img::CVector3d getPosition() const;

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
    CPivot &operator=(const CPivot &pivot);

    // Undo providing
    //! Create snapshot of the current state. 
    virtual CSnapshot *getSnapshot(CSnapshot * VPL_UNUSED(snapshot))
    {
        CPivotSnapshot *s = new CPivotSnapshot(data::UNDO_ALL, this);
        s->m_position = m_position;
        return s;
    }

    //! Restore state from the snapshot
    virtual void restore(CSnapshot *snapshot)
    {
        if (snapshot == NULL)
        {
            return;
        }

        CPivotSnapshot *s = dynamic_cast<CPivotSnapshot *>(snapshot);
        if (s == NULL)
        {
            return;
        }

        m_position = s->m_position;
    }

    //! Serialize
    virtual void serialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Writer);

    //! Deserialize
    virtual void deserialize(vpl::mod::CChannelSerializer<vpl::mod::CBinarySerializer> &Reader);

protected:
    //! Position
    vpl::img::CVector3d m_position;
};

typedef CPivot::tSmartPtr CPivotPtr;

//! Serialization class for the implant
DECLARE_SERIALIZATION_WRAPPER(CPivot)

namespace Storage
{
    DECLARE_OBJECT(ModelPivot, data::CPivot, CORE_STORAGE_MODEL_PIVOT_ID);
}

} // namespace data

#endif // CPivot_H_INCLUDED
