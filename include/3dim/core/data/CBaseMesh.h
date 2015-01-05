////////////////////////////////////////////////////////////
// $Id$
////////////////////////////////////////////////////////////

#ifndef CBASEMESH_H
#define CBASEMESH_H

#define _USE_MATH_DEFINES

#ifndef OM_STATIC_BUILD
#define OM_STATIC_BUILD
#endif

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMeshT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#include <core/data/OMMesh.h>

namespace data
{

	class CBaseMesh : public OMMesh
	{
	public:
		//! Default constructor.
		CBaseMesh();

		//! Copy constructor.
		CBaseMesh(const CBaseMesh &mesh);

		//! Constructor
		CBaseMesh(const data::OMMesh &mesh);

		//! Assignment operator
		CBaseMesh &operator=(const CBaseMesh &mesh);

		//! Destructor.
		~CBaseMesh();

		//! calculates boundary count
		virtual int  boundaryCount();

		//! calculates component count
		virtual int  componentCount();

		//! is model closed
		virtual bool isClosed();

		//! is model 2-manifold
		virtual bool isTwoManifold();

		//! Calculate mesh volume
		virtual double meshVolume();

		//! Invert all normals
		virtual void invertNormals();

		//! Attempt to fix open components
		virtual int fixOpenComponents();
	};
}

#endif // CBASEMESH_H