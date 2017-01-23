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

#ifndef _CUSTOMDATA_H
#define _CUSTOMDATA_H

#include <data/CStorageInterface.h>
#include <VPL/Base/Object.h>
#include <data/CSerializableData.h>
#include <AppVersion.h>
#include <QDateTime>
#include <QVariant>
#include <stdlib.h>

#include <data/CStorageInterface.h>
#include <data/storage_ids_qtgui.h>

namespace data
{
    //! Class that saves custom data in XML format
    class CCustomData
        : public vpl::base::CObject
        , public vpl::mod::CSerializable
    {
		public:
			struct sEntry
			{
				QString name;
				QString value;
				QVariant data;
			};
		public :
		    //! Smart pointer type.
		    VPL_SHAREDPTR(CCustomData);
            //! Default class name.
	        VPL_ENTITY_NAME("CustomData");
            //! Default compression method.
            VPL_ENTITY_COMPRESSION(vpl::mod::CC_GZIP); // CC_RAW

            //! Constructor
            CCustomData();

	        //! Regenerates the object state according to any changes in the data storage.
            void update(const data::CChangedEntries & Changes) {}

	        //! Initializes the object to its default state.
	        virtual void init();

	        //! Returns true if changes of a given parent entry may affect this object.
	        virtual bool checkDependency(data::CStorageEntry *pParent) { return false; }

            //! Does object contain relevant data?
            virtual bool hasData() { return true; }

            //! Serializer
	        template < class tpSerializer >
	        void serialize( vpl::mod::CChannelSerializer<tpSerializer> & Writer )
	        {
		        Writer.beginWrite( * this );

				std::string str = writeXML();
				Writer.write( str );

		        Writer.endWrite( * this );
	        }

	        //! Deserialize
	        template < class tpSerializer >
	        void deserialize( vpl::mod::CChannelSerializer<tpSerializer> & Reader )
	        {
		        Reader.beginRead( * this );
		
				std::string str;
				Reader.read(str);
				readXML(str);

                Reader.endRead( * this ); 
 	        }

			//! Deserialize XML
			void readXML(const std::string& str);

			//! Serialize XML
			std::string writeXML();

			//! Set data
			void setDataValue(const QString& group, const QString& entry, const QString& value, const QVariant& data);

			//! Get data
			QString getDataValue(const QString& group, const QString& entry);

			//! Get data
			bool getDataValue(const QString& group, const QString& entry, QString& value, QVariant& data);

			//! Get group data
			const QList<sEntry>& getGroupData(const QString& group);

			//! Clear group data
			void clearGroup(const QString& group);

			//! Clear all data
			void clearAll();

            //! Detect presence of this block (needs that you save and restore channel pos though)
            template < class tpSerializer >
            bool isAvailable( vpl::mod::CChannelSerializer<tpSerializer> & Reader )
            {
                static const char *pcName = this->getName();
                int Compression;
                vpl::tSize BlockSize;
                if( !vpl::mod::BinarySerializer::readGroupHeader(*Reader.getChannelPtr(), pcName, Compression, BlockSize) || BlockSize < 0 )
                    return false;
                return true;
            }

    public:
		QMap<QString, QList<sEntry> > m_data;
    }; // CCustomData


    DECLARE_SERIALIZATION_WRAPPER( CCustomData )
	
	namespace Storage
	{
		//! Document version
		DECLARE_OBJECT(CustomData, CCustomData, QTGUI_STORAGE_CUSTOM_DATA_ID);
	}
	
} // namespace

#endif