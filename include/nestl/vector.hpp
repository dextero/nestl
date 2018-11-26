#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <limits>

#include <nestl/allocator.hpp>
#include <nestl/result.hpp>

namespace nestl {

class out_of_bounds {};

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

template <typename T, typename Allocator = system_allocator>
class vector {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = nestl::reverse_iterator<iterator>;
    using const_reverse_iterator = nestl::reverse_iterator<const_iterator>;

private:
    Allocator m_allocator;
    T* m_data = nullptr;
    size_t m_size = 0;
    size_t m_capacity = 0;

    [[nodiscard]] result<void, out_of_memory> grow(size_t new_size) {
        if (auto res = m_allocator.reallocate(m_data, new_size * sizeof(T))) {
            m_data = static_cast<T*>(res.ok());
            m_capacity = new_size;
            return {ok_t{}};
        } else {
            return {std::move(res).err()};
        }
    }

        [[nodiscard]] result<void, out_of_memory> grow() {
        if (capacity() == 0) {
            return grow(10);
        } else {
            return grow(capacity() * 3 / 2);
        }
    }

    template <typename... Args>
    void emplace_back_unchecked(Args&&... args) {
        assert(size() < capacity());
        new (m_data + size()) T(std::forward<Args>(args)...);
        ++m_size;
    }

    enum class compare_result { less, equal, greater };

    [[nodiscard]] compare_result compare(const vector& other) const {
        auto mismatch = std::mismatch(begin(), end(), other.begin());
        if (mismatch.first == end() && mismatch.second == other.end()) {
            return compare_result::equal;
        } else if (std::lexicographical_compare(mismatch.first, end(),
                                                mismatch.second)) {
            return compare_result::less;
        } else {
            return compare_result::greater;
        }
    }

    public : vector() noexcept
        : m_allocator() {}
    explicit vector(const Allocator& alloc) noexcept : m_allocator(alloc) {}

    // constructors that fill vector with data not provided (allocation may
    // fail)

    vector(vector&& src) noexcept { *this = std::move(src); }
    vector& operator=(vector&& src) noexcept {
        if (this != &src) {
            swap(src);
            src.clear();
        }
    }

    // use copy() instead
    vector(const vector&) = delete;
    vector& operator=(const vector&) = delete;

    [[nodiscard]] result<vector, out_of_memory> copy() noexcept {
        vector copy;
        if (auto res = copy.reserve(m_size)) {
            for (const T& e : *this) {
                copy.push_back(e);
            }
            return {copy};
        } else {
            return {res.err()};
        }
    }

    ~vector() noexcept {
        clear();
        m_allocator.free(m_data);
    }

    result<void, out_of_memory> assign(size_t count, const T& value) noexcept {
        if (auto res = reserve(count); !res) {
            return {res.err()};
        }

        clear();
        return insert(end(), count, value).map([](iterator&&){});
    }

    template <typename It>
    result<void, out_of_memory> assign(It first, It last) noexcept {
        if (auto res = reserve(last - first); !res) {
            return {res.err()};
        }

        clear();
        return insert(end(), first, last).map([](iterator&&){});
    }

    result<void, out_of_memory> assign(
        std::initializer_list<T> ilist) noexcept {
        return assign(ilist.begin(), ilist.end());
    }

    allocator_type get_allocator() const noexcept { return m_allocator; }

    [[nodiscard]] result<std::reference_wrapper<T>, out_of_bounds> at(
        size_t idx) noexcept {
        if (idx < m_size) {
            return {std::reference_wrapper<T>{(*this)[idx]}};
        } else {
            return {out_of_bounds{}};
        }
    }

    [[nodiscard]] result<std::reference_wrapper<const T>, out_of_bounds> at(
        size_t idx) const noexcept {
        if (idx < m_size) {
            return {std::reference_wrapper<const T>{(*this)[idx]}};
        } else {
            return {out_of_bounds{}};
        }
    }

    [[nodiscard]] T& operator[](size_t idx) noexcept { return m_data[idx]; }

    [[nodiscard]] const T& operator[](size_t idx) const noexcept {
        return m_data[idx];
    }

    [[nodiscard]] T& front() noexcept { return m_data[0]; }
    [[nodiscard]] const T& front() const noexcept { return m_data[0]; }

    [[nodiscard]] T& back() noexcept { return m_data[m_size - 1]; }
    [[nodiscard]] const T& back() const noexcept { return m_data[m_size - 1]; }

    [[nodiscard]] T* data() noexcept { return m_data; }
    [[nodiscard]] const T* data() const noexcept { return m_data; }

    [[nodiscard]] iterator begin() noexcept { return m_data; }
    [[nodiscard]] const_iterator begin() const noexcept { return m_data; }
    [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

    [[nodiscard]] iterator end() {
        return m_data + m_size;
    }[[nodiscard]] const_iterator end() const noexcept {
        return m_data + m_size;
    }
    [[nodiscard]] const_iterator cend() const noexcept { return end(); }

    [[nodiscard]] reverse_iterator rbegin() noexcept {
        return reverse_iterator{end() - 1};
    }
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator{end() - 1};
    }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    [[nodiscard]] reverse_iterator rend() noexcept {
        return reverse_iterator{begin() - 1};
    }
    [[nodiscard]] const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator{begin() - 1};
    }
    [[nodiscard]] const_reverse_iterator crend() const noexcept {
        return rend();
    }

    [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
    [[nodiscard]] size_t max_size() const noexcept {
        return std::numeric_limits<size_t>::max();
    }
    [[nodiscard]] size_t size() const { return m_size; }

    result<void, out_of_memory> reserve(size_t new_size) {
        if (new_size > m_capacity) {
            return grow(new_size);
        } else {
            return {ok_t{}};
        }
    }

    [[nodiscard]] size_t capacity() const { return m_capacity; }

    void shrink_to_fit() noexcept {
        m_allocator.reallocate(m_data, m_size * sizeof(T));
    }

    void clear() { erase(begin(), end()); }

    result<iterator, out_of_memory> insert(const_iterator pos,
                                           const T& e) noexcept {
        return emplace(pos, e);
    }

    result<iterator, out_of_memory> insert(const_iterator pos, T&& e) noexcept {
        return emplace(pos, std::move(e));
    }

    result<iterator, out_of_memory> insert(const_iterator pos, size_t count,
                                           const T& e) noexcept {
        assert(begin() <= pos && pos <= end());

        size_t idx = (pos - begin());
        if (auto res = reserve(size() + count); !res) {
            return {res.err()};
        }

        pos = begin() + idx;
        std::copy_backward(const_cast<iterator>(pos), end(), end() + count);
        for (size_t i = 0; i < count; ++i) {
            new (const_cast<iterator>(pos + i)) T(e);
        }

        m_size += count;
        return {const_cast<iterator>(pos)};
    }

    template <typename It>
    result<iterator, out_of_memory> insert(const_iterator pos, It first,
                                           It last) noexcept {
        assert(begin() <= pos && pos <= end());

        size_t count = last - first;
        size_t idx = pos - begin();
        if (auto res = reserve(size() + count); !res) {
            return {res.err()};
        }

        pos = begin() + idx;
        std::copy_backward(const_cast<iterator>(pos), end(), end() + count);
        for (const_iterator at = pos; first != last; ++first, ++at) {
            new (const_cast<iterator>(at)) T(*first);
        }

        m_size += count;
        return {const_cast<iterator>(pos)};
    }

    result<iterator, out_of_memory> insert(
        const_iterator pos, std::initializer_list<T> ilist) noexcept {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template <typename... Args>
    result<iterator, out_of_memory> emplace(const_iterator pos,
                                            Args&&... args) noexcept {
        assert(begin() <= pos && pos <= end());

        size_t idx = (pos - begin());
        if (auto res = reserve(size() + 1); !res) {
            return {out_of_memory{}};
        }

        pos = begin() + idx;
        std::copy_backward(const_cast<iterator>(pos), end(), end() + 1);
        new (const_cast<iterator>(pos)) T(std::forward<Args>(args)...);
        ++m_size;
        return {const_cast<iterator>(pos)};
    }

    iterator erase(const_iterator pos) noexcept {
        return erase(pos, pos + 1);
    }

    iterator erase(const_iterator first, const_iterator last) noexcept {
        assert(begin() <= first && first <= end());
        assert(begin() <= last && last <= end());

        size_t count = last - first;
        for (const_iterator p = first; p != last; ++p) {
            p->~T();
        }

        std::copy(const_cast<iterator>(last), end(), const_cast<iterator>(first));
        m_size -= count;
        return const_cast<iterator>(first);
    }

    result<std::reference_wrapper<T>, out_of_memory> push_back(T&& e) noexcept {
        return emplace_back(std::forward<T>(e));
    }

    result<std::reference_wrapper<T>, out_of_memory> push_back(
        const T& e) noexcept {
        return emplace_back(e);
    }

    template <typename... Args>
    result<std::reference_wrapper<T>, out_of_memory> emplace_back(
        Args&&... args) noexcept {
        if (auto res = reserve(size() + 1); !res) {
            return {out_of_memory{}};
        }

        emplace_back_unchecked(std::forward<Args>(args)...);
        return {std::reference_wrapper<T>{back()}};
    }

    void pop_back() noexcept {
        back().~T();
        --m_size;
    }

    result<void, out_of_memory> resize(size_t new_size) noexcept {
        if (new_size <= size()) {
            erase(begin() + new_size, end());
            return {ok_t{}};
        }

        auto res = grow(new_size);
        if (res) {
            while (size() < new_size) {
                emplace_back_unchecked();
            }
        }
        return res;
    }

    void swap(vector& other) noexcept {
        std::swap(m_allocator, other.m_allocator);
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    [[nodiscard]] bool operator==(const vector& other) const {
        if (size() != other.size()) {
            return false;
        }

        auto mismatch = std::mismatch(begin(), end(), other.begin());
        return mismatch.first == end();
    }

    [[nodiscard]] bool operator!=(const vector& other) const {
        return !(*this == other);
    }

    [[nodiscard]] bool operator<(const vector& other) const {
        return compare(other) == compare_result::less;
    }

    [[nodiscard]] bool operator<=(const vector& other) const {
        return compare(other) != compare_result::greater;
    }

    [[nodiscard]] bool operator>(const vector& other) const {
        return compare(other) == compare_result::greater;
    }

    [[nodiscard]] bool operator>=(const vector& other) const {
        return compare(other) != compare_result::less;
    }
};

}  // namespace nestl
