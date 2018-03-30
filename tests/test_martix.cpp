#include <catch/catch.hpp>
#include <kdenticon/internal/matrix_trait.hpp>
#include <kdenticon/internal/matrix.hpp>

TEST_CASE("array like", "[matrix_trait]")
{
    using namespace kd;
    
    SECTION("uint8_t const [10][20]") {
        using type  = uint8_t const [10][20];
        using trait = matrix_trait<type>;
        type value = {};
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == false);
        REQUIRE( width(value) == 20);
        REQUIRE(height(value) == 10);
        REQUIRE(at(value,0,0) == 0);
    } SECTION("float [20][10]") {
        using type  = float [20][10];
        using trait = matrix_trait<type>;
        type value = {};
        at(value, 0, 1) = 1;
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == true);
        REQUIRE( width(value) == 10);
        REQUIRE(height(value) == 20);
        REQUIRE(at(value,0,1) == 1);
    } SECTION("uint32_t [2][1][4]") {
        using type  = uint32_t [2][1][4];
        using trait = matrix_trait<type>;
        type value = {};
        at(value, 1, 0)[2] = 1;
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == true);
        REQUIRE( width(value) == 1);
        REQUIRE(height(value) == 2);
        REQUIRE(    at(value,1,0)[2] == 1);
    } SECTION("matrix<uint8_t> const") {
        using type  = kd::matrix<uint8_t> const;
        using trait = matrix_trait<type>;
        type value(20, 10);
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == false);
        REQUIRE( width(value) == 20);
        REQUIRE(height(value) == 10);
        REQUIRE(at(value,0,0) == 0);
    } SECTION("matrix<float>") {
        using type  = kd::matrix<float>;
        using trait = matrix_trait<type>;
        type value(10, 20);
        at(value, 0, 1) = 1;
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == true);
        REQUIRE( width(value) == 10);
        REQUIRE(height(value) == 20);
        REQUIRE(at(value,0,1) == 1);
    } SECTION("matrix<uint32_t> [4]") {
        using type  = kd::matrix<uint32_t[4]>;
        using trait = matrix_trait<type>;
        type value(1, 2);
        at(value, 1, 0)[2] = 1;
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == true);
        REQUIRE( width(value) == 1);
        REQUIRE(height(value) == 2);
        REQUIRE(at(value,1,0)[2] == 1);
    }
}

namespace test
{
    struct mat2x2
    {
        int m[2][2];
    };

    auto  width(mat2x2 const & m) { return 2; }
    auto height(mat2x2 const & m) { return 2; }
    auto & at(mat2x2       & m, size_t x, size_t y) { return m.m[y][x]; }
    auto & at(mat2x2 const & m, size_t x, size_t y) { return m.m[y][x]; }

    class mat3x3
    {
    private:
        int m[3][3];
    public:
        auto  width() const { return 3; }
        auto height() const { return 3; }
        auto & at(size_t x, size_t y)       { return m[y][x]; }
        auto & at(size_t x, size_t y) const { return m[y][x]; }
    };
}

TEST_CASE("custom matrix", "[matrix_trait]")
{
    using namespace kd;

    SECTION("custom mat2x2") {
        using type  = test::mat2x2;
        using trait = matrix_trait<type>;
        type value;
        at(value, 1, 0) = 1;
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == true);
        REQUIRE( width(value) == 2);
        REQUIRE(height(value) == 2);
        REQUIRE(at(value,1,0) == 1);
    } SECTION("custom mat2x2 const") {
        using type  = test::mat2x2 const;
        using trait = matrix_trait<type>;
        type value = type();
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == false);
        REQUIRE( width(value) == 2);
        REQUIRE(height(value) == 2);
    } SECTION("custom mat3x3") {
        using type  = test::mat3x3;
        using trait = matrix_trait<type>;
        type value;
        at(value, 1, 0) = 1;
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == true);
        REQUIRE( width(value) == 3);
        REQUIRE(height(value) == 3);
        REQUIRE(at(value,1,0) == 1);
    } SECTION("custom mat3x3 const") {
        using type  = test::mat3x3 const;
        using trait = matrix_trait<type>;
        type value = type();
        REQUIRE(trait::is_readable == true);
        REQUIRE(trait::is_writable == false);
        REQUIRE( width(value) == 3);
        REQUIRE(height(value) == 3);
    }
}
