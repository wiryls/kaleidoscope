#pragma once

#include <limits>
#include <type_traits>

namespace kd { namespace detail
{
    /************************************************************************
     * saturate_cast helper
     ***********************************************************************/

    template<typename T, typename U>
    struct saturate_mixed
    {
        using type = decltype(std::declval<T>() + std::declval<U>());
    };

    template<typename T, typename U>
    using saturate_mixed_t = typename saturate_mixed<T, U>::type;

    template<typename T, typename U>
    constexpr bool is_same_sign_v =
        (std::is_signed  <T>::value && std::is_signed  <U>::value) !=
        (std::is_unsigned<T>::value && std::is_unsigned<U>::value)
        ;

    template<typename T>
    constexpr bool is_same_sign_v<T, T> =
        (std::is_signed<T>::value || std::is_unsigned<T>::value)
        ;

    template<typename T, typename U> constexpr
    T saturate_cast_impl(U src, T min, T max)
    {
        using safe_t = saturate_mixed_t<T, U>;
        auto mix_src = static_cast<safe_t>(src);
        auto mix_min = static_cast<safe_t>(min);
        auto mix_max = static_cast<safe_t>(max);

        return(mix_src >= mix_max) ? (max)
            : (mix_src <= mix_min) ? (min)
            : (static_cast<T>(src))
            ;
    }

    /************************************************************************
     * saturate_cast
     ***********************************************************************/

    template<typename T, typename U> constexpr
    std::enable_if_t<is_same_sign_v<T, U> == true, T>
    saturate_cast(U src)
    {
        return saturate_cast_impl<T>
        (
            src,
            std::numeric_limits<T>::min(),
            std::numeric_limits<T>::max()
        );
    }

    template<typename T, typename U> constexpr
    std::enable_if_t<is_same_sign_v<T, U> == false, T>
    saturate_cast(U src)
    {
        return saturate_cast_impl<T>(src, 0, std::numeric_limits<T>::max());
    }

}}
