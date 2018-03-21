#pragma once

#include <cstddef>
#include <utility>
#include <type_traits>

namespace kd { namespace detail
{
    /************************************************************************
     * detail :: helper
     ***********************************************************************/

    template<typename T>
    using at_retval_t = decltype(std::declval<T>().at(size_t(), size_t()));

    /************************************************************************
     * detail :: readable & writable
     ***********************************************************************/

    template<typename T>
    struct readable_matrix_trait_base
    {
        using readable = T;
        static constexpr bool is_readable = true;

        static inline auto & at(T const & m, size_t x, size_t y)
        { return m.at(x, y); }
    };

    template<typename, typename = void>
    struct writable_matrix_trait_base
    {
        static constexpr bool is_writable = false;
    };

    template<typename T>
    struct writable_matrix_trait_base<T, std::enable_if_t<(
        !std::is_const                        <at_retval_t<T>> ::value &&
        !std::is_const<std::remove_reference_t<at_retval_t<T>>>::value
    )>> {
        using writable = T;
        static constexpr bool is_writable = true;

        static inline auto & at(T & m, size_t x, size_t y)
        { return m.at(x, y); }
    };

    template<typename T, size_t H, size_t W>
    struct readable_matrix_trait_sarray_base
    {
        using readable = T[H][W];
        static constexpr bool is_readable = true;

        static inline auto & at(T const (& m)[H][W], size_t x, size_t y)
        { return m[y][x]; }
    };

    template<typename, size_t, size_t, typename = void>
    struct writable_matrix_trait_sarray_base
    {
        static constexpr bool is_writable = false;
    };

    template<typename T, size_t H, size_t W>
    struct writable_matrix_trait_sarray_base<T, H, W, std::enable_if_t<(
        !std::is_const                        <T> ::value &&
        !std::is_const<std::remove_reference_t<T>>::value
    )>> {
        using writable = T[H][W];
        static constexpr bool is_writable = true;

        static inline auto & at(T (& m)[H][W], size_t x, size_t y)
        { return m[y][x]; }
    };

    template<typename, typename, typename, typename = void>
    struct accessible_matrix_trait_base {};

    template<typename T, typename R, typename W>
    struct accessible_matrix_trait_base<T, R, W, std::enable_if_t<
        R::is_readable == true &&
        W::is_writable == true
    >>: R, W
    {
        using R::at;
        using W::at;
    };

    template<typename T, typename R, typename W>
    struct accessible_matrix_trait_base<T, R, W, std::enable_if_t<
        R::is_readable == true &&
        W::is_writable == false
    >>: R, W
    {
        using R::at;
    };

    template<typename T, typename R, typename W>
    struct accessible_matrix_trait_base<T, R, W, std::enable_if_t<
        R::is_readable == false &&
        W::is_writable == true
    >>: R, W
    {
        using W::at;
    };

    /************************************************************************
     * detail :: width & height
     ***********************************************************************/

    template<typename, typename = void>
    struct matrix_trait_base {};

    template<typename T>
    struct matrix_trait_base<T, std::void_t<decltype(
        std::declval<T const>(). width() == size_t(),
        std::declval<T const>().height() == size_t(),
        std::declval<T const>().at(size_t(),size_t())
    )>>:accessible_matrix_trait_base<
        T,
        readable_matrix_trait_base<T>,
        writable_matrix_trait_base<T>
    > {
        using type = T;
        using value_type = std::remove_cv_t<std::remove_reference_t<
            /* Note: `std::decay_t` will change `T[N]` to `T*`. */
            at_retval_t<T>
        >>;

        static inline auto  width(T const & m) { return m. width(); }
        static inline auto height(T const & m) { return m.height(); }
    };

    template<typename T, size_t H, size_t W, typename = void>
    struct matrix_trait_sarray_base : accessible_matrix_trait_base<
        T[H][W],
        readable_matrix_trait_sarray_base<T, H, W>,
        writable_matrix_trait_sarray_base<T, H, W>
    > {
        using value_type = T;
        using type = value_type[H][W];

        static inline auto  width(T const (&)[H][W]) { return W; }
        static inline auto height(T const (&)[H][W]) { return H; }
    };
}}

namespace kd
{
    /************************************************************************
     * matrix_trait
     ***********************************************************************/

    template<typename T>
    struct matrix_trait
        : detail::matrix_trait_base<T>
    {};

    template<typename T, size_t W, size_t H>
    struct matrix_trait<T[H][W]>
        : detail::matrix_trait_sarray_base<T, H, W>
    {};

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
