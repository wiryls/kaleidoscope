#pragma once
#include <algorithm>
#include <array>
#include <concepts>
#include <type_traits>
#include <utility>

namespace model
{

template <std::integral I> class scoped_triangle
{
public:
    using point    = std::array<I, 2>;
    using vertices = std::array<point, 3>;

public:
    scoped_triangle()
    {
        resize(1280, 720);
        update();
    }

    auto resize(std::integral auto width, std::integral auto height) -> bool
    {
        if (static_cast<decltype(width)>(screen_width) == width ||
            static_cast<decltype(height)>(screen_height) == height)
            return false;

        screen_width  = std::max(0., static_cast<double>(width));
        screen_height = std::max(0., static_cast<double>(height));

        top_x  = screen_width / 2.;
        top_y  = screen_height / 2.;
        length = std::min(top_y / sqrt3, top_x / 2.);

        update();
        return true;
    }

    auto move(std::integral auto dx, std::integral auto dy) -> void
    {
        move_to(top_x + dx, top_y + dy);
    }

    auto move_to(std::integral auto x, std::integral auto y) -> void
    {
        auto x_min = length / 2.;
        auto x_max = std::max(x_min, screen_width - x_min);
        auto y_min = 0.;
        auto y_max = std::max(y_min, screen_height - length / 2. * sqrt3);
        top_x      = std::clamp(static_cast<double>(x), x_min, x_max);
        top_y      = std::clamp(static_cast<double>(y), y_min, y_max);

        update();
    }

    auto zoom(std::integral auto d) -> void
    {
        zoom_to(static_cast<I>(std::floor(length)) + static_cast<I>(d));
    }

    auto zoom_to(std::integral auto s) -> void
    {
        auto lower_bound = 8.0;
        auto x_limit     = std::min(screen_width - top_x, top_x) * 2.;
        auto y_limit     = (screen_height - top_y) * 2. / sqrt3;
        auto upper_bound = std::max(lower_bound, std::min(x_limit, y_limit));
        length           = std::clamp(static_cast<double>(s), lower_bound, upper_bound);

        update();
    }

    auto top() const -> point
    {
        return {static_cast<I>(top_x), static_cast<I>(top_y)};
    }

    auto side() const -> I
    {
        return static_cast<I>(length);
    }

    auto positions() const -> vertices const &
    {
        return points;
    }

private:
    auto inline update() -> void
    {
        auto k    = length / 2.;
        auto x    = top_x;
        auto y    = top_y;
        points[0] = {static_cast<I>(x), static_cast<I>(y)};
        points[1] = {static_cast<I>(x + k), static_cast<I>(y + k * sqrt3)};
        points[2] = {static_cast<I>(x - k), points[1][1]};
    }

private:
    // constants
    double sqrt3{std::sqrt(3)};

    // properties
    double screen_width{};
    double screen_height{};
    double top_x{};
    double top_y{};
    double length{};

    // targets / output
    vertices points{};
};
} // namespace model
