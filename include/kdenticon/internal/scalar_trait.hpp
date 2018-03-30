#pragma once

#include <limits>
#include <cstddef>
#include <utility>
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
     * static_array_trait
     ***********************************************************************/

    template<typename T>
    struct static_array_trait {};

    template<typename T, typename V, size_t N>
    struct static_array_trait_base
    {
        using type = T;
        using value_type = V;
        static constexpr size_t size = N;
    };

    template<typename T, size_t N>
    struct static_array_trait<T[N]>
        : static_array_trait_base<T[N], T, N> {};

    template<typename T, size_t N>
    struct static_array_trait<std::array<T, N>>
        : static_array_trait_base<std::array<T, N>, T, N> {};

    /************************************************************************
     * size_specified_static_array
     ***********************************************************************/

    template<typename T, size_t N, typename = void>
    struct size_specified_static_array_base {};

    template<typename T, size_t N>
    struct size_specified_static_array_base<T, N, std::enable_if_t<
        static_array_trait<T>::size == N
    >> :static_array_trait<T> {};

    template<typename T, size_t N>
    struct size_specified_static_array
        :  size_specified_static_array_base<T, N> {};

    template<typename T, size_t N>
    using size_specified_static_array_t
        = typename size_specified_static_array<T, N>::type
        ;

    template<typename T, size_t N>
    using size_specified_static_array_vt
        = typename size_specified_static_array<T, N>::value_type
        ;
}}

namespace kd
{
    /************************************************************************
     * interface
     ***********************************************************************/

    /* RGBA */

    template<typename T> inline constexpr auto   red(T       & m) -> decltype
    (detail::size_specified_static_array_vt<T, 4>(), m[0]  = 0, m[0])
    { return m[0]; }
    template<typename T> inline constexpr auto   red(T const & m) -> decltype
    (detail::size_specified_static_array_vt<T, 4>(), m[0] == 0, m[0])
    { return m[0]; }

    template<typename T> inline constexpr auto green(T       & m) -> decltype
    (detail::size_specified_static_array_vt<T, 4>(), m[1]  = 0, m[1])
    { return m[1]; }
    template<typename T> inline constexpr auto green(T const & m) -> decltype
    (detail::size_specified_static_array_vt<T, 4>(), m[1] == 0, m[1])
    { return m[1]; }

    template<typename T> inline constexpr auto  blue(T       & m) -> decltype
    (detail::size_specified_static_array_vt<T, 4>(), m[2]  = 0, m[2])
    { return m[2]; }
    template<typename T> inline constexpr auto  blue(T const & m) -> decltype
    (detail::size_specified_static_array_vt<T, 4>(), m[2] == 0, m[2])
    { return m[2]; }

    template<typename T> inline constexpr auto alpha(T       & m) -> decltype
    (detail::size_specified_static_array_vt<T, 4>(), m[3]  = 0, m[3])
    { return m[3]; }
    template<typename T> inline constexpr auto alpha(T const & m) -> decltype
    (detail::size_specified_static_array_vt<T, 4>(), m[3] == 0, m[3])
    { return m[3]; }

    /* RGB */

    template<typename T> inline constexpr auto   red(T       & m) -> decltype
    (detail::size_specified_static_array_vt<T, 3>(), m[0]  = 0, m[0])
    { return m[0]; }
    template<typename T> inline constexpr auto   red(T const & m) -> decltype
    (detail::size_specified_static_array_vt<T, 3>(), m[0] == 0, m[0])
    { return m[0]; }

    template<typename T> inline constexpr auto green(T       & m) -> decltype
    (detail::size_specified_static_array_vt<T, 3>(), m[1]  = 0, m[1])
    { return m[1]; }
    template<typename T> inline constexpr auto green(T const & m) -> decltype
    (detail::size_specified_static_array_vt<T, 3>(), m[1] == 0, m[1])
    { return m[1]; }

    template<typename T> inline constexpr auto  blue(T       & m) -> decltype
    (detail::size_specified_static_array_vt<T, 3>(), m[2]  = 0, m[2])
    { return m[2]; }
    template<typename T> inline constexpr auto  blue(T const & m) -> decltype
    (detail::size_specified_static_array_vt<T, 3>(), m[2] == 0, m[2])
    { return m[2]; }

    /* G */

    template<typename T> inline constexpr auto  gray(T       & m) -> decltype
    (detail::size_specified_static_array_vt<T, 1>(), m[0]  = 0, m[0])
    { return m[0]; }
    template<typename T> inline constexpr auto  gray(T const & m) -> decltype
    (detail::size_specified_static_array_vt<T, 1>(), m[0] == 0, m[0])
    { return m[0]; }

    template<typename T> inline constexpr auto  gray(T       & m) ->
    std::enable_if_t<std::is_arithmetic<T>::value, T       &>
    { return m; }
    template<typename T> inline constexpr auto  gray(T const & m) ->
    std::enable_if_t<std::is_arithmetic<T>::value, T const &>
    { return m; }
}

namespace kd { namespace detail
{
    /************************************************************************
     * or_type
     ***********************************************************************/

    template<typename... Ts> struct pack {};
    template<typename... Ts> struct make_void { using type = void; };
    template<typename... Ts> using  void_t = typename make_void<Ts...>::type;

    template
        < typename T
        , typename = void
        > struct is_using_type_base
        : std::false_type
    {};

    template
        < typename T
        > struct is_using_type_base<T, void_t<typename T::type>>
        : std::true_type
    {};

    template
        < typename T
        > struct is_using_type
        : is_using_type_base<T>
    {};

    template
        < typename T
        , typename = void
        > struct or_type_base {}
    ;

    template
        < typename    F
        , typename... T
        > struct or_type_base<pack<F, T...>, void_t<typename F::type>>
    {
        using type = F;
    };

    template
        < typename    F
        , typename... T
        > struct or_type_base
            < pack<F, T...>
            , std::enable_if_t
                < is_using_type<F>::value == false
                , void_t<typename or_type_base<pack<T...>>::type>
                >
            >
    {
        using type = typename or_type_base<pack<T...>>::type;
    };

    template
        < typename... Ts
        > struct or_type
        : or_type_base<pack<Ts...>>
    {};

    template<typename... Ts> using or_t = typename or_type<Ts...>::type;

    /* Reference:

    [SFINAE with variadic template classes?]
    (https://stackoverflow.com/a/13949007)
    [std::void_t (since C++17)]
    (http://en.cppreference.com/w/cpp/types/void_t)
    */
}}

namespace kd { namespace detail
{
    /************************************************************************
     * ?_scalar_trait
     ***********************************************************************/

    template<typename T, typename = void>
    struct rgba_scalar_trait_base {};

    template<typename T>
    struct rgba_scalar_trait_base<T, void_t<decltype(
          red(std::declval<T>()),
        green(std::declval<T>()),
         blue(std::declval<T>()),
        alpha(std::declval<T>()))
    >> {
        using type = T;
        using value_type = std::remove_cv_t<std::remove_reference_t<
            decltype(red(std::declval<T>()))
        >>;

        using rgba = rgba_scalar_trait_base;
        static constexpr size_t channel = 4;
    };

    template<typename T, typename = void>
    struct rgb_scalar_trait_base {};

    template<typename T>
    struct rgb_scalar_trait_base<T, void_t<decltype(
          red(std::declval<T>()),
        green(std::declval<T>()),
         blue(std::declval<T>()))
    >> {
        using type = T;
        using value_type = std::remove_cv_t<std::remove_reference_t<
            decltype(red(std::declval<T>()))
        >>;

        using rgb = rgb_scalar_trait_base;
        static constexpr size_t channel = 3;
    };

    template<typename T, typename = void>
    struct gray_scalar_trait_base {};

    template<typename T>
    struct gray_scalar_trait_base<T, void_t<decltype(
        gray(std::declval<T>()))
    >> {
        using type = T;
        using value_type = std::remove_cv_t<std::remove_reference_t<
            decltype(gray(std::declval<T>()))
        >>;
      
        using gray = gray_scalar_trait_base;
        static constexpr size_t channel = 1;
    };

    /************************************************************************
     * scalar_trait_base
     ***********************************************************************/

    template<typename T> using scalar_trait_base_list = or_t
        < rgba_scalar_trait_base<T>
        ,  rgb_scalar_trait_base<T>
        , gray_scalar_trait_base<T>
        >
    ;

    template
        < typename T
        , typename = void
        > struct scalar_trait_base {}
    ;

    template
        < typename T
        > struct scalar_trait_base<T, void_t<scalar_trait_base_list<T>>>
        : scalar_trait_base_list<T> {}
    ;
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
    using rgba_scalar_trait = typename scalar_trait<T>::rgba;

    template<typename T>
    using  rgb_scalar_trait = typename scalar_trait<T>:: rgb;

    template<typename T>
    using gray_scalar_trait = typename scalar_trait<T>::gray;
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
}}

namespace kd { namespace detail
{
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
    scalar_convert(T const & src, U & dst) -> decltype
    (gray_scalar_trait<T>(), rgb_scalar_trait<U>(), void())
    ```
    */
    {
        value_scale(gray(src), gray(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        gray_scalar_trait<T>::channel && rgb_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        value_scale(gray(src), blue(dst));
        red(dst) = green(dst) = blue(dst);
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        gray_scalar_trait<T>::channel && rgba_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        using namespace detail;
        value_scale(gray(src), blue(dst));
        red(dst) = green(dst) = blue(dst);
        aplha(dst) = value_clamp(std::numeric_limits<U>::max());
    }

    /* RGB */

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgb_scalar_trait<T>::channel && gray_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        auto r = 0.0, g = 0.0, b = 0.0;
        value_scale(  red(src), r);
        value_scale(green(src), g);
        value_scale( blue(src), b);
        value_scale(r * 0.299 + g * 0.587 + b * 0.114, gray(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgb_scalar_trait<T>::channel && rgb_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        value_scale(  red(src),   red(dst));
        value_scale(green(src), green(dst));
        value_scale( blue(src),  blue(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgb_scalar_trait<T>::channel && rgba_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        value_scale(  red(src),   red(dst));
        value_scale(green(src), green(dst));
        value_scale( blue(src),  blue(dst));
        dt::alpha(dst) = value_clamp(std::numeric_limits<U>::max());
    }
    
    /* RGBA */

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgba_scalar_trait<T>::channel && gray_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        auto r = 0.0, g = 0.0, b = 0.0;
        value_scale(  red(src), r);
        value_scale(green(src), g);
        value_scale( blue(src), b);
        value_scale(r * 0.299 + g * 0.587 + b * 0.114, gray(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgba_scalar_trait<T>::channel && rgb_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        value_scale(  red(src),  red(dst));
        value_scale(green(src),green(dst));
        value_scale( blue(src), blue(dst));
    }

    template<typename T, typename U> inline constexpr std::enable_if_t<
        rgba_scalar_trait<T>::channel && rgba_scalar_trait<U>::channel
    > scalar_convert(T const & src, U & dst)
    {
        value_scale(  red(src),   red(dst));
        value_scale(green(src), green(dst));
        value_scale( blue(src),  blue(dst));
        value_scale(alpha(src), alpha(dst));
    }
}}
