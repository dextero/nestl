#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace nestl {

template <typename T, typename... Args>
constexpr size_t max_alignment = std::max(max_alignment<T>, max_alignment<Args...>);

template <typename T>
constexpr size_t max_alignment<T> = alignof(T);

template <>
constexpr size_t max_alignment<void> = static_cast<size_t>(0);


template <typename T, typename... Args>
constexpr size_t max_sizeof = std::max(max_sizeof<T>, max_sizeof<Args...>);

template <typename T>
constexpr size_t max_sizeof<T> = sizeof(T);

template <>
constexpr size_t max_sizeof<void> = static_cast<size_t>(0);


template <typename Query, typename T, typename... Args>
constexpr bool is_one_of = is_one_of<Query, T> || is_one_of<Query, Args...>;

template <typename Query, typename T>
constexpr bool is_one_of<Query, T> = std::is_same_v<Query, T>;


template <typename Query, typename T, typename... Args>
constexpr bool is_none_of = is_none_of<Query, T> && is_none_of<Query, Args...>;

template <typename Query, typename T>
constexpr bool is_none_of<Query, T> = !std::is_same_v<Query, T>;


constexpr size_t invalid_type_index = std::numeric_limits<size_t>::max();

namespace detail {

template <typename Query, size_t N, typename T, typename... Args>
constexpr size_t type_index_impl =
    std::is_same_v<Query, T> ? N : type_index_impl<Query, N, Args...>;

template <typename Query, size_t N, typename T>
constexpr size_t type_index_impl<Query, N, T> =
    std::is_same_v<Query, T> ? N : invalid_type_index;

} // namespace detail

template <typename Query, typename... Args>
constexpr size_t type_index = detail::type_index_impl<Query, 0, Args...>;


template <typename T>
struct tag {
    using type = T;
};

} // namespace nestl
