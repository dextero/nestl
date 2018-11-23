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

static_assert(max_alignment<void> == 0);
static_assert(max_alignment<int> == alignof(int));
static_assert(max_alignment<int, int> == alignof(int));
static_assert(max_alignment<char, int> == alignof(int));
static_assert(max_alignment<char, double, int> == alignof(double));


template <typename T, typename... Args>
constexpr size_t max_sizeof = std::max(max_sizeof<T>, max_sizeof<Args...>);

template <typename T>
constexpr size_t max_sizeof<T> = sizeof(T);

template <>
constexpr size_t max_sizeof<void> = static_cast<size_t>(0);

static_assert(max_sizeof<void> == 0);
static_assert(max_sizeof<int> == sizeof(int));
static_assert(max_sizeof<int, int> == sizeof(int));
static_assert(max_sizeof<char, int> == sizeof(int));
static_assert(max_sizeof<char, double, int> == sizeof(double));


template <template <typename> class Predicate, typename... Args>
constexpr bool all_of = (Predicate<Args>::value && ... && true);

static_assert(all_of<std::is_scalar, int, char>);
static_assert(!all_of<std::is_scalar, int, char[3]>);


template <template <typename> class Predicate, typename... Args>
constexpr bool any_of = (Predicate<Args>::value || ... || false);

static_assert(any_of<std::is_scalar, int, char[3]>);
static_assert(!any_of<std::is_scalar, int[3], char[3]>);


template <typename Query, typename... Args>
constexpr bool is_one_of = (std::is_same_v<Query, Args> || ... || false);


constexpr size_t invalid_type_index = std::numeric_limits<size_t>::max();

namespace detail {

template <typename Query, size_t N, typename T, typename... Args>
constexpr size_t type_index_impl =
    std::is_same_v<Query, T> ? N : type_index_impl<Query, N + 1, Args...>;

template <typename Query, size_t N, typename T>
constexpr size_t type_index_impl<Query, N, T> =
    std::is_same_v<Query, T> ? N : invalid_type_index;

} // namespace detail

template <typename Query, typename... Args>
constexpr size_t type_index = detail::type_index_impl<Query, 0, Args...>;

static_assert(type_index<int, int> == 0);
static_assert(type_index<int, char, int> == 1);
static_assert(type_index<int, char, int, long> == 1);
static_assert(type_index<int, char, long, double> == invalid_type_index);


template <typename T>
struct tag {
    using type = T;
};

} // namespace nestl
