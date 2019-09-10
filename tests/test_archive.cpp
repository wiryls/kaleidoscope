#include <cmath>
#include <sstream>
#include <iomanip>

#include <catch/catch.hpp>
#include <kdenticon/kdenticon.hpp>

#include <kdenticon/internal/shape.hpp>
#include <kdenticon/internal/scalar.hpp>
#include <kdenticon/internal/matrix.hpp>
#include <kdenticon/internal/archive.hpp>
#include <kdenticon/internal/graphic.hpp>

TEST_CASE("sample percentage bar")
{
	auto w = 416;
	auto h = 10;
	auto l = static_cast<float>(h / 2);
	auto r = static_cast<float>(w - l);
	auto y = static_cast<float>(h / 2);

	for (auto i = 0; i <= 100; i++)
	{
		auto m = kd::matrix<kd::scalar<float, kd::RGBA>>(w, h);
		kd::detail::graphic::line
		( m
		, kd::vec2<float>{l, y}
		, kd::vec2<float>{r, y}
		, h - 6
		, kd::scalar<uint8_t, kd::RGBA>{216, 216, 216, 255}
		);

		if (i != 0)
		{
			auto u = l + (r - l) * static_cast<float>(i - 1) / 99.f;
			kd::detail::graphic::line
			( m
			, kd::vec2<float>{l, y}
			, kd::vec2<float>{u, y}
			, h - 6
			, kd::scalar<uint8_t, kd::RGBA>{0, 84, 72, 255}
			);
		}

		auto o = std::ostringstream();
		o << "line\\teams-percentagebar-green-" << std::setfill('0') << std::setw(3) << i << ".bmp";
        REQUIRE(kd::save(o.str().c_str(), m) == kd::save_result::ok);
	}
}

TEST_CASE("archive::save")
{
	auto mat = kd::matrix<kd::scalar<float, kd::RGBA>>(1024, 1024);
	auto w = static_cast<float>(mat. width());
	auto h = static_cast<float>(mat.height());
	
	float cx = w * 0.5f, cy = h * 0.5f;
	for (int j = 0; j < 5; j++) {
	    float r1 = fminf(w, h) * (j + 0.5f) * 0.085f;
	    float r2 = fminf(w, h) * (j + 1.5f) * 0.085f;
	    float t = j * 3.14159f / 64.0f, r = (j + 1) * 1.f;
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
	, kd::vec<int>{0, 0}
	, kd::vec<int>{1024, 1024}
	, 64
	, kd::scalar<float, kd::RGBA>{0.4f, 0.5f, 0.6f, 0.8f}
	);

    //REQUIRE(kd::save("sample.lines.bmp", mat) == kd::save_result::ok);
}

TEST_CASE("bresenham")
{
	auto mat = kd::matrix<kd::scalar<float, kd::RGBA>>(1024, 1024);
	auto w = static_cast<float>(mat. width());
	auto h = static_cast<float>(mat.height());

	kd::detail::graphic::line0
	( mat
	, kd::vec<int>{0, 0}
	, kd::vec<int>{1024, 1024}
	, 64
	, kd::scalar<float, kd::RGBA>{0.4f, 0.5f, 0.6f, 0.8f}
	);

	//REQUIRE(kd::save("sample.bresenham.bmp", mat) == kd::save_result::ok);
}

TEST_CASE("binary::exact_writter_t")
{
    using kd::detail::exact_writter_t;
    using array_t = uint8_t[8];

    array_t u8 = {};
    auto ew = exact_writter_t<array_t, 0, 8>(u8)
        << uint16_t(123)
        << uint32_t(123)
        << uint16_t(123)
        ;

    auto a = exact_writter_t<array_t, 0, 8>(u8)
        << uint32_t(123)
        ;

    auto l = std::move(a)
        << uint16_t(123)
        << uint16_t(123)
        ;
    
    auto &i = ew;
    auto p = static_cast<array_t &>(i);
}
