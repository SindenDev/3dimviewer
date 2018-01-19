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
 *   $Revision: 990 $                                                         *
 *   $Date: 2014-02-05 10:01:07 +0100 (Mi, 05 Feb 2014) $                   *
 *                                                                           *
\*===========================================================================*/


//== INCLUDES =================================================================
#include <geometry/base/CTMWriter.h>

//STL
#include <fstream>

// OpenMesh
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/IOManager.hh>

//=== NAMESPACES ==============================================================



namespace OpenMesh {
namespace IO {


//=== INSTANCIATE =============================================================


	_CTMWriter_  __CTMWriterInstance;
_CTMWriter_& CTMWriter() { return __CTMWriterInstance; }


//=== IMPLEMENTATION ==========================================================


_CTMWriter_::_CTMWriter_() { IOManager().register_module(this); }


//-----------------------------------------------------------------------------


bool
_CTMWriter_::
write(const std::string& _filename, BaseExporter& _be, Options _opt, std::streamsize _precision) const {


	std::vector<CTMfloat> vertices;
	std::vector<CTMuint> indices;


	Vec3f  a, b, c;					//vertices + normal
	std::vector<VertexHandle> vhandles;
	FaceHandle face_handle;




	//get all vertices first
	VertexHandle v_handle;

	int no_of_vertices = _be.n_vertices();	

	for (size_t i = 0; i < no_of_vertices; i++) {
		v_handle = VertexHandle(i);


		Vec3f v = _be.point(v_handle);
		Vec3f n = _be.normal(v_handle);

		vertices.push_back(*(v.data() + 0));
		vertices.push_back(*(v.data() + 1));
		vertices.push_back(*(v.data() + 2));

	}

	

	int number_of_faces = _be.n_faces();

	// and define structure by indexing
	for (int i = 0; i < number_of_faces; ++i) {

		//QDebug debug = qDebug();

		face_handle = FaceHandle(i);
		int number_of_vertices = _be.get_vhandles(face_handle, vhandles);

		//debug << "face no." << i;
		//debug << "vhandles" << vhandles[0].idx() << vhandles[1].idx() << vhandles[2].idx();

		if (number_of_vertices == 3) {


			indices.push_back(vhandles[0].idx());
			indices.push_back(vhandles[1].idx());
			indices.push_back(vhandles[2].idx());


		} else 
			omerr() << "[CTMWriter] : Warning: Skipped non-triangle data!\n";


	}


	CTMcontext context;
	// Create a new exporter context
	context = ctmNewContext(CTM_EXPORT);

	ctmCompressionMethod(context, CTM_METHOD_MG1);
	ctmCompressionLevel(context, 4);	//0 - 9, default is 1

	// Define our mesh representation to OpenCTM
	ctmDefineMesh(context, vertices.data(), vertices.size() / 3, indices.data(), number_of_faces, NULL);
  
	// Save the OpenCTM file
	ctmSave(context, _filename.c_str());
	// Free the context
	ctmFreeContext(context);

	return true;
}

//-----------------------------------------------------------------------------

/*
size_t
_CTMWriter_::
binary_size(BaseExporter& _be, Options  _opt ) const
{
  size_t bytes(0);
  size_t _12floats(12*sizeof(float));

  bytes += 80; // header
  bytes += 4;  // #faces


  int i, nF(int(_be.n_faces()));
  std::vector<VertexHandle> vhandles;

  for (i=0; i<nF; ++i)
    if (_be.get_vhandles(FaceHandle(i), vhandles) == 3)
      bytes += _12floats + sizeof(short);
    else
      omerr() << "[CTMWriter] : Warning: Skipped non-triangle data!\n";

  return bytes;
}
*/

//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
