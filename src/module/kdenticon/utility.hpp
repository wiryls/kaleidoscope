#pragma once

#include <cstdint>
#include <limits>
#include <utility>
#include <type_traits>

namespace kd { namespace detail { namespace utility
{
    template<typename ...>
    struct voider { using type = void; };

    // [Specialize template based on whether a specific member exists]
    // (https://stackoverflow.com/a/30500870)
}}}

namespace kd
{
    using size = std::size_t;
}

namespace kd
{
    using std::true_type;
    using std::false_type;

    template<bool conditon, typename T = void>
    using enable_if = std::enable_if_t<conditon, T>;

    template <typename... Ts>
    using exist = typename detail::utility::voider<Ts...>::type;

    template<typename T>
    using limit = std::numeric_limits<T>;
}

namespace kd { namespace detail { namespace utility
{
    template <typename T, typename E = void>
    struct is_complete : false_type {};

    template <typename T>
    struct is_complete<T, enable_if<(sizeof(T) > 0)>> : true_type {};

    // [Check if type is defined]
    // (https://stackoverflow.com/a/39816909)
}}}

namespace kd
{
    template<typename T, typename U>
    constexpr bool is_same = std::is_same_v<T, U>;

    template<typename T, typename U>
    constexpr bool is_not_same = false == is_same<T, U>;

    template<typename T>
    constexpr bool is_pod = std::is_pod_v<T>;

    template<typename T>
    constexpr bool is_unsigned = std::is_unsigned<T>::value;

    template<typename T>
    constexpr bool is_floating = std::is_floating_point<T>::value;

    template<typename T>
    constexpr bool is_arithmetic = std::is_arithmetic<T>::value;

    template<typename T>
    constexpr bool is_complete = detail::utility::is_complete<T>::value;
}
