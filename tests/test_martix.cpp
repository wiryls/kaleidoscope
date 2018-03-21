#include <catch/catch.hpp>
#include <kdenticon/internal/matrix_trait.hpp>

TEST_CASE("matrix_trait", "matrix")
{
    SECTION("uint8_t const [10][20]") {
        using type  = uint8_t const [10][20];
        using trait = kd::matrix_trait<type>;
        type value = {};
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == false);
        REQUIRE(trait::width (value) == 20);
        REQUIRE(trait::height(value) == 10);
        REQUIRE(trait::at(value,0,0) == 0);
    }
    SECTION("float [20][10]") {
        using type  = float [20][10];
        using trait = kd::matrix_trait<type>;
        type value = {};
        trait::at(value, 0, 1) = 1;
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == true);
        REQUIRE(trait::width (value) == 10);
        REQUIRE(trait::height(value) == 20);
        REQUIRE(trait::at(value,0,1) == 1);
    }
    SECTION("uint32_t [2][1][4]") {
        using type  = uint32_t [2][1][4];
        using trait = kd::matrix_trait<type>;
        type value = {};
        trait::at(value, 1, 0)[2] = 1;
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == true);
        REQUIRE(trait::width (value) == 1);
        REQUIRE(trait::height(value) == 2);
        REQUIRE(trait::at(value,1,0)[2] == 1);
    }
}
