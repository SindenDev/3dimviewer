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

#ifndef CImageLoaderInfo_H
#define CImageLoaderInfo_H

#include <VPL/Base/SharedPtr.h>

#include "data/CObjectHolder.h"
#include "data/CStorageEntry.h"
#include <data/storage_ids_core.h>

// STL
#include <string>
#include <vector>

// Serialization
#include <VPL/Module/Serializable.h>

namespace data
{

///////////////////////////////////////////////////////////////////////////////
//! Class encapsulates a list of DICOM files loaded by an user.

class CImageLoaderInfo : public vpl::base::CObject
{
public:
    //! Smart pointer type.
    VPL_SHAREDPTR(CImageLoaderInfo);

    //! List of image files.
    typedef std::vector<vpl::sys::tString> tFiles;

public:
	//! Default constructor
    CImageLoaderInfo() {}

	//! Destructor.
    ~CImageLoaderInfo() {}

    //! Returns the input directory.
    const vpl::sys::tString& getPath() const { return m_Path; }

    //! Stores the input directory.
    CImageLoaderInfo& setPath(const vpl::sys::tString& Path)
    {
        m_Path = Path;
        return *this;
    }
    
    //! Returns true if no image file was loaded.
    bool isEmpty() const { return m_Files.empty(); }

    //! Returns number of files in the list.
    int getNumOfFiles() const { return int(m_Files.size()); }

    //! Adds a given file to the list.
    CImageLoaderInfo& add(const vpl::sys::tString& Filename)
    {
        m_Files.push_back(Filename);
        return *this;
    }

    //! Clears the list of loaded image files.
    CImageLoaderInfo& clear()
    {
        m_Files.clear();
        return *this;
    }

    //! Sets the list of files.
    CImageLoaderInfo& setList(const tFiles& Files)
    {
        m_Files = Files;
        return *this;
    }

    //! Returns reference to the internal list.
    tFiles& getList() { return m_Files; }
    const tFiles& getList() const { return m_Files; }


    //! Regenerates the object state according to any changes in the data storage.
    void update(const CChangedEntries& VPL_UNUSED(Changes))
    {
        // Does nothing...
    }

	//! Initializes the default state of the object.
    void init()
    {
        m_Files.clear();
        m_Path.clear();
    }

    //! Returns DICOM id
    std::string getId()
    {
        return m_Id;
    }

    //! Sets DICOM id
    void    setId( const std::string & id )
    {
        m_Id = id;
    }

    //! Returns true if changes of a given parent entry may affect this object.
    bool checkDependency(CStorageEntry * VPL_UNUSED(pParent))
    {
//		vpl::mod::CBinarySerializer serializer( 0 );
//		serialize< vpl::mod::CBinarySerializer >( serializer );
        return true;
    }

    //! Does object contain relevant data?
    virtual bool hasData() { return false; }

	//! Serialize
	template < class tpSerializer >
	void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
	{
		// Store files list size
		Writer.write( (vpl::sys::tUInt32) m_Files.size() );

		// Store files
//		tFiles::iterator it, itEnd( m_Files.end() );
//			Writer.write( *it );

		// Store source directory
//		Writer.write( m_Path );

		// Store dicom id
		Writer.write( m_Id );
	}

	//! Deserialize
	template < class tpSerializer >
	void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
	{
		// Read files list size
		vpl::sys::tUInt32 size = 0;
		Reader.read( size );
		m_Files.resize( size );

		// Read files
		//		tFiles::iterator it, itEnd( m_Files.end() );
		//			Reader.read( *it );

		// Read source directory
		//		Reader.read( m_Path );

		// Read dicom id
		Reader.read( m_Id );
	}

	
protected:
    //! List of loaded files.
    tFiles m_Files;

    //! Input directory.
    vpl::sys::tString m_Path;

    //! Dicom id 
    std::string m_Id;
};

namespace Storage
{
	//! Image loader info
	DECLARE_OBJECT(ImageLoaderInfo, CImageLoaderInfo, CORE_STORAGE_IMAGE_LOADER_INFO_ID);
}

} // namespace data

#endif // CImageLoaderInfo_H

