#include <iostream>
#include <type_traits>
#include <memory>
#include <vector>
#include <array>
#include <utility>
#include <tuple>
#include <functional>
#include <cassert>

#include <catch/catch.hpp>
#include <kdenticon/kdenticon.hpp>

#include <kdenticon/internal/shape.hpp>
#include <kdenticon/internal/scalar.hpp>
#include <kdenticon/internal/matrix.hpp>
#include <kdenticon/internal/archive.hpp>
#include <kdenticon/internal/graphic.hpp>

// archive << pattern << shape << hash << text

namespace test
{
    struct canvas {};
    struct style  {};
    struct shape
    {
        using type = style;
    };

    struct drawable  {};

    canvas & operator << (canvas & c, drawable && p)
    {
        std::cout << "paint\n";
        return c;
    }

    canvas & operator << (canvas & c, shape & s)
    {
        drawable p;
        return c << std::move(p);
    }

    drawable && operator + (style & c, shape & s)
    {
        std::cout << "merge\n";
        drawable p;
        return std::move(p);
    }

    drawable && operator + (shape & s, style & c)
    {
        return c + s;
    }

    //template<typename T>
    //drawable && operator + (T & s, style<T> & c)
    //{
    //    return c + s;
    //}


    // draw(matrix, shape, style<shape> = default()));
}

TEST_CASE()
{
    //using namespace test;
    //canvas ca;
    //style st;
    //shape sh;
    //drawable pa;

    //ca
    //    << sh + st
    //    << sh + st
    //    ;

    // ca << line().from(x, y).to(x, y) + style().thick(t).color(c)
    // ca << line().from(x, y).to(x, y).with.thick(t).color(c)

    kd::matrix<kd::scalar<float, kd::RGBA>> mat(1024, 1024);
    auto w = static_cast<float>(mat. width());
    auto h = static_cast<float>(mat.height());
    
    float cx = w * 0.5f, cy = h * 0.5f;
    for (int j = 0; j < 5; j++) {
        float r1 = fminf(w, h) * (j + 0.5f) * 0.085f;
        float r2 = fminf(w, h) * (j + 1.5f) * 0.085f;
        float t = j * 3.14159f / 64.0f, r = (j + 1) * 0.5f;
        for (int i = 1; i <= 64; i++, t += 2.0f * 3.14159f / 64.0f) {
            float ct = cosf(t), st = sinf(t);
            kd::detail::graphic::line
            ( mat
            , kd::vec<float>{cx + r1 * ct, cy - r1 * st}
            , kd::vec<float>{cx + r2 * ct, cy - r2 * st}
            , r
            , kd::scalar<float, kd::RGBA>{0, 0, 0, 1}
            );
        }
    }

    kd::detail::graphic::line
    ( mat
    , kd::vec<float>{0, 0}
    , kd::vec<float>{1024, 1024}
    , 32.f
    , kd::scalar<float, kd::RGBA>{0.4f, 0.5f, 0.6f, 0.8f}
    );

    kd::save("test.bmp", mat);
}
