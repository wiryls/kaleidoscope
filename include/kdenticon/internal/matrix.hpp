#pragma once

#include <cstddef>
#include <memory>
#include <algorithm>

#include "matrix_trait.hpp"

namespace kd
{
    /// matrix
    template<typename T>
    class matrix
    {
    public:
        using value_type      = T;
        using pointer         = value_type *;
        using reference       = value_type &;
        using const_pointer   = value_type const *;
        using const_reference = value_type const &;

    public:
        virtual ~matrix() = default;
        matrix();
        matrix(matrix      && rhs);
        matrix(matrix const & rhs);
        matrix(size_t w, size_t h);
        matrix(size_t w, size_t h, const_reference v);

    public:
        matrix & operator=(matrix      && rhs);
        matrix & operator=(matrix const & rhs);

    public:
        constexpr size_t width () const;
        constexpr size_t height() const;
        constexpr size_t size  () const;
        constexpr bool   empty () const;
        const_reference at(size_t x, size_t y) const;
              reference at(size_t x, size_t y);
    
    public:
        const_pointer begin() const;
              pointer begin();
        const_pointer   end() const;
              pointer   end();

    public:
        template<typename U, typename = std::enable_if_t<
            std::is_same<T, matrix_trait_vt<U>>::value
        >> matrix(U const & rhs);

        template<typename U, typename = std::enable_if_t<
            std::is_same<T, matrix_trait_vt<U>>::value
        >> matrix & operator=(U const & rhs);

    private:
        size_t wid;
        size_t hgt;
        std::unique_ptr<value_type[]> ptr;
    };
}

namespace kd
{
    template<typename T> inline matrix<T>::
    matrix()
        : wid(0)
        , hgt(0)
        , ptr()
    {}

    template<typename T> inline matrix<T>::
    matrix(matrix && rhs)
        : wid(rhs.wid)
        , hgt(rhs.hgt)
        , ptr(std::move(rhs.ptr))
    {
        rhs.wid = 0;
        rhs.hgt = 0;
    }

    template<typename T> inline matrix<T>::
    matrix(matrix const & rhs)
        : wid(rhs.wid)
        , hgt(rhs.hgt)
        , ptr(std::make_unique<value_type[]>(wid * hgt))
    {
        std::copy(rhs.begin(), rhs.end(), begin());
    }

    template<typename T> inline matrix<T>::
    matrix(size_t w, size_t h)
        : wid(w)
        , hgt(h)
        , ptr(std::make_unique<value_type[]>(w * h))
    {}

    template<typename T> inline matrix<T>::
    matrix(size_t w, size_t h, const_reference v)
        : wid(w)
        , hgt(h)
        , ptr(std::make_unique<value_type[]>(w * h))
    {
        std::fill(begin(), end(), v);
    }

    template<typename T> inline
    matrix<T> & matrix<T>::
    operator=(matrix && rhs)
    {
        wid = rhs.wid;
        hgt = rhs.hgt;
        ptr = std::move(rhs.ptr);
        rhs.wid = 0;
        rhs.hgt = 0;
        return *this;
    }

    template<typename T> inline
    matrix<T> & matrix<T>::
    operator=(matrix const & rhs)
    {
        wid = rhs.wid;
        hgt = rhs.hgt;
        ptr = std::make_unique<value_type[]>(size());
        std::copy(rhs.begin(), rhs.end(), begin());
        return *this;
    }

    template<typename T> inline constexpr
    size_t matrix<T>::
    width() const
    {
        return wid;
    }

    template<typename T> inline constexpr
    size_t matrix<T>::
    height() const
    {
        return hgt;
    }

    template<typename T> inline constexpr
    size_t matrix<T>::
    size() const
    {
        return wid * hgt;
    }

    template<typename T> inline constexpr
    bool matrix<T>::
    empty() const
    {
        return wid == 0 || hgt == 0;
    }

    template<typename T> inline
    typename matrix<T>::reference matrix<T>::
    at(size_t x, size_t y)
    {
        auto & it = static_cast<const matrix &>(*this);
        return const_cast<reference>(it.at(x, y));
    }

    template<typename T> inline
    typename matrix<T>::const_reference matrix<T>::
    at(size_t x, size_t y) const
    {
        return ptr[x + y * wid];
    }

    template<typename T> inline
    typename matrix<T>::pointer matrix<T>::
    begin()
    {
        auto & it = static_cast<const matrix &>(*this);
        return const_cast<pointer>(it.begin());
    }

    template<typename T> inline
    typename matrix<T>::const_pointer matrix<T>::
    begin() const
    {
        return ptr.get();
    }

    template<typename T> inline
    typename matrix<T>::pointer matrix<T>::
    end()
    {
        auto & it = static_cast<const matrix &>(*this);
        return const_cast<pointer>(it.end());
    }

    template<typename T> inline
    typename matrix<T>::const_pointer matrix<T>::
    end() const
    {
        return ptr.get() + wid * hgt;
    }

    template<typename T> template<typename U, typename> inline
    matrix<T>::
    matrix(U const & rhs)
        : wid(kd:: width(rhs))
        , hgt(kd::height(rhs))
        , ptr(std::make_unique<value_type[]>(wid * hgt))
    {
        for (size_t y = 0; y < wid; ++y)
            for (size_t x = 0; x < hgt; ++x)
                (*this).at(x, y) = kd::at(rhs, x, y);
    }

    template<typename T> template<typename U, typename> inline
    matrix<T> & matrix<T>::
    operator=(U const & rhs)
    {
        wid = kd:: width(rhs);
        hgt = kd::height(rhs);
        ptr = std::make_unique<value_type[]>(wid * hgt);

        for (size_t y = 0; y < wid; ++y)
            for (size_t x = 0; x < hgt; ++x)
                (*this).at(x, y) = kd::at(rhs, x, y);

        return *this;
    }
}
