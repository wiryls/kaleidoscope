#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>

namespace kd
{
    /************************************************************************
     * interface
     ***********************************************************************/

    /* size_t width(T const &) */

    template<typename T, size_t W, size_t H> inline constexpr auto
    width(T const (&)[H][W]) -> size_t { return W; }
    
    template<typename T> inline auto
    width(T const & m) -> decltype(m.width()) { return m.width(); }

    /* size_t height(T const &) */

    template<typename T, size_t W, size_t H> inline constexpr auto
    height(T const (&)[H][W]) -> size_t { return H; }
    
    template<typename T> inline auto
    height(T const & m) -> decltype(m.height()) { return m.height(); }

    /* value_type at(T const &, size_t, size_t) */

    template<typename T, size_t W, size_t H> inline constexpr auto
    at(T const (& m)[H][W], size_t x, size_t y) -> T const &
    { return m[y][x]; }
    
    template<typename T, size_t W, size_t H> inline constexpr auto
    at(T       (& m)[H][W], size_t x, size_t y) -> T       &
    { return m[y][x]; }

    /* value_type at(T const &, size_t, size_t) */

    template<typename T> inline auto
    at(T const & m, size_t x, size_t y) -> decltype(m.at(x, y))
    { return m.at(x, y); }
    
    template<typename T> inline auto
    at(T       & m, size_t x, size_t y) -> decltype(m.at(x, y))
    { return m.at(x, y); }

    /* References:
    [Argument-dependent lookup]
    (http://en.cppreference.com/w/cpp/language/adl)
    */
}

namespace kd { namespace detail
{
    /************************************************************************
     * matrix_trait_base
     ***********************************************************************/

    /* readable base */

    template<typename T>
    struct readable_matrix_trait_base
    {
        using readable = std::remove_reference_t<T>;
        static constexpr bool is_readable = true;
    };

    /* writable base */

    template<typename, typename = void>
    struct writable_matrix_trait_base
    {
        static constexpr bool is_writable = false;
    };

    template<typename T>
    struct writable_matrix_trait_base<T, std::enable_if_t<(
        !std::is_const                        <T> ::value &&
        !std::is_const<std::remove_reference_t<T>>::value
    )>> {
        using writable = std::remove_reference_t<T>;
        static constexpr bool is_writable = true;
    };

    /* base */

    template<typename T, typename = void>
    struct matrix_trait_base {};

    template<typename T>
    struct matrix_trait_base<T, decltype(
        height(std::declval<T>()),
         width(std::declval<T>()),
            at(std::declval<T>(), size_t{}, size_t{}),
        void()
    )>  : readable_matrix_trait_base<T>
        , writable_matrix_trait_base<T>
    {
        using type = T;
        using value_type = std::remove_cv_t<std::remove_reference_t<
            /* Note: `std::decay_t` will change `T[N]` to `T*`. */
            decltype(at(std::declval<T>(), size_t{}, size_t{}))
        >>;
    };
}}

namespace kd
{
    /************************************************************************
     * matrix_trait && export
     ***********************************************************************/

    template<typename T>
    struct matrix_trait : detail::matrix_trait_base<T> {};

    template<typename T>
    using matrix_trait_t = typename matrix_trait<T>::type;

    template<typename T>
    using matrix_trait_vt = typename matrix_trait<T>::value_type;

    template<typename T>
    using matrix_trait_rt = typename matrix_trait<T>::readable;

    template<typename T>
    using matrix_trait_wt = typename matrix_trait<T>::writable;

    template<typename T>
    using matrix_trait_rvt = matrix_trait_vt<matrix_trait_rt<T>>;

    template<typename T>
    using matrix_trait_wvt = matrix_trait_vt<matrix_trait_wt<T>>;
}
