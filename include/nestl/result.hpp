#pragma once

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

#include <nestl/detail/variant_base.hpp>

namespace nestl {

struct ok_t {};
struct err_t {};

template <
    typename T,
    typename E
>
class result
{
private:
    template <typename Tag, typename V>
    struct wrapper {
        V value;

        template <typename... Args>
        wrapper(Args&&... args)
            : value(std::forward<Args>(args)...)
        {}
    };

    template <typename Tag>
    struct wrapper<Tag, void> {};

    using ok_v = wrapper<ok_t, T>;
    using err_v = wrapper<err_t, E>;
    struct empty_t {};

    using variant = nestl::detail::unchecked_variant<ok_v, err_v, empty_t>;

    variant m_value;

    template <typename TagT, typename... Args>
    result(tag<TagT> tag, Args&&... args)
        : m_value(tag, std::forward<Args>(args)...)
    {}

public:
    result(const T& t, ok_t = {}) noexcept
        : m_value(ok_v{t})
    {}

    result(T&& t, ok_t = {}) noexcept
        : m_value(ok_v{std::forward<T>(t)})
    {}

    result(const E& e, err_t = {}) noexcept
        : m_value(err_v{e})
    {}

    result(E&& e, err_t = {}) noexcept
        : m_value(err_v{std::forward<E>(e)})
    {}

    result(result&& r) noexcept = default;
    result& operator =(result&& r) noexcept = default;

    result(const result& r) = delete;
    result& operator =(const result& r) = delete;

    ~result() noexcept
    {
        m_value.~variant();
    }

    template <
        typename Ok = std::enable_if_t<!std::is_void_v<T>, T>
    >
    [[nodiscard]]
    static result ok(Ok&& t) noexcept
    {
        return { tag<ok_v>{}, std::forward<T>(t) };
    }

    template <typename... Args>
    [[nodiscard]]
    static result emplace_ok(Args&&... args)
    {
        return { tag<ok_v>{}, std::forward<Args>(args)... };
    }

    template <
        typename Err = std::enable_if_t<!std::is_void_v<E>, E>
    >
    [[nodiscard]]
    static result err(Err&& e) noexcept
    {
        return { tag<err_v>{}, std::forward<E>(e) };
    }

    template <typename... Args>
    [[nodiscard]]
    static result emplace_err(Args&&... args)
    {
        return { tag<err_v>{}, std::forward<Args>(args)... };
    }

    [[nodiscard]]
    bool is_ok() const noexcept
    {
        return m_value.template is<ok_v>();
    }

    [[nodiscard]]
    bool is_err() const noexcept
    {
        return m_value.template is<err_v>();
    }

    [[nodiscard]]
    operator bool() const noexcept
    {
        return is_ok();
    }

    [[nodiscard]]
    T ok() && noexcept
    {
        return std::move(m_value).template get_unchecked<ok_v>().value;
    }

    [[nodiscard]]
    const T &ok() const& noexcept
    {
        return m_value.template get_unchecked<ok_v>().value;
    }

    [[nodiscard]]
    E err() && noexcept
    {
        return std::move(m_value).template get_unchecked<err_v>().value;
    }

    [[nodiscard]]
    const E& err() const& noexcept
    {
        return m_value.template get_unchecked<err_v>().value;
    }

    template <typename F>
    auto map(F&& f) && -> result<decltype(f(std::declval<T&&>())), E>
    {
        using R = result<decltype(f(std::declval<T&&>())), E>;

        if (m_value.template is<ok_v>()) {
            return R::ok(f(std::move(m_value).template get_unchecked<ok_v>().value));
        } else {
            assert(m_value.template is<err_v>());
            return R::err(std::move(m_value).template get_unchecked<err_v>().value);
        }
    }

    template <typename F>
    auto map_err(F&& f) && -> result<T, decltype(f(std::declval<E&&>()))>
    {
        using R = result<T, decltype(f(std::declval<E&&>()))>;

        if (m_value.template is<ok_v>()) {
            return R::ok(std::move(m_value).template get_unchecked<ok_v>().value);
        } else {
            assert(m_value.template is<err_v>());
            return R::err(f(std::move(m_value).template get_unchecked<err_v>().value));
        }
    }
};

} // namespace nestl
