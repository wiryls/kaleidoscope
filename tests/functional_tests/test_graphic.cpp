#include <utility>
#include <string>
#include <vector>
#include <tuple>

#include <catch2/catch_test_macros.hpp>

#include <kdenticon/internal/geometry.hpp>
#include <kdenticon/internal/graphic.hpp>

TEST_CASE("rectangular clipper", "[graphic]")
{
    using namespace kd::detail::graphic;
    using kd::vec2;

    SECTION("outside") {
        using type = uint16_t;
        using vec2 = vec2<type>;
        using clipper = rectangular_clipper<type>;
        using usecase = std::tuple<clipper, vec2, vec2, bool, vec2, vec2>;

        auto cs = std::vector<usecase>{
            // vertical, horizontal
            {
                { 10, 10, 30, 30 }, { 9, 31 }, { 9, 9 },
                false,              { 9, 31 }, { 9, 9 },
            },
            {
                { 10, 10, 30, 30 }, { 9, 9 }, { 31, 9 },
                false,              { 9, 9 }, { 31, 9 },
            },
            {
                { 10, 10, 30, 30 }, { 31, 9 }, { 31, 31 },
                false,              { 31, 9 }, { 31, 31 },
            },
            {
                { 10, 10, 30, 30 }, { 31, 31 }, { 9, 31 },
                false,              { 31, 31 }, { 9, 31 },
            },
            // point
            {
                { 10, 10, 30, 30 }, { 9, 9 }, { 9, 9 },
                false,              { 9, 9 }, { 9, 9 },
            },
            {
                { 10, 10, 30, 30 }, { 31, 31 }, { 31, 31 },
                false,              { 31, 31 }, { 31, 31 },
            },
            // slash
            {
                { 10, 10, 30, 30 }, { 4, 14 }, { 14, 4 },
                false,              { 4, 14 }, { 14, 4 },
            },
            {
                { 10, 10, 30, 30 }, { 26, 4 }, { 36, 14 },
                false,              { 26, 4 }, { 36, 14 },
            },
            {
                { 10, 10, 30, 30 }, { 36, 26 }, { 26, 36 },
                false,              { 36, 26 }, { 26, 36 },
            },
            {
                { 10, 10, 30, 30 }, { 4, 26 }, { 14, 36 },
                false,              { 4, 26 }, { 14, 36 },
            },
        };

        auto count = 0;
        for (auto const & c : cs) {
            auto clipper = std::get<0>(c);
            auto accept  = std::get<3>(c);

            SECTION("case " + std::to_string(count)) {
                auto a = std::get<1>(c);
                auto b = std::get<2>(c);
                auto u = std::get<4>(c);
                auto v = std::get<5>(c);

                REQUIRE(clipper(a, b) == accept);
                REQUIRE(a == u);
                REQUIRE(b == v);
            }

            SECTION("esac " + std::to_string(count)) {
                auto b = std::get<1>(c);
                auto a = std::get<2>(c);
                auto v = std::get<4>(c);
                auto u = std::get<5>(c);

                REQUIRE(clipper(a, b) == accept);
                REQUIRE(a == u);
                REQUIRE(b == v);
            }

            count++;
        }
    }

    SECTION("boundary") {
        using type = uint16_t;
        using vec2 = vec2<type>;
        using clipper = rectangular_clipper<type>;
        using usecase = std::tuple<clipper, vec2, vec2, bool, vec2, vec2>;

        auto cs = std::vector<usecase>{
            // longer
            {
                { 10, 10, 30, 30 }, {  5, 10 }, { 35, 10 },
                true,               { 10, 10 }, { 30, 10 },
            },
            {
                { 10, 10, 30, 30 }, { 30,  5 }, { 30, 35 },
                true,               { 30, 10 }, { 30, 30 },
            },
            {
                { 10, 10, 30, 30 }, { 35, 30 }, {  5, 30 },
                true,               { 30, 30 }, { 10, 30 },
            },
            {
                { 10, 10, 30, 30 }, { 10, 35 }, { 10,  5 },
                true,               { 10, 30 }, { 10, 10 },
            },
            // equal
            {
                { 10, 10, 30, 30 }, { 10, 10 }, { 30, 10 },
                true,               { 10, 10 }, { 30, 10 },
            },
            {
                { 10, 10, 30, 30 }, { 10, 10 }, { 30, 10 },
                true,               { 10, 10 }, { 30, 10 },
            },
            {
                { 10, 10, 30, 30 }, { 10, 10 }, { 30, 10 },
                true,               { 10, 10 }, { 30, 10 },
            },
            {
                { 10, 10, 30, 30 }, { 10, 10 }, { 30, 10 },
                true,               { 10, 10 }, { 30, 10 },
            },
            // shorter
            {
                { 10, 10, 30, 30 }, { 15, 10 }, { 25, 10 },
                true,               { 15, 10 }, { 25, 10 },
            },
            {
                { 10, 10, 30, 30 }, { 30, 15 }, { 30, 25 },
                true,               { 30, 15 }, { 30, 25 },
            },
            {
                { 10, 10, 30, 30 }, { 25, 30 }, { 15, 30 },
                true,               { 25, 30 }, { 15, 30 },
            },
            {
                { 10, 10, 30, 30 }, { 10, 25 }, { 10, 15 },
                true,               { 10, 25 }, { 10, 15 },
            },
        };

        auto count = 0;
        for (auto const & c : cs) {
            auto clipper = std::get<0>(c);
            auto accept = std::get<3>(c);

            SECTION("case " + std::to_string(count))
            {
                auto a = std::get<1>(c);
                auto b = std::get<2>(c);
                auto u = std::get<4>(c);
                auto v = std::get<5>(c);

                REQUIRE(clipper(a, b) == accept);
                REQUIRE(a == u);
                REQUIRE(b == v);
            }

            SECTION("esac " + std::to_string(count))
            {
                auto b = std::get<1>(c);
                auto a = std::get<2>(c);
                auto v = std::get<4>(c);
                auto u = std::get<5>(c);

                REQUIRE(clipper(a, b) == accept);
                REQUIRE(a == u);
                REQUIRE(b == v);
            }

            count++;
        }
    }
    
    SECTION("inside") {
        using type = uint16_t;
        using vec2 = vec2<type>;
        using clipper = rectangular_clipper<type>;
        using usecase = std::tuple<clipper, vec2, vec2, bool, vec2, vec2>;

        auto cs = std::vector<usecase>{
            // vertical, horizontal
            {
                { 10, 10, 30, 30 }, {  0, 20 }, { 40, 20 },
                true,               { 10, 20 }, { 30, 20 },
            },
            {
                { 10, 10, 30, 30 }, { 20,  0 }, { 20, 40 },
                true,               { 20, 10 }, { 20, 30 },
            },
            {
                { 10, 10, 30, 30 }, { 15, 20 }, { 25, 20 },
                true,               { 15, 20 }, { 25, 20 },
            },
            {
                { 10, 10, 30, 30 }, { 20, 15 }, { 20, 25 },
                true,               { 20, 15 }, { 20, 25 },
            },
            // slash
            {
                { 10, 10, 30, 30 }, {  0,  0 }, { 40, 40 },
                true,               { 10, 10 }, { 30, 30 },
            },
            {
                { 10, 10, 30, 30 }, {  0, 40 }, { 40,  0 },
                true,               { 10, 30 }, { 30, 10 },
            },
            {
                { 10, 10, 30, 30 }, {  5, 20 }, { 20, 35 },
                true,               { 10, 25 }, { 15, 30 },
            },
            {
                { 10, 10, 30, 30 }, { 20, 35 }, { 35, 20 },
                true,               { 25, 30 }, { 30, 25 },
            },
            {
                { 10, 10, 30, 30 }, { 35, 20 }, { 20,  5 },
                true,               { 30, 15 }, { 25, 10 },
            },
            {
                { 10, 10, 30, 30 }, { 20,  5 }, {  5, 20 },
                true,               { 15, 10 }, { 10, 15 },
            },
            // point
            {
                { 10, 10, 30, 30 }, { 11,  9 }, {  9, 11 },
                true,               { 10, 10 }, { 10, 10 },
            },
            {
                { 10, 10, 30, 30 }, { 29,  9 }, { 31, 11 },
                true,               { 30, 10 }, { 30, 10 },
            },
            {
                { 10, 10, 30, 30 }, { 29, 31 }, { 31, 29 },
                true,               { 30, 30 }, { 30, 30 },
            },
            {
                { 10, 10, 30, 30 }, {  9, 29 }, { 11, 31 },
                true,               { 10, 30 }, { 10, 30 },
            },
        };

        auto count = 0;
        for (auto const & c : cs) {
            auto clipper = std::get<0>(c);
            auto accept = std::get<3>(c);

            SECTION("case " + std::to_string(count))
            {
                auto a = std::get<1>(c);
                auto b = std::get<2>(c);
                auto u = std::get<4>(c);
                auto v = std::get<5>(c);

                REQUIRE(clipper(a, b) == accept);
                REQUIRE(a == u);
                REQUIRE(b == v);
            }

            SECTION("esac " + std::to_string(count))
            {
                auto b = std::get<1>(c);
                auto a = std::get<2>(c);
                auto v = std::get<4>(c);
                auto u = std::get<5>(c);

                REQUIRE(clipper(a, b) == accept);
                REQUIRE(a == u);
                REQUIRE(b == v);
            }

            count++;
        }
    }
}
