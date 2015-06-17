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
            MESH_NOT_CHANGED = 1 << 3,
            POSITION_NOT_CHANGED = 1 << 4,
            SELECTION_NOT_CHANGED = 1 << 5,

            NOTHING_CHANGED = MESH_NOT_CHANGED | POSITION_NOT_CHANGED | SELECTION_NOT_CHANGED,
        };

    public:
        //! Default constructor.
        CModel()
            : m_bVisibility(false)
            , m_Color(1.0f, 1.0f, 1.0f, 1.0f)
            , m_bSelected(false)
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

        //! Set model property
        void setProperty(const std::string &prop, const std::string &value) { m_properties[prop] = value; }

        //! Clear all properties
        void clearAllProperties() { m_properties.clear(); }

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

            WRITEINT32(3); // version

            Writer.write((vpl::sys::tInt32)m_bVisibility);
            Writer.write((vpl::sys::tInt32)m_bSelected);
            m_Color.serialize(Writer);
            Writer.write(m_label);
            CSerializableData< osg::Matrix >::serialize(&m_transformationMatrix, Writer);
            Writer.write((vpl::sys::tInt32)m_properties.size());
            for (auto it = m_properties.begin(); it != m_properties.end(); it++)
            {
                Writer.write(it->first);
                Writer.write(it->second);
            }

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
                Reader.read(bSel); m_bSelected = bVis != 0;
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
