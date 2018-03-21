#include <array>

#include <catch/catch.hpp>
#include <kdenticon/internal/scalar_trait.hpp>
#include <kdenticon/internal/scalar.hpp>

template<typename T> using lim = std::numeric_limits<T>;

TEST_CASE("*_scalar_trait", "[scalar_trait]")
{
    using namespace kd;
    SECTION("channel") {
        REQUIRE(gray_scalar_trait< uint8_t[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait< uint8_t[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait< uint8_t[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait<uint16_t[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait<uint16_t[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait<uint16_t[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait<uint32_t[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait<uint32_t[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait<uint32_t[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait<uint64_t[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait<uint64_t[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait<uint64_t[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait<  int8_t[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait<  int8_t[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait<  int8_t[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait< int16_t[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait< int16_t[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait< int16_t[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait< int32_t[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait< int32_t[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait< int32_t[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait< int64_t[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait< int64_t[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait< int64_t[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait<   float[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait<   float[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait<   float[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait<  double[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait<  double[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait<  double[4]>::channel == 4U);
        REQUIRE(gray_scalar_trait<    bool[1]>::channel == 1U);
        REQUIRE( rgb_scalar_trait<    bool[3]>::channel == 3U);
        REQUIRE(rgba_scalar_trait<    bool[4]>::channel == 4U);

        REQUIRE(gray_scalar_trait<scalar<   G,  uint8_t>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB,  uint8_t>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA,  uint8_t>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G, uint16_t>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB, uint16_t>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA, uint16_t>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G, uint32_t>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB, uint32_t>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA, uint32_t>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G, uint64_t>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB, uint64_t>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA, uint64_t>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G,   int8_t>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB,   int8_t>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA,   int8_t>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G,  int16_t>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB,  int16_t>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA,  int16_t>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G,  int32_t>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB,  int32_t>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA,  int32_t>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G,  int64_t>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB,  int64_t>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA,  int64_t>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G,    float>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB,    float>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA,    float>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G,   double>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB,   double>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA,   double>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   G,     bool>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< RGB,     bool>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<RGBA,     bool>>::channel == 4U);

        REQUIRE(gray_scalar_trait<std::array<bool    , 1>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<std::array<bool    , 3>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<std::array<bool    , 4>>::channel == 4U);
        REQUIRE(gray_scalar_trait<std::array<uint64_t, 1>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<std::array<uint64_t, 3>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<std::array<uint64_t, 4>>::channel == 4U);
        REQUIRE(gray_scalar_trait<std::array<double  , 1>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<std::array<double  , 3>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<std::array<double  , 4>>::channel == 4U);
    } SECTION("gray_scalar_trait<float>") {
        using type = float;
        using trait = gray_scalar_trait<type>;
        type s = 1.f;
        REQUIRE( trait::gray(s)        == 1.f);
        REQUIRE((trait::gray(s) = 2.f) == 2.f);
    } SECTION("rgb_scalar_trait<scalar<RGB, float>>") {
        using type = scalar<RGB, int>;
        using trait = rgb_scalar_trait<type>;
        type s = {0x12345678, 0x23456789, 0x34567890};
        REQUIRE(trait::  red(s) == 0x12345678);
        REQUIRE(trait::green(s) == 0x23456789);
        REQUIRE(trait:: blue(s) == 0x34567890);
        REQUIRE((trait::  red(s) = 0x01020304) == 0x01020304);
        REQUIRE((trait::green(s) = 0x02030405) == 0x02030405);
        REQUIRE((trait:: blue(s) = 0x03040506) == 0x03040506);
    } SECTION("rgba_scalar_trait<uint64_t[4]>") {
        using type = uint64_t[4];
        using trait = rgba_scalar_trait<type>;
        type s = {1, 2, 3, 4};
        REQUIRE(trait::  red(s) == 1ULL);
        REQUIRE(trait::green(s) == 2ULL);
        REQUIRE(trait:: blue(s) == 3ULL);
        REQUIRE(trait::alpha(s) == 4ULL);
        REQUIRE((trait::  red(s) = 4ULL) == 4ULL);
        REQUIRE((trait::green(s) = 3ULL) == 3ULL);
        REQUIRE((trait:: blue(s) = 2ULL) == 2ULL);
        REQUIRE((trait::alpha(s) = 1ULL) == 1ULL);
    }
}

TEST_CASE("scalar_convert<gray, ...>(...)", "[scalar_trait]")
{
    using namespace kd;
    using detail::scalar_convert;

    uint64_t u64 = 0;
    uint32_t u32 = 0;
    uint16_t u16 = 0;
    uint8_t  u08 = 0;
    
    int64_t i64 = 0;
    int32_t i32 = 0;
    int16_t i16 = 0;
    int8_t  i08 = 0;

    double f64 = 0;
    float  f32 = 0;
    char   c08 = 0;
    bool   b01 = 0;

    SECTION("int") {
        auto v = -1;
        REQUIRE((scalar_convert(v, u64), u64) == 0);
        REQUIRE((scalar_convert(v, u32), u32) == 0);
        REQUIRE((scalar_convert(v, u16), u16) == 0);
        REQUIRE((scalar_convert(v, u08), u08) == 0);
        REQUIRE((scalar_convert(v, i64), i64) == 0);
        REQUIRE((scalar_convert(v, i32), i32) == 0);
        REQUIRE((scalar_convert(v, i16), i16) == 0);
        REQUIRE((scalar_convert(v, i08), i08) == 0);
        REQUIRE((scalar_convert(v, f64), f64) == 0);
        REQUIRE((scalar_convert(v, f32), f32) == 0);
        REQUIRE((scalar_convert(v, c08), c08) == 0);
        REQUIRE((scalar_convert(v, b01), b01) == false);
    } SECTION("float") {
        auto v = 1.f;
        REQUIRE((scalar_convert(v, u64), u64) == lim<decltype(u64)>::max());
        REQUIRE((scalar_convert(v, u32), u32) == lim<decltype(u32)>::max());
        REQUIRE((scalar_convert(v, u16), u16) == lim<decltype(u16)>::max());
        REQUIRE((scalar_convert(v, u08), u08) == lim<decltype(u08)>::max());
        REQUIRE((scalar_convert(v, i64), i64) == lim<decltype(i64)>::max());
        REQUIRE((scalar_convert(v, i32), i32) == lim<decltype(i32)>::max());
        REQUIRE((scalar_convert(v, i16), i16) == lim<decltype(i16)>::max());
        REQUIRE((scalar_convert(v, i08), i08) == lim<decltype(i08)>::max());
        REQUIRE((scalar_convert(v, f64), f64) == 1.0);
        REQUIRE((scalar_convert(v, f32), f32) == 1.f);
        REQUIRE((scalar_convert(v, c08), c08) == lim<decltype(c08)>::max());
        REQUIRE((scalar_convert(v, b01), b01) == lim<decltype(b01)>::max());
    } SECTION("double") {
        auto v = 1.1;
        REQUIRE((scalar_convert(v, u64), u64) == lim<decltype(u64)>::max());
        REQUIRE((scalar_convert(v, u32), u32) == lim<decltype(u32)>::max());
        REQUIRE((scalar_convert(v, u16), u16) == lim<decltype(u16)>::max());
        REQUIRE((scalar_convert(v, u08), u08) == lim<decltype(u08)>::max());
        REQUIRE((scalar_convert(v, i64), i64) == lim<decltype(i64)>::max());
        REQUIRE((scalar_convert(v, i32), i32) == lim<decltype(i32)>::max());
        REQUIRE((scalar_convert(v, i16), i16) == lim<decltype(i16)>::max());
        REQUIRE((scalar_convert(v, i08), i08) == lim<decltype(i08)>::max());
        REQUIRE((scalar_convert(v, f64), f64) == 1.0);
        REQUIRE((scalar_convert(v, f32), f32) == 1.f);
        REQUIRE((scalar_convert(v, c08), c08) == lim<decltype(c08)>::max());
        REQUIRE((scalar_convert(v, b01), b01) == lim<decltype(b01)>::max());
    } SECTION("uint64_t") {
        auto v = lim<uint64_t>::max();
        REQUIRE((scalar_convert(v, u64), u64) == lim<decltype(u64)>::max());
        REQUIRE((scalar_convert(v, u32), u32) == lim<decltype(u32)>::max());
        REQUIRE((scalar_convert(v, u16), u16) == lim<decltype(u16)>::max());
        REQUIRE((scalar_convert(v, u08), u08) == lim<decltype(u08)>::max());
        REQUIRE((scalar_convert(v, i64), i64) == lim<decltype(i64)>::max());
        REQUIRE((scalar_convert(v, i32), i32) == lim<decltype(i32)>::max());
        REQUIRE((scalar_convert(v, i16), i16) == lim<decltype(i16)>::max());
        REQUIRE((scalar_convert(v, i08), i08) == lim<decltype(i08)>::max());
        REQUIRE((scalar_convert(v, f64), f64) == 1.0);
        REQUIRE((scalar_convert(v, f32), f32) == 1.f);
        REQUIRE((scalar_convert(v, c08), c08) == lim<decltype(c08)>::max());
        REQUIRE((scalar_convert(v, b01), b01) == lim<decltype(b01)>::max());
    }

}
