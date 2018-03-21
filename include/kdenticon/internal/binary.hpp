#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace kd { namespace detail { namespace binary
{
    /************************************************************************
     * write primitives to buffer
     ***********************************************************************/

    template<typename T> constexpr
    std::enable_if_t<std::is_integral<T>::value, uint8_t *>
    write(uint8_t * dst, T src)
    {
        auto cnt = sizeof(T);
        while (cnt --> 0) {
            *dst++ = static_cast<uint8_t>(src);
            src >>= 8;
        }
        return dst;
    }

    inline
    uint8_t *
    write(uint8_t * dst, double src)
    {
        uint64_t u64 = 0;
        std::memcpy(&u64, &src, sizeof(double));

        /* Note:
        Although type-punning via union works in most C++ compilers,
        still, it is UB.
         
        So instead of doing:
        ```
        union {
            double   f64 = 0.0;
            uint64_t u64;
        };
        f64 = val;
        return write(mem, u64);
        ```
        I choose `memcpy`.
        
        References:
        [Unions and type-punning]
        (https://stackoverflow.com/a/25672839)
        [Proper way of type-punning a float to an int and vice-versa]
        (https://stackoverflow.com/a/17790026)
        */

        return write(dst, u64);
    }

    inline
    uint8_t *
    write(uint8_t * dst, float src)
    {
        uint32_t u32 = 0;
        std::memcpy(&u32, &src, sizeof(float));
        return write(dst, u32);
    }

    template<typename T, typename ...U> inline
    uint8_t *
    write(uint8_t * dst, T src, U... res)
    {
        return write(write(dst, src), res...);
    }

    template<size_t N, typename T> inline
    uint8_t *
    write(uint8_t * dst, T src)
    {
        auto cnt = N;
        while (cnt --> 0)
            dst = write(dst, src);
        return dst;
    }

}}}
