#ifndef POLYGONOPERATIONS_H
#define POLYGONOPERATIONS_H

#include "clipperExtension.h"
#include "geometry/base/types.h"

namespace geometry
{
    class CPolyline : public std::vector<geometry::Vec2>
    {
    public:
        //! Constructor
        CPolyline();

        //! Destructor
        ~CPolyline();

        //! Return true if polyline forms a ring
        bool isRing() const;

        //! Returns signed area of polygon/hole formed by this polyline
        double area() const;

        //! Returns true if polyline is a polygon, false if it is a hole
        bool orientation() const;

        //! Returns length of the polyline
        double length() const;

        //! Make ring from polyline if not yet
        void setRing();
    };

    #define CLIPPER_MULT 1000.0

    enum EJoinType
    {
        SQUARE = ClipperLib::jtSquare,
        ROUND = ClipperLib::jtRound,
        MITER = ClipperLib::jtMiter,
    };


    //! Basic class representing geometry
    class CGeometry
    {
    protected:
        ClipperLib::Paths m_geometry;

    public:
        //! Constructor (creates EMPTY geometry)
        CGeometry();

        //! Constructor (creates polygon/hole if polyline forms a ring)
        CGeometry(const CPolyline &polyline);

        //! Constructor (creates multigeometry - polygons/holes from polylines that form a rings, other polylines are just polylines)
        CGeometry(const std::vector<CPolyline> &polylines);

        //! Copy constructor (deep copy of other geometry)
        CGeometry(const CGeometry &other);

        //! Destructor
        virtual ~CGeometry();

        //! Assignment (deep copy of other geometry)
        CGeometry &operator=(const CGeometry &other);

        //! Calculates area of geometry
        double area() const;

        //! Calculates perimeter of geometry
        double perimeter() const;

        //! Offsetting
        void offset(double distance, EJoinType join = SQUARE, double limit = 0);

        //! Union of geometries (this = this | other)
        void merge(const CGeometry &other);

        //! Intersection of geometries (this = this & other)
        void intersection(const CGeometry &other);

        //! Difference of geometries (this = this - other)
        void difference(const CGeometry &other);

        //! Just appends to list other geometry polygons
        void append(const CGeometry &other);

        //! Checks if geometry is empty
        bool isEmpty();

        //! Checks if geometry is empty
        bool isEmptyConst() const;

        //! Returns polygon count
        int polygonCount();

        //! Exports geometry to a vector of polylines
        void getAsPolylines(std::vector<CPolyline> &polylines) const;

        //! Cleans up polygons a bit
        virtual void cleanPolygons(double distance);

        void simplifyGeometry(double dotThreshold);

        //! Computes convex hull of geometry
        void convexHull();

    protected:
        //! Returns internal geometry representation (use only if you know what you are doing)
        ClipperLib::Paths *getInternalGeometry();
        const ClipperLib::Paths *getConstInternalGeometry() const;

        void closeRingGeometry();
        static double magnitude(const ClipperLib::IntPoint &pt1, const ClipperLib::IntPoint &pt2);
        static bool distancePointLine(const ClipperLib::IntPoint &pt, const ClipperLib::IntPoint &pt0, const ClipperLib::IntPoint &pt1, double &distance);

        static double area(const ClipperLib::Paths& polys);
        static double perimeter(const ClipperLib::Paths& polys);

    public:
        friend class CLineGeometry;
    };

    class CLineGeometry : public CGeometry
    {
    public:
        //! Constructor (creates EMPTY geometry)
        CLineGeometry();

        //! Constructor
        CLineGeometry(const CPolyline &polyline);

        //! Constructor
        CLineGeometry(const std::vector<CPolyline> &polylines);

        //! Copy constructor (deep copy of other geometry)
        CLineGeometry(const CLineGeometry &other);

        //! Destructor
        ~CLineGeometry();

        //! Assignment (deep copy of other geometry)
        CLineGeometry &operator=(const CLineGeometry &other);

        //! Offsetting
        void offset(double distance, CGeometry &result) const;

        //! Union of geometries (this = this | other)
        void merge(const CLineGeometry &other);

        //! Intersection of geometries (this = this & other)
        void intersection(const CGeometry &other);

        //! Checks if geometry is empty
        bool isEmpty();

        //! Calculates length of lines
        double length();
    };
}

#endif // POLYGONOPERATIONS_H
