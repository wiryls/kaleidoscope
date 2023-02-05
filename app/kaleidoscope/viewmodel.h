#pragma once
#include <chrono>
#include "model.h"

namespace viewmodel
{
    template<std::integral I>
    class state
    {
    private:
        using model_type = model::scoped_triangle<I>;
        using point_type = model_type::point;
        using timed_type = std::chrono::time_point<std::chrono::steady_clock>;

    public:
        using point = point_type;
        using vertices = model_type::vertices;

    public:
        auto on_start_moving(std::integral auto x, std::integral auto y) -> void
        {
            auto base = viewport.top();
            relative_position[0] = base[0] - static_cast<I>(x);
            relative_position[1] = base[1] - static_cast<I>(y);
            is_dragging = true;
        }

        auto on_moving(std::integral auto x, std::integral auto y) -> void
        {
            if (is_dragging)
            {
                viewport.move_to(
                    static_cast<I>(x) + relative_position[0],
                    static_cast<I>(y) + relative_position[1]);
            }
        }

        auto on_stop_moving() -> void
        {
            is_dragging = false;
        }

        auto on_length_changed(std::integral auto delta) -> void
        {
            using namespace std::chrono_literals;
            using time_unit = std::chrono::duration<I, std::milli>;
            auto static constexpr threshold = 333ms;
            auto static constexpr accelerate = std::array<std::pair<time_unit, I>, 6>
            {
                std::pair<time_unit, I>
                {1600ms, 13},
                {1000ms, 8},
                {600ms, 5},
                {400ms, 3},
                {200ms, 2},
                {0ms, 1}
            };

            auto factor = accelerate.back().second;
            if (auto now = std::chrono::steady_clock::now(); now - zooming_previous > threshold)
            {
                zooming_previous = now;
                zooming_beginning = now;
            }
            else
            {
                zooming_previous = now;

                for (auto x = now - zooming_beginning; auto && [k, y] : accelerate)
                {
                    if (x > k)
                    {
                        factor = y;
                        break;
                    }
                }
            }

            viewport.zoom(static_cast<I>(delta) * factor);
        }

        auto update_monitor_size(std::integral auto x, std::integral auto y) -> void
        {
            if (viewport.resize(x, y))
            {
                is_dragging = false;
                relative_position = {};
            }
        }

        auto is_moving() const -> bool
        {
            return is_dragging;
        }

        // Order: top, right, left
        auto viewport_vertices() const -> vertices const &
        {
            return viewport.positions();
        }

        auto triangle_top() const -> point
        {
            return viewport.top();
        }

        auto triangle_side_length() const -> I
        {
            return viewport.side();
        }

    private:
        // model
        model_type viewport{};

        // states
        bool       is_dragging{};
        point_type relative_position{};
        timed_type zooming_beginning{std::chrono::steady_clock::now()};
        timed_type zooming_previous{zooming_beginning};
    };
}
