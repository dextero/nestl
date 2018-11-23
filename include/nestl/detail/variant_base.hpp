#pragma once

#include <cstddef>
#include <type_traits>

#include <nestl/utility.hpp>
#include <nestl/detail/storage.hpp>

#include <map>
#include <vector>

struct Dupa {
    bool constructed;
    const char *file;
    int line;
};

static std::map<const void *, std::vector<Dupa>> CONSTRUCTED;

namespace nestl {
namespace detail {

template <typename... Ts>
class variant_base
{
public:
    template <
        typename T,
        typename = std::enable_if_t<
            is_one_of<std::decay_t<T>, Ts...>
        >
    >
    variant_base(T&& t) noexcept
        : variant_base(tag<std::decay_t<T>>{},
                       std::forward<std::decay_t<T>>(t))
    {}

    template <typename T, typename... Args>
    variant_base(tag<T> tag, Args&&... args) noexcept
        : m_current(type_index<T, Ts...>),
          m_storage(tag, std::forward<Args>(args)...)
    {
        assert(CONSTRUCTED[this].empty() || !::CONSTRUCTED[this].back().constructed);
        static_assert(is_one_of<T, Ts...>);
        ::CONSTRUCTED[this].push_back({ true, __FILE__, __LINE__ });
    }

    variant_base(variant_base&& src) noexcept {
        *this = std::move(src);
    }

    variant_base& operator =(variant_base&& src) noexcept {
        if (this != &src) {
            src.move_into(*this);
        }
        return *this;
    }

    variant_base(const variant_base& src) noexcept {
        *this = src;
    }

    variant_base& operator =(const variant_base& src) noexcept {
        static_assert(all_of<std::is_copy_constructible, Ts...>);
        if (this != &src) {
            src.copy_into(*this);
        }
        return *this;
    }

    ~variant_base() noexcept {
        assert(invalid_type_index || (!CONSTRUCTED[this].empty() && ::CONSTRUCTED[this].back().constructed));
        destruct<0, Ts...>();
        m_current = invalid_type_index;
        ::CONSTRUCTED[this].push_back({ false, __FILE__, __LINE__ });
    }

    template <typename T>
    bool is() const noexcept {
        static_assert(is_one_of<T, Ts...>);
        return type_index<T, Ts...> == m_current;
    }

protected:
    size_t m_current = invalid_type_index;
    detail::storage<Ts...> m_storage;

    template <size_t N, typename T, typename... Rest>
    inline void destruct() noexcept {
        if (m_current == N) {
            m_storage.template destroy<T>();
        } else {
            destruct<N + 1, Rest...>();
        }
    }

    template <size_t>
    inline void destruct() {}

    inline void move_into(variant_base& dst) noexcept {
        move_into_impl<0, Ts...>(dst);
    }

    template <size_t N, typename T, typename... Rest>
    inline void move_into_impl(variant_base& dst) noexcept {
        if (m_current == N) {
            dst.~variant_base();
            new (&dst) variant_base(tag<T>{}, std::move(m_storage.template as<T>()));
            this->~variant_base();
            m_current = invalid_type_index;
        } else {
            move_into_impl<N + 1, Rest...>(dst);
        }
    }

    template <size_t>
    inline void move_into_impl(variant_base&) noexcept {}

    inline void copy_into(variant_base& dst) const noexcept {
        copy_into_impl<0, Ts...>(dst);
    }

    template <size_t N, typename T, typename... Rest>
    inline void copy_into_impl(variant_base& dst) const noexcept {
        if (m_current == N) {
            dst.~variant_base();
            new (&dst) variant_base(tag<T>{}, m_storage.template as<T>());
        } else {
            copy_into_impl<N + 1, Rest...>(dst);
        }
    }

    template <size_t>
    inline void copy_into_impl(variant_base&) const noexcept {}
};

template <typename... Ts>
class unchecked_variant final
    : public variant_base<Ts...>
{
public:
    template <typename... Args>
    unchecked_variant(Args&&... args) noexcept
        : variant_base<Ts...>(std::forward<Args>(args)...)
    {}

    template <typename T, typename... Args>
    static unchecked_variant emplace(Args&&... args) noexcept
    {
        return {
            tag<T>{},
            std::forward<Args>(args)...
        };
    }

    template <typename T>
    const T& get_unchecked() const noexcept
    {
        assert(!CONSTRUCTED[this].empty() && ::CONSTRUCTED[this].back().constructed);
        assert(this->template is<T>());
        return this->m_storage.template as<T>();
    }

    template <typename T>
    T&& get_unchecked() && noexcept
    {
        assert(!CONSTRUCTED[this].empty() && ::CONSTRUCTED[this].back().constructed);
        assert(this->template is<T>());
        return std::move(this->m_storage).template as<T>();
    }
};

} // namespace detail
} // namespace nestl
