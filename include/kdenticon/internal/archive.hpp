#pragma once

#include <limits>
#include <cstdint>

#include <memory>
#include <fstream>

#include "binary.hpp"
#include "matrix_trait.hpp"
#include "scalar_trait.hpp"

namespace kd
{
    /************************************************************************
     * Save an RGBA bitmap to Stream or File
     ***********************************************************************/

    template<typename T, typename U> inline auto
    save(T & os, U const & mat) ->
    decltype(scalar_trait_vt<matrix_trait_rvt<U>>{}, bool())
    {
        using detail::binary::write;
        using detail::scalar_assign;

        // size check
        if ( width(mat) > std::numeric_limits<uint16_t>::max() ||
            height(mat) > std::numeric_limits<uint16_t>::max() )
            return false;

        // size
        static constexpr uint32_t bmph_size = 14;
        static constexpr uint32_t dibh_size = 108;
        static constexpr uint32_t head_size = bmph_size + dibh_size;
        auto w = static_cast<uint32_t>( width(mat));
        auto h = static_cast<uint32_t>(height(mat));
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
        `AliasedType is the (possibly cv-qualified) signed or unsigned
        variant of DynamicType.`
        
        References:
        [What is the strict aliasing rule?]
        (https://stackoverflow.com/a/7005988)
        */

        return true;
    }

    template<typename T> inline auto
    save(const char * path, T && mat) ->
    decltype(scalar_trait_vt<matrix_trait_rvt<T>>{}, bool())
    {
        std::ofstream ofs(path, std::ios::binary);
        return(ofs.is_open())
            ? (save(ofs, std::forward<T>(mat)))
            : (false)
            ;
    }
}
