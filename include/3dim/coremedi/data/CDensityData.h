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

#ifndef CDensityData_H
#define CDensityData_H

///////////////////////////////////////////////////////////////////////////////
// include files

#include <VPL/Base/Lock.h>
#include <VPL/Image/DensityVolume.h>
#include <VPL/Module/Serializable.h>
#include <VPL/ImageIO/DicomSlice.h>
#include <VPL/Math/StaticMatrix.h>
#include <VPL/Math/StaticVector.h>

// serialization
#include <data/CSerializableData.h>

// undo support
#include <data/CVolumeUndo.h>

#include "data/CObjectHolder.h"

// STL
#include <string>

#include <data/CStorageInterface.h>
#include <data/storage_ids_core.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Class manages source 3D volume data.

class CDensityData : public vpl::img::CDensityVolume//, public vpl::base::CLockableObject<CDensityData>
{
public:
    //! Standard method getEntityName().
    VPL_ENTITY_NAME("DensityData");

    //! Standard method getEntityCompression().
    VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);

    //! Smart pointer type.
    //! - Declares type tSmartPtr.
    VPL_SHAREDPTR(CDensityData);

    //! Lock type.
//    typedef CLockableObject<CDensityData>::CLock tLock;

    //! Initial size.
    enum { INIT_SIZE = 64 };

    //! Helper flag passed to the invalidate() method (only the pixel values has been changed).
    enum { DENSITY_MODIFIED = 1 << 2 };

    //! Transform matrix used for DICOM import.
    typedef vpl::math::CDMatrix3x3 tMatrix;

    //! Translation vector used for DICOM import.
    typedef vpl::math::CDVector3 tVector;

	//! Volume snapshot provider type
	typedef data::CVolumeUndo< vpl::img::CDensityVolume > tVolumeUndo;

public:
    //! Default constructor.
    CDensityData();

    //! Copy constructor.
    CDensityData(const CDensityData& Data);

    //! Destructor.
    ~CDensityData();

    //! Init volume undo
    void initVolumeUndo(int storageID);

    //! Gets header data (patient info, series info, etc.) from a given DICOM slice.
    void takeDicomData(vpl::img::CDicomSlice& DicomSlice);

    //! Sets image position (can't be taken from the same slice as in the function above)
    void setImagePosition(double x, double y, double z);

    //! Clears all the additional dicom data (patient name, etc.).
    void clearDicomData();

    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& Changes);

	//! Initializes the object to its default state.
    void init();

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

    //! Serialize
    template < class tpSerializer >
    void serialize( vpl::mod::CChannelSerializer<tpSerializer> & Writer )
    {
        vpl::img::CDensityVolume::serialize( Writer );

        Writer.beginWrite( *this );

        WRITEINT32( 4 ); // version

        Writer.write( (vpl::sys::tInt32) m_iSeriesNumber );
        Writer.write( m_sPatientName );
        Writer.write( m_sPatientId );
        Writer.write( m_sModality );
        Writer.write( m_sSeriesUid );
        Writer.write( m_sSeriesDate );
        Writer.write( m_sSeriesTime );
        Writer.write( m_sPatientPosition );
        Writer.write( m_sManufacturer );
        Writer.write( m_sModelName );

        // Serialize vectors
        Writer.write( m_ImageOrientationX.getX() );
        Writer.write( m_ImageOrientationX.getY() );
        Writer.write( m_ImageOrientationX.getZ() );

        Writer.write( m_ImageOrientationY.getX() );
        Writer.write( m_ImageOrientationY.getY() );
        Writer.write( m_ImageOrientationY.getZ() );

        // version 2 data        
        Writer.write( m_sPatientBirthday );
        Writer.write( m_sPatientSex );
        Writer.write( m_ImagePosition.getX() );
        Writer.write( m_ImagePosition.getY() );
        Writer.write( m_ImagePosition.getZ() );

        // version 3 data        
        Writer.write( m_ImageSubSampling.getX() );
        Writer.write( m_ImageSubSampling.getY() );
        Writer.write( m_ImageSubSampling.getZ() );

		// version 4 data        
		Writer.write( m_sPatientDescription );
		Writer.write( m_sStudyUid );
		Writer.write( m_sStudyId );
		Writer.write( m_sStudyDate );
		Writer.write( m_sStudyDescription );
		Writer.write( m_sSeriesDescription );
		Writer.write( m_sScanOptions );

        Writer.endWrite( *this );
    }

    //! Does object contain relevant data?
    bool hasData() { return (getXSize() != INIT_SIZE || getYSize() != INIT_SIZE || getZSize() != INIT_SIZE); }

	//! Get checksum of voxel data
	unsigned int getDataCheckSum() const;

    //! Deserialize
    template < class tpSerializer >
    void deserialize( vpl::mod::CChannelSerializer<tpSerializer> & Reader )
    {
        // Try to load the data
        bool readError = false;

        try
        {
            vpl::img::CDensityVolume Data;
            Data.deserialize( Reader );

            // Use the data
            makeRef(Data);
        }
        catch (std::bad_alloc &e)
        {
            VPL_LOG_INFO("Exception: " << e.what());
            readError = true;

            // Use the data
            resize(INIT_SIZE, INIT_SIZE, INIT_SIZE);
        }

        // Fill the margin
        mirrorMargin();

        Reader.beginRead( *this );

        int version = 0;
        READINT32( version );

        READINT32( m_iSeriesNumber );
        Reader.read( m_sPatientName );
        Reader.read( m_sPatientId );
        Reader.read( m_sModality );
        Reader.read( m_sSeriesUid );
        Reader.read( m_sSeriesDate );
        Reader.read( m_sSeriesTime );
        Reader.read( m_sPatientPosition );
        Reader.read( m_sManufacturer );
        Reader.read( m_sModelName );

        // Read vectors
        vpl::img::CVector3D::tComponent x, y, z;

        Reader.read( x );Reader.read( y ); Reader.read( z );
        m_ImageOrientationX.setXYZ( x, y, z );

        Reader.read( x );Reader.read( y ); Reader.read( z );
        m_ImageOrientationY.setXYZ( x, y, z );

        if (version>1)
        {
            Reader.read( m_sPatientBirthday );
            Reader.read( m_sPatientSex );
            Reader.read( x );Reader.read( y ); Reader.read( z );
            m_ImagePosition.setXYZ( x, y, z );
        }
        if (version>2)
        {
            Reader.read( x );
            Reader.read( y ); 
            Reader.read( z );
            m_ImageSubSampling.setXYZ( x, y, z );
        }
		if (version>3)
		{
			Reader.read( m_sPatientDescription );
			Reader.read( m_sStudyUid );
			Reader.read( m_sStudyId );
			Reader.read( m_sStudyDate );
			Reader.read( m_sStudyDescription );
			Reader.read( m_sSeriesDescription );
			Reader.read( m_sScanOptions );
		}

        Reader.endRead( *this );

        if (readError)
        {
            throw vpl::mod::Serializer::CReadFailed();
        }
    }

    //! Get snapshot of the whole volume
    data::CSnapshot * getVolumeSnapshot() { return m_volumeUndo.getSnapshotVolume( ); }

    //! Get snapshot of the plane XY 
    data::CSnapshot * getPlaneXYSnapshot( int position ) { return m_volumeUndo.getSnapshotXY( position ); }

    //! Get snapshot of the plane XZ 
    data::CSnapshot * getPlaneXZSnapshot( int position ) { return m_volumeUndo.getSnapshotXZ( position ); }

    //! Get snapshot of the plane YZ 
    data::CSnapshot * getPlaneYZSnapshot( int position ) { return m_volumeUndo.getSnapshotYZ( position ); }

    //! Set subsampling information
    void setImageSubSampling(const vpl::img::CVector3D& subSampling) { m_ImageSubSampling = subSampling; }

    //! Get subsampling information
    vpl::img::CVector3D getImageSubSampling() const { return m_ImageSubSampling; }

protected:
    //! Volume undo object
    tVolumeUndo m_volumeUndo;

public:
    //! DICOM series number.
    int m_iSeriesNumber;

    //! DICOM patient name.
    std::string m_sPatientName;

    //! DICOM patient ID.
    std::string m_sPatientId;

    //! Patient birthday.
    std::string m_sPatientBirthday;

    //! Patient sex.
    std::string m_sPatientSex;

    //! DICOM data modality.
    std::string m_sModality;

    //! DICOM series ID.
    std::string m_sSeriesUid;

    //! DICOM series date.
    std::string m_sSeriesDate;

    //! DICOM series time.
    std::string m_sSeriesTime;

    //! DICOM patient position, on table.
    std::string m_sPatientPosition;

    //! Manufacturer.
    std::string m_sManufacturer;

    //! Manufacturer�s model name.
    std::string m_sModelName;

    //! DICOM image slice position
    vpl::img::CVector3D m_ImagePosition;

    //! DICOM image slice orientation, axis X.
    vpl::img::CVector3D m_ImageOrientationX;

    //! DICOM image slice orientation, axis Y.
    vpl::img::CVector3D m_ImageOrientationY;

    //! Image subsampling
    vpl::img::CVector3D m_ImageSubSampling;

	// added 2014-09-24

    //! Patient description.
    std::string m_sPatientDescription;

    //! Study uid.
    std::string m_sStudyUid;

    //! Study id.
    std::string m_sStudyId;

    //! Study date.
    std::string m_sStudyDate;

    //! Study description.
    std::string m_sStudyDescription;

    //! Series description.
    std::string m_sSeriesDescription;

    //! Scan options (AXIAL, etc.).
    std::string m_sScanOptions;
};


///////////////////////////////////////////////////////////////////////////////
//! Serialization wrapper.

DECLARE_SERIALIZATION_WRAPPER( CDensityData )

namespace Storage
{
	//! Patient density data.
	DECLARE_OBJECT(PatientData, CDensityData, PATIENT_DATA);
	//! Helper density data.
	DECLARE_OBJECT(AuxData, CDensityData, AUX_DATA);
}

} // namespace data

#endif // CDensityData_H

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
