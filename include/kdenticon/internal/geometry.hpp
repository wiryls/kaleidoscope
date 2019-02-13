#pragma once

namespace kd
{
    /************************************************************************
     * vec
     ***********************************************************************/

    template<typename T, unsigned N = 2> struct vec;

    template<typename T> using vec2 = vec<T, 2>;

    /************************************************************************
     * vec<T, 2>
     ***********************************************************************/

    template<typename T>
    struct vec<T, 2>
    {
        T x, y;
    };

    template<typename T> inline
    vec<T, 2> const & operator+(vec<T, 2> const & it)
    {
        return it;
    }

    template<typename T> inline
    vec<T, 2> operator-(vec<T, 2> const & it)
    {
        return { -it.x, -it.y };
    }

    template<typename T> inline
    T operator*(vec<T, 2> const & lhs, vec<T, 2> const & rhs)
    {
        return lhs.x * rhs.x + lhs.y * rhs.y;
    }

    template<typename T> inline
    T operator%(vec<T, 2> const & lhs, vec<T, 2> const & rhs)
    {
        return lhs.x * rhs.y - rhs.x * lhs.y;
    }

    template<typename T> inline
    vec<T, 2> operator*(vec<T, 2> const & lhs, T const & rhs)
    {
        return { lhs.x * rhs, lhs.y * rhs };
    }

    template<typename T> inline
    vec<T, 2> operator/(vec<T, 2> const & lhs, T const & rhs)
    {
        return { lhs.x / rhs, lhs.y / rhs };
    }

    template<typename T> inline
    vec<T, 2> operator+(vec<T, 2> const & lhs, vec<T, 2> const & rhs)
    {
        return { lhs.x + rhs.x, lhs.y + rhs.y };
    }

    template<typename T> inline
    vec<T, 2> operator-(vec<T, 2> const & lhs, vec<T, 2> const & rhs)
    {
        return { lhs.x - rhs.x, lhs.y - rhs.y };
    }

    template<typename T> inline
    bool operator==(vec<T, 2> const & lhs, vec<T, 2> const & rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    template<typename T> inline
    bool operator!=(vec<T, 2> const & lhs, vec<T, 2> const & rhs)
    {
        return lhs.x != rhs.x || lhs.y != rhs.y;
    }

    template<typename T> inline
    vec<T, 2> & operator*=(vec<T, 2> & lhs, T const & rhs)
    {
        lhs.x *= rhs;
        lhs.y *= rhs;
        return lhs;
    }

    template<typename T> inline
    vec<T, 2> & operator/=(vec<T, 2> & lhs, T const & rhs)
    {
        lhs.x /= rhs;
        lhs.y /= rhs;
        return lhs;
    }

    template<typename T> inline
    vec<T, 2> & operator+=(vec<T, 2> & lhs, vec<T, 2> const & rhs)
    {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        return lhs;
    }

    template<typename T> inline
    vec<T, 2> & operator-=(vec<T, 2> & lhs, vec<T, 2> const & rhs)
    {
        lhs.x -= rhs.x;
        lhs.y -= rhs.y;
        return lhs;
    }

    /************************************************************************
     * method
     ***********************************************************************/

    template<typename T> inline vec2<T> & transpose(vec2<T> & v)
    {
        std::swap(v.x, v.y);
        return v;
    }

    template<typename T> inline T norm_squared(vec2<T> & v)
    {
        return v.x * v.x + v.y * v.y;
    }

    template<typename T> inline T norm(vec2<T> & v)
    {
        return static_cast<T>(std::sqrt(norm_squared(v)));
    }
}
