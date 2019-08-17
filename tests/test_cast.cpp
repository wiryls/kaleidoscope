#include <utility>
#include <string>
#include <vector>

#include <catch/catch.hpp>
#include <kdenticon/internal/cast.hpp>

TEST_CASE("saturate_cast", "[cast]")
{
    using namespace kd::detail;

    SECTION("int8_t <- int8_t") {
        using x_t = int8_t;
        using y_t = int8_t;

        auto constexpr min = std::numeric_limits<y_t>::min();
        auto constexpr max = std::numeric_limits<y_t>::max();
        auto cs = std::vector<std::pair<x_t, y_t>>{
            {x_t(1), y_t(1)},
            {x_t(0), y_t(0)},
            {x_t(-1), y_t(-1)},

            {x_t(min), min},
            {x_t(min) + x_t(1), min + y_t(1)},
            {x_t(min) + x_t(2), min + y_t(2)},

            {x_t(max) - x_t(1), max - y_t(1)},
            {x_t(max) - x_t(2), max - y_t(2)},
            {x_t(max), max},
        };

        for (auto & c : cs)
            REQUIRE(saturate_cast<y_t>(c.first) == y_t(c.second));
    }

    SECTION("uint8_t <- int8_t") {
        using x_t = int8_t;
        using y_t = uint8_t;

        auto constexpr min = std::numeric_limits<y_t>::min();
        auto cs = std::vector<std::pair<x_t, y_t>>{
            {x_t(min), min},
            {x_t(min) + x_t(1), min + y_t(1)},
            {x_t(min) + x_t(2), min + y_t(2)},

            {x_t(0), y_t(0)},
            {x_t(1), y_t(1)},

            {x_t(127), y_t(127)},
            {x_t(126), y_t(126)},
        };

        for (auto & c : cs)
            REQUIRE(saturate_cast<y_t>(c.first) == y_t(c.second));
    }

    SECTION("int8_t <- uint8_t") {
        using x_t = uint8_t;
        using y_t = int8_t;

        auto cs = std::vector<std::pair<x_t, y_t>>{
            {x_t(0), y_t(0)},
            {x_t(1), y_t(1)},

            {x_t(127), y_t(127)},
            {x_t(128), y_t(127)},

            {x_t(254), y_t(127)},
            {x_t(255), y_t(127)},
        };

        for (auto & c : cs)
            REQUIRE(saturate_cast<y_t>(c.first) == y_t(c.second));
    }

    SECTION("int32_t <- float") {
        using x_t = float;
        using y_t = int32_t;
        
        auto constexpr min = std::numeric_limits<y_t>::min();
        auto constexpr max = std::numeric_limits<y_t>::max();
        auto cs = std::vector<std::pair<x_t, y_t>> {
            {x_t(min) - 1024.f, min},
            {x_t(min) - 1.f, min},
            {x_t(min), min},
            {x_t(min) + 1.f, min},
            {x_t(min) + 1024.f, min + y_t(1024)},

            {-1.f, -1},
            {0.f, 0},
            {1.f, 1},

            {x_t(max) - 1024.f, max - y_t(1023)},
            {x_t(max) - 1.f, max},
            {x_t(max), max},
            {x_t(max) + 1.f, max},
            {x_t(max) + 1024.f, max},
        };

        for (auto & c : cs)
            REQUIRE(saturate_cast<y_t>(c.first) == y_t(c.second));
    }

    SECTION("float <- uint32_t") {
        using x_t = uint32_t;
        using y_t = float;

        auto constexpr max = std::numeric_limits<x_t>::max();
        auto cs = std::vector<std::pair<x_t, y_t>>{
            {x_t(0), 0.f},
            {x_t(1), 1.f},

            {x_t(max - 1024.f), y_t(max - 1024)},
            {x_t(max), y_t(max - 4)},
            {x_t(max), y_t(max - 2)},
            {x_t(max), y_t(max - 1)},
            {x_t(max), y_t(max)},
        };

        for (auto & c : cs)
            REQUIRE(saturate_cast<y_t>(c.first) == y_t(c.second));
    }
}
