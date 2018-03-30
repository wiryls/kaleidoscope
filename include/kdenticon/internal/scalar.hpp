#pragma once

namespace kd
{
    /************************************************************************
     * colorspace
     ***********************************************************************/

    enum colorspace
    {
        G,
        RGB,
        RGBA
    };
}

namespace kd
{
    /************************************************************************
     * scalar
     ***********************************************************************/

    template<colorspace C, typename T> struct scalar;
    template<typename T>               struct scalar_accessor;

    template<typename T>
    struct scalar<G, T>
    {
        using accessor = scalar_accessor<scalar>;
        T g;
    };

    template<typename T>
    struct scalar<RGB, T>
    {
        using accessor = scalar_accessor<scalar>;
        T r, g, b;
    };

    template<typename T>
    struct scalar<RGBA, T>
    {
        using accessor = scalar_accessor<scalar>;
        T r, g, b, a;
    };
}

namespace kd
{
    /************************************************************************
     * scalar_accessor
     ***********************************************************************/

    template<typename T> inline constexpr auto &
    gray (scalar<G, T>       & s) { return s.g; }
    template<typename T> inline constexpr auto &
    gray (scalar<G, T> const & s) { return s.g; }

    template<typename T> inline constexpr auto &
    red  (scalar<RGB, T>       & s) { return s.r; }
    template<typename T> inline constexpr auto &
    red  (scalar<RGB, T> const & s) { return s.r; }
    template<typename T> inline constexpr auto &
    green(scalar<RGB, T>       & s) { return s.g; }
    template<typename T> inline constexpr auto &
    green(scalar<RGB, T> const & s) { return s.g; }
    template<typename T> inline constexpr auto &
    blue (scalar<RGB, T>       & s) { return s.b; }
    template<typename T> inline constexpr auto &
    blue (scalar<RGB, T> const & s) { return s.b; }

    template<typename T> inline constexpr auto &
    red  (scalar<RGBA, T>       & s) { return s.r; }
    template<typename T> inline constexpr auto &
    red  (scalar<RGBA, T> const & s) { return s.r; }
    template<typename T> inline constexpr auto &
    green(scalar<RGBA, T>       & s) { return s.g; }
    template<typename T> inline constexpr auto &
    green(scalar<RGBA, T> const & s) { return s.g; }
    template<typename T> inline constexpr auto &
    blue (scalar<RGBA, T>       & s) { return s.b; }
    template<typename T> inline constexpr auto &
    blue (scalar<RGBA, T> const & s) { return s.b; }
    template<typename T> inline constexpr auto &
    alpha(scalar<RGBA, T>       & s) { return s.a; }
    template<typename T> inline constexpr auto &
    alpha(scalar<RGBA, T> const & s) { return s.a; }
}
