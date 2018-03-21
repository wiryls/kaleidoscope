#pragma once

namespace kd { namespace detail { namespace shape
{

}}}

namespace kd
{
    /// shape
    template<typename T>
    class shape
    {
    public:
        template<typename U> inline draw();
    };

    /// line
    class line : public shape<line>
    {

    };
}
