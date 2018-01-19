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

#ifndef CModel_H
#define CModel_H

#include <VPL/Base/BasePtr.h>
#include <VPL/Base/SharedPtr.h>

#include <data/CSerializableData.h>
#include <data/CSerializableOSG.h>

#include "data/CColorVector.h"
#include "data/CObjectHolder.h"

#include "geometry/base/CMesh.h"
#include <data/CSnapshot.h>

#include <data/CStorageInterface.h>

namespace data
{

    class CMeshSnapshot : public data::CSnapshot
    {
    protected:
        int *propertyVertexFlags;
        geometry::CMesh::Point *vertices;
        int vertexCount;
        int *indices;
        int indexCount;

    public:
        CMeshSnapshot(CUndoProvider *provider = NULL);
        ~CMeshSnapshot();
        virtual long getDataSize();

        friend class CModel;
    };

    ///////////////////////////////////////////////////////////////////////////////
    //! Polygonal surface model...

    class CModel
        : public vpl::base::CObject
        , public vpl::mod::CSerializable
        , public CUndoProvider
    {
    public:
        //! Default class name.
        VPL_ENTITY_NAME("Model");

        //! Default compression method.
        //VPL_ENTITY_COMPRESSION(vpl::mod::CC_RAW);
        VPL_ENTITY_COMPRESSION(vpl::mod::CC_GZIP);

        //! Smart pointer type.
        //! - Declares type tSmartPtr.
        VPL_SHAREDPTR(CModel);

        //! Undefined model identifier.
        enum { UNDEFINED = -1 };

        //! Helper flags you can pass to the invalidate() method.
        enum
        {
            MESH_CHANGED = 1 << 3,
            POSITION_CHANGED = 1 << 4,
            SELECTION_CHANGED = 1 << 5,
            COLORING_CHANGED = 1 << 6,
			VISIBILITY_CHANGED = 1 << 7,
			LABEL_CHANGED = 1 << 8,
			PROPERTY_CHANGED = 1 << 9
        };

    public:
        //! Default constructor.
        CModel()
            : m_bVisibility(false)
            , m_Color(1.0f, 1.0f, 1.0f, 1.0f)
            , m_bSelected(false)
            , m_bUseVertexColors(false)
			, m_regionId(-1)
			, m_bLinkedWithRegion(true)
        { }

        //! Destructor.
        ~CModel()
        { }

        //! Returns the model geometry.
        geometry::CMesh *getMesh()
        {
            return m_spModel.get();
        }

        //! Returns the model geometry and clears Destroy flag in smart pointer
        geometry::CMesh *releaseMesh()
        {
            return m_spModel.release();
        }

        // Undo provider interface
        virtual CSnapshot *getSnapshot(CSnapshot *snapshot);
        virtual void restore(CSnapshot *snapshot);
        void createAndStoreSnapshot();

        //! Sets the model geometry.
        //! - Deletes a given object automatically.
        void setMesh(geometry::CMesh *model)
        {
            m_spModel = model;
        }

        //! Returns true if the model is visible.
        bool isShown() const { return m_bVisibility; }

        //! Turns on visibility of the model.
        void show() { m_bVisibility = true; }

        //! Turns off visibility of the model.
        void hide() { m_bVisibility = false; }

        //! Turns on/off visibility of the model. 
        void setVisibility(bool visible) { m_bVisibility = visible; }

        //! Turns on/off visibility of the model. 
        void setUseVertexColors(bool value) { m_bUseVertexColors = value; }
        bool getUseVertexColors() const { return m_bUseVertexColors; }

        //! (De)selects model
        void select(bool value = true)
        {
            m_bSelected = value;
        }

        void deselect()
        {
            select(false);
        }

        //! Returns true if model is selected
        bool isSelected() const
        {
            return m_bSelected;
        }

        //! Returns the model color.
        void getColor(float& r, float& g, float& b, float& a) const
        {
            m_Color.getColor(r, g, b, a);
        }

        //! Returns the model color.
        const CColor4f& getColor() const { return m_Color; }

        //! Sets the model color.
        void setColor(float r, float g, float b, float a = 1.0f)
        {
            m_Color.setColor(r, g, b, a);
        }

        //! Sets the model color.
        void setColor(const CColor4f& Color) { m_Color.setColor(Color); }

		//! Sets the region id.
		void setRegionId(int id)
		{
			m_regionId = id;
		}

		//! Returns the region id.
		int getRegionId()
		{
			return m_regionId;
		}

		//! Sets if model mirrors region data.
		void setLinkedWithRegion(bool linked)
		{
			m_bLinkedWithRegion = linked;
		}

		//! Returns true if model mirrors region data.
		bool isLinkedWithRegion()
		{
			return m_bLinkedWithRegion;
		}

        //! Returns voluntary transformation matrix
        const osg::Matrix& getTransformationMatrix() { return m_transformationMatrix; }

        //! Sets transformation matrix for the model
        void setTransformationMatrix(const osg::Matrix& matrix) { m_transformationMatrix = matrix; }

        //! Get model name
        const std::string& getLabel() const { return m_label; }

        //! Set model name
        void setLabel(const std::string &label) { m_label = label; }

        //! Get model property
        std::string getProperty(const std::string &prop) const
        {
            auto it = m_properties.find(prop);
            if (it != m_properties.end())
            {
                return it->second;
            }
            return "";
        }

		//! Get floating point model property
		double getFloatProperty(const std::string &prop) const
		{
			std::string val = getProperty(prop);
			return strtod(val.c_str(),NULL);
		}

		//! Get int model property
		long getIntProperty(const std::string &prop) const
		{
			std::string val = getProperty(prop);
			return strtol(val.c_str(),NULL,10);
		}

        //! Set model property
        void setProperty(const std::string &prop, const std::string &value) { m_properties[prop] = value; }

        //! Set int property
        void setIntProperty(const std::string &prop, int value) 
		{ 
			std::stringstream ss;
			ss << value;
			m_properties[prop] = ss.str(); 
		}

        //! Set floating point property
        void setFloatProperty(const std::string &prop, double value) 
		{ 
			std::stringstream ss;
			ss << value;
			m_properties[prop] = ss.str(); 
		}

        //! Clear all properties
        void clearAllProperties() { m_properties.clear(); }

		//! Copy all properties
		void copyAllProperties( const data::CModel & model )
		{
			m_properties = model.m_properties;
		}

		//! Direct access to all properties
		const std::map<std::string, std::string> & getAllProperties() const { return m_properties; }

        //! Clear the model.
        void clear() { m_spModel = new geometry::CMesh; }

        //! Regenerates the object state according to any changes in the data storage.
        void update(const CChangedEntries& Changes);

        //! Initializes the object to its default state.
        void init();

        //! Does object contain relevant data?
        bool hasData() { return (m_spModel.get() && m_spModel->n_vertices() > 0); }

        //! Returns true if changes of a given parent entry may affect this object.
        bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

        //! Compute areas oriented up and down
        void getUpDownSurfaceAreas(float &up_area, float &down_area);

        //! Copy model
        CModel &operator=(const CModel &model);

        //! Serialize
        template<class tpSerializer>
        void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
        {
            Writer.beginWrite(*this);

            WRITEINT32(5); // version

            Writer.write((vpl::sys::tInt32)m_bVisibility);
            Writer.write((vpl::sys::tInt32)m_bSelected);
            Writer.write((vpl::sys::tInt32)m_bUseVertexColors);
            m_Color.serialize(Writer);
            Writer.write(m_label);
            CSerializableData< osg::Matrix >::serialize(&m_transformationMatrix, Writer);
            Writer.write((vpl::sys::tInt32)m_properties.size());

            for (auto it = m_properties.begin(); it != m_properties.end(); it++)
            {
                Writer.write(it->first);
                Writer.write(it->second);
            }

			Writer.write((vpl::sys::tInt32)m_regionId);
			Writer.write((vpl::sys::tInt32)m_bLinkedWithRegion);

            Writer.endWrite(*this);

            m_spModel->serialize(Writer);
        }

        //! Deserialize
        template<class tpSerializer>
        void deserialize(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
        {
            Reader.beginRead(*this);

            int version = 0;
            READINT32(version);

            vpl::sys::tInt32 bVis = 0;
            Reader.read(bVis); m_bVisibility = bVis != 0;

            if (version > 2)
            {
                vpl::sys::tInt32 bSel = 0;
                Reader.read(bSel); m_bSelected = bSel != 0;
            }

            if (version > 3)
            {
                vpl::sys::tInt32 bColors = 0;
                Reader.read(bColors); m_bUseVertexColors = bColors != 0;
            }

            m_Color.deserialize(Reader);
            Reader.read(m_label);
            CSerializableData< osg::Matrix >::deserialize(&m_transformationMatrix, Reader);
            if (version>1)
            {
                vpl::sys::tInt32 nProps = 0;
                Reader.read(nProps);
                for (int i = 0; i < nProps; i++)
                {
                    std::string key, value;
                    Reader.read(key);
                    Reader.read(value);
                    m_properties[key] = value;
                }
            }

			if (version > 4)
			{
				vpl::sys::tInt32 regionId = -1;
				Reader.read(regionId); m_regionId = regionId;

				vpl::sys::tInt32 bLinked = 0;
				Reader.read(bLinked); m_bLinkedWithRegion = bLinked != 0;
			}

            Reader.endRead(*this);

            m_spModel->deserialize(Reader);
        }

    protected:
        //! Model storage.
        vpl::base::CScopedPtr<geometry::CMesh> m_spModel;

        //! Model visibility.
        bool m_bVisibility;

        //! Model visibility.
        bool m_bSelected;

        //! Model visibility.
        bool m_bUseVertexColors;

		//! Id of region from which the model was created.
		int m_regionId;

		//! If model mirrors region data.
		bool m_bLinkedWithRegion;

        //! Model color (RGBA components).
        CColor4f m_Color;

        //! Model transformation matrix
        osg::Matrix m_transformationMatrix;

        //! Model name
        std::string m_label;

        //! Named model properties
        std::map<std::string, std::string> m_properties;
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //! Serialization wrapper. 

    DECLARE_SERIALIZATION_WRAPPER(CModel)

} // namespace data

#endif // CModel_H
