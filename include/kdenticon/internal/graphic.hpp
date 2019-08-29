#pragma once

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <algorithm>

#include "geometry.hpp"
#include "matrix_trait.hpp"
#include "scalar_trait.hpp"

/***************************************************************************
 * bounding box
 ***************************************************************************/

namespace kd { namespace detail
{
    // AABB
    template<typename T>
    struct axis_aligned_bounding_box
    {
        T left, bottom, top, right;
    };

    // OBB
    template<typename T>
    struct oriented_bounding_box
    {
        vec2<T> vertex[4]; // counterclockwise, from a to b.
    };

    // simple variant of some bounding box.
    template<typename T>
    struct bounding_box
    {
        // union tag
        enum
        {
            aabb,
            obb,
        } tag;

        // union body
        union
        {
            axis_aligned_bounding_box<T> aabb;
                oriented_bounding_box<T> obb;
        } box;
    };

    template<typename T, typename U> inline void visit
    ( axis_aligned_bounding_box<T> const & box
    , U f
    ) {
        ;
    }

    template<typename T, typename U> inline std::array<vec2<T>, 4>
    build_bounding_box(vec2<T> const & a, vec2<T> const & b, U const & thick)
    {
        auto r = std::max(static_cast<T>(thick), T(1));
        auto n = norm(a - b);

        if (n <= r) {
            return
            {
                /* .tag = */ bounding_box<T>::aabb,
                /* .box = */ axis_aligned_bounding_box<T>
                {
                    std::min(a.x, b.x) - r / 2,
                    std::min(a.y, b.y) - r / 2,
                    std::max(a.y, b.y) + r / 2,
                    std::max(a.x, b.x) + r / 2,
                }
            };
        }
    }
}}

namespace kd { namespace detail { namespace graphic
{
    /************************************************************************
     * vec2
     ***********************************************************************/

    template<typename T> inline vec2<bool>
    sign(vec2<T> const & a, vec2<T> const & b)
    {
        return vec2<bool>
        {
            a.x >= b.x,
            a.y >= b.y,
        };
    }

    template<typename T> inline vec2<T>
    delta(vec2<T> const & a, vec2<T> const & b)
    {
        auto s = sign(a, b);
        return vec2<T>
        {
            s.x ? static_cast<T>(a.x - b.x) : static_cast<T>(b.x - a.x),
            s.y ? static_cast<T>(a.y - b.y) : static_cast<T>(b.y - a.y),
        };
    }

    template<typename T> inline std::pair<vec2<T>, vec2<bool>>
    delta_and_sign(vec2<T> const & a, vec2<T> const & b)
    {
        auto s = sign(a, b);
        auto d = vec2<T>
        {
            s.x ? static_cast<T>(a.x - b.x) : static_cast<T>(b.x - a.x),
            s.y ? static_cast<T>(a.y - b.y) : static_cast<T>(b.y - a.y),
        };

        return std::make_pair(d, s);
    }

    /************************************************************************
     * common
     ***********************************************************************/

    template<typename T, typename U> inline void
    bresenham(vec2<T> a, vec2<T> b, U f)
    {
        auto d = delta(a, b);

        auto swap = d.y > d.x;
        if (swap) {
            transpose(a);
            transpose(b);
            transpose(d);
        }

        if (a.x > b.x) {
            std::swap(a, b);
        }
        
        auto err = d.x >> 1;
        auto inc = a.y < b.y ? T(1) : T(-1);

        auto x = a.x;
        auto y = a.y;
        auto & xf = swap ? y : x;
        auto & yf = swap ? x : y;

        for (auto x_max = b.x; x <= x_max; x++) {
            f(xf, yf);

            if (err < d.y) {
                err += d.x;
                y   += inc;
            }

            err -= d.y;
        }

        /* Reference:
         *
         * [Bitmap/Bresenham's line algorithm]
         * (https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C.2B.2B)
         */
    }

    /************************************************************************
     * clipper
     ***********************************************************************/

    /// rect clipper
    template<typename T>
    struct rectangular_clipper
    {
    public:
        /*  H      J      K      L */
        T x_min, y_min, y_max, x_max;

    public:
        inline bool operator()(vec2<T> & a, vec2<T> & b) const;

    private:
        enum outcode_t : unsigned {
            inside = 0,
            left   = 1 << 0,
            right  = 1 << 1,
            bottom = 1 << 2,
            top    = 1 << 3,
        };

    private:
        inline outcode_t  to_outcode(vec2<T> const & v) const;
        inline bool cohen_sutherland(vec2<T> & a, vec2<T> & b) const;
    };

    template<typename T> inline bool rectangular_clipper<T>::
    operator()(vec2<T> & a, vec2<T> & b) const
    {
        return cohen_sutherland(a, b);
    }

    template<typename T> inline typename rectangular_clipper<T>::
    outcode_t rectangular_clipper<T>::
    to_outcode(vec2<T> const & v) const
    {
        unsigned code = inside;

        /**/ if (v.x > x_max)
            code |= right;
        else if (v.x < x_min)
            code |= left;

        /**/ if (v.y > y_max)
            code |= top;
        else if (v.y < y_min)
            code |= bottom;

        return static_cast<outcode_t>(code);
    }

    template<typename T> inline
    bool rectangular_clipper<T>::
    cohen_sutherland(vec2<T> & a, vec2<T> & b) const
    {
        auto u = a, v = b;

        auto outcode_u = to_outcode(u);
        auto outcode_v = to_outcode(v);
        bool accept = !(outcode_u | outcode_v);

        while (!accept &&!(outcode_u & outcode_v))
        {
            auto & outcode = inside  != outcode_u ? outcode_u : outcode_v;
            auto &       o = outcode == outcode_u ? u : v;

            auto p = delta_and_sign(u, v);
            auto s = p.second.x == p.second.y ? T(1) : T(-1);
            auto & d = p.first;

            /*--*/ if (outcode & right) {
                o.y -= d.y * (o.x - x_max) / d.x * s;
                o.x = x_max;
            } else if (outcode & top) {
                o.x -= d.x * (o.y - y_max) / d.y * s;
                o.y = y_max;
            } else if (outcode & bottom) {
                o.x += d.x * (y_min - o.y) / d.y * s;
                o.y = y_min;
            } else if (outcode & left) {
                o.y += d.y * (x_min - o.x) / d.x * s;
                o.x = x_min;
            }

            outcode = to_outcode(o);
            accept = !(outcode_u | outcode_v);
        }

        if (accept) {
            a = u;
            b = v;
        }

        return accept;

        /* Reference:
         *
         * [Cohen�CSutherland Algorithm]
         * (https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm)
         */
    }

}}}

namespace kd { namespace detail { namespace graphic
{
    /************************************************************************
     * line
     ***********************************************************************/

    using real = float;
    using rgba = real[4];
	static constexpr auto min = real(0);
	static constexpr auto max = real(1);

    inline void alpha_blend(rgba const & src, rgba const & dst, rgba & out)
    {
		using namespace detail::abbr;

        /* src over dst */

        R(out) = R(src) * A(src) + R(dst) * A(dst) * (max - A(src));
        G(out) = G(src) * A(src) + G(dst) * A(dst) * (max - A(src));
        B(out) = B(src) * A(src) + B(dst) * A(dst) * (max - A(src));
        A(out) = A(src) + A(dst) * (max - A(src));

        if (A(out) > min) {
            R(out) /= A(out);
            G(out) /= A(out);
            B(out) /= A(out);
        } else {
            R(out) = min;
            G(out) = min;
            B(out) = min;
            A(out) = min;
        }

        /* Reference:
         *
         * [Alpha compositing]
         * (https://en.wikipedia.org/wiki/Alpha_compositing#Alpha_blending)
         */
    }

    inline real capsule_sdf
    ( vec2<real> const & p
    , vec2<real> const & a
    , vec2<real> const & b
    , real thick
    ) {
        auto pa = p - a;
        auto ba = b - a;
		auto bas = ba * ba;

		auto d = pa;
		if (bas != real()) {
			d -= ba * std::max(std::min((pa * ba) / bas, max), min);
		}

		return std::sqrt(d * d) - thick;
    }

    template<typename T, typename U, typename V, typename W> inline void_t
    < scalar_trait_t<matrix_trait_wvt<T>>
    , scalar_trait_t<W>
	, value_like_t<V>
    > line0
    ( T             & mat
    , vec2<U> const & from
    , vec2<U> const & to
    , V               thick
    , W       const & color
    ) {
        auto w = static_cast<real>( width(mat));
        auto h = static_cast<real>(height(mat));

        rgba brush; scalar_assign(brush, color);
        rgba blend; scalar_assign(blend, brush);
        rgba pixel;

		auto a = saturate_cast<size_t>(from);
		auto b = saturate_cast<size_t>(to);
		auto r = static_cast<real>(thick) / 2.f;
        
		bresenham(a, b, [&mat, w, h, &brush, &blend, &pixel](size_t x, size_t y)
        {
			using namespace abbr;

            if (0 <= x && x < w && 0 <= y && y < h) {
                scalar_assign(pixel, at(mat, x, y));

                A(brush) = A(brush) * A(pixel);
                alpha_blend(blend, pixel, pixel);

                scalar_assign(at(mat, x, y), pixel);
            }
        });
    }

    template<typename T, typename U, typename V, typename W> inline void_t
    < scalar_trait_t<matrix_trait_wvt<T>>
    , scalar_trait_t<W>
	, value_like_t<V>
    > line
    ( T             & mat
    , vec2<U> const & from
    , vec2<U> const & to
    , V               thick
    , W       const & color
    ) {
		auto start  = saturate_cast<real>(from);
		auto end    = saturate_cast<real>(to);
		auto radius = static_cast<real>(thick) / 2.f;

        auto left   = std::floor(std::min(start.x, end.x) - radius);
        auto bottom = std::floor(std::min(start.y, end.y) - radius);
        auto right  = std:: ceil(std::max(start.x, end.x) + radius + real(1));
        auto top    = std:: ceil(std::max(start.y, end.y) + radius + real(1));

        auto w = static_cast<real>( width(mat));
        auto h = static_cast<real>(height(mat));

        auto x_min = static_cast<size_t>(std::min(std::max(  left, min), w));
        auto x_max = static_cast<size_t>(std::min(std::max( right, min), w));
        auto y_min = static_cast<size_t>(std::min(std::max(bottom, min), h));
        auto y_max = static_cast<size_t>(std::min(std::max(   top, min), h));

        rgba brush; scalar_assign(brush, color);
        rgba blend; scalar_assign(blend, brush);
        rgba pixel;

        for (auto y = y_min; y < y_max; ++y) {
            for (auto x = x_min; x < x_max; ++x) {
				using namespace abbr;
                auto pos = vec2<real>{static_cast<real>(x), static_cast<real>(y)};
                auto sdf = real(0.5) - capsule_sdf(pos, start, end, radius);
                A(blend) = A(brush) * std::min(std::max(sdf, min), max);

                scalar_assign(pixel, at(mat, x, y));
                alpha_blend  (blend, pixel, pixel);
                scalar_assign(at(mat, x, y), pixel);
            }
        }

        /* Reference:
         *
         * [TODO:]
         * (https://zhuanlan.zhihu.com/p/30553006)
         */
    }
}}}
