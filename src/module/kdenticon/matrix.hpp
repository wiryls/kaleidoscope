#pragma once

#include "scalar.hpp"

namespace kd { namespace detail
{

}}

namespace kd
{
    /// matrix
    template<typename T, channel C>
    class matrix
    {
    public:
        template<typename, channel> friend class matrix;
        template<typename>          friend class column;

    public:
        using scalar = scalar<T, C>;
        using column = column<scalar>;

    public:
        matrix();

    public:
        //column const & operator [](size x) const;
        //column       & operator [](size x);

    private:
        scalar * data_begin;
        scalar * data_end;
    };
    
    template<typename T, channel C>
    inline matrix<T, C>::matrix()
        : data_begin(nullptr)
        , data_end(data_begin)
    {}
}

namespace kd { namespace detail
{

}}

namespace kd
{

}

namespace kd
{
    /// column
    //template<typename T>
    //class column;

    //template<typename T, channel C>
    //class column<scalar<T, C>>
    //{
    //public:
    //    using         pointer = T       *;
    //    using   const_pointer = T const *;
    //    using        iterator =       pointer;
    //    using  const_iterator = const_pointer;

    //public:
    //    column              (column const & rhs) = default;
    //    column & operator = (column const & rhs) = default;

    //public:
    //    constexpr column(matrix       & mat, size x);
    //    constexpr column(matrix const & mat, size x);

    //public:
    //    constexpr operator const_pointer() const;
    //    constexpr const_iterator begin() const;
    //    constexpr const_iterator end() const;
    //    constexpr size empty() const;
    //    constexpr size size() const;
    //  
    //    constexpr operator pointer();
    //    constexpr iterator begin();
    //    constexpr iterator end();

    //private:
    //    const_pointer data_begin;
    //    const_pointer data_end;
    //};

    //template<typename T, channel C>
    //inline constexpr column<scalar<T, C>>::
    //column(const_pointer begin, const_pointer end)
    //    : data_begin(begin)
    //    , data_end(end)
    //{}

    //template<typename T, channel C>
    //inline constexpr column<scalar<T, C>>::operator const_pointer() const
    //{
    //    return data_begin;
    //}

    //template<typename T, channel C>
    //inline constexpr typename column<scalar<T, C>>::const_iterator
    //column<scalar<T, C>>::begin() const
    //{
    //    return data_begin;
    //}

    //template<typename T, channel C>
    //inline constexpr typename column<scalar<T, C>>::const_iterator
    //column<scalar<T, C>>::end() const
    //{
    //    return data_end;
    //}

    //template<typename T, channel C>
    //inline constexpr size column<scalar<T, C>>::empty() const
    //{
    //    return data_begin == data_end;
    //}

    //template<typename T, channel C>
    //inline constexpr size column<scalar<T, C>>::size() const
    //{
    //    return data_end > data_begin
    //        ?  data_end - data_begin
    //        :  0
    //        ;
    //}

    //template<typename T, channel C>
    //inline constexpr column<scalar<T, C>>::operator pointer()
    //{
    //    auto & it = static_cast<column const &>(*this);
    //    return const_cast<pointer>(it.operator const_pointer());
    //}

    //template<typename T, channel C>
    //inline constexpr typename column<scalar<T, C>>::iterator
    //column<scalar<T, C>>::begin()
    //{
    //    auto & it = static_cast<column const &>(*this);
    //    return const_cast<pointer>(it.begin());
    //}

    //template<typename T, channel C>
    //inline constexpr typename column<scalar<T, C>>::iterator
    //column<scalar<T, C>>::end()
    //{
    //    auto & it = static_cast<column const &>(*this);
    //    return const_cast<pointer>(it.end());
    //}
}
