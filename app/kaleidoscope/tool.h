#pragma once
#include <type_traits>

namespace xerror
{
	template<typename T, typename U>
	auto inline operator >>(T && value, U && handle) -> std::invoke_result_t<U, T>
	{
		return std::forward<U>(handle)(std::forward<T>(value));
	}
}
