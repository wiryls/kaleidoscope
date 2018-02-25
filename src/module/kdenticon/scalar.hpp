#pragma once

#include "condition.hpp"
#include "utility.hpp"

namespace kd
{
    /// Channel
    enum class channel
    {
        G,
        BGR,
        BGRA,
    };
}

namespace kd { namespace detail
{
    /// channel_trait
    template<channel C>
    struct channel_trait;

    template<>
    struct channel_trait<channel::G>
    {
        static constexpr size depth = 1;
    };
    
    template<>
    struct channel_trait<channel::BGR>
    {
        static constexpr size depth = 3;
    };
    
    template<>
    struct channel_trait<channel::BGRA>
    {
        static constexpr size depth = 4;
    };
}}

namespace kd
{
    /// scalar
    template<typename T, channel C, typename E = void>
    struct scalar;
}

namespace kd { namespace detail
{
    /// scalar_operator
    template<typename T>
    struct scalar_operator;

    template<typename T, channel C>
    struct scalar_operator<scalar<T, C>>
    {
    protected:
        using derived = scalar<T, C>;
        using trait = channel_trait<C>;
        using type = T[trait::depth];

    public:
        using       reference = type       &;
        using const_reference = type const &;
        using         pointer = T       *;
        using   const_pointer = T const *;
        using        iterator =       pointer;
        using  const_iterator = const_pointer;

    protected:
        scalar_operator() = default;

    public:
        #ifndef WALK_AROUND_C2593
        constexpr operator const_reference() const;
        #else
        constexpr operator reference() const;
        #endif // WALK_AROUND_C2593
        
        constexpr const_iterator begin() const;
        constexpr const_iterator end() const;
        constexpr size empty() const;
        constexpr size size() const;
      
        #ifndef WALK_AROUND_C2593
        constexpr operator reference();
        #endif // WALK_AROUND_C2593

        constexpr iterator begin();
        constexpr iterator end();
    };

    #ifndef WALK_AROUND_C2593
    template<typename T, channel C>
    inline constexpr scalar_operator<scalar<T, C>>::
    operator const_reference() const
    {
        auto & it = static_cast<derived const &>(*this);
        return it.v;

        static_assert(scalar<T, C>::offset_guard(), "incorrect offset.");
        static_assert(is_same<decltype(it.v), type>, "incorrect type.");
    }
    #else
    template<typename T, channel C>
    inline constexpr scalar_operator<scalar<T, C>>::
    operator reference() const
    {
        auto & it = static_cast<derived const &>(*this);
        return const_cast<reference>(it.v);

        static_assert(scalar<T, C>::offset_guard(), "incorrect offset.");
        static_assert(is_same<decltype(it.v), type>, "incorrect type.");
    }
    #endif // WALK_AROUND_C2593

    template<typename T, channel C>
    inline constexpr typename
    scalar_operator<scalar<T, C>>::const_iterator
    scalar_operator<scalar<T, C>>::begin() const
    {
        auto & it = static_cast<derived const &>(*this);
        return it.v;
    }

    template<typename T, channel C>
    inline constexpr typename
    scalar_operator<scalar<T, C>>::const_iterator
    scalar_operator<scalar<T, C>>::end() const
    {
        return (*this).begin() + (*this).size();
    }

    template<typename T, channel C>
    inline constexpr size scalar_operator<scalar<T, C>>::empty() const
    {
        return false;
    }

    template<typename T, channel C>
    inline constexpr size scalar_operator<scalar<T, C>>::size() const
    {
        return trait::depth;
    }

    #ifndef WALK_AROUND_C2593
    template<typename T, channel C>
    inline constexpr scalar_operator<scalar<T, C>>::
    operator reference()
    {
        auto & it = static_cast<scalar_operator const &>(*this);
        return const_cast<reference>(it.operator const_reference());
    }
    #endif // WALK_AROUND_C2593

    template<typename T, channel C>
    inline constexpr typename
    scalar_operator<scalar<T, C>>::iterator
    scalar_operator<scalar<T, C>>::begin()
    {
        auto & it = static_cast<scalar_operator const &>(*this);
        return const_cast<iterator>(it.begin());
    }

    template<typename T, channel C>
    inline constexpr typename
    scalar_operator<scalar<T, C>>::iterator
    scalar_operator<scalar<T, C>>::end()
    {
        auto & it = static_cast<scalar_operator const &>(*this);
        return const_cast<iterator>(it.end());
    }
}}

namespace kd
{
    /// scalar - G
    template<typename T>
    struct scalar<T, channel::G, enable_if<is_pod<T>>>
        : public detail::scalar_operator
            <scalar<T, channel::G, enable_if<is_pod<T>>>>
    {
    private:
        using base = detail::scalar_operator<scalar>;
        using type = typename base::type;
        friend struct base;

    public:
        union
        {
            struct { T g; };
            type v;
        };

    private:
        static constexpr bool offset_guard();
    };

    /// scalar - BGR
    template<typename T>
    struct scalar<T, channel::BGR, enable_if<is_pod<T>>>
        : public detail::scalar_operator
            <scalar<T, channel::BGR, enable_if<is_pod<T>>>>
    {
    private:
        using base = detail::scalar_operator<scalar>;
        using type = typename base::type;
        friend struct base;

    public:
        union
        {
            struct { T b, g, r; };
            type v;
        };

    private:
        static constexpr bool offset_guard();
    };

    /// scalar - BGRA
    template<typename T>
    struct scalar<T, channel::BGRA, enable_if<is_pod<T>>>
        : public detail::scalar_operator
            <scalar<T, channel::BGRA, enable_if<is_pod<T>>>>
    {
    private:
        using base = detail::scalar_operator<scalar>;
        using type = typename base::type;
        friend struct base;

    public:
        union
        {
            struct { T b, g, r, a; };
            type v;
        };

    private:
        static constexpr bool offset_guard();
    };

    /// offset guards

    template<typename T>
    inline constexpr bool
    scalar<T, channel::G, enable_if<is_pod<T>>>::offset_guard()
    {
        return offsetof(scalar, g) == offsetof(scalar, v[0]);
    }

    template<typename T>
    inline constexpr bool
    scalar<T, channel::BGR, enable_if<is_pod<T>>>::offset_guard()
    {
        return offsetof(scalar, b) == offsetof(scalar, v[0])
            && offsetof(scalar, g) == offsetof(scalar, v[1])
            && offsetof(scalar, r) == offsetof(scalar, v[2])
            ;
    }

    template<typename T>
    inline constexpr bool
    scalar<T, channel::BGRA, enable_if<is_pod<T>>>::offset_guard()
    {
        return offsetof(scalar, b) == offsetof(scalar, v[0])
            && offsetof(scalar, g) == offsetof(scalar, v[1])
            && offsetof(scalar, r) == offsetof(scalar, v[2])
            && offsetof(scalar, a) == offsetof(scalar, v[3])
            ;
    }

}
