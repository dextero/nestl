#pragma once

#include <cstddef>
#include <type_traits>

#include <nestl/detail/variant_base.hpp>
#include <nestl/result.hpp>
#include <nestl/utility.hpp>

namespace nestl {
struct variant_type_error {};

template <typename... Ts>
struct variant final : public detail::variant_base<Ts...> {
public:
    template <typename... Args>
    variant(Args&&... args) noexcept
        : detail::variant_base<Ts...>(std::forward<Args>(args)...) {}

    template <typename T, typename... Args>
    [[nodiscard]] static variant emplace(Args&&... args) noexcept {
        return {tag<T>{}, std::forward<Args>(args)...};
    }

    template <typename T>
        [[nodiscard]] result<std::reference_wrapper<T>, variant_type_error>
        get() & noexcept {
        static_assert(is_one_of<T, Ts...>);
        return get_impl<T, 0, Ts...>();
    }

    template <typename T>
        [[nodiscard]] result<std::reference_wrapper<T>, variant_type_error>
        get() && noexcept {
        static_assert(is_one_of<T, Ts...>);
        return get_impl<T, 0, Ts...>();
    }

    template <typename T>
    [[nodiscard]] result<std::reference_wrapper<const T>, variant_type_error>
    get() const noexcept {
        static_assert(is_one_of<T, Ts...>);
        return const_cast<variant*>(this)->get_impl<const T, 0, Ts...>();
    }

private:
    template <typename T, size_t N, typename First, typename... Rest>
    [[nodiscard]] inline result<std::reference_wrapper<T>, variant_type_error>
    get_impl() noexcept {
        if (this->m_current == N) {
            if (std::is_same_v<std::remove_const_t<T>, First>) {
                return {std::reference_wrapper<T>{
                    this->m_storage.template as<std::remove_const_t<T>>()}};
            } else {
                return {variant_type_error{}};
            }
        } else {
            return get_impl<T, N + 1, Rest...>();
        }
    }

    template <typename T, size_t>
    [[nodiscard]] inline result<std::reference_wrapper<T>, variant_type_error>
    get_impl() noexcept {
        return {variant_type_error{}};
    }
};

}  // namespace nestl
