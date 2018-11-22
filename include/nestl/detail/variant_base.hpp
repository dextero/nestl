#pragma once

#include <cstddef>
#include <type_traits>

#include <nestl/utility.hpp>

namespace nestl {
namespace detail {

template <typename... Args>
class storage {
public:
    alignas(max_alignment<Args...>) unsigned char data[max_sizeof<Args...>];

    storage() = default;

    storage(storage&&) = default;
    storage& operator =(storage&&) = default;

    storage(const storage&) = default;
    storage& operator =(const storage&) = default;

    template <typename T, typename... CtorArgs>
    storage(tag<T>, CtorArgs&&... args) {
        new (data) T(std::forward<CtorArgs>(args)...);
    }

    template <typename T>
    T& as() & {
        static_assert(is_one_of<T, Args...>);
        return *reinterpret_cast<T*>(data);
    }

    template <typename T>
    T&& as() && {
        static_assert(is_one_of<T, Args...>);
        return std::move(*reinterpret_cast<T*>(data));
    }

    template <typename T>
    const T& as() const {
        static_assert(is_one_of<std::remove_const_t<T>, Args...>);
        return *reinterpret_cast<const T*>(data);
    }

    template <typename T>
    void destroy() {
        destroy_impl<T, Args...>();
    }

private:
    template <typename Query, typename T, typename... Ts>
    void destroy_impl() {
        if (std::is_same_v<Query, T>) {
            as<T>().~T();
        } else {
            destroy_impl<Query, Ts...>();
        }
    }

    template <typename Query>
    void destroy_impl() {}
};

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
    variant_base(T&& t)
        : variant_base(tag<std::decay_t<T>>{},
                       std::forward<std::decay_t<T>>(t))
    {}

    template <typename T, typename... Args>
    variant_base(tag<T> tag, Args&&... args)
        : m_current(type_index<T, Ts...>),
          m_storage(tag, std::forward<Args>(args)...)
    {
        static_assert(is_one_of<T, Ts...>);
    }

    variant_base(variant_base&& src) noexcept {
        *this = std::move(src);
    }

    variant_base& operator =(variant_base&& src) noexcept {
        if (this != &src) {
            this->~variant_base();

            m_storage = std::move(src.m_storage);
            std::swap(m_current, src.m_current);
        }
        return *this;
    }

    variant_base(const variant_base& src) = default;
    variant_base& operator =(const variant_base& src) = default;

    ~variant_base() {
        destruct<0, Ts...>();
        m_current = invalid_type_index;
    }

    template <typename T>
    bool is() const {
        static_assert(is_one_of<T, Ts...>);
        return type_index<T, Ts...> == m_current;
    }

protected:
    size_t m_current = invalid_type_index;
    detail::storage<Ts...> m_storage;

    template <size_t N, typename T, typename... Rest>
    inline void destruct() {
        if (m_current == N) {
            m_storage.template destroy<T>();
        } else {
            destruct<N + 1, Rest...>();
        }
    }

    template <size_t>
    inline void destruct() {}
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
        assert(this->template is<T>());
        return this->m_storage.template as<T>();
    }

    template <typename T>
    T&& get_unchecked() && noexcept
    {
        assert(this->template is<T>());
        return std::move(this->m_storage).template as<T>();
    }
};

} // namespace detail
} // namespace nestl
