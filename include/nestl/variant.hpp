#pragma once

#include <cstddef>
#include <type_traits>

#include <nestl/result.hpp>
#include <nestl/utility.hpp>

namespace nestl {
namespace detail {

template <typename... Args>
struct storage {
    alignas(max_alignment<Args...>) unsigned char data[max_sizeof<Args...>];

    template <typename T, typename... CtorArgs>
    void emplace(CtorArgs&&... args) {
        new (data) T(std::forward<CtorArgs>(args)...);
    }

    template <typename T>
    T& as() {
        static_assert(is_one_of<std::remove_const_t<T>, Args...>);
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

} // namespace detaul

struct variant_type_error {};

template <typename... Ts>
struct variant
{
private:
    size_t m_current = invalid_type_index;
    detail::storage<Ts...> m_storage;

    template <typename T, size_t N, typename First, typename... Rest>
    inline size_t construct(T&& t) {
        if (std::is_same_v<T, First>) {
            assert(m_current == invalid_type_index);
            m_storage.template emplace<T>(std::forward<T>(t));
            return N;
        } else {
            return construct<T, N + 1, Rest...>(std::forward<T>(t));
        }
    }

    template <typename T, size_t>
    inline size_t construct(T&&) {
        abort();
        return invalid_type_index;
    }

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

    template <typename T, size_t N, typename First, typename... Rest>
    inline result<std::reference_wrapper<T>, variant_type_error> get_impl() {
        if (m_current == N) {
            if (std::is_same_v<std::remove_const_t<T>, First>) {
                return { m_storage.template as<T>() };
            } else {
                return { variant_type_error {} };
            }
        } else {
            return get_impl<T, N + 1, Rest...>();
        }
    }

    template <typename T, size_t>
    inline result<std::reference_wrapper<T>, variant_type_error> get_impl() {
        return { variant_type_error{} };
    }

public:
    template <typename T>
    variant(T&& t) {
        static_assert(is_one_of<std::decay_t<T>, Ts...>);
        m_current = construct<std::decay_t<T>, 0, Ts...>(std::forward<T>(t));
    }

    ~variant() {
        destruct<0, Ts...>();
        m_current = invalid_type_index;
    }

    template <typename T>
    result<std::reference_wrapper<T>, variant_type_error> get() & {
        static_assert(is_one_of<T, Ts...>);
        return get_impl<T, 0, Ts...>();
    }

    template <typename T>
    result<std::reference_wrapper<T>, variant_type_error> get() && {
        static_assert(is_one_of<T, Ts...>);
        return get_impl<T, 0, Ts...>();
    }

    template <typename T>
    result<std::reference_wrapper<const T>, variant_type_error> get() const {
        static_assert(is_one_of<T, Ts...>);
        return const_cast<variant*>(this)->get_impl<const T, 0, Ts...>();
    }
};

} // namespace nestl
