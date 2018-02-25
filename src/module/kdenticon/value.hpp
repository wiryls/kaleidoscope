#pragma once

#include "utility.hpp"

namespace kd { namespace detail { namespace value
{
    /// trait
    template<typename T, typename E = void>
    struct trait : false_type {};

    template<typename T> struct trait<
        T, enable_if<is_unsigned<T>>
    > : true_type
    {
        static constexpr bool is_unsigned = true;
        static constexpr bool is_floating = false;
        static constexpr size digit = limit<T>::digits;
        static constexpr T min =  T();
        static constexpr T max = ~T();
    };

    template<typename T> struct trait<
        T, enable_if<is_floating<T>>
    > : true_type
    {
        static constexpr bool is_unsigned = false;
        static constexpr bool is_floating = true;
        static constexpr T min = T( );
        static constexpr T max = T(1);
    };

    /// cast
    template<typename T>
    constexpr enable_if<trait<T>::value, T>
    cast(T const & src)
    {
        return src;
    }

    template<typename T, typename U>
    constexpr enable_if<
        trait<T>::is_unsigned &&
        trait<U>::is_unsigned &&
        (trait<T>::digit > trait<U>::digit),
        T>
    cast(U const & src)
    {
        static constexpr auto shift = trait<T>::digit - trait<U>::digit;
        return static_cast<T>(src) << shift;
    }

    template<typename T, typename U>
    constexpr enable_if<
        trait<T>::is_unsigned &&
        trait<U>::is_unsigned &&
        (trait<U>::digit > trait<T>::digit),
        T>
    cast(U const & src)
    {
        static constexpr auto shift = trait<U>::digit - trait<T>::digit;
        return static_cast<T>(src >> shift);
    }

    template<typename T, typename U>
    constexpr enable_if<trait<T>::is_unsigned && trait<U>::is_floating, T>
    cast(U const & src)
    {
        using trait = trait<T>;
        static constexpr U scale = U(trait::max - trait::min);
        return static_cast<T>(src * scale);
    }

    template<typename T, typename U>
    constexpr enable_if<trait<T>::is_floating && trait<U>::is_unsigned, T>
    cast(U const & src)
    {
        using trait = trait<U>;
        static constexpr T scale = T(1) / T(trait::max - trait::min);
        return static_cast<T>(src) * scale;
    }

    template<typename T, typename U>
    constexpr enable_if<
        trait<T>::is_floating &&
        trait<U>::is_floating &&
        is_not_same<T, U>,
        T>
    cast(U const & src)
    {
        return static_cast<T>(src);
    }

    /// clamp
    template<typename T>
    constexpr enable_if<trait<T>::is_unsigned, T> clamp(T const & src)
    {
        return src;
    }

    template<typename T, typename U>
    constexpr enable_if<is_not_same<T, U> || !trait<T>::is_unsigned, T>
    clamp(U const & src)
    {
        using trait = trait<T>;
        return(src < trait::min)
            ? trait::min
            : (src > trait::max)
            ? trait::max
            : static_cast<T>(src)
            ;
    }

}}}

namespace kd
{
    template<typename T, typename = void>
    class value;

    template<typename T>
    class value<T, enable_if<detail::value::trait<T>::value>>
    {
    private:
        using type = T;

        template<typename U>
        using trait = detail::value::trait<U>;

        template<typename U>
        using other = value<U, enable_if<trait<U>::value>>;

    private:
        template<typename, typename>
        friend class value;

    public:
        static constexpr T max = trait<T>::max;
        static constexpr T min = trait<T>::min;

    public:
        constexpr value() = default;
        constexpr value(value const & rhs) = default;

        template<typename U>
        constexpr value(U const & rhs);

        template<typename U>
        constexpr value(other<U> const & rhs);

    public:
        constexpr operator type const & () const;

        constexpr value & operator =(value const & rhs) = default;

        template<typename U>
        constexpr enable_if<trait<U>::value, value &>
        operator = (U const & rhs);

        template<typename U>
        constexpr enable_if<trait<U>::value, value &>
        operator = (other<U> const & rhs);

    private:
        type field;
    };


    template<typename T>
    template<typename U>
    inline constexpr value<T, enable_if<detail::value::trait<T>::value>>::
    value(U const & rhs)
        : field(detail::value::clamp<T>(rhs))
    {}

    template<typename T>
    template<typename U>
    inline constexpr value<T, enable_if<detail::value::trait<T>::value>>::
    value(other<U> const & rhs)
        : field(detail::value::cast<T>(rhs.field))
    {}

    template<typename T>
    inline constexpr value<T, enable_if<detail::value::trait<T>::value>>::
    operator type const&() const
    {
        auto & it = const_cast<type &>(field);
        it = detail::value::clamp<type>(it);
        return it;
    }

    template<typename T>
    template<typename U>
    inline constexpr enable_if<
        detail::value::trait<U>::value,
        value<T, enable_if<detail::value::trait<T>::value>>
    & > value<T, enable_if<detail::value::trait<T>::value>>::
    operator=(U const & rhs)
    {
        field = detail::value::clamp<T>(rhs);
        return *this;
    }

    template<typename T>
    template<typename U>
    inline constexpr enable_if<
        detail::value::trait<U>::value,
        value<T, enable_if<detail::value::trait<T>::value>>
    & > value<T, enable_if<detail::value::trait<T>::value>>::
    operator= (other<U> const & rhs)
    {
        field = detail::value::cast<T>(rhs.field);
        return *this;
    }
}
