///////////////////////////////////////////////////////////////////////////////
// This file comes from DentalViewer software and was modified for 
// 
// BlueSkyPlan version 4.x
// Diagnostic and implant planning software for dentistry.
//
// The original DentalViewer legal notice can be found below.
//
// Copyright 2018 Blue Sky Bio, LLC
// All rights reserved 
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// DentalViewer
// Implant planning software for dentistry.
// 
// Copyright (c) 2008-2012 by 3Dim Laboratory s.r.o.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TriIterator_H
#define TriIterator_H

// VPL
//#include <VPL/Base/Iterator.h>
#include <VPL/Base/IteratorBase.h>
#include <VPL/Image/Image.h>

// STL
#if defined(__APPLE__) && !defined(_LIBCPP_VERSION)
  #include <tr1/array>
#else
  #include <array>
#endif

// OpenMesh
#include <geometry/base/CMesh.h>

#include "FixedNum.h"

namespace reg { class TriIterator; }

namespace vpl
{
    // Declare iterator traits first...
    template <>
    struct CIteratorTraits<reg::TriIterator>
    {
        typedef vpl::img::tFloatPixel tItem;
    };
}


namespace reg
{

//==============================================================================
//! Triangle rasterization iterator.

// Usage:
//   TriIterator it(pImage, pTriangle);
//   for( ; !it.isEnd(); ++it )
//   {
//       *it = 0;
//   }

class TriIterator
    : public vpl::base::CIteratorBase<TriIterator>
{
public:
    // Image type
	typedef vpl::img::CFImage tImage;
    
    // Pixel type
    typedef vpl::img::tFloatPixel tPixel;

    // Internal fixed-point arithmetics type
//    typedef float tFloat;
    typedef vpl::math::FixedNum<vpl::sys::tInt64, 16> tFloat;

public:
    //! Constructor.
    TriIterator(tImage* pImage, const geometry::CMesh* pMesh, geometry::CMesh::FaceHandle face, float scale, bool bUseCCWCorrection = true)
    {
		init(pImage, pMesh, face, scale, bUseCCWCorrection);
    }

	//! Constructor - initialized by vertices array
	template<class tTrianglePointsArray>
    TriIterator(tImage* pImage, const tTrianglePointsArray &vertices, float scale, bool bUseCCWCorrection = true);

    //! Destructor
    ~TriIterator() {}

    //! Returns current iterator position.
    vpl::tSize x() const { return getX(); }
    vpl::tSize y() const { return getY(); }
    float z() const { return getZ(); }

    //! Returns current iterator position.
    vpl::tSize getX() const { return m_Data.x; }
    vpl::tSize getY() const { return m_Data.y; }
    float getZ() const { return m_Data.z; }

    //! Returns true if the iterator points at the end.
    bool atEnd() const { return isEnd(); }

    //! Returns true if iterator points after the last pixel.
    bool isEnd() const { return (m_Data.y > m_Data.maxY); }

    //! Moves iterator to the next triangle pixel.
    void advance()
    {
        do {
            if( isEnd() )
            {
                break;
            }
            else
            {
                next();
            }
        } while( !isInner() );
    }

    //! Check whether image pointer was set
    bool hasImage() const { return nullptr != m_pImage; }

    //! Returns the current pixel value.
    const tPixel& value() const
    {
        assert(nullptr != m_pImage);
        return m_pImage->at(getX(), getY());
    }

    //! Returns reference to the current pixel.
    tPixel& valueRef() const
    {
        assert(nullptr != m_pImage);
        return m_pImage->at(getX(), getY());
    }

    //! Returns pointer to the current pixel.
    tPixel *valuePtr() const
    {
        assert(nullptr != m_pImage);
        return m_pImage->getPtr(getX(), getY());
    }

protected:
    //! All members.
    struct SData
    {
        //! Bounding rectangle.
        vpl::tSize minX, maxX, minY, maxY;

        //! Vertex coordinates.
        tFloat x0, x1, x2;
        tFloat y0, y1, y2;
        tFloat z0, z1, z2;

        //! Flags saying that an edge is top-left
        bool e0, e1, e2;

        //! Triangle area
        tFloat area;

        //! Current iterator position.
        vpl::tSize x, y;

        //! Edge functions
        tFloat w0, w1, w2;
       
        //! Interpolated z-value
        float z;
    };

    //! Image pointer
    tImage *m_pImage;
    
    //! Data members.
    SData m_Data;

protected:
	// Triangle vertices in one array
#if defined(__APPLE__) && !defined(_LIBCPP_VERSION)
    typedef std::tr1::array<geometry::CMesh::Point, 3> tVertices;
#else
	typedef std::array<geometry::CMesh::Point, 3> tVertices;
#endif

	//! Returns true if the current pixel is inside the triangle.
    bool isInner();

    //! Edge function is the cross product of vectors 12 and 13
    tFloat edgeFunction(const tFloat& x1, const tFloat& y1, const tFloat& x2, const tFloat& y2, const tFloat& x3, const tFloat& y3);

    //! Initializes the iterator.
    void init(tImage *pImage, const geometry::CMesh* mesh, geometry::CMesh::FaceHandle face, float scale, bool bUseCCWCorrection);
    
	//! Initialize triangle iterator by three vertices
    void initByVertices(tVertices &vertices, bool bUseCCWCorrection);


    //! Moves iterator to the next pixel.
    void next();

private:
    //! Private assignment operator.
    TriIterator& operator=(const TriIterator& It);
};

/**
 * \fn	template<class tVertices> reg::CTriIterator::CTriIterator(tImage* pImage, const tVertices &vertices, float scale)
 *
 * \brief Constructor.
 *
 * \tparam	tVertices Type of the vertices.
 * \param [in,out]	pImage If non-null, the image.
 * \param	vertices	   The vertices.
 * \param	scale		   The scale.
 */
template<class tTrianglePointsArray>
reg::TriIterator::TriIterator(tImage* pImage, const tTrianglePointsArray &_vertices, float scale, bool bUseCCWCorrection)
{
	m_pImage = pImage;

	// Get scaled vertices of the triangle
	tVertices vertices;
	for (int i = 0; i < 3; ++i)
	{
		vertices[i][0] = _vertices[i][0] * scale;
		vertices[i][1] = _vertices[i][1] * scale;
		vertices[i][2] = _vertices[i][2] * scale;
	}

	// Reorder vertices
	initByVertices(vertices, bUseCCWCorrection);
}


} // namespace data

#endif // TriIterator_H

