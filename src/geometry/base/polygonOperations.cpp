#include "geometry/base/polygonOperations.h"

#include <algorithm>
//#include <CModel.h>

namespace geometry
{
    //
    CPolyline::CPolyline()
    { }

    //
    CPolyline::~CPolyline()
    { }

    //
    bool CPolyline::isRing() const
    {
        return ((size() >= 3) && ((*this)[0] == (*this)[size() - 1]));
    }

    //
    double CPolyline::area() const
    {
        if (!isRing())
        {
            return 0.0;
        }

        int last = size() - 1;

        double a = ((*this)[last][0] + (*this)[0][0]) * ((*this)[0][1] - (*this)[last][1]);
        for (int i = 1; i < size(); ++i)
        {
            geometry::Vec2 vertex0 = (*this)[i];
            geometry::Vec2 vertex1 = (*this)[i - 1];

            a += (vertex1[0] + vertex0[0]) * (vertex0[1] - vertex1[1]);
        }

        return a / 2;
    }

    //
    bool CPolyline::orientation() const
    {
        return area() >= 0;
    }

    //
    double CPolyline::length() const
    {
        if (size() < 2)
        {
            return 0.0;
        }

        double sum = 0.0;

        // Compute polyline length
        geometry::Vec2 last = front();
        for (CPolyline::const_iterator it = ++begin(); it != end(); ++it)
        {
            last -= *it;
            sum += last.length();
            last = *it;
        }

        // Add connection from the last polyline point to the first one
        if (isRing())
        {
            last -= front();
            sum += last.length();
        }

        return sum;
    }

    //
    void CPolyline::setRing()
    {
        if (!isRing() && size() > 0)
        {
            push_back(front());
        }
    }

    //
    CGeometry::CGeometry()
    { }

    //
    CGeometry::CGeometry(const CPolyline &polyline)
    {
        if (polyline.isRing())
        {
            m_geometry.resize(1);
            ClipperLib::Path &polygon = m_geometry.back();
            polygon.clear();

            for (int i = 0; i < polyline.size(); ++i)
            {
                polygon.push_back(ClipperLib::IntPoint(polyline[i][0] * CLIPPER_MULT, polyline[i][1] * CLIPPER_MULT));
            }
        }
    }

    //
    CGeometry::CGeometry(const std::vector<CPolyline> &polylines)
    {
        m_geometry.clear();

        for (int p = 0; p < polylines.size(); ++p)
        {
            const CPolyline &polyline = polylines[p];

            if (polyline.isRing())
            {
                m_geometry.resize(m_geometry.size() + 1);
                ClipperLib::Path &polygon = m_geometry.back();

                for (int i = 0; i < polyline.size(); ++i)
                {
                    polygon.push_back(ClipperLib::IntPoint(polyline[i][0] * CLIPPER_MULT, polyline[i][1] * CLIPPER_MULT));
                }
            }
        }
    }

    //
    CGeometry::CGeometry(const CGeometry &other)
    {
        m_geometry = other.m_geometry;
    }

    //
    CGeometry::~CGeometry()
    { }

    //
    CGeometry &CGeometry::operator=(const CGeometry &other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_geometry = other.m_geometry;

        return *this;
    }

    double CGeometry::area(const ClipperLib::Paths& polys)
    {
        double sum = 0.0;

        for (int i = 0; i < polys.size(); ++i)
        {
            sum += ClipperLib::Area(polys[i]);
        }

        return sum / (CLIPPER_MULT * CLIPPER_MULT);
    }

    double CGeometry::area() const
    {
        return area(m_geometry);
    }

    double CGeometry::perimeter(const ClipperLib::Paths& polys)
    {
        double sum = 0.0;

        for (int i = 0; i < polys.size(); ++i)
        {
            sum += ClipperLib::Perimeter(polys[i]);
        }

        return sum / CLIPPER_MULT;
    }

    double CGeometry::perimeter() const
    {
        return perimeter(m_geometry);
    }

    //
    void CGeometry::offset(double distance, EJoinType join, double limit)
    {
        ClipperLib::Paths result;
        for (int p = 0; p < m_geometry.size(); ++p)
        {
            if (m_geometry[p].size() > 0)
            {
                if (m_geometry[p].front() == m_geometry[p].back())
                {
                    m_geometry[p].pop_back();
                }
            }
        }
        ClipperLib::OffsetPolygons(m_geometry, result, distance * CLIPPER_MULT, (ClipperLib::JoinType)join, limit);
        m_geometry = result;
        closeRingGeometry();
    }

    //
    void CGeometry::merge(const CGeometry &other)
    {
        ClipperLib::Paths result;

        ClipperLib::Clipper clipper;
        clipper.AddPaths(m_geometry, ClipperLib::ptSubject, true);
        clipper.AddPaths(other.m_geometry, ClipperLib::ptClip, true);
        clipper.Execute(ClipperLib::ctUnion, result);
        m_geometry = result;
        closeRingGeometry();
    }

    //
    void CGeometry::intersection(const CGeometry &other)
    {
        ClipperLib::Paths result;

        ClipperLib::Clipper clipper;
        clipper.AddPaths(m_geometry, ClipperLib::ptSubject, true);
        clipper.AddPaths(other.m_geometry, ClipperLib::ptClip, true);
        clipper.Execute(ClipperLib::ctIntersection, result);
        m_geometry = result;
        closeRingGeometry();
    }

    //
    void CGeometry::difference(const CGeometry &other)
    {
        ClipperLib::Paths result;

        ClipperLib::Clipper clipper;
        clipper.AddPaths(m_geometry, ClipperLib::ptSubject, true);
        clipper.AddPaths(other.m_geometry, ClipperLib::ptClip, true);
        clipper.Execute(ClipperLib::ctDifference, result);

        m_geometry = result;
        closeRingGeometry();
    }

    void CGeometry::append(const CGeometry &other)
    {
        const int nAdd = other.m_geometry.size();
        for (int i = 0; i < nAdd; i++)
        {
            if (other.m_geometry[i].size() > 0)
            {
                m_geometry.push_back(other.m_geometry[i]);
            }
        }
    }

    void CGeometry::closeRingGeometry()
    {
        simplifyGeometry(0.0005);
        int cnt = m_geometry.size();
        for (int i = 0; i < cnt; i++)
        {
            if (m_geometry[i].size() != 0)
            {
                m_geometry[i].push_back(m_geometry[i].front());
            }
        }
    }

    double CGeometry::magnitude(const ClipperLib::IntPoint &pt1, const ClipperLib::IntPoint &pt2)
    {
        double vX = pt2.X - pt1.X;
        double vY = pt2.Y - pt1.Y;
        return sqrt(vX * vX + vY * vY);
    }

    bool CGeometry::distancePointLine(
        const ClipperLib::IntPoint &pt,
        const ClipperLib::IntPoint &pt0,
        const ClipperLib::IntPoint &pt1,
        double& distance)
    {
        double LineMag = magnitude(pt1, pt0);
        double U = (((pt.X - pt0.X) * (pt1.X - pt0.X)) + ((pt.Y - pt0.Y) * (pt1.Y - pt0.Y))) / (LineMag * LineMag);
        if (U < 0.0 || U > 1.0)
        {
            return false;   // closest point does not fall within the line segment
        }

        ClipperLib::IntPoint inter;
        inter.X = pt0.X + U * (pt1.X - pt0.X);
        inter.Y = pt0.Y + U * (pt1.Y - pt0.Y);
        distance = magnitude(pt, inter);
        return true;
    }

    void CGeometry::simplifyGeometry(double dotThreshold)
    {
        const double distanceThreshold = 10;
        ClipperLib::Paths workPolys;
        for (int p = 0; p < m_geometry.size(); ++p)
        {
            ClipperLib::Path dstPoly;
            workPolys.push_back(dstPoly);
        }
#pragma omp parallel for
        for (int p = 0; p < m_geometry.size(); ++p)
        {
            ClipperLib::Path *dstPoly = &(workPolys.operator[](p));
            size_t sizeGeom = m_geometry[p].size();
            std::vector<ClipperLib::IntPoint> skipped;
            if (sizeGeom > 25) // optimize only geometries with at least 25 points
            {
                for (int i = 0; i < sizeGeom; ++i)
                {
                    bool bskip = false;
                    if (i > 1)
                    {
                        // compare angle between first skipped segment and new candidate
                        int iD = dstPoly->size();
                        const ClipperLib::IntPoint &pt0 = (*dstPoly)[iD - 2];
                        const ClipperLib::IntPoint &pt1 = m_geometry[p][i];
                        ClipperLib::cInt x0, y0, x1, y1;
                        ClipperLib::cInt dx0, dy0, dx1, dy1;
                        x0 = pt0.X;
                        y0 = pt0.Y;
                        if (!skipped.empty())
                        {
                            dx0 = skipped.front().X - x0;
                            dy0 = skipped.front().Y - y0;
                        }
                        else
                        {
                            dx0 = dstPoly->back().X - x0;
                            dy0 = dstPoly->back().Y - y0;
                        }
                        x1 = pt1.X;
                        y1 = pt1.Y;
                        dx1 = x1 - x0;
                        dy1 = y1 - y0;
                        geometry::Vec2 v1(dx1, dy1);
                        geometry::Vec2 v0(dx0, dy0);
                        v1.normalize();
                        v0.normalize();
                        double dot = v1 * v0;
                        bskip = (dot > 1 - dotThreshold);
                        // if fit enough
                        if (bskip)
                        {
                            skipped.push_back(dstPoly->back());
                            // perform additional check on distance error caused by this
                            for (int j = 0; j < skipped.size(); j++)
                            {
                                // compute distance of point to line and compare to threshold
                                double dist = 0;
                                int res = distancePointLine(skipped[j], pt0, pt1, dist);
                                if (0 == res || dist > distanceThreshold)
                                {
                                    bskip = false;
                                    break;
                                }
                            }
                        }
                    }
                    if (bskip)
                    {
                        dstPoly->pop_back();
                    }
                    else
                    {
                        skipped.clear();
                    }
                    dstPoly->push_back(m_geometry[p][i]);
                }
            }
            else
            {
                workPolys[p] = m_geometry[p];
            }
        }
        m_geometry = workPolys;
    }

    void CGeometry::cleanPolygons(double distance)
    {
        ClipperLib::SimplifyPolygons(m_geometry);
        if (distance > 0)
        {
            simplifyGeometry(0.001);
            ClipperLib::Paths result = m_geometry;
#pragma omp parallel for
            for (int p = 0; p < m_geometry.size(); ++p)
            {
                ClipperLib::CleanPolygon(m_geometry[p], result[p], distance);
            }
            m_geometry = result;
        }
        closeRingGeometry();
    }

    //
    ClipperLib::Paths *CGeometry::getInternalGeometry()
    {
        return &m_geometry;
    }

    //
    const ClipperLib::Paths *CGeometry::getConstInternalGeometry() const
    {
        return &m_geometry;
    }

    //
    bool CGeometry::isEmpty()
    {
        bool retVal = true;

        for (int i = 0; i < m_geometry.size(); ++i)
        {
            if (m_geometry[i].size() != 0)
            {
                retVal = false;
                break;
            }
        }

        if (retVal && m_geometry.size() > 0)
        {
            m_geometry.clear();
        }

        return retVal;
    }

    bool CGeometry::isEmptyConst() const
    {
        bool retVal = true;

        for (int i = 0; i < m_geometry.size(); ++i)
        {
            if (m_geometry[i].size() != 0)
            {
                retVal = false;
                break;
            }
        }

        return retVal;
    }

    int CGeometry::polygonCount()
    {
        if (isEmpty())
        {
            return 0;
        }
        return m_geometry.size();
    }

    //
    void CGeometry::getAsPolylines(std::vector<CPolyline> &polylines) const
    {
        for (int p = 0; p < m_geometry.size(); ++p)
        {
            CPolyline poly;
            for (int i = 0; i < m_geometry[p].size(); ++i)
            {
                double x = m_geometry[p][i].X / CLIPPER_MULT;
                double y = m_geometry[p][i].Y / CLIPPER_MULT;
                poly.push_back(geometry::Vec2(x, y));
            }
            polylines.push_back(poly);
        }
    }

    bool comparePoints(const ClipperLib::IntPoint &p1, const ClipperLib::IntPoint &p2)
    {
        return p1.X < p2.X || (p1.X == p2.X && p1.Y < p2.Y);
    }

    // 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
    // Returns a positive value, if OAB makes a counter-clockwise turn,
    // negative for clockwise turn, and zero if the points are collinear.
    double cross(const ClipperLib::IntPoint &O, const ClipperLib::IntPoint &A, const ClipperLib::IntPoint &B)
    {
        return (A.X - O.X) * (B.Y - O.Y) - (A.Y - O.Y) * (B.X - O.X);
    }

    // Implementation of Andrew's monotone chain 2D convex hull algorithm.
    // Asymptotic complexity: O(n log n).
    void CGeometry::convexHull()
    {
        // put all geometry points to an array
        std::vector<ClipperLib::IntPoint> points;
        for (int p = 0; p < m_geometry.size(); ++p)
        {
            for (int i = 0; i < m_geometry[p].size(); ++i)
            {
                points.push_back(m_geometry[p][i]);
            }
        }

        // sort points
        std::sort(points.begin(), points.end(), comparePoints);

        int k = 0;
        int n = points.size();
        std::vector<ClipperLib::IntPoint> H(2 * n); // a list of points on the convex hull in counter-clockwise order
        // Build lower hull
        for (int i = 0; i < n; i++)
        {
            while (k >= 2 && cross(H[k - 2], H[k - 1], points[i]) <= 0)
            {
                k--;
            }
            H[k++] = points[i];
        }

        // Build upper hull
        for (int i = n - 2, t = k + 1; i >= 0; i--)
        {
            while (k >= t && cross(H[k - 2], H[k - 1], points[i]) <= 0)
            {
                k--;
            }
            H[k++] = points[i];
        }
        H.resize(k);

        // update geometry
        m_geometry.clear();
        ClipperLib::Path poly;
        for (int i = 0; i < k; i++)
        {
            poly.push_back(H[i]);
        }
        m_geometry.push_back(poly);
    }

    //
    CLineGeometry::CLineGeometry()
        : CGeometry()
    { }

    //
    CLineGeometry::CLineGeometry(const CPolyline &polyline)
    {
        if (polyline.isRing())
        {
            m_geometry.resize(2);

            ClipperLib::Path &line0 = m_geometry[0];
            for (int i = 0; i <= polyline.size() / 2; ++i)
            {
                line0.push_back(ClipperLib::IntPoint(polyline[i][0] * CLIPPER_MULT, polyline[i][1] * CLIPPER_MULT));
            }

            ClipperLib::Path &line1 = m_geometry[1];
            for (int i = polyline.size() / 2; i < polyline.size(); ++i)
            {
                line1.push_back(ClipperLib::IntPoint(polyline[i][0] * CLIPPER_MULT, polyline[i][1] * CLIPPER_MULT));
            }
        }
        else
        {
            m_geometry.resize(1);
            ClipperLib::Path &line = m_geometry.back();
            line.clear();

            for (int i = 0; i < polyline.size(); ++i)
            {
                line.push_back(ClipperLib::IntPoint(polyline[i][0] * CLIPPER_MULT, polyline[i][1] * CLIPPER_MULT));
            }
        }
    }

    //
    CLineGeometry::CLineGeometry(const std::vector<CPolyline> &polylines)
    {
        m_geometry.clear();

        for (int p = 0; p < polylines.size(); ++p)
        {
            const CPolyline &polyline = polylines[p];

            if (polyline.isRing())
            {
                m_geometry.resize(m_geometry.size() + 1);
                ClipperLib::Path &line0 = m_geometry.back();

                for (int i = 0; i <= polyline.size() / 2; ++i)
                {
                    line0.push_back(ClipperLib::IntPoint(polyline[i][0] * CLIPPER_MULT, polyline[i][1] * CLIPPER_MULT));
                }

                m_geometry.resize(m_geometry.size() + 1);
                ClipperLib::Path &line1 = m_geometry.back();

                for (int i = polyline.size() / 2; i < polyline.size(); ++i)
                {
                    line1.push_back(ClipperLib::IntPoint(polyline[i][0] * CLIPPER_MULT, polyline[i][1] * CLIPPER_MULT));
                }
            }
            else
            {
                m_geometry.resize(m_geometry.size() + 1);
                ClipperLib::Path &line = m_geometry.back();

                for (int i = 0; i < polyline.size(); ++i)
                {
                    line.push_back(ClipperLib::IntPoint(polyline[i][0] * CLIPPER_MULT, polyline[i][1] * CLIPPER_MULT));
                }
            }
        }
    }

    //
    CLineGeometry::CLineGeometry(const CLineGeometry &other)
        : CGeometry(other)
    { }

    //
    CLineGeometry::~CLineGeometry()
    { }

    //
    CLineGeometry &CLineGeometry::operator=(const CLineGeometry &other)
    {
        if (this == &other)
        {
            return *this;
        }

        m_geometry = other.m_geometry;

        return *this;
    }

    //
    void CLineGeometry::offset(double distance, CGeometry &result) const
    {
        result.m_geometry.clear();
        ClipperLib::OffsetPolyLines(m_geometry, result.m_geometry, distance * CLIPPER_MULT);
        result.closeRingGeometry();
    }

    //
    void CLineGeometry::merge(const CLineGeometry &other)
    {
        m_geometry.insert(m_geometry.end(), other.m_geometry.begin(), other.m_geometry.end());
    }

    //
    void CLineGeometry::intersection(const CGeometry &other)
    {
        ClipperLib::PolyTree result;
        ClipperLib::Clipper clipper;

        clipper.AddPaths(m_geometry, ClipperLib::ptSubject, false);
        clipper.AddPaths(other.m_geometry, ClipperLib::ptClip, true);
        clipper.Execute(ClipperLib::ctIntersection, result);

        ClipperLib::PolyTreeToPaths(result, m_geometry);
    }

    //
    bool CLineGeometry::isEmpty()
    {
        return CGeometry::isEmpty();
    }

    double CLineGeometry::length()
    {
        double length = 0;
        for (int p = 0; p < m_geometry.size(); ++p)
        {
            double seglength = 0;
            double lx = 0, ly = 0;
            for (int i = 0; i < m_geometry[p].size(); ++i)
            {
                double x = m_geometry[p][i].X / CLIPPER_MULT;
                double y = m_geometry[p][i].Y / CLIPPER_MULT;
                if (i>0)
                {
                    seglength += sqrt((x - lx) * (x - lx) + (y - ly) * (y - ly));
                }
                lx = x;
                ly = y;
            }
            length += seglength;
        }
        return length;
    }
}
