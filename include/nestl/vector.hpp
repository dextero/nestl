#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <limits>

#include <nestl/allocator.hpp>
#include <nestl/result.hpp>

namespace nestl {
    class out_of_bounds {};

    template <typename T, typename Allocator>
    class vector
    {
        private:
            Allocator m_allocator;
            T* m_data = nullptr;
            size_t m_size = 0;
            size_t m_capacity = 0;

            [[nodiscard]] result<void, out_of_memory> grow(size_t new_size) {
                if (auto res = m_allocator.reallocate(new_size * sizeof(T))) {
                    m_data = res.ok();
                    m_capacity = new_size;
                } else {
                    return { std::move(res).err() };
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
                new (m_data + size) T(std::forward<Args>(args)...);
                ++m_size;
            }

            [[nodiscard]]
            enum class compare_result {
                less,
                equal,
                greater
            } compare(const vector& other) const {
                auto mismatch = std::mismatch(begin(), end(), other.begin());
                if (mismatch.first == end() && mismatch.second == other.end()) {
                    return compare_result::equal;
                } else if (std::lexicographical_compare(mismatch.first, end(), mismatch.second)) {
                    return compare_result::less;
                } else {
                    return compare_result::greater;
                }
            }

        public:
            vector() noexcept : m_allocator() {}
            explicit vector(const Allocator& alloc) noexcept : m_allocator(alloc) {}

            // constructors that fill vector with data not provided (allocation may fail)

            vector(vector&& src) noexcept { *this = std::move(src); }
            vector& operator = (vector&& src) noexcept {
                if (this != &src) {
                    swap(src);
                    src.clear();
                }
            }

            // use copy() instead
            vector(const vector&) = delete;
            vector& operator =(const vector&) = delete

            [[nodiscard]]
            result<vector<T>, out_of_memory> copy() noexcept {
                vector copy;
                if (auto res = copy.reserve(m_size)) {
                    for (const T& e : *this) {
                        copy.push_back(e);
                    }
                    return { copy };
                } else {
                    return { res.err() };
                }
            }

            // TODO: assign()
            // TODO: get_allocator()

            ~vector() noexcept {
                clear();
                m_allocator.free(m_data);
            }

            [[nodiscard]]
            result<std::reference_wrapper<T>, out_of_bounds> at(size_t idx) noexcept {
                if (idx < m_size) {
                    return { std::reference_wrapper<T>{(*this)[idx]} };
                } else {
                    return { out_of_bounds{} };
                }
            }

            [[nodiscard]]
            result<std::reference_wrapper<const T>, out_of_bounds> at(size_t idx) const noexcept {
                if (idx < m_size) {
                    return { std::reference_wrapper<const T>{(*this)[idx]} };
                } else {
                    return { out_of_bounds{} };
                }
            }

            [[nodiscard]] T& operator[](size_t idx) noexcept {
                return m_data[idx];
            }

            [[nodiscard]] const T& operator[](size_t idx) const noexcept {
                return m_data[idx];
            }

            [[nodiscard]] T& front() noexcept { return m_data[0]; }
            [[nodiscard]] const T& front() noexcept const { return m_data[0]; }

            [[nodiscard]] T& back() noexcept { return m_data[m_size - 1]; }
            [[nodiscard]] const T& back() noexcept const { return m_data[m_size - 1]; }

            [[nodiscard]] T* data() noexcept { return m_data; }
            [[nodiscard]] const T* data() const noexcept { return m_data; }

            [[nodiscard]] T* begin() noexcept { return m_data; }
            [[nodiscard]] const T* begin() const noexcept { return m_data; }
            [[nodiscard]] const T* cbegin() const noexcept { return begin(); }

            [[nodiscard]] T* end() { return m_data + m_size; }
            [[nodiscard]] const T* end() const noexcept { return m_data + m_size; }
            [[nodiscard]] const T* cend() const noexcept { return end(); }

            // TODO: rbegin/crbegin/rend/crend

            [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
            [[nodiscard]] size_t max_size() const noexcept { return std::numeric_limits<size_t>::max(); }
            [[nodiscard]] size_t size() const { return m_size; }

            result<void, out_of_memory> reserve(size_t new_size) {
                if (new_size > m_capacity) {
                    return grow(new_size);
                } else {
                    return { ok_t{} };
                }
            }

            [[nodiscard]] size_t capacity() const { return m_capacity; }

            void shrink_to_fit() noexcept {
                m_allocator.reallocate(m_data, m_size * sizeof(T));
            }

            void clear() {
                erase(begin());
            }

            result<T*, out_of_memory> insert(T* pos, const T& e) noexcept {
                return emplace(pos, e);
            }

            result<T*, out_of_memory> insert(const T* pos, const T& e) noexcept {
                return emplace(pos, e);
            }

            result<T*, out_of_memory> insert(const T* pos, T&& e) noexcept {
                return emplace(pos, std::move(e));
            }

            result<T*, out_of_memory> insert(const T* pos, size_t count, const T& e) noexcept {
                assert(begin() <= pos && pos <= end());

                size_t idx = (pos - begin());
                if (auto res = reserve(size() + count); !res) {
                    return { res.err() };
                }

                pos = begin() + idx;
                std::copy_backward(pos, end(), end() + count);
                for (size_t i = 0; i < count; ++i) {
                    new (pos + i) T(std::forward<Args>(args)...);
                }

                return { pos };
            }

            template <typename It>
            result<T*, out_of_memory> insert(const T* pos, It first, It last) noexcept {
                assert(begin() <= pos && pos <= end());

                size_t count = last - first;
                size_t idx = pos - begin();
                if (auto res = reserve(size() + count); !res) {
                    return { res.err() };
                }

                pos = begin() + idx;
                std::copy_backward(pos, end(), end() + count);
                for (T* at = pos; first != last; ++first, ++pos) {
                    new (pos) T(*first);
                }

                return { pos };
            }

            result<T*, out_of_memory> insert(const T* pos, std::initializer_list<T> ilist) noexcept {
                return insert(pos, ilist.begin(), ilist.end());
            }

            template <typename... Args>
            result<T*, out_of_memory> emplace(T* pos, Args&&... args) noexcept {
                assert(begin() <= pos && pos <= end());

                size_t idx = (pos - begin());
                if (auto res = reserve(size() + 1); !res) {
                    return { out_of_memory{} };
                }

                pos = begin() + idx;
                std::copy_backward(pos, end(), end() + 1);
                new (pos) T(std::forward<Args>(args)...);
                return { pos };
            }

            T* erase(T* pos) noexcept {
                return erase(pos, end());
            }

            T* erase(const T* pos) noexcept {
                return erase(const_cast<T*>(pos));
            }

            T* erase(T* first, T* last) noexcept {
                assert(begin() <= pos && pos <= end());

                size_t count = last - first;
                for (T* p = first; p != last; ++p) {
                    p->~T();
                }

                std::copy_backward(last, end(), first);
                return first;
            }

            T* erase(const T* first, const T* last) noexcept {
                return erase(const_cast<T*>(first), const_cast<T*>(last));
            }

            result<T&, out_of_memory> push_back(T&& e) noexcept {
                return emplace_back(std::forward<T>(e));
            }

            result<T&, out_of_memory> push_back(const T& e) noexcept {
                return emplace_back(e);
            }

            template <typename... Args>
            result<T&, out_of_memory> emplace_back(Args&&... args) noexcept {
                if (auto res = reserve(size() + 1); !res) {
                    return { out_of_memory{} };
                }

                emplace_back_unchecked(std::forward<Args>(args)...);
                return { back() };
            }

            void pop_back() noexcept {
                back().~T();
                --m_size;
            }

            result<void, out_of_memory> resize(size_t new_size) noexcept {
                if (new_size <= size()) {
                    erase(begin() + new_size, end());
                    return { ok_t{} };
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

            [[nodiscard]]
            bool operator ==(const vector& other) const {
                if (size() != other.size()) {
                    return false;
                }

                auto mismatch = std::mismatch(begin(), end(), other.begin());
                return mismatch.first == end();
            }

            [[nodiscard]]
            bool operator !=(const vector& other) const {
                return !(*this == other);
            }

            [[nodiscard]]
            bool operator <(const vector& other) const {
                return compare(other) == compare_result::less;
            }

            [[nodiscard]]
            bool operator <=(const vector& other) const {
                return compare(other) != compare_result::greater;
            }

            [[nodiscard]]
            bool operator >(const vector& other) const {
                return compare(other) == compare_result::greater;
            }

            [[nodiscard]]
            bool operator >=(const vector& other) const {
                return compare(other) != compare_result::less;
            }
    };
}
