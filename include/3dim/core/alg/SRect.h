#pragma once

/** A rectangle. */
struct SRect
{
    /** Defines an alias representing the vector of two integers used as 2D coordinate */
    typedef vpl::math::CIVector2 Vec2i;

    int left, right, top, bottom;

    SRect() { init(); }

    SRect(int sx, int sy, int ex, int ey)
    {
        left = std::min(sx, ex);
        right = std::max(sx, ex);
        bottom = std::min(sy, ey);
        top = std::max(sy, ey);
    }

    SRect(int x, int y)
    {
        left = right = x;
        top = bottom = y;
    }

    SRect(const Vec2i point)
    {
        left = right = point[0];
        top = bottom = point[1];
    }

    SRect(const Vec2i min, const Vec2i max)
    {
        left = std::min(min[0], max[0]);
        right = std::max(min[0], max[0]);
        bottom = std::min(min[1], max[1]);
        top = std::max(min[1], max[1]);
    }

    void enlarge(const Vec2i &point)
    {
        left = std::min(point[0], left);
        right = std::max(point[0], right);
        bottom = std::min(point[1], bottom);
        top = std::max(point[1], top);

    }

    bool operator == (const SRect &other) const
    {
        return left == other.left && right == other.right && top == other.top && bottom == other.bottom;
    }

    void init()
    {
        left = bottom = std::numeric_limits<int>::max();
        right = top = std::numeric_limits<int>::min();
    }

    void init(const Vec2i &point)
    {
        left = right = point[0];
        top = bottom = point[1];
    }

    bool valid() const
    {
        return left <= right && bottom <= top;
    }

    int width() const {
        return right - left;
    }

    int height() const {
        return top - bottom;
    }

    bool inside(const Vec2i &point) const
    {
        assert(valid());
        return point[0] >= left && point[0] <= right && point[1] >= bottom && point[1] <= top;
    }

    Vec2i min() const {
        return Vec2i(left, bottom);
    }

    Vec2i max() const {
        return Vec2i(right, top);
    }

    void addMargin(int size)
    {
        left -= size;
        right += size;
        bottom -= size;
        top += size;
    }

    static SRect enlarged(const SRect &rect, int margin)
    {
        SRect copy(rect);
        copy.addMargin(margin);
        return copy;
    }
};
