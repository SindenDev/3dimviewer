#include <geometry/base/clipperExtension.h>
#include <assert.h>

namespace ClipperLib
{
    ClipperConvert::ClipperConvert(const double scaling_factor)
        : scale(scaling_factor)
    {
        if (NEAR_ZERO(scale)) throw "Invalid scaling factor";
    }

    IntPoint ClipperConvert::operator()(const DoublePoint &v)
    {
        return IntPoint(cInt(v.X * scale), cInt(v.Y * scale));
    }

    inline cInt Round(double val)
    {
        if ((val < 0)) return static_cast<cInt>(val - 0.5);
        else return static_cast<cInt>(val + 0.5);
    }

    void ClipperConvert::ToIntPoints(const std::vector<DoublePoint> &dps, std::vector<IntPoint> &ips)
    {
        ips.resize(dps.size());
        for (size_t i = 0; i < dps.size(); ++i)
        {
            ips[i].X = Round(dps[i].X * scale);
            ips[i].Y = Round(dps[i].Y * scale);
        }
    }

    DoublePoint ClipperConvert::operator()(const IntPoint &v)
    {
        return DoublePoint((double)v.X / scale, (double)v.Y / scale);
    }

    void ClipperConvert::ToDoublePoints(const std::vector<IntPoint> &ips, std::vector<DoublePoint> &dps)
    {
        dps.resize(ips.size());
        for (size_t i = 0; i < ips.size(); ++i)
        {
            dps[i].X = double(ips[i].X) / scale;
            dps[i].Y = double(ips[i].Y) / scale;
        }
    }

    void OffsetPolygons(const Paths &in_polys, Paths &out_polys, double delta, JoinType jointype, double limit /*= 0*/)
    {
        ClipperOffset co(limit, limit);
        co.AddPaths(in_polys, jointype, ClipperLib::etClosedPolygon);
        co.Execute(out_polys, delta);
    }

    void OffsetPolyLines(const Paths &in_lines, Paths &out_lines, double delta, JoinType jointype, EndType endtype, double limit, bool autoFix)
    {
        ClipperOffset co(limit, limit);
        co.AddPaths(in_lines, jointype, (EndType)endtype);
        co.Execute(out_lines, delta);
    }
}
