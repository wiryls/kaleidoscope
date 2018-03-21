#include <iostream>
#include <type_traits>
#include <memory>
#include <vector>
#include <array>
#include <tuple>
#include <functional>
#include <cassert>

#include <catch/catch.hpp>
#include <kdenticon/kdenticon.hpp>

#include <kdenticon/internal/scalar.hpp>
#include <kdenticon/internal/matrix.hpp>
#include <kdenticon/internal/archive.hpp>
//#include "archive.hpp"
//#include "pattern.hpp"

// https://zhuanlan.zhihu.com/p/30553006

float capsule(float px, float py, float ax, float ay, float bx, float by, float r) {
    float pax = px - ax, pay = py - ay, bax = bx - ax, bay = by - ay;
    float h = fmaxf(fminf((pax * bax + pay * bay) / (bax * bax + bay * bay), 1.0f), 0.0f);
    float dx = pax - bax * h, dy = pay - bay * h;
    return sqrtf(dx * dx + dy * dy) - r;
}

float sample(float x, float y, int w, int h) {
    float s = 0.0f, cx = w * 0.5f, cy = h * 0.5f;
    for (int j = 0; j < 5; j++) {
        float r1 = fminf(w, h) * (j + 0.5f) * 0.085f;
        float r2 = fminf(w, h) * (j + 1.5f) * 0.085f;
        float t = j * 3.14159f / 64.0f, r = (j + 1) * 0.5f;
        for (int i = 1; i <= 64; i++, t += 2.0f * 3.14159f / 64.0f) {
            float ct = cosf(t), st = sinf(t);
            s = fmaxf(s, fminf(0.5f - capsule(x, y, cx + r1 * ct, cy - r1 * st, cx + r2 * ct, cy - r2 * st, r), 1.0f));
        }
    }
    return s;
}

// archive << pattern << shape << hash << text

TEST_CASE()
{
    //kd::scalar_trait<float const[3]>;

    kd::matrix<kd::scalar<kd::RGBA, float>> mat(1024, 1024);
    auto w = mat. width();
    auto h = mat.height();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            auto & c = mat.at(x, y);
            c.r = c.g = c.b = 1.0f - sample(x, y, w, h);
            c.a = 1.0f - c.r;
        }
    }
    kd::save("test.bmp", mat);
}
