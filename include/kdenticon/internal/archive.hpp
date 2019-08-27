#pragma once

#include <limits>
#include <cctype>
#include <cstdint>
#include <cstring>

#include <memory>
#include <fstream>
#include <type_traits>

#include "matrix_trait.hpp"
#include "scalar_trait.hpp"

namespace kd { namespace detail
{
    /************************************************************************
     * helper
     ***********************************************************************/

    // `last_t<T, U..., Z>` is similar to `decltype(T{}, U...{}, Z{})`, and
    // the last type Z in will be returned.

    template<typename T, typename... U>
    struct last_type
    {
        using type = typename last_type<U...>::type;
    };

    template<typename T>
    struct last_type<T>
    {
        using type = T;
    };
    
    template<typename... T>
    using last_t = typename last_type<T...>::type;
}}

namespace kd
{
    /************************************************************************
     * save result
     ***********************************************************************/

    enum struct save_result
    {
        ok,
        stream_unavailable,
        format_not_supported,
        image_size_is_zero,
        image_size_too_large,
    };

    template<typename T>
    using save_result_t = typename detail::last_t
        < scalar_trait_vt<matrix_trait_rvt<T>>
        , save_result
        >
        ;
}

namespace kd
{
    /************************************************************************
     * Save an RGBA bitmap to Stream or File
     ***********************************************************************/

    template<typename T> inline
    save_result_t<T> save(const char * path, T && mat)
    {
        static auto constexpr size = 5;
        char type[size] = { 0 };
        {
            auto ext = std::strrchr(path, '.');
            if (ext != nullptr) {
                ext++;
                auto cnt = std::min(std::strlen(ext), size - size_t(1));
                std::memcpy(type, ext, cnt * sizeof(char));
                for (auto & c: type)
                    c = static_cast<char>(std::tolower(c));
            }
        }

        std::ofstream ofs(path, std::ios::binary);
        /**/ if (!ofs.is_open())
            return save_result::stream_unavailable;
        else if (!std::strcmp(type, "bmp"))
            return save_bmp4(ofs, std::forward<T>(mat));
        else
            return save_result::format_not_supported;
    }

    template<typename T, typename U> inline
    save_result_t<U> save_bmp4(T & os, U const & mat)
    {
        using detail::binary::write;
        using detail::scalar_assign;

        // size check
        auto w = static_cast<uint32_t>( width(mat));
        auto h = static_cast<uint32_t>(height(mat));
        {
            auto constexpr m = std::numeric_limits<uint16_t>::max();
            if (w > m || h > m)
                return save_result::image_size_too_large;
        }

        // size
        static constexpr uint32_t bmph_size = 14;
        static constexpr uint32_t dibh_size = 108;
        static constexpr uint32_t head_size = bmph_size + dibh_size;
        auto bimg_size = w * h * uint32_t(sizeof(uint32_t));
        auto size = head_size + bimg_size;

        // buffer
        auto buffer = std::make_unique<uint8_t[]>(size);
        auto cur = buffer.get();
        
        // write BMP Header
        cur = write<uint16_t>(cur, 0x4d42);
        cur = write<uint32_t>(cur, size);
        cur = write<uint16_t>(cur, 0);
        cur = write<uint16_t>(cur, 0);
        cur = write<uint32_t>(cur, head_size);

        // write DIB Header
        cur = write<uint32_t>(cur, dibh_size);  /* DIB header size */
        cur = write<uint32_t>(cur, w);          /* width */
        cur = write<uint32_t>(cur, h);          /* height */
        cur = write<uint16_t>(cur, 1);          /* color plane */
        cur = write<uint16_t>(cur, 32);         /* bits per pixel */
        cur = write<uint32_t>(cur, 3);          /* compression */
        cur = write<uint32_t>(cur, bimg_size);  /* bitmap size */
        cur = write<uint32_t>(cur, 3780);       /* 96 h-dpi */
        cur = write<uint32_t>(cur, 3780);       /* 96 v-dpi */
        cur = write<uint32_t>(cur, 0);          /* 0 colors in the palette */
        cur = write<uint32_t>(cur, 0);          /* important colors: all */
        cur = write<uint32_t>(cur, 0x00FF0000); /* BI_BITFIELDS: Mask R */
        cur = write<uint32_t>(cur, 0x0000FF00); /* BI_BITFIELDS: Mask G */
        cur = write<uint32_t>(cur, 0x000000FF); /* BI_BITFIELDS: Mask B */
        cur = write<uint32_t>(cur, 0xFF000000); /* BI_BITFIELDS: Mask A */
        cur = write<uint32_t>(cur, 0x206E6957); /* LCS_WINDOWS_COLOR_SPACE */
        cur = write<0x24>(cur, uint8_t ());     /* CIEXYZTRIPLE Color Space*/
        cur = write<0x03>(cur, uint32_t());     /* R,G,B Gamma */

        /* References:
        [BMP file format]
        (https://en.wikipedia.org/wiki/BMP_file_format)
        [BITMAPINFOHEADER structure]
        (https://msdn.microsoft.com/en-us/library/dd183376(v=vs.85).aspx)
        [BITMAPV4HEADER structure]
        (https://msdn.microsoft.com/en-us/library/dd183380(v=vs.85).aspx)
        */

        // write Pixels (starts with the lower left hand corner)
        for (size_t y = h - 1; y < h; --y) {
            for (size_t x = 0; x < w; ++x) {
                uint8_t u8x4[4];
                scalar_assign(u8x4, at(mat, x, y));
                uint32_t u32
                    = red  (u8x4) << 8 * 2
                    | green(u8x4) << 8 * 1
                    | blue (u8x4) << 8 * 0
                    | alpha(u8x4) << 8 * 3
                    ;
                cur = write(cur, u32);
            }
        }

        // write to file
        auto data = reinterpret_cast<char const *>(buffer.get());
        os.write(data, size);

        /* Note:
        > AliasedType is the (possibly cv-qualified) signed or unsigned
        > variant of DynamicType.
        
        References:
        [What is the strict aliasing rule?]
        (https://stackoverflow.com/a/7005988)
        */

        return save_result::ok;
    }

    template<typename T, typename U> inline
    save_result_t<T> save_png4(T & os, U const & mat)
    {
        using detail::binary::write;
        using detail::scalar_assign;

        auto w = static_cast<uint32_t>( width(mat));
        auto h = static_cast<uint32_t>(height(mat));
        {   // check size
            auto constexpr m = uint32_t(std::numeric_limits<int32_t>::max());
            if (w > m || h > m)
                return save_result::image_size_too_large;
            if (w == 0 || h == 0)
                return save_result::image_size_is_zero;
        }

        // TODO:
        // calc size

        // buffer
        auto buffer = std::make_unique<uint8_t[]>(0);
        auto cur = buffer.get();
        
        // write PNG signature
        cur = write<uint64_t>(cur, 0x0A1A0A0D474E5089); /* magic number */

        // write IHDR chunk
        cur = write<uint32_t>(cur, 13);                 /* data length */

        cur = write(cur, "IDHR");                       /* chunk type */

        cur = write<uint32_t>(cur, w);                  /* width */
        cur = write<uint32_t>(cur, h);                  /* height */
        cur = write<uint8_t>(cur, 8);                   /* bit depth */
        cur = write<uint8_t>(cur, 6);                   /* color type */
        cur = write<uint8_t>(cur, 0);                   /* compression */
        cur = write<uint8_t>(cur, 0);                   /* filter */
        cur = write<uint8_t>(cur, 0);                   /* interlace */

        cur = write<uint32_t>(cur, 0);                  /* CRC of data */

        // write IDAT chunk
        cur = write<uint32_t>(cur, 0);                  /* data length */
        cur = write(cur, "IDAT");                       /* chunk type */

        // write IEND chunk


        /* References:
        [File Structure](https://www.w3.org/TR/PNG-Structure.html)
        [Chunk Specifications](https://www.w3.org/TR/PNG-Chunks.html)
        */

        // write Pixels (starts with the lower left hand corner)

        // write to file
        auto data = reinterpret_cast<char const *>(buffer.get());

        return save_result::ok;
    }
}

namespace kd { namespace detail
{
    /************************************************************************
     * helper
     ***********************************************************************/



}}

namespace kd { namespace detail { namespace binary
{
    /************************************************************************
     * write primitives to buffer
     ***********************************************************************/

    template<typename T> constexpr
    std::enable_if_t<std::is_integral_v<T>, uint8_t *>
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
    uint8_t * write(uint8_t * dst, double src)
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
    uint8_t * write(uint8_t * dst, float src)
    {
        uint32_t u32 = 0;
        std::memcpy(&u32, &src, sizeof(float));
        return write(dst, u32);
    }

    template<size_t N, typename T> inline
    std::enable_if_t<std::is_arithmetic_v<T>, uint8_t *>
    write(uint8_t * dst, T src)
    {
        auto cnt = N;
        while (cnt --> 0)
            dst = write(dst, src);
        return dst;
    }

    template<size_t N> constexpr inline
    uint8_t * write(uint8_t * dst, char const (&src)[N])
    {
        auto cnt = N - size_t(1);
        auto ptr = src;
        while (cnt --> 0)
            *dst++ = static_cast<uint8_t>(*ptr++);
        return dst;
    }

    /************************************************************************
     * static output binary stream
     ***********************************************************************/

    template<size_t N> class sobs
    {
    private:
        /* a fixed size buffer */
        class buffer_t
        {
        public:
            buffer_t() noexcept;
           ~buffer_t() noexcept;
            buffer_t(buffer_t const& rhs) noexcept;
            inline buffer_t& operator=(buffer_t const& rhs) noexcept;
            inline operator const uint8_t* () const noexcept;
            inline void put(uint8_t c) noexcept;
            inline void clr() noexcept;
        private:
            inline void fin() noexcept;
        private:
            static constexpr size_t capacity = N + 1;
            uint8_t* cur;
            uint8_t* end;
            uint8_t  beg[capacity];
        };

    private:
        /* a counter_t for recording the rest size of buffer */
        template<size_t I> class counter_t
        {
        public:
            counter_t(buffer_t& buffer) noexcept;
            inline operator const uint8_t* () const noexcept;
        public:
            template<typename T> inline
            std::enable_if_t
                < std::is_integral_v<T> && sizeof(T) <= I && I <= N
                , counter_t<I - sizeof(T)>
                >
            operator|(T rhs) noexcept;

            template<size_t R> inline
            std::enable_if_t<(R <= I + 1 && I <= N), counter_t<I + 1 - R>>
            operator|(char const(&rhs)[R]) noexcept;

            inline std::enable_if_t< 4 <= I && I <= N, counter_t<I - 4>>
            operator|(float rhs) noexcept;

            inline std::enable_if_t< 8 <= I && I <= N, counter_t<I - 8>>
            operator|(double rhs) noexcept;

        private:
            buffer_t & buffer;
        };

    public:
        /* method */
        sobs() noexcept;

        template<typename T> inline auto
        operator|(T rhs) noexcept ->
        decltype(std::declval<counter_t<N>>() | rhs);
    };


}}}
