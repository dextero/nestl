#pragma once

#include <nestl/utility.hpp>

namespace nestl {
namespace detail {

template <typename... Args>
class storage {
public:
    alignas(max_alignment<Args...>) unsigned char data[max_sizeof<Args...>];

    storage() noexcept = default;

    storage(storage&&) noexcept = default;
    storage& operator=(storage&&) noexcept = default;

    storage(const storage&) = delete;
    storage& operator=(const storage&) = delete;

    template <typename T, typename... CtorArgs>
    storage(tag<T>, CtorArgs&&... args) noexcept {
        new (data) T(std::forward<CtorArgs>(args)...);
    }

    template <typename T>
        T& as() & noexcept {
        static_assert(is_one_of<T, Args...>);
        return *reinterpret_cast<T*>(data);
    }

    template <typename T>
        T&& as() && noexcept {
        static_assert(is_one_of<T, Args...>);
        return std::move(*reinterpret_cast<T*>(data));
    }

    template <typename T>
    const T& as() const noexcept {
        static_assert(is_one_of<std::remove_const_t<T>, Args...>);
        return *reinterpret_cast<const T*>(data);
    }

    template <typename T>
    void destroy() noexcept {
        destroy_impl<T, Args...>();
    }

private:
    template <typename Query, typename T, typename... Ts>
    void destroy_impl() noexcept {
        if (std::is_same_v<Query, T>) {
            as<T>().~T();
        } else {
            destroy_impl<Query, Ts...>();
        }
    }

    template <typename Query>
    void destroy_impl() noexcept {}
};

}  // namespace detail
}  // namespace nestl
