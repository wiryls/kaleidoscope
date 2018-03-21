#pragma once

#include <limits>
#include <cstddef>
#include <type_traits>

namespace std
{
    /************************************************************************
     * forward declaration
     ***********************************************************************/

    template<class T, size_t N> class array;
}

namespace kd { namespace detail
{
    /************************************************************************
     * scalar_trait_base
     ***********************************************************************/

    template<typename, typename = void>
    struct scalar_trait_base {};

    template<typename T>
    struct scalar_trait_base<T, std::enable_if_t<
        std::is_arithmetic<typename T::accessor::value_type>::value &&
        T::accessor::channel != 0,
        std::void_t<typename T::accessor::type>
    >> : T::accessor {};

    template<typename, typename, size_t>
    struct scalar_trait_sarray_base {};

    template<typename T, size_t N>
    struct scalar_trait_base<T[N], std::enable_if_t<
        std::is_arithmetic<T>::value
    >> : scalar_trait_sarray_base<T[N], T, N> {};

    template<typename T, size_t N>
    struct scalar_trait_base<std::array<T, N>, std::enable_if_t<
        std::is_arithmetic<T>::value
    >> : scalar_trait_sarray_base<std::array<T, N>, T, N> {};

    /************************************************************************
     * scalar_trait_base : RGBA
     ***********************************************************************/

    template<typename T, typename U>
    struct scalar_trait_sarray_base<T, U, 4>
    {
        using type       = T;
        using value_type = U;
        static constexpr const size_t channel = 4;
        static constexpr inline auto & red  (type       & s) { return s[0]; }
        static constexpr inline auto & red  (type const & s) { return s[0]; }
        static constexpr inline auto & green(type       & s) { return s[1]; }
        static constexpr inline auto & green(type const & s) { return s[1]; }
        static constexpr inline auto & blue (type       & s) { return s[2]; }
        static constexpr inline auto & blue (type const & s) { return s[2]; }
        static constexpr inline auto & alpha(type       & s) { return s[3]; }
        static constexpr inline auto & alpha(type const & s) { return s[3]; }
    };

    /************************************************************************
     * scalar_trait_base : RGB
     ***********************************************************************/

    template<typename T, typename U>
    struct scalar_trait_sarray_base<T, U, 3>
    {
        using type       = T;
        using value_type = U;
        static constexpr const size_t channel = 3;
        static constexpr inline auto & red  (type       & s) { return s[0]; }
        static constexpr inline auto & red  (type const & s) { return s[0]; }
        static constexpr inline auto & green(type       & s) { return s[1]; }
        static constexpr inline auto & green(type const & s) { return s[1]; }
        static constexpr inline auto & blue (type       & s) { return s[2]; }
        static constexpr inline auto & blue (type const & s) { return s[2]; }
    };

    /************************************************************************
     * scalar_trait_base : G
     ***********************************************************************/

    template<typename T, typename U>
    struct scalar_trait_sarray_base<T, U, 1>
    {
        using type       = T;
        using value_type = U;
        static constexpr const size_t channel = 1;
        static constexpr inline auto & gray (type       & s) { return s[0]; }
        static constexpr inline auto & gray (type const & s) { return s[0]; }
    };

    template<typename T>
    struct scalar_trait_base<T, std::enable_if_t<
        std::is_arithmetic<T>::value
    >> {
        using type       = T;
        using value_type = T;
        static constexpr const size_t channel = 1;
        static constexpr inline auto & gray (type       & s) { return s; }
        static constexpr inline auto & gray (type const & s) { return s; }
    };
}}

namespace kd
{
    /************************************************************************
     * scalar_trait
     ***********************************************************************/

    template<typename T>
    struct scalar_trait
        : detail::scalar_trait_base<T>
    {};
}

namespace kd { namespace detail
{
    /************************************************************************
     * scalar_colormode
     ***********************************************************************/

    template<typename, typename = void> struct scalar_colormode {};

    template<typename T>
    struct scalar_colormode<T, std::enable_if_t<
        scalar_trait<T>::channel == 4, std::void_t<decltype(
        scalar_trait<T>::red  (std::declval<T>()),
        scalar_trait<T>::green(std::declval<T>()),
        scalar_trait<T>::blue (std::declval<T>()),
        scalar_trait<T>::alpha(std::declval<T>())
    )>>> {
        using type = T;
        using rgba = type;
    };

    template<typename T>
    struct scalar_colormode<T, std::enable_if_t<
        scalar_trait<T>::channel == 3, std::void_t<decltype(
        scalar_trait<T>::red  (std::declval<T>()),
        scalar_trait<T>::green(std::declval<T>()),
        scalar_trait<T>::blue (std::declval<T>())
    )>>> {
        using type = T;
        using rgb = type;
    };

    template<typename T>
    struct scalar_colormode<T, std::enable_if_t<
        scalar_trait<T>::channel == 1, std::void_t<decltype(
        scalar_trait<T>::gray(std::declval<T>())
    )>>> {
        using type = T;
        using gray = type;
    };
}}

namespace kd
{
    /************************************************************************
     * scalar_trait
     ***********************************************************************/

    template<typename T>
    using scalar_trait_t = typename scalar_trait<T>::type;

    template<typename T>
    using scalar_trait_vt = typename scalar_trait<T>::value_type;

    template<typename T>
    using gray_scalar_trait =
        scalar_trait<typename detail::scalar_colormode<T>::gray>;

    template<typename T>
    using rgb_scalar_trait =
        scalar_trait<typename detail::scalar_colormode<T>::rgb>;

    template<typename T>
    using rgba_scalar_trait =
        scalar_trait<typename detail::scalar_colormode<T>::rgba>;
}

namespace kd { namespace detail
{
    /************************************************************************
     * value_clamp
     ***********************************************************************/

    template<typename T> inline constexpr std::enable_if_t<
        std::is_floating_point<T>::value, T
    > value_clamp(T val)
    {
        return val < T() ? T() : val > T(1) ? T(1) : val;
    }

    template<typename T> inline constexpr std::enable_if_t<
        std::is_integral<T>::value && std::is_signed<T>::value, T
    > value_clamp(T val)
    {
        return val < T() ? T() : val;
    }

    template<typename T> inline constexpr std::enable_if_t<
        std::is_integral<T>::value && std::is_unsigned<T>::value, T
    > value_clamp(T val)
    {
        return val;
    }

    /************************************************************************
     * value_scale
     ***********************************************************************/

    template<typename T> inline constexpr void
    value_scale(T const & src, T & dst)
    {
        dst = value_clamp(src);
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<(
        std::is_integral<T>::value && std::is_integral<U>::value &&
        std::numeric_limits<T>::digits > std::numeric_limits<U>::digits
    )> value_scale(T const & src, U & dst)
    {
        static auto shift
            = std::numeric_limits<T>::digits
            - std::numeric_limits<U>::digits
            ;
        dst = src == std::numeric_limits<T>::max()
            ? value_clamp(std::numeric_limits<U>::max())
            : static_cast<U>(value_clamp(src) >> shift)
            ;
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<(
        std::is_integral<T>::value && std::is_integral<U>::value &&
        std::numeric_limits<T>::digits < std::numeric_limits<U>::digits
    )> value_scale(T const & src, U & dst)
    {
        static auto shift
            = std::numeric_limits<U>::digits
            - std::numeric_limits<T>::digits
            ;
        dst = src == std::numeric_limits<T>::max()
            ? value_clamp(std::numeric_limits<U>::max())
            : static_cast<U>(value_clamp(src)) << shift
            ;
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<(
        std::is_floating_point<T>::value && std::is_floating_point<U>::value
    )> value_scale(T const & src, U & dst)
    {
        dst = static_cast<U>(value_clamp(src));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<(
        std::is_floating_point<T>::value && std::is_integral<U>::value
    )> value_scale(T const & src, U & dst)
    {
        static auto m = value_clamp(std::numeric_limits<T>::max());
        auto clamped = value_clamp(src);
        dst = clamped == m
            ? value_clamp(std::numeric_limits<U>::max())
            : static_cast<U>(clamped * std::numeric_limits<U>::max())
            ;
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<(
        std::is_integral<T>::value && std::is_floating_point<U>::value
    )> value_scale(T const & src, U & dst)
    {
        dst = src == std::numeric_limits<T>::max()
            ? value_clamp(std::numeric_limits<U>::max())
            : static_cast<U>(value_clamp(src))
            / static_cast<U>(std::numeric_limits<T>::max())
            ;
    }

    /************************************************************************
     * scalar_convert
     ***********************************************************************/

    /* G */

    template<typename T, typename U> inline constexpr std::enable_if_t<
        gray_scalar_trait<T>::channel && gray_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    /* [Bug] of VS2017, we cannot use:
    ```
    template<typename T, typename U> inline constexpr auto
    scalar_convert(T const & src, U & dst) ->
    decltype(gray_scalar_trait<T>(), rgb_scalar_trait<U>(), void())
    ```
    */
    {
        using namespace detail;
        using st = gray_scalar_trait<T>;
        using dt = gray_scalar_trait<U>;
        value_scale(st::gray(src), dt::gray(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        gray_scalar_trait<T>::channel && rgb_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        using namespace detail;
        using st = gray_scalar_trait<T>;
        using dt =  rgb_scalar_trait<U>;
        value_scale(st::gray(src), dt::blue(dst));
        dt::red(dst) = dt::green(dst) = dt::blue(dst);
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        gray_scalar_trait<T>::channel && rgba_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        using namespace detail;
        using st = gray_scalar_trait<T>;
        using dt = rgba_scalar_trait<U>;
        value_scale(st::gray(src), dt::blue(dst));
        dt::red(dst) = dt::green(dst) = dt::blue(dst);
        dt::aplha(dst) = value_clamp(std::numeric_limits<U>::max());
    }

    /* RGB */

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgb_scalar_trait<T>::channel && gray_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        using namespace detail;
        using st =  rgb_scalar_trait<T>;
        using dt = gray_scalar_trait<U>;
        auto r = 0.0, g = 0.0, b = 0.0;
        value_scale(st::  red(src), r);
        value_scale(st::green(src), g);
        value_scale(st:: blue(src), b);
        value_scale(r * 0.299 + g * 0.587 + b * 0.114, dt::gray(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgb_scalar_trait<T>::channel && rgb_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        using namespace detail;
        using st = rgb_scalar_trait<T>;
        using dt = rgb_scalar_trait<U>;
        value_scale(st::  red(src), dt::  red(dst));
        value_scale(st::green(src), dt::green(dst));
        value_scale(st:: blue(src), dt:: blue(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgb_scalar_trait<T>::channel && rgba_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        using namespace detail;
        using st =  rgb_scalar_trait<T>;
        using dt = rgba_scalar_trait<U>;
        value_scale(st::  red(src), dt::  red(dst));
        value_scale(st::green(src), dt::green(dst));
        value_scale(st:: blue(src), dt:: blue(dst));
        dt::alpha(dst) = value_clamp(std::numeric_limits<U>::max());
    }
    
    /* RGBA */

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgba_scalar_trait<T>::channel && gray_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        using namespace detail;
        using st = rgba_scalar_trait<T>;
        using dt = gray_scalar_trait<U>;
        auto r = 0.0, g = 0.0, b = 0.0;
        value_scale(st::  red(src), r);
        value_scale(st::green(src), g);
        value_scale(st:: blue(src), b);
        value_scale(r * 0.299 + g * 0.587 + b * 0.114, dt::gray(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgba_scalar_trait<T>::channel && rgb_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        using namespace detail;
        using st = rgba_scalar_trait<T>;
        using dt =  rgb_scalar_trait<U>;
        value_scale(st::  red(src), dt::  red(dst));
        value_scale(st::green(src), dt::green(dst));
        value_scale(st:: blue(src), dt:: blue(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgba_scalar_trait<T>::channel && rgba_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        using namespace detail;
        using st = rgba_scalar_trait<T>;
        using dt = rgba_scalar_trait<U>;
        value_scale(st::  red(src), dt::  red(dst));
        value_scale(st::green(src), dt::green(dst));
        value_scale(st:: blue(src), dt:: blue(dst));
        value_scale(st::alpha(src), dt::alpha(dst));
    }
}}
