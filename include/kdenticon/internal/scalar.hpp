#pragma once

namespace kd
{
    /************************************************************************
     * colorspace
     ***********************************************************************/

    enum colorspace
    {
        G    = 1,
        RGB  = 3,
        RGBA = 4
    };
}

namespace kd
{
    /************************************************************************
     * scalar
     ***********************************************************************/
    
    template
        < typename T
        , colorspace C
        > struct scalar
        ;

    template
        < typename T
        > struct scalar<T, G>
    {
        T g;
    };

    template
        < typename T
        > struct scalar<T, RGB>
    {
        T r, g, b;
    };

    template
        < typename T
        > struct scalar<T, RGBA>
    {
        T r, g, b, a;
    };
}

namespace kd
{
    /************************************************************************
     * scalar_accessor
     ***********************************************************************/

    template<typename T> inline constexpr auto &
    gray (scalar<T, G>       & s) { return s.g; }
    template<typename T> inline constexpr auto &
    gray (scalar<T, G> const & s) { return s.g; }

    template<typename T> inline constexpr auto &
    red  (scalar<T, RGB>       & s) { return s.r; }
    template<typename T> inline constexpr auto &
    red  (scalar<T, RGB> const & s) { return s.r; }
    template<typename T> inline constexpr auto &
    green(scalar<T, RGB>       & s) { return s.g; }
    template<typename T> inline constexpr auto &
    green(scalar<T, RGB> const & s) { return s.g; }
    template<typename T> inline constexpr auto &
    blue (scalar<T, RGB>       & s) { return s.b; }
    template<typename T> inline constexpr auto &
    blue (scalar<T, RGB> const & s) { return s.b; }

    template<typename T> inline constexpr auto &
    red  (scalar<T, RGBA>       & s) { return s.r; }
    template<typename T> inline constexpr auto &
    red  (scalar<T, RGBA> const & s) { return s.r; }
    template<typename T> inline constexpr auto &
    green(scalar<T, RGBA>       & s) { return s.g; }
    template<typename T> inline constexpr auto &
    green(scalar<T, RGBA> const & s) { return s.g; }
    template<typename T> inline constexpr auto &
    blue (scalar<T, RGBA>       & s) { return s.b; }
    template<typename T> inline constexpr auto &
    blue (scalar<T, RGBA> const & s) { return s.b; }
    template<typename T> inline constexpr auto &
    alpha(scalar<T, RGBA>       & s) { return s.a; }
    template<typename T> inline constexpr auto &
    alpha(scalar<T, RGBA> const & s) { return s.a; }
}
