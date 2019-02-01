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

#include <alg/TriIterator.h>

//#define TEST_TRIITERATOR
#ifdef TEST_TRIITERATOR
#include <QDebug>
#include <VPL/Image/Image.h> 
#include <VPL/ImageIO/PNG.h>
#pragma comment(lib, "x:\\msvc14\\x64\\Libpng 1.6.34\\lib\\libpng.lib" )

void testTriIterator(std::array<geometry::CMesh::Point, 3> &triangle);
#endif

namespace reg
{
/**
 * Checks orientation of a triangle.
 *
**/
double _ccw(geometry::CMesh::Point a, geometry::CMesh::Point b, geometry::CMesh::Point c) 
{
    double val = (b[1] - a[1]) * (c[0] - b[0]) - (b[0] - a[0]) * (c[1] - b[1]);

    return (val);
}

} // namespace reg


/**
 * Query if point is inner.
 *
 * \return	true if inner, false if not.
**/
bool reg::TriIterator::isInner()
{
//    return (m_Data.w0 >= tFloat(0.0f) && m_Data.w1 >= tFloat(0.0f) && m_Data.w2 >= tFloat(0.0f));

    bool overlaps = true;

    // If the point is on the edge, test if it is a top or left edge, 
    // otherwise test if  the edge function is positive
    overlaps &= (m_Data.w0 == tFloat(0.0f) ? m_Data.e0 : (m_Data.w0 > tFloat(0.0f)));
    overlaps &= (m_Data.w1 == tFloat(0.0f) ? m_Data.e1 : (m_Data.w1 > tFloat(0.0f)));
    overlaps &= (m_Data.w2 == tFloat(0.0f) ? m_Data.e2 : (m_Data.w2 > tFloat(0.0f)));

    return overlaps;
}


reg::TriIterator::tFloat reg::TriIterator::edgeFunction(const tFloat& x0, const tFloat& y0, const tFloat& x1, const tFloat& y1, const tFloat& x2, const tFloat& y2)
{
    return (x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0);
}


/**
 * Initialises this object.
 *
 * \param [in,out]	pImage	If non-null, the image.
 * \param [in,out]	mesh  	If non-null, the mesh.
 * \param	face		  	The face.
 * \param	scale		  	The scale.
**/
void reg::TriIterator::init(tImage *pImage, const geometry::CMesh* mesh, geometry::CMesh::FaceHandle face, float scale, bool bUseCCWCorrection)
{
	m_pImage = pImage;

	// Get vertices of the triangle
	tVertices vertices;
	int i = 0;
	for( geometry::CMesh::ConstFaceVertexIter fvit = mesh->cfv_begin(face); fvit != mesh->cfv_end(face); ++fvit, ++i )
	{
		assert( i < 3 );
		vertices[i] = mesh->point( fvit.handle() );
		for( int j = 0; j < 3; j++ )
        {
			vertices[i][j] *= scale;
        }
	}

    // Call by-vertex-array initialization
	initByVertices(vertices, bUseCCWCorrection);
}

/**
 * Go to next point.
**/
void reg::TriIterator::next()
{
    if( ++m_Data.x > m_Data.maxX )
	{
	    m_Data.x = m_Data.minX;
		++m_Data.y;
	}

    tFloat px = static_cast<float>(m_Data.x) + 0.5f;
    tFloat py = static_cast<float>(m_Data.y) + 0.5f;

    m_Data.w0 = edgeFunction(m_Data.x1, m_Data.y1, m_Data.x2, m_Data.y2, px, py);
    m_Data.w1 = edgeFunction(m_Data.x2, m_Data.y2, m_Data.x0, m_Data.y0, px, py);
    m_Data.w2 = edgeFunction(m_Data.x0, m_Data.y0, m_Data.x1, m_Data.y1, px, py);

    // Z-interpolation using barycentric coords
    tFloat z = m_Data.z1;
    if (m_Data.area > tFloat(0.0f))
    {
        tFloat a = m_Data.w0 / m_Data.area;
        tFloat b = m_Data.w1 / m_Data.area;
        tFloat g = m_Data.w2 / m_Data.area;
        z = a * m_Data.z0 + b * m_Data.z1 + g * m_Data.z2;
    }

    m_Data.z = z.toFloat();
}

void reg::TriIterator::initByVertices(tVertices &vertices, bool bUseCCWCorrection)
{    
#ifdef TEST_TRIITERATOR
    if (vertices[0][0] != -2000)
    {
        int xoff = 0;
        int yoff = 0;
        vertices[0][0] = -200 + xoff;
        vertices[0][1] = -10 + yoff;
        vertices[0][2] = 50;

        vertices[1][0] = 100 + xoff;
        vertices[1][1] = -7 + yoff;
        vertices[1][2] = 150;

        vertices[2][0] = 100 + xoff;
        vertices[2][1] = 10 + yoff;
        vertices[2][2] = 100;
    }

    testTriIterator(vertices);
#endif
    // #TODO When CCW correction in use, rasterization sometimes goes wrong. Don't know why...
    if (bUseCCWCorrection && _ccw(vertices[0], vertices[1], vertices[2]) < 0.0)
    {
        std::swap(vertices[1], vertices[2]);
    }

    // Decimal vertex coordinates
    m_Data.x0 = tFloat(vertices[0][0]);
    m_Data.x1 = tFloat(vertices[1][0]);
    m_Data.x2 = tFloat(vertices[2][0]);
    m_Data.y0 = tFloat(vertices[0][1]);
    m_Data.y1 = tFloat(vertices[1][1]);
    m_Data.y2 = tFloat(vertices[2][1]);

    // Triangle bounding box
    m_Data.minX = static_cast<vpl::tSize>(vpl::math::getMin(vertices[0][0], vertices[1][0], vertices[2][0]));
    m_Data.maxX = static_cast<vpl::tSize>(vpl::math::getMax(vertices[0][0], vertices[1][0], vertices[2][0]) + 0.5f);
    m_Data.minY = static_cast<vpl::tSize>(vpl::math::getMin(vertices[0][1], vertices[1][1], vertices[2][1]));
    m_Data.maxY = static_cast<vpl::tSize>(vpl::math::getMax(vertices[0][1], vertices[1][1], vertices[2][1]) + 0.5f);

    if (hasImage())
    {
        m_Data.minX = vpl::math::getMax(m_Data.minX, (vpl::tSize)0);
        m_Data.maxX = vpl::math::getMin(m_Data.maxX, m_pImage->getXSize());
        m_Data.minY = vpl::math::getMax(m_Data.minY, (vpl::tSize)0);
        m_Data.maxY = vpl::math::getMin(m_Data.maxY, m_pImage->getYSize());
    }

    // Find top-left edges
    m_Data.e0 = ((m_Data.y2 == m_Data.y1 && m_Data.x2 > m_Data.x1) || m_Data.y2 > m_Data.y1);
    m_Data.e1 = ((m_Data.y0 == m_Data.y2 && m_Data.x0 > m_Data.x2) || m_Data.y0 > m_Data.y2);
    m_Data.e2 = ((m_Data.y1 == m_Data.y0 && m_Data.x1 > m_Data.x0) || m_Data.y1 > m_Data.y0);

    // Triangle area
    m_Data.area = edgeFunction(m_Data.x0, m_Data.y0, m_Data.x1, m_Data.y1, m_Data.x2, m_Data.y2);

    // Top-left corner
    m_Data.x = m_Data.minX;
    m_Data.y = m_Data.minY;

    // Save the Z-coords
    m_Data.z0 = tFloat(vertices[0][2]);
    m_Data.z1 = tFloat(vertices[1][2]);
    m_Data.z2 = tFloat(vertices[2][2]);

    // Initialization
    tFloat px = static_cast<float>(m_Data.x) + 0.5f;
    tFloat py = static_cast<float>(m_Data.y) + 0.5f;

    m_Data.w0 = edgeFunction(m_Data.x1, m_Data.y1, m_Data.x2, m_Data.y2, px, py);
    m_Data.w1 = edgeFunction(m_Data.x2, m_Data.y2, m_Data.x0, m_Data.y0, px, py);
    m_Data.w2 = edgeFunction(m_Data.x0, m_Data.y0, m_Data.x1, m_Data.y1, px, py);

    // Z-interpolation using barycentric coords
    tFloat z = m_Data.z1;
    if (m_Data.area > tFloat(0.0f))
    {
        tFloat a = m_Data.w0 / m_Data.area;
        tFloat b = m_Data.w1 / m_Data.area;
        tFloat g = m_Data.w2 / m_Data.area;
        z = a * m_Data.z0 + b * m_Data.z1 + g * m_Data.z2;
    }

    m_Data.z = z.toFloat();

	// Find the first pixel
	if (!isInner())
	{
		advance();
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Floating point implementation of the triiterator to be able to compare the output of the fixed point implementation

namespace reg
{

    class TriIteratorX
        : public vpl::base::CIteratorBase<TriIteratorX>
    {
    public:
        // Image type
        typedef vpl::img::CFImage tImage;

        // Pixel type
        typedef vpl::img::tFloatPixel tPixel;

        // Internal fixed-point arithmetics type
        typedef vpl::sys::tInt64 tFixed;

    public:
        //! Constructor.
        TriIteratorX(tImage* pImage, const geometry::CMesh* pMesh, geometry::CMesh::FaceHandle face, float scale, bool bUseCCWCorrection = true)
        {
            init(pImage, pMesh, face, scale, bUseCCWCorrection);
        }

        //! Constructor - initialized by vertices array
        template<class tTrianglePointsArray>
        TriIteratorX(tImage* pImage, const tTrianglePointsArray &vertices, float scale, bool bUseCCWCorrection = true);

        //! Destructor
        ~TriIteratorX() {}

        //! Returns current iterator position.
        int getX() const { return int(m_data.x); }
        int getY() const { return int(m_data.y); }
        float getZ() const { return float(m_data.z); }

        //! Returns true if iterator points after the last pixel.
        bool isEnd() const { return (m_data.y > m_data.iMaxY); }

        //! Moves iterator to the next triangle pixel.
        void advance()
        {
            do {
                if (isEnd())
                {
                    break;
                }
                else
                {
                    next();
                }
            } while (!isInner());
        }

        //! Returns the current pixel value.
        const tPixel& value() const
        {
            return m_pImage->at(getX(), getY());
        }

        //! Returns reference to the current pixel.
        tPixel& valueRef() const
        {
            return m_pImage->at(getX(), getY());
        }

        //! Returns pointer to the current pixel.
        tPixel *valuePtr() const
        {
            return m_pImage->getPtr(getX(), getY());
        }

        int getMinX() const { return m_data.iMinX;  }
        int getMinY() const { return m_data.iMinY; }
        int getWidth() const { return m_data.iMaxX - m_data.iMinX + 1; }
        int getHeight() const { return m_data.iMaxY - m_data.iMinY + 1; }

    protected:
        //! All members.
        struct SData
        {
            //! Current iterator position.
            int x, y;

            //! Interpolated Z
            float z;

            //! w params for current point
            float w1, w2, w3;

            //! Bounding rectangle.
            float minX, maxX, minY, maxY;
            int iMinX, iMaxX, iMinY, iMaxY;

            //! delta values
            float dY23;
            float dX32;
            float dY31;
            float dX13;

            // cached coordinates and values
            float x3, y3;
            float z1, z2, z3;

            //! 1.0/denom of the barycentric formula
            float invdenom;
        };

        //! Image pointer
        tImage *m_pImage;

        //! Data members.
        SData m_data;

    protected:
        // Triangle vertices in one array
#if defined(__APPLE__) && !defined(_LIBCPP_VERSION)
        typedef std::tr1::array<geometry::CMesh::Point, 3> tVertices;
#else
        typedef std::array<geometry::CMesh::Point, 3> tVertices;
#endif

        //! Returns true if the current pixel is inside the triangle.
        inline bool isInner()
        {
            return (m_data.w1 >= 0.0f && m_data.w2 >= 0.0f && m_data.w3 >= 0.0f);
        }

        //! Initializes the iterator.
        void init(tImage *pImage, const geometry::CMesh* mesh, geometry::CMesh::FaceHandle face, float scale, bool bUseCCWCorrection);

        //! Initialize triangle iterator by three vertices
        void initByVertices(tVertices &vertices, bool bUseCCWCorrection);


        //! Moves iterator to the next pixel.
        void next();

    private:
        //! Private assignment operator.
        TriIteratorX & operator=(const TriIteratorX& It);
    };

}

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
reg::TriIteratorX::TriIteratorX(tImage* pImage, const tTrianglePointsArray &_vertices, float scale, bool bUseCCWCorrection)
{
    VPL_ASSERT(pImage);

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

/**
* Initialises this object.
*
* \param [in,out]	pImage	If non-null, the image.
* \param [in,out]	mesh  	If non-null, the mesh.
* \param	face		  	The face.
* \param	scale		  	The scale.
**/
void reg::TriIteratorX::init(tImage *pImage, const geometry::CMesh* mesh, geometry::CMesh::FaceHandle face, float scale, bool bUseCCWCorrection)
{
    VPL_ASSERT(pImage);

    m_pImage = pImage;

    // Get vertices of the triangle
    tVertices vertices;
    int i = 0;
    for (geometry::CMesh::ConstFaceVertexIter fvit = mesh->cfv_begin(face); fvit != mesh->cfv_end(face); ++fvit, ++i)
    {
        assert(i < 3);
        vertices[i] = mesh->point(fvit.handle());
        for (int j = 0; j < 3; j++)
        {
            vertices[i][j] *= scale;
        }
    }

    // Call by-vertex-array initialization
    initByVertices(vertices, bUseCCWCorrection);
}

/**
* Go to next point.
**/
void reg::TriIteratorX::next()
{
    m_data.x++;
    if (m_data.x > m_data.iMaxX)
    {
        m_data.x = m_data.iMinX;
        ++m_data.y;
    }
    const float dXP3 = m_data.x - m_data.x3;
    const float dYP3 = m_data.y - m_data.y3;

    m_data.w1 = m_data.dY23 * dXP3 + m_data.dX32 * dYP3;
    m_data.w1 = m_data.w1 * m_data.invdenom;

    m_data.w2 = m_data.dY31 * dXP3 + m_data.dX13 * dYP3;
    m_data.w2 = m_data.w2 * m_data.invdenom;

    m_data.w3 = 1 - m_data.w1 - m_data.w2;
    m_data.z = m_data.w1 * m_data.z1 + m_data.w2 * m_data.z2 + m_data.w3 * m_data.z3;
}

void reg::TriIteratorX::initByVertices(tVertices &vertices, bool bUseCCWCorrection)
{
    if (bUseCCWCorrection && _ccw(vertices[0], vertices[1], vertices[2]) > 0.0)
    {
        std::swap(vertices[1], vertices[2]);
    }

    float x1, x2, x3, y1, y2, y3;
    x1 = vertices[0][0];
    x2 = vertices[1][0];
    x3 = vertices[2][0];
    y1 = vertices[0][1];
    y2 = vertices[1][1];
    y3 = vertices[2][1];
    m_data.z1 = vertices[0][2];
    m_data.z2 = vertices[1][2];
    m_data.z3 = vertices[2][2];

    m_data.minX = std::min(x1, std::min(x2, x3));
    m_data.maxX = std::max(x1, std::max(x2, x3));
    m_data.minY = std::min(y1, std::min(y2, y3));
    m_data.maxY = std::max(y1, std::max(y2, y3));
    m_data.iMinX = round(m_data.minX);
    m_data.iMinY = round(m_data.minY);
    m_data.iMaxX = round(m_data.maxX);
    m_data.iMaxY = round(m_data.maxY);

    // helper values for barycentric coordinates computation, taken from https://codeplea.com/triangular-interpolation
    m_data.dY23 = y2 - y3; 
    m_data.dX32 = x3 - x2;
    m_data.dY31 = y3 - y1;
    m_data.dX13 = x1 - x3;
    m_data.invdenom = 1.0 / (m_data.dY23 * m_data.dX13 - m_data.dY31 * m_data.dX32);
    m_data.x3 = x3;
    m_data.y3 = y3;

    // set initial point
    // conversion to int position, therefore round
    m_data.x = round(m_data.minX);
    m_data.y = round(m_data.minY);

    const float dXP3 = m_data.x - x3;
    const float dYP3 = m_data.y - y3;

    m_data.w1 = m_data.dY23 * dXP3 + m_data.dX32 * dYP3;
    m_data.w1 = m_data.w1 * m_data.invdenom;

    m_data.w2 = m_data.dY31 * dXP3 + m_data.dX13 * dYP3;
    m_data.w2 = m_data.w2 * m_data.invdenom;    

    m_data.w3 = 1 - m_data.w1 - m_data.w2;
    assert(m_data.w3 >= 0 && m_data.w3 <= 1);

    // Find the first pixel
    if (!isInner())
    {
        advance();
    }
    else
    {
        assert(m_data.w1 >= 0 && m_data.w1 <= 1);
        assert(m_data.w2 >= 0 && m_data.w2 <= 1);
    }
}

void _testTriIterator(std::array<geometry::CMesh::Point, 3> &triangle)
{
#ifdef _OPENMP
    if (0 != omp_get_thread_num())
        return;
#endif
    static bool inside = false;
    static int counter = 0;
    if (inside)
        return;
    inside = true;
    #define RESOLUTION 10.0		// Rasterization resolution
    reg::TriIterator it(nullptr, triangle, RESOLUTION);    
    reg::TriIteratorX itx(nullptr, triangle, RESOLUTION);
    const int width = itx.getWidth();
    const int height = itx.getHeight();
    const int minx = itx.getMinX();
    const int miny = itx.getMinY();    
#ifdef TEST_TRIITERATOR
    //qDebug() << width << " " << height << " " << minx << " " << miny;
    vpl::img::CImage8 aux(width, height, 0);
    vpl::img::CImage8 aux2(width, height, 0);
    aux.fillEntire(0);
    aux2.fillEntire(0);
    bool bSaveImage = false;
#endif    
    // Triangle iterator cycle
    int px = 0;
    for (; !it.isEnd(); ++it)
    {
        int x = it.getX();
        int y = it.getY();
        float z = it.getZ();
#ifdef TEST_TRIITERATOR
        //aux.at(x - minx, y - miny) = 255;
        aux.at(x - minx, y - miny) = z / RESOLUTION;
#endif
        px++;
    }
    int pxx = 0;
    for (;!itx.isEnd(); ++itx)
    {
        int x2 = itx.getX();
        int y2 = itx.getY();
        float z2 = itx.getZ();

#ifdef TEST_TRIITERATOR
        //aux2.at(x2 - minx, y2 - miny) = 255;
        aux2.at(x2 - minx, y2 - miny) = z2 / RESOLUTION;
#endif
        pxx++;
    }    

#ifdef TEST_TRIITERATOR
    bSaveImage = px != pxx;
    if (bSaveImage)
    {
        counter++;
        vpl::mod::CFileChannel image_channel(vpl::mod::CH_OUT, "s:\\dump\\" + std::to_string(counter) + "aux1.png");
        vpl::mod::CFileChannel image_channel2(vpl::mod::CH_OUT, "s:\\dump\\" + std::to_string(counter) + "aux2.png");
        vpl::img::savePNG( aux, image_channel);
        vpl::img::savePNG(aux2, image_channel2);
    }
#endif
    /*
    {
        int clk = clock();
        reg::CTriIterator itx(nullptr, triangle, RESOLUTION);

        const int width = itx.getWidth();
        const int height = itx.getHeight();
        const int minx = itx.getMinX();
        const int miny = itx.getMinY();

        float sumz = 0;
        for (; !itx.isEnd(); ++itx)
        {
            int x2 = itx.getX();
            int y2 = itx.getY();
            float z2 = itx.getZ();
            sumz += z2;
        }
        int clk2 = clock() - clk;
        clk = clock();

        reg::TriIterator it(nullptr, triangle, RESOLUTION);

        sumz = 0;
        for (; !it.isEnd(); ++it)
        {
            int x = it.getX();
            int y = it.getY();
            float z = it.getZ();
            sumz += z;
        }
        clk = clock() - clk;
        qDebug() << clk2 << " " << clk;
    }
    */
    inside = false;
}
