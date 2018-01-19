#ifndef CLIPPEREXTENSION_H
#define CLIPPEREXTENSION_H

#include <geometry/base/clipper.hpp>
#include <algorithm>

namespace ClipperLib
{
#ifdef use_int32
    typedef unsigned int cUInt;
#else
    typedef unsigned long long cUInt;
#endif

    #define TOLERANCE (1.0e-20)
    #define NEAR_ZERO(val) (((val) > -TOLERANCE) && ((val) < TOLERANCE))
    void OffsetPolyLines(const Paths &in_lines, Paths &out_lines, double delta, JoinType jointype = jtSquare, EndType endtype = etOpenSquare, double limit = 0, bool autoFix = true);
    void OffsetPolygons(const Paths &in_polys, Paths &out_polys, double delta, JoinType jointype, double limit = 0);

    //ClipperConvert: converts IntPoint to and from DoublePoint based on "scaling_factor"
    class ClipperConvert
    {
    private:
        const double scale;
    public:
        IntPoint operator()(const DoublePoint& v);
        void ToIntPoints(const std::vector<DoublePoint>& dps, std::vector<IntPoint>& ips);
        DoublePoint operator()(const IntPoint& v);
        void ToDoublePoints(const std::vector<IntPoint>& ips, std::vector<DoublePoint>& dps);
        ClipperConvert(const double scaling_factor);
    };

    //! checks if polygon is really a polygon
    static bool IsPolygon(Path p)
    {
        return Orientation(p);
    }

    //! checks if polygon is a hole
    static bool IsHole(Path p)
    {
        return !Orientation(p);
    }

    //! checks if polygon contains another polygon
    static bool Contains(Path parent, Path child)
    {
        Paths solution;
        Clipper clipper;
        clipper.AddPath(child, ptSubject, true);
        clipper.AddPath(parent, ptClip, true);
        clipper.Execute(ctDifference, solution);

        return solution.size() == 0;
    }

    //! calculates square root from an integer
    static cInt isqrt(cUInt n)
    {
        cUInt root, remainder, place;

        root = 0;
        remainder = n;
        place = 0x4000000000000000;

        while (place > remainder)
            place = place >> 2;

        while (place)
        {
            if (remainder >= root + place)
            {
                remainder = remainder - root - place;
                root = root + (place << 1);
            }
            root = root >> 1;
            place = place >> 2;
        }
        return root;
    }

    //! calculates square distance between two points
    static cUInt DistanceSq(const ClipperLib::IntPoint &a, const ClipperLib::IntPoint &b)
    {
        cInt x = b.X - a.X;
        cInt y = b.Y - a.Y;

        return x * x + y * y;
    }

    //! calculates distance between two points
    static cUInt Distance(const ClipperLib::IntPoint &a, const ClipperLib::IntPoint &b)
    {
        return isqrt(DistanceSq(a, b));
    }

    //! calculates square distance between line segment and a point
    static cUInt DistanceSq(const ClipperLib::IntPoint &v, const ClipperLib::IntPoint &w, const ClipperLib::IntPoint &p)
    {
        if (v == w)
        {
            return DistanceSq(v, p);
        }

        // consider the line extending the segment, parameterized as v + t (w - v).
        // we find projection of point p onto the line. 
        // it falls where t = [(p-v) . (w-v)] / |w-v|^2
        IntPoint pmv = IntPoint(p.X - v.X, p.Y - v.Y);
        IntPoint wmv = IntPoint(w.X - v.X, w.Y - v.Y);
        cInt dotPmvWmv = pmv.X * wmv.X + pmv.Y * wmv.Y;
        cUInt lengthSq = DistanceSq(v, w); // lengthSq is always > 0, therefore only dotPmvWmv affects sign of t

        if (dotPmvWmv > static_cast<cInt>(lengthSq)) // t > 1 (beyond the 'w' end of the segment)
        {
            return DistanceSq(p, w);
        }
        else if (dotPmvWmv < 0) // t < 0 (beyond the 'v' end of the segment)
        {
            return DistanceSq(p, v);
        }
        else // projection falls on the segment
        {
            ClipperConvert conv(1000.0);
            IntPoint intPoint(dotPmvWmv, lengthSq);
            DoublePoint doublePoint = conv(intPoint);
            double t = doublePoint.X / doublePoint.Y;

            IntPoint projection;
            projection.X = v.X + (w.X - v.X) * t;
            projection.Y = v.Y + (w.Y - v.Y) * t;

            return DistanceSq(p, projection);
        }
    }

    //! calculates distance between line segment and a point
    static cUInt Distance(const ClipperLib::IntPoint &v, const ClipperLib::IntPoint &w, const ClipperLib::IntPoint &p)
    {
        return isqrt(DistanceSq(v, w, p));
    }

    //! calculates square distance between two line segments
    static cUInt DistanceSq(const ClipperLib::IntPoint &a0, const ClipperLib::IntPoint &b0, const ClipperLib::IntPoint &a1, const ClipperLib::IntPoint &b1)
    {
        // check for zero-length segments
        if (a0 == b0)
        {
            return DistanceSq(a1, b1, a0);
        }

        if (a1 == b1)
        {
            return DistanceSq(a0, b0, a1);
        }

        cInt r_top = (a0.X * b0.Y - a0.Y * b0.X) * (a1.X - b1.X) - (a0.X - b0.X) * (a1.X * b1.Y - a1.Y * b1.X);
        cInt r_bot = (a0.X - b0.X) * (a1.Y - b1.Y) - (a0.Y - b0.Y) * (a1.X - b1.X);
        cInt s_top = (a0.X * b0.Y - a0.Y * b0.X) * (a1.Y - b1.Y) - (a0.Y - b0.Y) * (a1.X * b1.Y - a1.Y * b1.X);
        cInt s_bot = (a0.X - b0.X) * (a1.Y - b1.Y) - (a0.Y - b0.Y) * (a1.X - b1.X);

        if ((r_bot == 0) || (s_bot == 0))
        {
            return std::min(DistanceSq(a1, b1, a0), std::min(DistanceSq(a1, b1, b0), std::min(DistanceSq(a0, b0, a1), DistanceSq(a0, b0, b1))));
        }

        cInt s = s_top / s_bot;
        cInt r = r_top / r_bot;

        if ((r < 0) || (r > 1) || (s < 0) || (s > 1))
        {
            return std::min(DistanceSq(a1, b1, a0), std::min(DistanceSq(a1, b1, b0), std::min(DistanceSq(a0, b0, a1), DistanceSq(a0, b0, b1))));
        }

        return 0; // intersection exists
    }

    //! calculates distance between two line segments
    static cUInt Distance(const ClipperLib::IntPoint &a0, const ClipperLib::IntPoint &b0, const ClipperLib::IntPoint &a1, const ClipperLib::IntPoint &b1)
    {
        return isqrt(DistanceSq(a0, b0, a1, b1));
    }

    //! calculates square distance between two polygons
    static cUInt DistanceSq(const ClipperLib::Path &a, const ClipperLib::Path &b)
    {
        cUInt minDistanceSq = 0;
        bool first = true;

        for (int i0 = 0; i0 < (int)a.size() - 1; ++i0)
        {
            for (int i1 = 0; i1 < (int)b.size() - 1; ++i1)
            {
                cUInt currDistanceSq = DistanceSq(a[i0], a[i0 + 1], b[i1], b[i1 + 1]);

                if ((currDistanceSq < minDistanceSq) || first)
                {
                    first = false;
                    minDistanceSq = currDistanceSq;
                }
            }
        }

        return minDistanceSq;
    }

    //! calculates distance between two polygons
    static cUInt Distance(const ClipperLib::Path &a, const ClipperLib::Path &b)
    {
        return isqrt(DistanceSq(a, b));
    }

    //! calculates perimeter of a polygon
    static cUInt Perimeter(const ClipperLib::Path &polygon, bool treatAsLine = false)
    {
        cUInt sum = 0.0;
        for (int v = 0; v < (int)polygon.size() - (treatAsLine ? 1 : 0); ++v)
        {
            sum += Distance(polygon[v], polygon[(v + 1) % polygon.size()]);
        }
        return sum;
    }

    //! rotates start point of a polygon (jumps over existing points)
    static void RotatePolygonIndex(Path &polygon, unsigned int newStartPointIndex)
    {
        // "fix" input value
        newStartPointIndex = newStartPointIndex % polygon.size();

        // test if polygon is closed explicitly
        bool startEndEqual = polygon.front() == polygon.back();

        if (startEndEqual)
        {
            polygon.resize(polygon.size() - 1);
        }

        std::rotate(polygon.begin(), polygon.begin() + newStartPointIndex, polygon.end());

        if (startEndEqual)
        {
            polygon.push_back(polygon[0]);
        }
    }

    //! rotates start point of a polygon (jumps over exact distance and can create new point)
    static void RotatePolygonOffset(Path &polygon, cUInt newStartPointOffset)
    {
        // "fix" input value
        cUInt perimeter = Perimeter(polygon);
        newStartPointOffset = newStartPointOffset % perimeter;

        // test if polygon is closed explicitly
        bool startEndEqual = polygon.front() == polygon.back();

        // find index of vertex preceding the exact place of new start/end point
        unsigned int newStartPointIndex = 0;
        cUInt sum = 0;
        for (int v = 0; v < (int)polygon.size(); ++v)
        {
            cUInt distance = Distance(polygon[v], polygon[(v + 1) % polygon.size()]);
            if (sum + distance > newStartPointOffset)
            {
                break;
            }
            sum += distance;
            newStartPointIndex++;
        }
        newStartPointOffset -= sum;

        // rotate polygon
        RotatePolygonIndex(polygon, newStartPointIndex);

        // calculate the exact point
        IntPoint vec = IntPoint(polygon[1].X - polygon[0].X, polygon[1].Y - polygon[0].Y);
        long double length = Distance(polygon[0], polygon[1]);
        long double position = newStartPointOffset / length;
        IntPoint newPoint = IntPoint(polygon[0].X + position * vec.X, polygon[0].Y + position * vec.Y);

        polygon.push_back(startEndEqual ? newPoint : polygon[0]);
        polygon[0] = newPoint;
    }

    const double half = 0.5;
}

#endif // SLICER_CLIPPEREXT_H
