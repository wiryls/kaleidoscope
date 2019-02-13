#pragma once

#include <cstddef>
#include <cstdint>
#include "matrix_trait.hpp"
#include "scalar_trait.hpp"

namespace kd
{
    /************************************************************************
     * shape
     ***********************************************************************/

    template<typename T>
    class shape
    {
    public:

        template<typename U, typename = scalar_trait_vt<matrix_trait_wvt<U>>>
        inline void draw(U & mat);

    protected:

        ~shape() = default;
    };

    template<typename T> template<typename U, typename> inline
    void shape<T>::draw(U & mat)
    {
        auto & it = static_cast<T &>(*this);
        return it.draw(mat);
    }
}

namespace kd
{
    /************************************************************************
     * line
     ***********************************************************************/

    class line
    {
    public:

        template<typename T, typename = scalar_trait_vt<T>>
        line
        ( float from_x
        , float from_y
        , float to_x
        , float to_y
        , float thick
        , T const & color
        );
    
    public:

        template<typename T, typename = scalar_trait_vt<T>>
        inline line & color(T const & c);

        inline line & thick(float t);

        inline line &  from(float x, float y);

        inline line &    to(float x, float y);

    private:

        float point_from[2];
        float point_to[2];
        float thickness;
        float color_rgba[4];
    };



    //class line : public shape<line>
    //{
    //public:

    //    template<typename T, typename = scalar_trait_vt<T>> line
    //    ( float x0
    //    , float y0
    //    , float x1
    //    , float y1
    //    , float thick
    //    , T const & color
    //    );

    //public:

    //    template<typename T, typename = scalar_trait_vt<T>>
    //    inline line & color(T const & c);

    //    inline line & thick(float t);

    //    inline line &  from(float x, float y);

    //    inline line &    to(float x, float y);

    //public:

    //    template<typename U, typename = scalar_trait_vt<matrix_trait_rvt<U>>>
    //    inline void draw(U & mat);

    //private:

    //    float xs[2];
    //    float ys[2];
    //    float th;
    //    uint8_t co[4];
    //};

    //template<typename T, typename>
    //inline line::line
    //    ( float x0
    //    , float y0
    //    , float x1
    //    , float y1
    //    , float t
    //    , T const & c
    //    )
    //    : xs{x0, x1}
    //    , ys{y0, y1}
    //    , th(t)
    //    , co()
    //{
    //    color(c);
    //}

    //template<typename T, typename>
    //inline line & line::color(T const & c)
    //{
    //    detail::scalar_assign(c, co);
    //    return *this;
    //}

    //inline line & line::thick(float t)
    //{
    //    th = t;
    //    return *this;
    //}

    //inline line & line::from(float x, float y)
    //{
    //    xs[0] = x;
    //    ys[0] = y;
    //    return *this;
    //}

    //inline line & line::to(float x, float y)
    //{
    //    xs[1] = x;
    //    ys[1] = y;
    //    return *this;
    //}

    //template<typename U, typename>
    //inline void line::draw(U & mat)
    //{
    //    auto capsule_sdf = []
    //    (float px, float py, float ax, float ay, float bx, float by, float r) {
    //        float pax = px - ax, pay = py - ay;
    //        float bax = bx - ax, bay = by - ay;
    //        float h = (pax * bax + pay * bay) / (bax * bax + bay * bay);
    //        float h = ;
    //        float dx = pax - bax * h, dy = pay - bay * h;
    //        return sqrtf(dx * dx + dy * dy) - r;
    //    };

    //    //void alphablend(int x, int y, float alpha, float r, float g, float b);

    //    ;
    //}

}
