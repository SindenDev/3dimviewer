////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////

#ifndef COrthoSlice_H
#define COrthoSlice_H

////////////////////////////////////////////////////////////
// include files

#include "CSlice.h"
#include "CDensityData.h"
#include <data/storage_ids_core.h>

namespace data
{

////////////////////////////////////////////////////////////
//! Orthogonal slice through density volume data.
//! - Still abstract class!

class COrthoSlice : public CSlice
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(COrthoSlice);

    //! Concrete plane.
    enum EPlane
    {
        PLANE_XY = 0,
        PLANE_XZ = 1,
        PLANE_YZ = 2
    };

    //! Imaging mode.
    enum EMode
    {
        MODE_SLICE   = 0,
        MODE_MIP     = 1,
        MODE_RTG     = 2,
        DEFAULT_MODE = MODE_SLICE
    };

    //! Helper flag passed to the invalidate() method.
    enum { MODE_CHANGED = 1 << 3 };

public:
    //! Constructor.
    COrthoSlice(EPlane Plane) : m_Plane(Plane), m_Position(0), m_Mode(DEFAULT_MODE) {}

    //! Destructor.
    virtual ~COrthoSlice() {}

    //! Returns the parallel plane.
    int getPlane() const { return m_Plane; }

    //! Changes the slice position.
    COrthoSlice& setPosition(int Position)
    {
        m_Position = Position;
        return *this;
    }

    //! Try to increase position
    COrthoSlice & increasePosition( )
    {
       ++m_Position;
       return *this;
    }

    //! Decrease position
    COrthoSlice & decreasePosition(  )
    {
       --m_Position;
       return *this;
    }

    //! Does object contain relevant data?
    virtual bool hasData(){ return false; }

    //! Returns current position.
    int getPosition() const { return m_Position; }

    //! Changes the rendering mode.
    COrthoSlice& setMode(EMode Mode)
    {
        m_Mode = Mode;
        return *this;
    }

    //! Returns current rendering mode.
    int getMode() const { return m_Mode; }


    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries& Changes) = 0;

    //! Re-initializes the slice.
    virtual void init();

    //! Returns true if changes of a given parent entry may affect this object.
    virtual bool checkDependency(CStorageEntry *pParent);


    //! Regenerates image data using internal parameters.
    //! - MIP (Maxima Intensity Projection) rendering mode.
    virtual void updateMIP(const CDensityData& Volume) = 0;

    //! Regenerates image data using internal parameters.
    //! - RTG (X-Ray mode) mode.
    virtual void updateRTG(const CDensityData& Volume) = 0;

protected:
    //! Current plane.
    EPlane m_Plane;

    //! Slice position.
    int m_Position;

    //! Current rendering mode.
    EMode m_Mode;
};


////////////////////////////////////////////////////////////
//! Orthogonal slice through density volume data.

class COrthoSliceXY : public COrthoSlice
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(COrthoSliceXY);

public:
    //! Constructor.
    COrthoSliceXY() : COrthoSlice(PLANE_XY) {}

    //! Destructor.
    virtual ~COrthoSliceXY() { }

    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries& Changes);

    //! Regenerates image data using internal parameters.
    //! - MIP (Maxima Intensity Projection) rendering mode.
    virtual void updateMIP(const CDensityData& Volume);

    //! Regenerates image data using internal parameters.
    //! - RTG (X-Ray mode) mode.
    virtual void updateRTG(const CDensityData& Volume);

    template<class VolumeType, class SliceType>
    bool updateProperty(VolumeType *volume, SliceType *slice)
    {
        slice->resize(volume->getXSize(), volume->getYSize());
        return volume->getPlaneXY(m_Position, *slice);
    }
};


////////////////////////////////////////////////////////////
//! Orthogonal slice through density volume data.

class COrthoSliceXZ : public COrthoSlice
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(COrthoSliceXZ);

public:
    //! Constructor.
    COrthoSliceXZ() : COrthoSlice(PLANE_XZ) {}

    //! Destructor.
    virtual ~COrthoSliceXZ() { }

    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries& Changes);


    //! Regenerates image data using internal parameters.
    //! - MIP (Maxima Intensity Projection) rendering mode.
    virtual void updateMIP(const CDensityData& Volume);

    //! Regenerates image data using internal parameters.
    //! - RTG (X-Ray mode) mode.
    virtual void updateRTG(const CDensityData& Volume);

    template<class VolumeType, class SliceType>
    bool updateProperty(VolumeType *volume, SliceType *slice)
    {
        slice->resize(volume->getXSize(), volume->getZSize());
        return volume->getPlaneXZ(m_Position, *slice);
    }
};


////////////////////////////////////////////////////////////
//! Orthogonal slice through density volume data.

class COrthoSliceYZ : public COrthoSlice
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(COrthoSliceYZ);

public:
    //! Constructor.
    COrthoSliceYZ() : COrthoSlice(PLANE_YZ) {}

    //! Destructor.
    virtual ~COrthoSliceYZ() { }

    //! Regenerates the object state according to any changes in the data storage.
    virtual void update(const CChangedEntries& Changes);


    //! Regenerates image data using internal parameters.
    //! - MIP (Maxima Intensity Projection) rendering mode.
    virtual void updateMIP(const CDensityData& Volume);

    //! Regenerates image data using internal parameters.
    //! - RTG (X-Ray mode) mode.
    virtual void updateRTG(const CDensityData& Volume);

    template<class VolumeType, class SliceType>
    bool updateProperty(VolumeType *volume, SliceType *slice)
    {
        slice->resize(volume->getYSize(), volume->getZSize());
        return volume->getPlaneYZ(m_Position, *slice);
    }
};

namespace Storage
{
	//! Identifier of a XY slice.
	DECLARE_OBJECT(SliceXY, COrthoSliceXY, CORE_STORAGE_SLICE_XY_ID);
	DECLARE_OBJECT(SliceXZ, COrthoSliceXZ, CORE_STORAGE_SLICE_XZ_ID);
	DECLARE_OBJECT(SliceYZ, COrthoSliceYZ, CORE_STORAGE_SLICE_YZ_ID);
}

} // namespace data

#endif // COrthoSlice_H

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
