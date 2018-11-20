#pragma once

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

namespace nestl {

template <
    typename T,
    typename E
>
class result
{
private:
    enum class state {
        Empty,
        Ok,
        Err
    } m_state;

    union holder {
        T ok;
        E err;

        holder() {}
    } m_value;

    result() : m_state(state::Empty) {}

public:
    result(const result& r) = delete;
    result& operator =(const result& r) = delete;

    template <
        typename = std::enable_if_t<
            !std::is_same_v<T, E>
        >
    >
    result(T&& t) noexcept
        : m_state(state::Ok)
    {
        new (&m_value.ok) T(std::forward<T>(t));
    }

    template <
        typename = std::enable_if_t<
            !std::is_same_v<T, E>
        >
    >
    result(E&& e) noexcept
        : m_state(state::Err)
    {
        new (&m_value.err) E(std::forward<E>(e));
    }

    result(result&& r) noexcept
        : m_state(state::Empty)
    {
        *this = std::move(r);
    }

    result& operator =(result&& r) noexcept
    {
        switch (m_state) {
        case state::Ok:
            m_value.ok = std::move(r.m_value.ok);
            break;
        case state::Err:
            m_value.err = std::move(r.m_value.err);
            break;
        case state::Empty:
            __builtin_unreachable();
        }

        std::swap(m_state, r.m_state);
        return *this;
    }

    ~result() noexcept
    {
        switch (m_state) {
        case state::Ok:
            m_value.ok.~T();
            break;
        case state::Err:
            m_value.err.~E();
            break;
        case state::Empty:
            break;
        }
    }

    [[nodiscard]]
    static result ok(T&& t) noexcept
    {
        result r;
        r.m_state = state::Ok;
        new (&r.m_value.ok) T(std::forward<T>(t));
        return r;
    }

    [[nodiscard]]
    static result err(E&& e) noexcept
    {
        result r;
        r.m_state = state::Err;
        new (&r.m_value.err) E(std::forward<E>(e));
        return r;
    }

    [[nodiscard]]
    operator bool() const noexcept
    {
        assert(m_state != state::Empty);
        return m_state == state::Ok;
    }

    [[nodiscard]]
    T ok() && noexcept
    {
        assert(m_state == state::Ok);
        m_state = state::Empty;
        return std::move(m_value.ok);
    }

    [[nodiscard]]
    const T &ok() const& noexcept
    {
        assert(m_state == state::Ok);
        return m_value.ok;
    }

    [[nodiscard]]
    T err() && noexcept
    {
        assert(m_state == state::Err);
        m_state = state::Empty;
        return std::move(m_value.ok);
    }

    [[nodiscard]]
    const T& err() const& noexcept
    {
        assert(m_state == state::Err);
        return m_value.err;
    }

    template <typename F>
    auto map(const F& f) -> result<decltype(f(m_value.ok)), E>
    {
        using R = result<decltype(f(m_value.ok)), E>;

        switch (m_state) {
        case state::Ok:
            return R::ok(f(m_value.ok));
        case state::Err:
            return R::err(m_value.err);
        case state::Empty:
            __builtin_unreachable();
        }
    }

    template <typename F>
    auto map_err(const F& f) -> result<T, decltype(f(m_value.err))>
    {
        using R = result<decltype(f(m_value.ok)), E>;

        switch (m_state) {
        case state::Ok:
            return R::ok(m_value.ok);
        case state::Err:
            return R::err(f(m_value.err));
        case state::Empty:
            __builtin_unreachable();
        }
    }
};

} // namespace nestl
