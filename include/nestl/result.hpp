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
        ~holder() {}
    } m_value;

    struct ok_t {};
    struct err_t {};
    struct inaccessible_t {};

    template <typename... Args>
    result(ok_t, Args&&... args) : m_state(state::Ok)
    {
        new (&m_value.ok) T(std::forward<Args>(args)...);
    }

    template <typename... Args>
    result(err_t, Args&&... args) : m_state(state::Err)
    {
        new (&m_value.err) E(std::forward<Args>(args)...);
    }

public:
    result(const result& r) = delete;
    result& operator =(const result& r) = delete;

    result(T&& t) noexcept
        : result(ok_t{}, std::forward<T>(t))
    {}

    result(E&& e, inaccessible_t = {}) noexcept
        : result(err_t{}, std::forward<E>(e))
    {}

    result(result&& r) noexcept
        : m_state(state::Empty)
    {
        *this = std::move(r);
    }

    result& operator =(result&& r) noexcept
    {
        if (&r != this) {
            return *this;
        }

        this->~result();

        switch (r.m_state) {
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

        m_state = state::Empty;
    }

    [[nodiscard]]
    static result ok(T&& t) noexcept
    {
        return { ok_t{}, std::forward<T>(t) };
    }

    template <typename... Args>
    [[nodiscard]]
    static result emplace_ok(Args&&... args)
    {
        return { ok_t{}, std::forward<Args>(args)... };
    }

    [[nodiscard]]
    static result err(E&& e) noexcept
    {
        return { err_t{}, std::forward<E>(e) };
    }

    template <typename... Args>
    [[nodiscard]]
    static result emplace_err(Args&&... args)
    {
        return { err_t{}, std::forward<Args>(args)... };
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
        return std::move(m_value.ok);
    }

    [[nodiscard]]
    const T &ok() const& noexcept
    {
        assert(m_state == state::Ok);
        return m_value.ok;
    }

    [[nodiscard]]
    E err() && noexcept
    {
        assert(m_state == state::Err);
        return std::move(m_value.err);
    }

    [[nodiscard]]
    const E& err() const& noexcept
    {
        assert(m_state == state::Err);
        return m_value.err;
    }

    template <typename F>
    auto map(const F& f) && -> result<decltype(f(m_value.ok)), E>
    {
        using R = result<decltype(f(m_value.ok)), E>;

        switch (m_state) {
        case state::Ok:
            return R::ok(f(m_value.ok));
        case state::Err:
            return R::err(std::move(m_value.err));
        case state::Empty:
            __builtin_unreachable();
        }
    }

    template <typename F>
    auto map_err(const F& f) && -> result<T, decltype(f(m_value.err))>
    {
        using R = result<T, decltype(f(m_value.err))>;

        switch (m_state) {
        case state::Ok:
            return R::ok(std::move(m_value.ok));
        case state::Err:
            return R::err(f(m_value.err));
        case state::Empty:
            __builtin_unreachable();
        }
    }
};

} // namespace nestl
