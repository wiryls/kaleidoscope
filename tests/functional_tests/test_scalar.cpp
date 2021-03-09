#include <array>

#include <catch2/catch_test_macros.hpp>

#include <kdenticon/internal/scalar_trait.hpp>
#include <kdenticon/internal/scalar.hpp>

TEST_CASE("scalar method", "[scalar_trait]")
{
    using namespace kd;

      SECTION("int") {
        using type = int;
        type value = {};
        gray(value) = 1;
        REQUIRE(gray(value) == 1);
    } SECTION("int [1]") {
        using type = detail::size_specified_array_t<int[1], 1>;
        type value = {};
        gray(value) = 1;
        REQUIRE(gray(value) == 1);
    } SECTION("int [3]") {
        using type = detail::size_specified_array_t<int[3], 3>;
        type value = {};
        red(value) = 1;
        REQUIRE(red(value) == 1);
    }
}

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

        REQUIRE(gray_scalar_trait<scalar< uint8_t,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< uint8_t,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar< uint8_t, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<uint16_t,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar<uint16_t,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<uint16_t, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<uint32_t,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar<uint32_t,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<uint32_t, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<uint64_t,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar<uint64_t,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<uint64_t, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<  int8_t,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar<  int8_t,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<  int8_t, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar< int16_t,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< int16_t,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar< int16_t, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar< int32_t,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< int32_t,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar< int32_t, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar< int64_t,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar< int64_t,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar< int64_t, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<   float,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar<   float,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<   float, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<  double,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar<  double,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<  double, RGBA>>::channel == 4U);
        REQUIRE(gray_scalar_trait<scalar<    bool,    G>>::channel == 1U);
        REQUIRE( rgb_scalar_trait<scalar<    bool,  RGB>>::channel == 3U);
        REQUIRE(rgba_scalar_trait<scalar<    bool, RGBA>>::channel == 4U);

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
        using type = float[1];
        using trait = gray_scalar_trait<type>;
        type s = { 1.f };
        REQUIRE( gray(s)        == 1.f);
        REQUIRE((gray(s) = 2.f) == 2.f);
    } SECTION("rgb_scalar_trait<scalar<RGB, float>>") {
        using type = scalar<int, RGB>;
        using trait = rgb_scalar_trait<type>;
        type s = {0x12345678, 0x23456789, 0x34567890};
        REQUIRE(  red(s) == 0x12345678);
        REQUIRE(green(s) == 0x23456789);
        REQUIRE( blue(s) == 0x34567890);
        REQUIRE((  red(s) = 0x01020304) == 0x01020304);
        REQUIRE((green(s) = 0x02030405) == 0x02030405);
        REQUIRE(( blue(s) = 0x03040506) == 0x03040506);
    } SECTION("rgba_scalar_trait<uint64_t[4]>") {
        using type = uint64_t[4];
        using trait = rgba_scalar_trait<type>;
        type s = {1, 2, 3, 4};
        REQUIRE(  red(s) == 1ULL);
        REQUIRE(green(s) == 2ULL);
        REQUIRE( blue(s) == 3ULL);
        REQUIRE(alpha(s) == 4ULL);
        REQUIRE((  red(s) = 4ULL) == 4ULL);
        REQUIRE((green(s) = 3ULL) == 3ULL);
        REQUIRE(( blue(s) = 2ULL) == 2ULL);
        REQUIRE((alpha(s) = 1ULL) == 1ULL);
    }
}

namespace test
{
    template<typename T> using lim = std::numeric_limits<T>;
}

TEST_CASE("scalar_assign<gray, ...>(...)", "[scalar_trait]")
{
    using namespace kd;
    using namespace test;
    using detail::scalar_assign;

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
        REQUIRE((scalar_assign(u64, v), u64) == 0);
        REQUIRE((scalar_assign(u32, v), u32) == 0);
        REQUIRE((scalar_assign(u16, v), u16) == 0);
        REQUIRE((scalar_assign(u08, v), u08) == 0);
        REQUIRE((scalar_assign(i64, v), i64) == 0);
        REQUIRE((scalar_assign(i32, v), i32) == 0);
        REQUIRE((scalar_assign(i16, v), i16) == 0);
        REQUIRE((scalar_assign(i08, v), i08) == 0);
        REQUIRE((scalar_assign(f64, v), f64) == 0);
        REQUIRE((scalar_assign(f32, v), f32) == 0);
        REQUIRE((scalar_assign(c08, v), c08) == 0);
        REQUIRE((scalar_assign(b01, v), b01) == false);
    } SECTION("float") {
        auto v = 1.f;
        REQUIRE((scalar_assign(u64, v), u64) == lim<decltype(u64)>::max());
        REQUIRE((scalar_assign(u32, v), u32) == lim<decltype(u32)>::max());
        REQUIRE((scalar_assign(u16, v), u16) == lim<decltype(u16)>::max());
        REQUIRE((scalar_assign(u08, v), u08) == lim<decltype(u08)>::max());
        REQUIRE((scalar_assign(i64, v), i64) == lim<decltype(i64)>::max());
        REQUIRE((scalar_assign(i32, v), i32) == lim<decltype(i32)>::max());
        REQUIRE((scalar_assign(i16, v), i16) == lim<decltype(i16)>::max());
        REQUIRE((scalar_assign(i08, v), i08) == lim<decltype(i08)>::max());
        REQUIRE((scalar_assign(f64, v), f64) == 1.0);
        REQUIRE((scalar_assign(f32, v), f32) == 1.f);
        REQUIRE((scalar_assign(c08, v), c08) == lim<decltype(c08)>::max());
        REQUIRE((scalar_assign(b01, v), b01) == lim<decltype(b01)>::max());
    } SECTION("double") {
        auto v = 1.1;
        REQUIRE((scalar_assign(u64, v), u64) == lim<decltype(u64)>::max());
        REQUIRE((scalar_assign(u32, v), u32) == lim<decltype(u32)>::max());
        REQUIRE((scalar_assign(u16, v), u16) == lim<decltype(u16)>::max());
        REQUIRE((scalar_assign(u08, v), u08) == lim<decltype(u08)>::max());
        REQUIRE((scalar_assign(i64, v), i64) == lim<decltype(i64)>::max());
        REQUIRE((scalar_assign(i32, v), i32) == lim<decltype(i32)>::max());
        REQUIRE((scalar_assign(i16, v), i16) == lim<decltype(i16)>::max());
        REQUIRE((scalar_assign(i08, v), i08) == lim<decltype(i08)>::max());
        REQUIRE((scalar_assign(f64, v), f64) == 1.0);
        REQUIRE((scalar_assign(f32, v), f32) == 1.f);
        REQUIRE((scalar_assign(c08, v), c08) == lim<decltype(c08)>::max());
        REQUIRE((scalar_assign(b01, v), b01) == lim<decltype(b01)>::max());
    } SECTION("uint64_t") {
        auto constexpr v = lim<uint64_t>::max();
        REQUIRE((scalar_assign(u64, v), u64) == lim<decltype(u64)>::max());
        REQUIRE((scalar_assign(u32, v), u32) == lim<decltype(u32)>::max());
        REQUIRE((scalar_assign(u16, v), u16) == lim<decltype(u16)>::max());
        REQUIRE((scalar_assign(u08, v), u08) == lim<decltype(u08)>::max());
        REQUIRE((scalar_assign(i64, v), i64) == lim<decltype(i64)>::max());
        REQUIRE((scalar_assign(i32, v), i32) == lim<decltype(i32)>::max());
        REQUIRE((scalar_assign(i16, v), i16) == lim<decltype(i16)>::max());
        REQUIRE((scalar_assign(i08, v), i08) == lim<decltype(i08)>::max());
        REQUIRE((scalar_assign(f64, v), f64) == 1.0);
        REQUIRE((scalar_assign(f32, v), f32) == 1.f);
        REQUIRE((scalar_assign(c08, v), c08) == lim<decltype(c08)>::max());
        REQUIRE((scalar_assign(b01, v), b01) == lim<decltype(b01)>::max());
    }
}
