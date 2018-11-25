#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <nestl/detail/storage.hpp>
#include <nestl/utility.hpp>

namespace nestl {
namespace detail {

template <typename... Ts>
class variant_base {
public:
    template <typename T,
              typename = std::enable_if_t<is_one_of<std::decay_t<T>, Ts...>>>
    variant_base(T&& t) noexcept
        : variant_base(tag<std::decay_t<T>>{},
                       std::forward<std::decay_t<T>>(t)) {}

    template <typename T, typename... Args>
    variant_base(tag<T> tag, Args&&... args) noexcept
        : m_current(type_index<T, Ts...>),
          m_storage(tag, std::forward<Args>(args)...) {
        static_assert(is_one_of<T, Ts...>);
    }

    variant_base(variant_base&& src) noexcept {
        static_assert(all_of<std::is_move_constructible, Ts...>);
        *this = std::move(src);
    }

    variant_base& operator=(variant_base&& src) noexcept {
        static_assert(all_of<std::is_move_constructible, Ts...>);
        if (this != &src) {
            src.move_into(*this);
        }
        return *this;
    }

    variant_base(const variant_base& src) noexcept {
        static_assert(all_of<std::is_copy_constructible, Ts...>);
        *this = src;
    }

    variant_base& operator=(const variant_base& src) noexcept {
        static_assert(all_of<std::is_copy_constructible, Ts...>);
        if (this != &src) {
            src.copy_into(*this);
        }
        return *this;
    }

    ~variant_base() noexcept {
        destruct<0, Ts...>();
        m_current = static_cast<uint8_t>(invalid_type_index);
    }

    template <typename T>
    [[nodiscard]] bool is() const noexcept {
        static_assert(is_one_of<T, Ts...>);
        return type_index<T, Ts...> == m_current;
    }

protected:
    uint8_t m_current = static_cast<uint8_t>(invalid_type_index);
    detail::storage<Ts...> m_storage;

    template <uint8_t N, typename T, typename... Rest>
    inline void destruct() noexcept {
        if (m_current == N) {
            m_storage.template destroy<T>();
        } else {
            destruct<N + 1, Rest...>();
        }
    }

    template <uint8_t>
    inline void destruct() {}

    inline void move_into(variant_base& dst) noexcept {
        move_into_impl<0, Ts...>(dst);
    }

    template <uint8_t N, typename T, typename... Rest>
    inline void move_into_impl(variant_base& dst) noexcept {
        if (m_current == N) {
            dst.~variant_base();
            new (&dst)
                variant_base(tag<T>{}, std::move(m_storage.template as<T>()));
            this->~variant_base();
            m_current = static_cast<uint8_t>(invalid_type_index);
        } else {
            static_assert(N < std::numeric_limits<uint8_t>::max());
            move_into_impl<N + 1, Rest...>(dst);
        }
    }

    template <uint8_t>
    inline void move_into_impl(variant_base&) noexcept {}

    inline void copy_into(variant_base& dst) const noexcept {
        copy_into_impl<0, Ts...>(dst);
    }

    template <uint8_t N, typename T, typename... Rest>
    inline void copy_into_impl(variant_base& dst) const noexcept {
        if (m_current == N) {
            dst.~variant_base();
            new (&dst) variant_base(tag<T>{}, m_storage.template as<T>());
            static_assert(N < std::numeric_limits<uint8_t>::max());
        } else {
            copy_into_impl<N + 1, Rest...>(dst);
        }
    }

    template <uint8_t>
    inline void copy_into_impl(variant_base&) const noexcept {}
};

template <typename... Ts>
class unchecked_variant final : public variant_base<Ts...> {
public:
    template <typename... Args>
    unchecked_variant(Args&&... args) noexcept
        : variant_base<Ts...>(std::forward<Args>(args)...) {}

    template <typename T, typename... Args>
    [[nodiscard]] static unchecked_variant emplace(Args&&... args) noexcept {
        return {tag<T>{}, std::forward<Args>(args)...};
    }

    template <typename T>
    [[nodiscard]] const T& get_unchecked() const noexcept {
        assert(this->template is<T>());
        return this->m_storage.template as<T>();
    }

    template <typename T>
        [[nodiscard]] T&& get_unchecked() && noexcept {
        assert(this->template is<T>());
        return std::move(this->m_storage).template as<T>();
    }
};

}  // namespace detail
}  // namespace nestl
