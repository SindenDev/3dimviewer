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

#ifndef CCoordinatesConv_H
#define CCoordinatesConv_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Math/Base.h>

#include "data/CObjectHolder.h"

#include <cmath>

// Storage
#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Projection from volume space (integer coordinates) to the real/scene
//! coordinates.

class CCoordinatesConv : public vpl::base::CObject
{
public:
    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CCoordinatesConv);

public:
    //! Default constructor.
    CCoordinatesConv();

    //! Just another constructor...
    CCoordinatesConv(double dDX, double dDY, double dDZ, int SizeX, int SizeY, int SizeZ);

    //! Copy constructor.
    CCoordinatesConv(const CCoordinatesConv& Conv);

    //! Assignment operator.
    CCoordinatesConv& operator =(const CCoordinatesConv& Conv);

    //! Destructor.
    ~CCoordinatesConv() {}

    //! Sets the real voxel size.
    CCoordinatesConv& setVoxel(double dDX, double dDY, double dDZ)
    {
        VPL_CHECK(dDX > 0.0 && dDY > 0.0 && dDZ > 0.0, return *this);

        m_dDX = dDX;
        m_dDY = dDY;
        m_dDZ = dDZ;

        m_dInvDX = 1.0 / dDX;
        m_dInvDY = 1.0 / dDY;
        m_dInvDZ = 1.0 / dDZ;

        return *this;
    }

    //! Returns current voxel size.
    double getDX() const { return m_dDX; }
    double getDY() const { return m_dDY; }
    double getDZ() const { return m_dDZ; }

    //! Initializes scaling and translation of the scene,
    //! so that center of the scene corresponds to the origin.
    //! - Please, remeber to call the setVoxel() method first!
    CCoordinatesConv& setupScene(int SizeX, int SizeY, int SizeZ)
    {
        VPL_CHECK(SizeX > 0 && SizeY > 0 && SizeZ > 0, return *this);

        m_dVX = SizeX;
        m_dVY = SizeY;
        m_dVZ = SizeZ;

        m_dMaxX = SizeX * m_dDX;
        m_dMaxY = SizeY * m_dDY;
        m_dMaxZ = SizeZ * m_dDZ;

        return *this;
    }

    //! Returns maximum scene coordinates (without shifting).
    double getSceneMaxX() const { return m_dMaxX; }
    double getSceneMaxY() const { return m_dMaxY; }
    double getSceneMaxZ() const { return m_dMaxZ; }

    //! Returns minimum volume coordinates (without shifting).
    double getSceneMinX() const { return 0.0; }
    double getSceneMinY() const { return 0.0; }
    double getSceneMinZ() const { return 0.0; }

    //! Returns translation used to center the scene.
    double getSceneShiftX() const { return -0.5 * m_dMaxX; }
    double getSceneShiftY() const { return -0.5 * m_dMaxY; }
    double getSceneShiftZ() const { return -0.5 * m_dMaxZ; }

    //! Returns scene voxel size (voxel count in direction).
    double getVoxelSizeX() { return m_dVX; }
    double getVoxelSizeY() { return m_dVY; }
    double getVoxelSizeZ() { return m_dVZ; }

    //! Forward conversion of coordinates (without scene shifting) - from voxel coordinates to the not-shifted scene.
    double toRealX(int x) const { return x * m_dDX; }
    double toRealY(int y) const { return y * m_dDY; }
    double toRealZ(int z) const { return z * m_dDZ; }

    //! Forward conversion of coordinates (without scene shifting) - from voxel coordinates to the not-shifted scene.
    void toReal(int x, int y, int z,
                  double& dX,
                  double& dY,
                  double& dZ
                  ) const
    {
        dX = x * m_dDX;
        dY = y * m_dDY;
        dZ = z * m_dDZ;
    }


    //! Backward conversion of coordinates (without shifting) - from the real (mm) coordinates to the voxel space.
    int fromRealX(double x) const { return int(x * m_dInvDX); }
    int fromRealY(double y) const { return int(y * m_dInvDY); }
    int fromRealZ(double z) const { return int(z * m_dInvDZ); }

    //! Backward conversion of coordinates (without shifting) - from the real (mm) coordinates to the voxel space in doubles.
    double fromRealXd(double x) const { return x * m_dInvDX; }
    double fromRealYd(double y) const { return y * m_dInvDY; }
    double fromRealZd(double z) const { return z * m_dInvDZ; }

    //! Backward conversion of coordinates (without shifting) - from the real (mm) coordinates to the voxel space.
    void fromReal(double dX, double dY, double dZ,
                   int& x,
                   int& y,
                   int& z
                   ) const
    {
        x = int(dX * m_dInvDX);
        y = int(dY * m_dInvDY);
        z = int(dZ * m_dInvDZ);
    }


    //! 
    double sceneToUnitCubeX(double x) const { return (x - getSceneShiftX()) / getSceneMaxX(); }
    double sceneToUnitCubeY(double y) const { return (y - getSceneShiftY()) / getSceneMaxY(); }
    double sceneToUnitCubeZ(double z) const { return (z - getSceneShiftZ()) / getSceneMaxZ(); }

    //!
    void sceneToUnitCube(double dX, double dY, double dZ, double & x, double & y, double & z)
    {
        x = ( dX - getSceneShiftX() ) / getSceneMaxX();
        y = ( dY - getSceneShiftY() ) / getSceneMaxY();
        z = ( dZ - getSceneShiftZ() ) / getSceneMaxZ();
    }

    //!
    double unitCubeToSceneX(double x) const { return ( x * getSceneMaxX() + getSceneShiftX() ); }
    double unitCubeToSceneY(double y) const { return ( y * getSceneMaxY() + getSceneShiftY() ); }
    double unitCubeToSceneZ(double z) const { return ( z * getSceneMaxZ() + getSceneShiftZ() ); }

    //!
    void unitCubeToScene( double dX, double dY, double dZ, double & x, double & y, double & z )
    {
        x = ( dX * getSceneMaxX() + getSceneShiftX() );
        y = ( dY * getSceneMaxY() + getSceneShiftY() );
        z = ( dZ * getSceneMaxZ() + getSceneShiftZ() );
    }

    //! Forward conversion of coordinates - from the voxel space to the shifted scene space.
    double toSceneX(int x) const { return x * m_dDX + getSceneShiftX(); }
    double toSceneY(int y) const { return y * m_dDY + getSceneShiftY(); }
    double toSceneZ(int z) const { return z * m_dDZ + getSceneShiftZ(); }

    //! Forward conversion of coordinates - from the voxel space to the shifted scene space.
    double toSceneXd(double x) const { return x * m_dDX + getSceneShiftX(); }
    double toSceneYd(double y) const { return y * m_dDY + getSceneShiftY(); }
    double toSceneZd(double z) const { return z * m_dDZ + getSceneShiftZ(); }

    //! Forward conversion of coordinates - from the voxel space to the shifted scene space.
    void toScene(int x, int y, int z,
                 double& dX,
                 double& dY,
                 double& dZ
                 ) const
    {
        dX = x * m_dDX + getSceneShiftX();
        dY = y * m_dDY + getSceneShiftY();
        dZ = z * m_dDZ + getSceneShiftZ();
    }


    //! To scene with double coordinates - from the voxel space to the shifted scene space
    void toScened(double x, double y, double z,
                 double& dX,
                 double& dY,
                 double& dZ
                 ) const
    {
        dX = x * m_dDX + getSceneShiftX();
        dY = y * m_dDY + getSceneShiftY();
        dZ = z * m_dDZ + getSceneShiftZ();
    }

    //! Backward conversion of coordinates - from the shifted scene to the voxel space.
    int fromSceneX(double x) const { return int( ( x - getSceneShiftX() ) * m_dInvDX); }
        int fromSceneY(double y) const { return int( ( y - getSceneShiftY() ) * m_dInvDY); }
    int fromSceneZ(double z) const { return int( ( z - getSceneShiftZ() ) * m_dInvDZ); }

    //! Backward conversion of coordinates in double - from the shifted scene to the voxel space.
    double fromSceneXd( double x ) const { return ( x - getSceneShiftX() ) * m_dInvDX; }
    double fromSceneYd( double y ) const { return ( y - getSceneShiftY() ) * m_dInvDY; }
    double fromSceneZd( double z ) const { return ( z - getSceneShiftZ() ) * m_dInvDZ; }

    //! Backward conversion of coordinates - from the shifted scene to the voxel space.
    void fromScene(double dX, double dY, double dZ,
                   int& x,
                   int& y,
                   int& z
                   ) const
    {
        x = int(dX * m_dInvDX);
        y = int(dY * m_dInvDY);
        z = int(dZ * m_dInvDZ);
    }

    //! From real coordinates to the scene (shifted) coordinates.
    void shift( double x, double y, double z, double &sx, double &sy, double &sz )
    {
        sx = x + getSceneShiftX();
        sy = y + getSceneShiftY();
        sz = z + getSceneShiftZ();
    }

    //! From the scene (shifted) to the real (not shifted) coordinates.
    void unshift( double sx, double sy, double sz, double &x, double &y, double &z )
    {
        x = sx - getSceneShiftX();
        y = sy - getSceneShiftY();
        z = sz - getSceneShiftZ();
    }


    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& Changes);

	//! Initializes the object to its default state.
    void init();

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    //! Does object contain relevant data?
    virtual bool hasData(){ return false; }

protected:
    //! Real voxel size.
    double m_dDX, m_dDY, m_dDZ;

    //! Backward conversion of coordinates.
    double m_dInvDX, m_dInvDY, m_dInvDZ;

    //! Scene scaling.
    double m_dMaxX, m_dMaxY, m_dMaxZ;

    //! Scene size in voxels
    double m_dVX, m_dVY, m_dVZ;
};

class CAuxCoordinatesConv : public CCoordinatesConv
{
	public :

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& Changes);

	//! Initializes the object to its default state.
    void init();

};

namespace Storage
{
	//! Coordinate conversion object.
	DECLARE_OBJECT(PatientConv, CCoordinatesConv, PATIENT_DATA + 1);
	DECLARE_OBJECT(AuxConv, CCoordinatesConv, AUX_DATA + 1);
}

} // namespace data

#endif // CCoordinatesConv_H
