//
// Copyright 2018 Marcin Radomski. All rights reserved.
//
// Licensed under the MIT license. See LICENSE file in the project root for
// details.
//
#pragma once

#include <iterator>

namespace nestl {
namespace detail {

template <typename I>
class reverse_iterator {
public:
    using difference_type = typename std::iterator_traits<I>::difference_type;
    using value_type = typename std::iterator_traits<I>::value_type;
    using pointer = typename std::iterator_traits<I>::pointer;
    using reference = typename std::iterator_traits<I>::reference;
    using iterator_category =
        typename std::iterator_traits<I>::iterator_category;

private:
    I m_it;

public:
    reverse_iterator() = delete;

    reverse_iterator(I it) : m_it(it) {}

    reverse_iterator(reverse_iterator&&) = default;
    reverse_iterator& operator=(reverse_iterator&&) = default;

    reverse_iterator(const reverse_iterator&) = default;
    reverse_iterator& operator=(const reverse_iterator&) = default;

    reverse_iterator& operator++() noexcept {
        --m_it;
        return *this;
    }

    reverse_iterator operator++(int) const noexcept {
        auto copy = *this;
        --m_it;
        return copy;
    }

    reverse_iterator& operator--() noexcept {
        ++m_it;
        return *this;
    }

    reverse_iterator operator--(int) const noexcept {
        auto copy = *this;
        ++m_it;
        return copy;
    }

    bool operator==(const reverse_iterator& other) const noexcept {
        return m_it == other.m_it;
    }

    bool operator!=(const reverse_iterator& other) const noexcept {
        return m_it != other.m_it;
    }

    auto operator-(const reverse_iterator& other) const noexcept
        -> decltype(m_it - other.m_it) {
        return m_it - other.m_it;
    }

    auto operator*() const noexcept -> decltype(*m_it) { return *m_it; }

    I operator->() const noexcept { return m_it; }
};

}  // namespace detail
}  // namespace nestl
