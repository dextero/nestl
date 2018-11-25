#pragma once

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

#include <nestl/detail/variant_base.hpp>

namespace nestl {

struct ok_t {};
struct err_t {};

template <typename T, typename E>
class result_base
{
protected:
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

    template <typename... Args>
    result_base(ok_t, Args&&... args)
        : m_value(ok_v{}, std::forward<Args>(args)...)
    {}

    template <typename... Args>
    result_base(err_t, Args&&... args)
        : m_value(err_v{}, std::forward<Args>(args)...)
    {}

    T&& get(ok_t) && noexcept
    {
        return std::move(m_value).template get_unchecked<ok_v>().value;
    }

    const T& get(ok_t) const&
    {
        return m_value.template get_unchecked<ok_v>().value;
    }

    E&& get(err_t) && noexcept
    {
        return std::move(m_value).template get_unchecked<err_v>().value;
    }

    const E& get(err_t) const&
    {
        return m_value.template get_unchecked<err_v>().value;
    }

public:
    result_base(result_base&& r) noexcept = default;
    result_base& operator =(result_base&& r) noexcept = default;

    result_base(const result_base& r) = delete;
    result_base& operator =(const result_base& r) = delete;

    ~result_base() noexcept
    {
        m_value.~variant();
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
};

template <typename T, typename E> class result;
template <typename Self, typename T, typename E, typename Base> class with_void_ok;
template <typename Self, typename T, typename E, typename Base> class with_nonvoid_ok;
template <typename Self, typename T, typename E, typename Base> class with_void_err;
template <typename Self, typename T, typename E, typename Base> class with_nonvoid_err;

template <typename T, typename E>
using choose_err = std::conditional_t<
    std::is_void_v<E>,
    with_void_err<
        result<T, E>,
        T, E,
        result_base<T, E>
    >,
    with_nonvoid_err<
        result<T, E>,
        T, E,
        result_base<T, E>
    >
>;

template <typename T, typename E>
using choose_ok = std::conditional_t<
        std::is_void_v<T>,
        with_void_ok<
            result<T, E>,
            T, E,
            choose_err<T, E>
        >,
        with_nonvoid_ok<
            result<T, E>,
            T, E,
            choose_err<T, E>
        >
    >;

template <typename T, typename F>
using Mapped = std::conditional_t<
    std::is_void_v<T>,
    decltype(std::declval<F>()(std::declval<T>())),
    decltype(std::declval<F>()())
>;

template <
    typename Self,
    typename T,
    typename E,
    typename Base
>
class with_nonvoid_ok : public Base
{
    static_assert(!std::is_void_v<T>);

protected:
    with_nonvoid_ok(T&& t) noexcept
        : Base(ok_t{}, std::forward<T>(t))
    {}

    template <typename... Args>
    with_nonvoid_ok(Args&&... args) noexcept
        : Base(std::forward<Args>(args)...)
    {}

    template <typename F>
    [[nodiscard]]
    static auto map_ok(Self&& self, F&& f) noexcept
        -> result<Mapped<T, F>, E>
    {
        using R = result<Mapped<T, F>, E>;

        return R::ok(f(std::move(self).ok()));
    }

    template <typename F>
    [[nodiscard]]
    static auto forward_ok(Self&& self, F&& f) noexcept
        -> result<T, Mapped<E, F>>
    {
        using R = result<T, Mapped<E, F>>;

        return R::ok(std::move(self).ok());
    }

public:
    [[nodiscard]]
    static Self ok(T&& t) noexcept
    {
        return { ok_t{}, std::forward<T>(t) };
    }

    template <typename... Args>
    [[nodiscard]]
    static Self emplace_ok(Args&&... args)
    {
        return { ok_t{}, std::forward<Args>(args)... };
    }

    [[nodiscard]]
    T ok() && noexcept
    {
        return std::move(this->m_value).get(ok_t{});
    }

    [[nodiscard]]
    const T& ok() const& noexcept
    {
        return this->m_value.get(ok_t{});
    }
};

template <
    typename Self,
    typename T,
    typename E,
    typename Base
>
class with_void_ok : public Base
{
    static_assert(std::is_void_v<T>);

protected:
    template <typename... Args>
    with_void_ok(Args&&... args) noexcept
        : Base(std::forward<Args>(args)...)
    {}

    template <typename F>
    [[nodiscard]]
    static auto map_ok(Self&& self, F&& f) noexcept
        -> result<decltype(std::declval<F>()()), E>
    {
        using R = result<decltype(std::declval<F>()()), E>;

        return R::ok(f());
    }

    template <typename F>
    [[nodiscard]]
    static auto forward_ok(Self&&, F&&) noexcept
        -> result<decltype(std::declval<F>()()), E>
    {
        using R = result<decltype(std::declval<F>()()), E>;

        return R::ok();
    }

public:
    [[nodiscard]]
    static Self ok() noexcept
    {
        return { ok_t{} };
    }
};

template <
    typename Self,
    typename T,
    typename E,
    typename Base
>
class with_nonvoid_err : public Base
{
    static_assert(!std::is_void_v<E>);

protected:
    with_nonvoid_err(E&& e) noexcept
        : Base(err_t{}, std::forward<E>(e))
    {}

    template <typename... Args>
    with_nonvoid_err(Args&&... args) noexcept
        : Base(std::forward<Args>(args)...)
    {}

    template <typename F>
    [[nodiscard]]
    static auto map_err(Self&& self, F&& f) noexcept
        -> result<T, Mapped<E, F>>
    {
        using R = result<T, Mapped<E, F>>;

        return R::err(f(get_err(std::move(self))));
    }

    template <typename F>
    [[nodiscard]]
    static auto forward_err(Self&& self, F&& f) noexcept
        -> result<Mapped<T, F>, E>
    {
        using R = result<Mapped<T, F>, E>;

        return R::err(std::move(self).err());
    }

public:
    [[nodiscard]]
    static Self err(E&& e) noexcept
    {
        return { err_t{}, std::forward<E>(e) };
    }

    template <typename... Args>
    [[nodiscard]]
    static Self emplace_err(Args&&... args)
    {
        return { err_t{}, std::forward<Args>(args)... };
    }

    [[nodiscard]]
    E err() && noexcept
    {
        return std::move(this->m_value).get(err_t{});
    }

    [[nodiscard]]
    const E& err() const& noexcept
    {
        return this->m_value.get(err_t{});
    }
};

template <
    typename Self,
    typename T,
    typename E,
    typename Base
>
class with_void_err : public Base
{
    static_assert(std::is_void_v<E>);

protected:
    template <typename... Args>
    with_void_err(Args&&... args) noexcept
        : Base(std::forward<Args>(args)...)
    {}

    template <typename F>
    static auto map_err(F&& f) noexcept
        -> result<T, Mapped<E, F>>
    {
        using R = result<T, Mapped<E, F>>;

        return R::err(f());
    }

    template <typename F>
    static auto forward_err(F&&) noexcept
        -> result<Mapped<T, F>, E>
    {
        using R = result<Mapped<T, F>, E>;

        return R::err();
    }

public:
    [[nodiscard]]
    static Self err() noexcept
    {
        return { err_t{} };
    }
};

template <typename T, typename E>
class result final
    : public choose_ok<T, E>
{
public:
    template <typename... Args>
    result(Args&&... args) noexcept
        : choose_ok<T, E>(std::forward<Args>(args)...)
    {}

    result(result&&) = default;
    result& operator =(result&&) = default;

    result(const result&) = delete;
    result& operator =(const result&) = delete;

    template <typename F>
    auto map(F&& f) && noexcept
        -> result<Mapped<T, F>, E>
    {
        if (this->is_ok()) {
            return std::move(*this).map_ok();
        } else {
            assert(this->is_err());
            return std::move(*this).forward_err();
        }
    }

    template <typename F>
    auto map_err(F&& f) &&
        -> result<T, Mapped<E, F>>
    {
        if (this->is_ok()) {
            return std::move(*this).forward_ok(f);
        } else {
            assert(this->is_err());
            return std::move(*this).map_err(f);
        }
    }
};

} // namespace nestl

