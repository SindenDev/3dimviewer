/*===========================================================================*\
*                                                                           *
*                               OpenMesh                                    *
*      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen      *
*                           www.openmesh.org                                *
*                                                                           *
*---------------------------------------------------------------------------*
*  This file is part of OpenMesh.                                           *
*                                                                           *
*  OpenMesh is free software: you can redistribute it and/or modify         *
*  it under the terms of the GNU Lesser General Public License as           *
*  published by the Free Software Foundation, either version 3 of           *
*  the License, or (at your option) any later version with the              *
*  following exceptions:                                                    *
*                                                                           *
*  If other files instantiate templates or use macros                       *
*  or inline functions from this file, or you compile this file and         *
*  link it with other files to produce an executable, this file does        *
*  not by itself cause the resulting executable to be covered by the        *
*  GNU Lesser General Public License. This exception does not however       *
*  invalidate any other reasons why the executable file might be            *
*  covered by the GNU Lesser General Public License.                        *
*                                                                           *
*  OpenMesh is distributed in the hope that it will be useful,              *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*  GNU Lesser General Public License for more details.                      *
*                                                                           *
*  You should have received a copy of the GNU LesserGeneral Public          *
*  License along with OpenMesh.  If not,                                    *
*  see <http://www.gnu.org/licenses/>.                                      *
*                                                                           *
\*===========================================================================*/

/*===========================================================================*\
*                                                                           *
*   $Revision: 1053 $                                                         *
*   $Date: 2014-05-09 14:44:18 +0200 (Fr, 09 Mai 2014) $                   *
*                                                                           *
\*===========================================================================*/


//== INCLUDES =================================================================

#if defined (TRIDIM_USE_PHYSFS)

// STL
#include <geometry/base/CTMReader.h>

#include <map>

#include <float.h>
#include <fstream>
#include <iostream>

// OpenMesh
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/IO/importer/BaseImporter.hh>

#include <physfs.h>

#include <VPL/Base/Logging.h>

//=== NAMESPACES ==============================================================


namespace OpenMesh
{
    namespace IO
    {
        //=== INSTANCIATE =============================================================

        // register the STLReader singleton with MeshReader
        _CTMReader_ __CTMReaderInstance;
        _CTMReader_& CTMReader()
        {
            return __CTMReaderInstance;
        }

        //=== IMPLEMENTATION ==========================================================
        _CTMReader_:: _CTMReader_()
            : eps_(FLT_MIN)
        {
            IOManager().register_module(this);

            if (PHYSFS_isInit() == 0)
            {
                PHYSFS_init(NULL);
            }
        }

        //-----------------------------------------------------------------------------
        bool _CTMReader_::read(const std::string& _filename, BaseImporter& _bi, Options& _opt)
        {
            CTMcontext context;
            CTMuint vertCount, triCount;
            const CTMuint *indices;
            const CTMfloat *vertices, *normals = nullptr;
            bool hasNormals;

            // support for reading from zip via PhysFS

            bool origReadFromPhysFS = readFromPhysFS;

            std::string filename = _filename;
            std::string suffix = EXT_SUFFIX_PHYSFS;
            int startIndex = filename.length() - suffix.length();
            if ((startIndex >= 0) && (filename.substr(startIndex) == EXT_SUFFIX_PHYSFS))
            {
                readFromPhysFS = true;
                filename = filename.substr(0, startIndex);
            }

            if (readFromPhysFS)
            {
                bool result = read_ctm_physfs(filename, _bi, _opt);
            
                readFromPhysFS = origReadFromPhysFS;
            
                return result;
            }

            // Create a new importer context
            context = ctmNewContext(CTM_IMPORT);
            // Load the OpenCTM file
            ctmLoad(context, _filename.c_str());

            if (ctmGetError(context) == CTM_NONE)
            {
                // Access the mesh data
                vertCount = ctmGetInteger(context, CTM_VERTEX_COUNT);
                vertices = ctmGetFloatArray(context, CTM_VERTICES);
                triCount = ctmGetInteger(context, CTM_TRIANGLE_COUNT);
                indices = ctmGetIntegerArray(context, CTM_INDICES);

                hasNormals = (ctmGetInteger(context, CTM_HAS_NORMALS) == CTM_TRUE);

                //"The normals section is optional, and only present if the per-vertex normals flag is set in the header"
                if (hasNormals)
                {
                    normals = ctmGetFloatArray(context, CTM_NORMALS);
                }
            }
            else
            {
                // Free the context
                ctmFreeContext(context);

                return false;
            }

            OpenMesh::Vec3f vertex;
            OpenMesh::Vec3f normal;
            BaseImporter::VHandles vhandles;

            OpenMesh::VertexHandle handle;
            OpenMesh::FaceHandle face_handle;

            /*// prints out indices of faces
            {
                QDebug debug = qDebug();

                for (size_t j = 0; j < triCount * 3; j += 1)
                {
                    if (j == 0)
                    {
                        debug << "";
                    }

                    if (j == 0 || j % 3 != 2)
                    {
                        debug << indices[j] + 1 << "/";
                    }
                    else
                    {
                        debug << indices[j] + 1 << "\n";
                    }
                }
            }
            */


            // Load vertices as handles first..
            vhandles.reserve(vertCount * 3);

            for (size_t i = 0; i < vertCount; i = ++i)
            {
                const CTMfloat *value = &vertices[i * 3];
                OpenMesh::Vec3f vertex = OpenMesh::Vec3f(*value, *(value + 1), *(value + 2));

                handle = _bi.add_vertex(vertex);
                if (!handle.is_valid())
                {
                    VPL_LOG_INFO("invalid vertex " << value[0] << " " << value[1] << " " << value[2]);
                }
                vhandles.push_back(handle);
            }

            BaseImporter::VHandles tmpvhandles;

            if (hasNormals && normals != nullptr)
            {
                //Reconstruct faces from vhandles
                for (size_t i = 0; i < triCount; i = ++i)
                {
                    int first = i * 3;

                    tmpvhandles.push_back(vhandles[indices[first + 0]]);
                    tmpvhandles.push_back(vhandles[indices[first + 1]]);
                    tmpvhandles.push_back(vhandles[indices[first + 2]]);

                    face_handle = _bi.add_face(tmpvhandles);
                    if (!face_handle.is_valid())
                    {
                        //VPL_LOG_INFO("trying different vertices order");
                        tmpvhandles.clear();
                        tmpvhandles.push_back(vhandles[indices[first + 0]]);
                        tmpvhandles.push_back(vhandles[indices[first + 2]]);
                        tmpvhandles.push_back(vhandles[indices[first + 1]]);
                        face_handle = _bi.add_face(tmpvhandles);
                    }
                    if (face_handle.is_valid())
                    {
                        //convert per-vertex normals to per face normals...
                        const CTMfloat *n1 = &normals[indices[first + 0] * 3];
                        const CTMfloat *n2 = &normals[indices[first + 1] * 3];
                        const CTMfloat *n3 = &normals[indices[first + 2] * 3];

                        OpenMesh::Vec3f normal1(*n1, *(n1 + 1), *(n1 + 2));
                        OpenMesh::Vec3f normal2(*n2, *(n2 + 1), *(n2 + 2));
                        OpenMesh::Vec3f normal3(*n3, *(n3 + 1), *(n3 + 2));

                        OpenMesh::Vec3f face_normal = (normal1 + normal2 + normal3) / 3.0f;

                        _bi.set_normal(face_handle, face_normal);
                    }
                    else
                    {
                        VPL_LOG_INFO("Invalid face in " << _filename);
                    }

                    tmpvhandles.clear();
                }
            }
            else
            {
                // Reconstruct faces from vhandles
                for (size_t i = 0; i < triCount; i = ++i)
                {
                    int first = i * 3;

                    tmpvhandles.push_back(vhandles[indices[first + 0]]);
                    tmpvhandles.push_back(vhandles[indices[first + 1]]);
                    tmpvhandles.push_back(vhandles[indices[first + 2]]);

                    face_handle = _bi.add_face(tmpvhandles);

                    tmpvhandles.clear();
                }
            }

            // qDebug() << "Loaded" << _bi.n_vertices() << "vertices in" << _bi.n_faces() << "faces";

            // Free the context
            ctmFreeContext(context);
            return true;
        }

        CTMuint streamRead(void *aBuf, CTMuint aCount, void *aUserData)
        {
            char *buffer = static_cast<char *>(aBuf);
            std::ifstream &in = *static_cast<std::ifstream *>(aUserData);

            int bytesRead = 0;
            for (int i = 0; i < aCount; ++i)
            {
                if (in.bad())
                {
                    break;
                }

                char c;
                in >> c;
                buffer[bytesRead++] = c;
            }
            return bytesRead;
        }

        bool _CTMReader_::read(std::istream& _in, BaseImporter& _bi, Options& _opt)
        {
            CTMcontext context;
            CTMuint vertCount, triCount;
            const CTMuint *indices;
            const CTMfloat *vertices, *normals = nullptr;
            bool hasNormals;

            // Create a new importer context
            context = ctmNewContext(CTM_IMPORT);

            // Load the OpenCTM file
            ctmLoadCustom(context, streamRead, static_cast<void *>(&_in));

            CTMenum error = ctmGetError(context);
            std::string errorString(ctmErrorString(error));
            if (error == CTM_NONE)
            {
                // Access the mesh data
                vertCount = ctmGetInteger(context, CTM_VERTEX_COUNT);
                vertices = ctmGetFloatArray(context, CTM_VERTICES);
                triCount = ctmGetInteger(context, CTM_TRIANGLE_COUNT);
                indices = ctmGetIntegerArray(context, CTM_INDICES);

                hasNormals = (ctmGetInteger(context, CTM_HAS_NORMALS) == CTM_TRUE);

                // "The normals section is optional, and only present if the per-vertex normals flag is set in the header"
                if (hasNormals)
                {
                    normals = ctmGetFloatArray(context, CTM_NORMALS);
                }
            }
            else
            {
                // Free the context
                ctmFreeContext(context);
                return false;
            }

            OpenMesh::Vec3f vertex;
            OpenMesh::Vec3f normal;
            BaseImporter::VHandles vhandles;
            OpenMesh::VertexHandle handle;
            OpenMesh::FaceHandle face_handle;

            // Load vertices as handles first..
            vhandles.reserve(vertCount * 3);

            for (size_t i = 0; i < vertCount; i = ++i)
            {
                const CTMfloat *value = &vertices[i * 3];
                OpenMesh::Vec3f vertex = OpenMesh::Vec3f(*value, *(value + 1), *(value + 2));

                handle = _bi.add_vertex(vertex);
                if (!handle.is_valid())
                {
                    VPL_LOG_INFO("invalid vertex " << value[0] << " " << value[1] << " " << value[2]);
                }
                vhandles.push_back(handle);
            }

            BaseImporter::VHandles tmpvhandles;

            if (hasNormals && normals != nullptr)
            {
                //Reconstruct faces from vhandles
                for (size_t i = 0; i < triCount; i = ++i)
                {
                    int first = i * 3;

                    tmpvhandles.push_back(vhandles[indices[first + 0]]);
                    tmpvhandles.push_back(vhandles[indices[first + 1]]);
                    tmpvhandles.push_back(vhandles[indices[first + 2]]);

                    face_handle = _bi.add_face(tmpvhandles);
                    if (!face_handle.is_valid())
                    {
                        //VPL_LOG_INFO("trying different vertices order");
                        tmpvhandles.clear();
                        tmpvhandles.push_back(vhandles[indices[first + 0]]);
                        tmpvhandles.push_back(vhandles[indices[first + 2]]);
                        tmpvhandles.push_back(vhandles[indices[first + 1]]);
                        face_handle = _bi.add_face(tmpvhandles);
                    }
                    if (face_handle.is_valid())
                    {

                        //convert per-vertex normals to per face normals...
                        const CTMfloat *n1 = &normals[indices[first + 0] * 3];
                        const CTMfloat *n2 = &normals[indices[first + 1] * 3];
                        const CTMfloat *n3 = &normals[indices[first + 2] * 3];

                        OpenMesh::Vec3f normal1(*n1, *(n1 + 1), *(n1 + 2));
                        OpenMesh::Vec3f normal2(*n2, *(n2 + 1), *(n2 + 2));
                        OpenMesh::Vec3f normal3(*n3, *(n3 + 1), *(n3 + 2));

                        OpenMesh::Vec3f face_normal = (normal1 + normal2 + normal3) / 3.0f;

                        _bi.set_normal(face_handle, face_normal);
                    }
                    else
                    {
                        VPL_LOG_INFO("Invalid face in ctm");
                    }

                    tmpvhandles.clear();
                }
            }
            else
            {
                // Reconstruct faces from vhandles
                for (size_t i = 0; i < triCount; i = ++i)
                {
                    int first = i * 3;

                    tmpvhandles.push_back(vhandles[indices[first + 0]]);
                    tmpvhandles.push_back(vhandles[indices[first + 1]]);
                    tmpvhandles.push_back(vhandles[indices[first + 2]]);

                    face_handle = _bi.add_face(tmpvhandles);

                    tmpvhandles.clear();
                }
            }

            // Free the context
            ctmFreeContext(context);
            return true;
        }

        bool _CTMReader_::read_ctm_physfs(const std::string & _filename, BaseImporter & _bi, Options& _opt)
        {
            // open file
            omlog() << "[STLReader] : read binary file\n";

            PHYSFS_File *in = PHYSFS_openRead(_filename.c_str());
            if (!in)
            {
                omerr() << "[STLReader] : cannot not open file " << _filename << std::endl;
                return false;
            }

            std::stringstream str(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
            str >> std::noskipws;
            char buffer[1000];

            // read file into stream
            unsigned int bytesRead = 0;
            PHYSFS_seek(in, 0);
            while (!PHYSFS_eof(in))
            {
                bytesRead = PHYSFS_readBytes(in, buffer, 1000);
                str.write(buffer, bytesRead);
            }
            PHYSFS_close(in);

            // read the mesh from stream
            str.seekg(0);
            return read(str, _bi, _opt);
        }

        //-----------------------------------------------------------------------------

#ifndef DOXY_IGNORE_THIS
        class CmpVec
        {
        public:
            CmpVec(float _eps = FLT_MIN) : eps_(_eps)
            { }

            bool operator()(const Vec3f& _v0, const Vec3f& _v1) const
            {
                if (fabs(_v0[0] - _v1[0]) <= eps_)
                {
                    if (fabs(_v0[1] - _v1[1]) <= eps_)
                    {
                        return (_v0[2] < _v1[2] - eps_);
                    }
                    else
                    {
                        return (_v0[1] < _v1[1] - eps_);
                    }
                }
                else
                {
                    return (_v0[0] < _v1[0] - eps_);
                }
            }

        private:
            float eps_;
        };

#endif

        //=============================================================================
    } // namespace IO
} // namespace OpenMesh
//=============================================================================

#endif