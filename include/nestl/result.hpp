#pragma once

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

#include <nestl/detail/variant_base.hpp>

namespace nestl {

template <
    typename T,
    typename E
>
class result
{
private:
    struct ok_tag {};
    struct err_tag {};

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

    using ok_t = wrapper<ok_tag, T>;
    using err_t = wrapper<err_tag, E>;
    struct empty_t {};

    using variant = nestl::detail::unchecked_variant<ok_t, err_t, empty_t>;

    variant m_value;

    template <typename> struct emplace_t {};
    struct inaccessible_t {};

    template <typename EmplaceT, typename... Args>
    result(emplace_t<EmplaceT>, Args&&... args)
        : m_value(variant::template emplace<EmplaceT>(std::forward<Args>(args)...))
    {}

public:
    result(T&& t) noexcept
        : m_value(ok_t{std::move(t)})
    {}

    result(E&& e, inaccessible_t = {}) noexcept
        : m_value(err_t{std::move(e)})
    {}

    result(result&& r) noexcept = default;
    result& operator =(result&& r) = default;

    result(const result& r) = delete;
    result& operator =(const result& r) = delete;

    ~result() noexcept
    {
        m_value = empty_t{};
    }

    [[nodiscard]]
    static result ok(T&& t) noexcept
    {
        return { emplace_t<ok_t>{}, std::forward<T>(t) };
    }

    template <typename... Args>
    [[nodiscard]]
    static result emplace_ok(Args&&... args)
    {
        return { emplace_t<ok_t>{}, std::forward<Args>(args)... };
    }

    [[nodiscard]]
    static result err(E&& e) noexcept
    {
        return { emplace_t<err_t>{}, std::forward<E>(e) };
    }

    template <typename... Args>
    [[nodiscard]]
    static result emplace_err(Args&&... args)
    {
        return { emplace_t<err_t>{}, std::forward<Args>(args)... };
    }

    [[nodiscard]]
    bool is_ok() const noexcept
    {
        return m_value.template is<ok_t>();
    }

    [[nodiscard]]
    bool is_err() const noexcept
    {
        return m_value.template is<err_t>();
    }

    [[nodiscard]]
    operator bool() const noexcept
    {
        return is_ok();
    }

    [[nodiscard]]
    T ok() && noexcept
    {
        return std::move(m_value).template get_unchecked<ok_t>().value;
    }

    [[nodiscard]]
    const T &ok() const& noexcept
    {
        return m_value.template get_unchecked<ok_t>().value;
    }

    [[nodiscard]]
    E err() && noexcept
    {
        return std::move(m_value).template get_unchecked<err_t>().value;
    }

    [[nodiscard]]
    const E& err() const& noexcept
    {
        return m_value.template get_unchecked<err_t>().value;
    }

#if 0
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
#endif
};

} // namespace nestl
