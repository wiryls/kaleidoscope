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

namespace kd { namespace detail
{
    /************************************************************************
     * scalar_accessor
     ***********************************************************************/

    template<typename T>
    struct scalar_accessor<scalar<G, T>>
    {
        using type       = scalar<G, T>;
        using value_type = T;
        static constexpr const size_t channel = 1;
        static constexpr inline auto & gray (type       & s) { return s.g; }
        static constexpr inline auto & gray (type const & s) { return s.g; }
    };

    template<typename T>
    struct scalar_accessor<scalar<RGB, T>>
    {
        using type       = scalar<RGB, T>;
        using value_type = T;
        static constexpr const size_t channel = 3;
        static constexpr inline auto & red  (type       & s) { return s.r; }
        static constexpr inline auto & red  (type const & s) { return s.r; }
        static constexpr inline auto & green(type       & s) { return s.g; }
        static constexpr inline auto & green(type const & s) { return s.g; }
        static constexpr inline auto & blue (type       & s) { return s.b; }
        static constexpr inline auto & blue (type const & s) { return s.b; }
    };

    template<typename T>
    struct scalar_accessor<scalar<RGBA, T>>
    {
        using type       = scalar<RGBA, T>;
        using value_type = T;
        static constexpr const size_t channel = 4;
        static constexpr inline auto & red  (type       & s) { return s.r; }
        static constexpr inline auto & red  (type const & s) { return s.r; }
        static constexpr inline auto & green(type       & s) { return s.g; }
        static constexpr inline auto & green(type const & s) { return s.g; }
        static constexpr inline auto & blue (type       & s) { return s.b; }
        static constexpr inline auto & blue (type const & s) { return s.b; }
        static constexpr inline auto & alpha(type       & s) { return s.a; }
        static constexpr inline auto & alpha(type const & s) { return s.a; }
    };
}}
