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

#include "geometry/base/CMesh.h"
#include "geometry/base/CArmature.h"
#include <geometry/base/kdtree/kdtree.h>
#include <data/CSnapshot.h>

#include <data/CStorageInterface.h>

namespace data
{

    class CMeshSnapshot : public data::CSnapshot
    {
    protected:
        geometry::CMesh m_mesh;

    public:
        CMeshSnapshot(CUndoProvider *provider = NULL);
        ~CMeshSnapshot();
        virtual long getDataSize();

        const geometry::CMesh &getMesh() const { return m_mesh; }
        void setMesh(const geometry::CMesh &mesh) { m_mesh = mesh; }
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
			PROPERTY_CHANGED = 1 << 9,
            ARMATURE_CHANGED = 1 << 10,
            ACTIONS_CHANGED = 1 << 11,
            ORTHO_TOOTH_ADDED = 1 << 12,
            VERTEX_COLORING_CHANGED = 1 << 13
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
            , m_bMirrored(false)
            , m_locked(false)
            , m_bExamined(false)
            , m_mesh_dirty(true)
        { }

        //! Default copy constructor
        CModel(const CModel&);
        
        //! Destructor.
        ~CModel()
        { }

        //! Returns the model geometry.
        geometry::CMesh *getMesh()
        {
            return m_spModel.get();
        }

        //! Const version of the previous method
        const geometry::CMesh *getMesh() const
        {
            return m_spModel.get();
        }

        //! Returns the model geometry and clears Destroy flag in smart pointer
        geometry::CMesh *releaseMesh()
        {
            return m_spModel.release();
            m_mesh_dirty = true;
        }

        //! Fills given mesh with mesh deformed by current armature. It should keep properties and just apply matrices given by armature if requested.
        void getDeformedMesh(geometry::CMesh &mesh, bool keepProperties = false);

        // Undo provider interface
        virtual CSnapshot *getSnapshot(CSnapshot *snapshot);
        virtual void restore(CSnapshot *snapshot);
        void createAndStoreSnapshot(CSnapshot *childSnapshot = NULL);

        //! Sets the model geometry.
        //! - Deletes a given object automatically.
        void setMesh(geometry::CMesh *model)
        {
            m_spModel = model;
            m_mesh_dirty = true;
        }

        //! This strictly MUST be called after any mesh change excluding setMesh method call (it updates dirty flag internally).
        void setMeshDirty() { m_mesh_dirty = true; }

        //! Returns true if the model is visible.
        bool isVisible() const
        {
            return m_bVisibility;
        }

        //! Turns on visibility of the model.
        void show()
        {
            m_bVisibility = true;
        }

        //! Turns off visibility of the model.
        void hide()
        {
            m_bVisibility = false;
        }

        //! Turns on/off visibility of the model. 
        void setVisibility(bool visible)
        {
            m_bVisibility = visible;
        }

        //! Turns on/off visibility of the model. 
        void setUseVertexColors(bool value)
        {
            m_bUseVertexColors = value;
        }

        bool getUseVertexColors() const
        {
            return m_bUseVertexColors;
        }

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
        const CColor4f& getColor() const
        {
            return m_Color;
        }

        //! Sets the model color.
        void setColor(float r, float g, float b, float a = 1.0f)
        {
            m_Color.setColor(r, g, b, a);
        }

        //! Sets the model color.
        void setColor(const CColor4f& Color)
        {
            m_Color.setColor(Color);
        }

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

        //! Returns voluntary transformation matrix - !!! Remember that model visualizers are anchored to center
        const osg::Matrix& getTransformationMatrix() const
		{
			return m_transformationMatrix;
		}

        //! Sets transformation matrix for the model
        void setTransformationMatrix(const osg::Matrix& matrix)
		{
			m_transformationMatrix = matrix;
		}

        //! Get model name
        const std::string& getLabel() const
        {
            return m_label;
        }

        //! Set model name
        void setLabel(const std::string &label) { m_label = label; }

        void setReserved(bool value = true) { m_reserved = value; };
        bool getReserved() { return m_reserved; };

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
        void setProperty(const std::string &prop, const std::string &value)
        {
            m_properties[prop] = value;
        }

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
        void clearAllProperties()
        {
            m_properties.clear();
        }

		//! Copy all properties
		void copyAllProperties( const data::CModel & model )
		{
			m_properties = model.m_properties;
		}

		//! Direct access to all properties
		const std::map<std::string, std::string> & getAllProperties() const
        {
            return m_properties;
        }

        //! Clear the model.
        void clear()
        {
            m_spModel = new geometry::CMesh;
            m_spArmature = new geometry::CArmature;
            m_mesh_dirty = true;
        }

        //! Regenerates the object state according to any changes in the data storage.
        void update(const CChangedEntries& Changes);

        //! Initializes the object to its default state.
        void init();

        //! Does object contain relevant data?
        bool hasData()
        {
            return (m_spModel.get() && m_spModel->n_vertices() > 0);
        }

        //! Returns true if changes of a given parent entry may affect this object.
        bool checkDependency(CStorageEntry * VPL_UNUSED(pParent)) { return true; }

        //! Compute areas oriented up and down
        void getUpDownSurfaceAreas(float &up_area, float &down_area);

        geometry::CArmature *getArmature()
        {
            return m_spArmature.get();
        }

        void setArmature(geometry::CArmature *armature)
        {
            m_spArmature = armature;
        }

        //! Copy model
        CModel &operator=(const CModel &model);

 
        /**
         * \fn  geometry::Matrix computeSurfaceAlignedPositionMatrix(const geometry::Vec3 &world_coordinates_point);
         *
         * \brief   Calculates the surface aligned position matrix - this matrix orients y axis in the closest face normal, 
         *          z axis directs to the same way as world coordinate system z-axis as possible and x axis is orthogonal
         *          to the both of them
         *
         * \param   world_coordinates_point The point in the world coordinates.
		 * \param   surface_offset The wanted orthogonal distance of the returned position from the mesh surface.
         *
         * \return  The calculated surface aligned position matrix in the world coordinate system.
         */
        osg::Matrix calculateSurfaceAlignedPositionMatrix(const osg::Vec3 &world_coordinates_point, float surface_offset = 0.0f);

        void buildNames(geometry::CBone *bone, int &name, std::map<geometry::CBone *, int> &names)
        {
            names[bone] = name++;
            const std::vector<geometry::CBone *> &children = bone->getChildren();
            for (int i = 0; i < children.size(); ++i)
            {
                buildNames(children[i], name, names);
            }
        }

		/*!
		 * \fn	osg::Vec3 calculateAverageVertexNormal(CMesh::FHandle vertex_handle);
		 *
		 * \brief	Calculates the average normal (composite normal of the given vertex and all neighbor vertices).
		 *
		 * \param	vertex_handle	Handle of the vertex.
		 *
		 * \return	The calculated average face normal.
		 */

		osg::Vec3 calculateAverageVertexNormal(geometry::CMesh::VHandle vertex_handle);

        /**
        * \struct  SClosestData
        *
        * \brief   This structure stores information of the point-model questions closest structures.
        */

        struct SClosestData
        {
            SClosestData() : distance(-1.0), precise_distance(-1.0) {}
            //! Closest vertex on model
            geometry::Vec3 point;
            //! Closest point on model - in the most of the cases it will lay on any of the closest faces. Precise point computation is on demand.
            geometry::Vec3 precise_point;
            //! Normal in the precise closest point position
            geometry::Vec3 precise_normal;
            //! Normal in the position of the closest vertex
            geometry::Vec3 normal;
            //! Vertex handle of the closest vertex
            geometry::CMesh::VHandle vhandle;
            //! Face handle of the closest face
            geometry::CMesh::FHandle fhandle;
            //! Squared distance between query point and the closest vertex and closest point on the surface.
            double distance, precise_distance;
        };

        /*!
         * \fn  double getNearestModelPoint(const geometry::Vec3 &point, geometry::Vec3 &closest_point, bool add_scene_shift_to_matrix, bool bPrecise = true);
         *
         * \brief   Gets nearest model point
         *
         * \param           point                       The query point.
         * \param [in,out]  closest_point               The closest point.
         * \param           add_scene_shift_to_matrix   True to add scene shift to the positioning matrix - without this the resulting coordinates will be in the world space.
         * \param           bPrecise                    (Optional) True if to calculate precise point on
         *                                              the surface or just closest vertex of the mesh.
         *
         * \return  The nearest model point distance. If no point found, returns std::numeric_limits&lt;
         *          double&gt;::max().
         */
        double getNearestModelPoint(const geometry::Vec3 &point, geometry::Vec3 &closest_point, bool add_scene_shift_to_matrix, bool bPrecise = true);

        /*!
         * \fn  double getNearestModelPoint(const geometry::Vec3 &point, SClosestData &data, bool bPrecise = true);
         *
         * \brief   The same as above but SClosestData contains more information. Gets nearest model point - closest vertex or precise point on the surface.
         *
         * \param           point       The query point.
         * \param [in,out]  data        The found point data.
         * \param           add_scene_shift_to_matrix   True to add scene shift to the positioning matrix - without this the resulting coordinates will be in the world space.
         * \param           bPrecise    (Optional) True if to calculate precise point on the surface or just closest vertex of the mesh.
         *
         * \return  The nearest model point. If no point found, returns std::numeric_limits<double>::max().
         */
        double getNearestModelPoint(const geometry::Vec3 &point, SClosestData &data, bool add_scene_shift_to_matrix, bool bPrecise = true);

        void buildInverseNames(geometry::CBone *bone, int &name, std::map<int, geometry::CBone *> &names)
        {
            names[name++] = bone;
            const std::vector<geometry::CBone *> &children = bone->getChildren();
            for (int i = 0; i < children.size(); ++i)
            {
                buildInverseNames(children[i], name, names);
            }
        }

        //! Serialize actions
        template<class tpSerializer>
        void serializeActions(vpl::mod::CChannelSerializer<tpSerializer> &Writer, const std::map<std::string, geometry::CAction> &actions, const std::map<geometry::CBone *, int> &boneNames)
        {
            Writer.write((vpl::sys::tInt32)actions.size());
            for (std::map<std::string, geometry::CAction>::const_iterator it = actions.begin(); it != actions.end(); ++it)
            {
                Writer.write(it->first);
                const geometry::CAction &action = it->second;

                Writer.write((vpl::sys::tInt32)action.getKeyframes().size());
                for (std::map<int, geometry::CKeyframe>::const_iterator kit = action.getKeyframes().begin(); kit != action.getKeyframes().end(); ++kit)
                {
                    Writer.write((vpl::sys::tInt32)kit->first);
                    const geometry::CKeyframe &keyframe = kit->second;

                    Writer.write((vpl::sys::tInt32)keyframe.getMatrices().size());
                    for (std::map<geometry::CBone *, geometry::Matrix>::const_iterator bit = keyframe.getMatrices().begin(); bit != keyframe.getMatrices().end(); ++bit)
                    {
                        int boneName = boneNames.find(bit->first)->second;
                        Writer.write((vpl::sys::tInt32)boneName);
                        geometry::Matrix matrix = bit->second;
                        CSerializableData<geometry::Matrix>::serialize(&matrix, Writer);
                    }
                }
            }
        }

        //! Serialize bone
        template<class tpSerializer>
        void serializeBone(vpl::mod::CChannelSerializer<tpSerializer> & Writer, geometry::CBone *bone)
        {
            if (bone == NULL)
            {
                return;
            }

            geometry::Matrix matrix;
            matrix = bone->getDefaultBoneMatrix();
            CSerializableData<geometry::Matrix>::serialize(&matrix, Writer);
            matrix = bone->getBoneTransformationMatrix();
            CSerializableData<geometry::Matrix>::serialize(&matrix, Writer);
            const std::vector<geometry::CBone *> &children = bone->getChildren();
            Writer.write((vpl::sys::tInt32)children.size());
            for (int i = 0; i < children.size(); ++i)
            {
                serializeBone(Writer, children[i]);
            }
        }

        //! Serialize armature
        template<class tpSerializer>
        void serializeArmature(vpl::mod::CChannelSerializer<tpSerializer> & Writer, geometry::CArmature *armature)
        {
            if (armature == NULL)
            {
                return;
            }

            // serialize bone itself
            serializeBone(Writer, armature);

            // build "names" for bones
            int name = 0;
            std::map<geometry::CBone *, int> boneNames;
            buildNames(armature, name, boneNames);

            // serialize actions using these names
            serializeActions(Writer, armature->getActions(), boneNames);
        }

        //! Copy model without const for pair
        CModel &operator=(CModel &model);


        //! Serialize
        template<class tpSerializer>
        void serialize(vpl::mod::CChannelSerializer<tpSerializer> & Writer)
        {
            Writer.beginWrite(*this);

            WRITEINT32(11); // version

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

            Writer.write((vpl::sys::tInt32)(m_spArmature.get() != NULL));
            serializeArmature(Writer, m_spArmature.get());

			//STUFF FOR TRAUMATECH
			CSerializableData< osg::Matrix >::serialize(&m_transformationMatrixBeforeReposition, Writer);
			CSerializableData< osg::Matrix >::serialize(&m_transformationMatrixAfterReposition, Writer);

            Writer.write((vpl::sys::tInt32)m_bMirrored);
            Writer.write((vpl::sys::tInt32)m_locked);
            Writer.write((vpl::sys::tInt32)m_bExamined);

            bool isCreatedAbutment = this->getProperty("createdAbutment").compare("true") == 0;
            Writer.write((vpl::sys::tInt32)isCreatedAbutment);

            Writer.endWrite(*this);

            m_spModel->serialize(Writer);
        }

        //! Deserialize actions
        template<class tpSerializer>
        void deserializeActions(vpl::mod::CChannelSerializer<tpSerializer> &Reader, std::map<std::string, geometry::CAction> &actions, const std::map<int, geometry::CBone *> &boneNames)
        {
            vpl::sys::tInt32 actionCount = 0;
            Reader.read(actionCount);
            for (int a = 0; a < actionCount; ++a)
            {
                std::string actionName;
                Reader.read(actionName);
                geometry::CAction &action = actions[actionName];

                vpl::sys::tInt32 keyframeCount = 0;
                Reader.read(keyframeCount);
                for (int k = 0; k < keyframeCount; ++k)
                {
                    vpl::sys::tInt32 keyframeFrame = 0;
                    Reader.read(keyframeFrame);
                    geometry::CKeyframe &keyframe = action.getKeyframes()[keyframeFrame];

                    vpl::sys::tInt32 boneCount = 0;
                    Reader.read(boneCount);
                    for (int b = 0; b < boneCount; ++b)
                    {
                        vpl::sys::tInt32 boneName = 0;
                        Reader.read(boneName);
                        geometry::CBone *bone = boneNames.find(boneName)->second;

                        geometry::Matrix matrix;
                        CSerializableData<geometry::Matrix>::deserialize(&matrix, Reader);

                        keyframe.getMatrices()[bone] = matrix;
                    }
                }
            }
        }

        //! Deserialize bone
        template<class tpSerializer>
        void deserializeBone(vpl::mod::CChannelSerializer<tpSerializer> & Reader, geometry::CBone *bone)
        {
            geometry::Matrix matrix;

            CSerializableData<geometry::Matrix>::deserialize(&matrix, Reader);
            bone->setDefaultBoneMatrix(matrix);

            CSerializableData<geometry::Matrix>::deserialize(&matrix, Reader);
            bone->setBoneTransformationMatrix(matrix);

            bone->makeDirty(); // just to be sure

            vpl::sys::tInt32 count = 0;
            Reader.read(count);
            for (int i = 0; i < count; ++i)
            {
                geometry::CBone *child = new geometry::CBone;
                bone->addChild(child);
                deserializeBone(Reader, child);
            }
        }

        //! Deserialize armature
        template<class tpSerializer>
        geometry::CArmature *deserializeArmature(vpl::mod::CChannelSerializer<tpSerializer> & Reader)
        {
            // deserialize bone
            geometry::CArmature *armature = new geometry::CArmature;;
            deserializeBone(Reader, armature);

            // build "names" for bones
            int name = 0;
            std::map<int, geometry::CBone *> boneNames;
            buildInverseNames(armature, name, boneNames);

            // deserialize actions using these names
            deserializeActions(Reader, armature->getActions(), boneNames);

            return armature;
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

            if (version > 5)
            {
                vpl::sys::tInt32 bArmature = 0;
                Reader.read(bArmature);

                m_spArmature = (bArmature != 0 ? deserializeArmature(Reader) : NULL);
            }

			//STUFF FOR TRAUMATECH
			if (version > 6)
			{
				CSerializableData< osg::Matrix >::deserialize(&m_transformationMatrixBeforeReposition, Reader);
				CSerializableData< osg::Matrix >::deserialize(&m_transformationMatrixAfterReposition, Reader);
			}

            if (version > 7)
            {
                vpl::sys::tInt32 bMir = 0;
                Reader.read(bMir); m_bMirrored = bMir != 0;
            }

            if (version > 8)
            {
                vpl::sys::tInt32 locked = 0;
                Reader.read(locked); m_locked = locked != 0;
            }

            if (version > 9)
            {
                vpl::sys::tInt32 bEx = 0;
                Reader.read(bEx); m_bExamined = bEx != 0;
            }

            if (version > 10)
            {
                vpl::sys::tInt32 isCreatedAbtment;
                Reader.read(isCreatedAbtment);

                if (isCreatedAbtment != 0)
                    this->setProperty("createdAbutment", "true");
            }

            Reader.endRead(*this);

            m_mesh_dirty = true;
            m_spModel->deserialize(Reader);
        }

		//STUFF FOR TRAUMATECH
		//! Returns transformation matrix before reposition
		const osg::Matrix& getTransformationMatrixBeforeReposition()
		{
			return m_transformationMatrixBeforeReposition;
		}

		//! Sets transformation matrix before reposition
		void setTransformationMatrixBeforeReposition(const osg::Matrix& matrix)
		{
			m_transformationMatrixBeforeReposition = matrix;
		}

		//! Returns transformation matrix after reposition
		const osg::Matrix& getTransformationMatrixAfterReposition()
		{
			return m_transformationMatrixAfterReposition;
		}

		//! Sets transformation matrix after reposition
		void setTransformationMatrixAfterReposition(const osg::Matrix& matrix)
		{
			m_transformationMatrixAfterReposition = matrix;
		}

        bool isMirrored()
        {
            return m_bMirrored;
        }

        void setMirrored(bool mirrored)
        {
            m_bMirrored = mirrored;
        }

        bool isExamined()
        {
            return m_bExamined;
        }

        void setExamined(bool examined)
        {
            m_bExamined = examined;
        }

        bool isLocked() const
        {
            return m_locked;
        }

        void setLocked(bool locked)
        {
            m_locked = locked;
        }

    private:
        //! Update KD tree - this is called on demand when somebody wants to use nearest/radius point serch methods and m_mesh_dirty flag is set to true...
        bool updateKDTree();

        //! Try to find precise closest point on the triangle
        geometry::Vec3 getClosestTrianglePoint(const geometry::Vec3 &t0, const geometry::Vec3 &t1, const geometry::Vec3 &t2, const geometry::Vec3 &point) const;

    protected:
        //! Model storage.
        vpl::base::CScopedPtr<geometry::CMesh> m_spModel;

        //! Armature
        vpl::base::CScopedPtr<geometry::CArmature> m_spArmature;

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

        bool m_reserved = false;

        //! Named model properties
        std::map<std::string, std::string> m_properties;

		//STUFF FOR TRAUMATECH
		//! Model transformation matrix before reposition
		osg::Matrix m_transformationMatrixBeforeReposition;

		//! Model transformation matrix after reposition
		osg::Matrix m_transformationMatrixAfterReposition;

        //! True if bone is mirrored
        bool m_bMirrored;

        //! True if it is examined bone (ORP stuff)
        bool m_bExamined;

        bool m_locked;

        //! Mesh KD tree 
        geometry::CKDTreeOM m_kd_tree;

        //! Should be kd tree updated (is mesh "dirty")?
        bool m_mesh_dirty;
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //! Serialization wrapper. 

    DECLARE_SERIALIZATION_WRAPPER(CModel)

} // namespace data

#endif // CModel_H
