#pragma once
#include <type_traits>

namespace aux
{
    template<typename T, typename U>
    auto inline operator >>(T && value, U && handle) -> std::invoke_result_t<U, T>
    {
        return std::forward<U>(handle)(std::forward<T>(value));
    }
}

namespace env
{
    auto inline constexpr is_debug() -> bool
    {
#ifdef _DEBUG
        return true;
#else
        return false;
#endif
    }
}
