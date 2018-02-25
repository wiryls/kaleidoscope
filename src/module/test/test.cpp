#include <iostream>
#include <vector>
#include <cassert>

#include "value.hpp"
#include "scalar.hpp"
#include "matrix.hpp"

void test_value()
{
    using namespace kd;
    {
        value<uint8_t>  u08;
        assert(value<uint8_t>::min <= u08 && u08 <= value<uint8_t>::max);

        value<uint16_t> u16;
        assert(value<uint16_t>::min <= u16 && u16 <= value<uint16_t>::max);

        value<uint32_t> u32;
        assert(value<uint32_t>::min <= u32 && u32 <= value<uint32_t>::max);

        value<uint64_t> u64;
        assert(value<uint64_t>::min <= u64 && u64 <= value<uint64_t>::max);

        value<float> f32;
        assert(value<float>::min <= f32 && f32 <= value<float>::max);

        value<double> f64;
        assert(value<double>::min <= f64 && f64 <= value<double>::max);
    }
    {
        uint8_t src = 127;

        value<uint8_t> u08(src);
        assert(u08 == src);

        value<double> f64 = u08;
        assert(std::abs(f64 - double(src) / 255.0) < 1e-5);
    }
    {
        auto src = 127;
        value<uint8_t> u08;
        value<uint16_t> u16;

        u16 = src;
        assert(u16 == 127);

        u08 = src;
        assert(u08 == u16);

        u16 = u08;
        assert(u08 != u16);
    }
}

void test_scalar()
{
    using namespace kd;
    {
        scalar<value<uint16_t>, channel::G> s;

        s[0] = 1;
        assert(s.g == 1);
    }
    {
        scalar<value<uint32_t>, channel::BGRA> s;
        static const uint32_t sample[] = {
            0x11223344u, 0x44112233u, 0x33441122u, 0x22334411u
        };

        s[0] = sample[0];
        s[1] = sample[1];
        s[2] = sample[2];
        s[3] = sample[3];

        assert(s.size() == 4);
        assert(s.b == sample[0]);
        assert(s.g == sample[1]);
        assert(s.r == sample[2]);
        assert(s.a == sample[3]);
    }
    {
        scalar<value<uint8_t>, channel::BGR> s;

        assert(s.size() == 3);

        s.b = 0;
        s.g = 1;
        s.r = 2;

        auto index = 0;
        for (auto & e : s)
            assert(e == index++);
    }
}

void test_matrix()
{
    using namespace kd;
    matrix<std::vector<int>, channel::BGRA> m;

    //m[1, 2];
}

int main(void)
{
    test_value();
    test_scalar();
    test_matrix();
    
    std::cout << "test all passed!" << std::endl;
    return 0;
}
