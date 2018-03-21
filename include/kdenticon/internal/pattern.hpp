#pragma once

#include "matrix.hpp"

namespace kd
{
    /// pattern
    template<typename T>
    class pattern
    {
    public:
        using       reference = typename matrix<T>::      reference;
        using const_reference = typename matrix<T>::const_reference;

    public:
        constexpr void  width  (size_t w);
        constexpr void height  (size_t h);
        constexpr void anchor_x(size_t x);
        constexpr void anchor_y(size_t y);

    public:
        constexpr size_t width () const;
        constexpr size_t height() const;
        const_reference at(size_t x, size_t y) const;

    private:
        size_t wid, hgt;
        size_t apx, apy;
        matrix<T>   mat;
    };
}

namespace kd
{
    template<typename T> inline constexpr
    size_t pattern<T>::
    width() const
    {
        return wid;
    }

    template<typename T> inline constexpr
    size_t pattern<T>::
    height() const
    {
        return hgt;
    }

    template<typename T> inline typename
    pattern<T>::const_reference pattern<T>::
    at(size_t x, size_t y) const
    {
        auto sq3 = std::sqrtf(3);
        auto n   = std::min<float>(mat.height(), mat.width());

        // affine transformation
        auto rtx = x   + y / sq3;
        auto ltx = x   - y / sq3;
        auto lty = 2.f * y / sq3;

        // floor
        auto rx = static_cast<int>(rtx / n) - static_cast<int>(rtx < 0);
        auto lx = static_cast<int>(ltx / n) - static_cast<int>(ltx < 0);
        auto ly = static_cast<int>(lty / n) - static_cast<int>(lty < 0);

        auto sx = ltx - lx * n;
        auto sy = lty - ly * n;

        // reflect
        if (lx != rx) {
            sx = sx - n / 2;
            sy = n * sq3 / 2.f - sy;
        }

        // rotate
        int k = (lx - ly) % 2;
        if (k != 0) {
            auto rx = ((3 - k) * n / 2.f       + k * sq3 * sy - sx) / 2.f;
            auto ry = ((3 + k) * n / 2.f / sq3 - k * sq3 * sx - sy) / 2.f;
            x = static_cast<int>(rx);
            y = static_cast<int>(ry);
        } else {
            x = static_cast<int>(sx);
            y = static_cast<int>(sy);
        }

        return matrix<T>::at(static_cast<size_t>(x), static_cast<size_t>(y));
    }
}
