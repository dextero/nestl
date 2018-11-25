#pragma once

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

#include <nestl/detail/variant_base.hpp>

namespace nestl {

struct ok_t {};
struct err_t {};

template <typename T, typename E> class result;

namespace detail {

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
    result_base(ok_t, Args&&... args) noexcept
        : m_value(tag<ok_v>{}, std::forward<Args>(args)...)
    {}

    template <typename... Args>
    result_base(err_t, Args&&... args) noexcept
        : m_value(tag<err_v>{}, std::forward<Args>(args)...)
    {}

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
struct mapped {
    typedef decltype(std::declval<F>()(std::declval<T>())) type;
};

template <typename F>
struct mapped<void, F> {
    typedef decltype(std::declval<F>()()) type;
};

template <typename T, typename F>
using mapped_t = typename mapped<T, F>::type;

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
        : Base(ok_t{}, std::move(t))
    {}

    with_nonvoid_ok(T& t) noexcept
        : Base(ok_t{}, t)
    {}

    with_nonvoid_ok(const T& t) noexcept
        : Base(ok_t{}, t)
    {}

    template <typename... Args>
    with_nonvoid_ok(Args&&... args) noexcept
        : Base(std::forward<Args>(args)...)
    {}

    template <typename F>
    [[nodiscard]]
    auto map_ok_impl(F&& f) && noexcept
        -> result<mapped_t<T, F>, E>
    {
        using R = result<mapped_t<T, F>, E>;

        return R::ok(f(std::move(*this).ok()));
    }

    template <typename F>
    [[nodiscard]]
    auto forward_ok(F&& f) && noexcept
        -> result<T, mapped_t<E, F>>
    {
        using R = result<T, mapped_t<E, F>>;

        return R::ok(std::move(*this).ok());
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
        return std::move(this->m_value).template get_unchecked<typename Base::ok_v>().value;
    }

    [[nodiscard]]
    const T& ok() const& noexcept
    {
        return this->m_value.template get_unchecked<typename Base::ok_v>().value;
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
    auto map_ok_impl(F&& f) && noexcept
        -> result<mapped_t<T, F>, E>
    {
        using R = result<mapped_t<T, F>, E>;

        return R::ok(f());
    }

    template <typename F>
    [[nodiscard]]
    auto forward_ok(F&&) && noexcept
        -> result<T, mapped_t<E, F>>
    {
        using R = result<T, mapped_t<E, F>>;

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
        : Base(err_t{}, std::move(e))
    {}

    with_nonvoid_err(E& e) noexcept
        : Base(err_t{}, e)
    {}

    with_nonvoid_err(const E& e) noexcept
        : Base(err_t{}, e)
    {}

    template <typename... Args>
    with_nonvoid_err(Args&&... args) noexcept
        : Base(std::forward<Args>(args)...)
    {}

    template <typename F>
    [[nodiscard]]
    auto map_err_impl(F&& f) && noexcept
        -> result<T, mapped_t<E, F>>
    {
        using R = result<T, mapped_t<E, F>>;

        return R::err(f(std::move(*this).err()));
    }

    template <typename F>
    [[nodiscard]]
    auto forward_err(F&& f) && noexcept
        -> result<mapped_t<T, F>, E>
    {
        using R = result<mapped_t<T, F>, E>;

        return R::err(std::move(*this).err());
    }

public:
    [[nodiscard]]
    static Self err(E&& e) noexcept
    {
        return { err_t{}, std::forward<E>(e) };
    }

    template <typename... Args>
    [[nodiscard]]
    static Self emplace_err(Args&&... args) noexcept
    {
        return { err_t{}, std::forward<Args>(args)... };
    }

    [[nodiscard]]
    E err() && noexcept
    {
        return std::move(this->m_value).template get_unchecked<typename Base::err_v>().value;
    }

    [[nodiscard]]
    const E& err() const& noexcept
    {
        return this->m_value.template get_unchecked<typename Base::err_v>().value;
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
    auto map_err_impl(F&& f) && noexcept
        -> result<T, mapped_t<E, F>>
    {
        using R = result<T, mapped_t<E, F>>;

        return R::err(f());
    }

    template <typename F>
    auto forward_err(F&&) && noexcept
        -> result<mapped_t<T, F>, E>
    {
        using R = result<mapped_t<T, F>, E>;

        return R::err();
    }

public:
    [[nodiscard]]
    static Self err() noexcept
    {
        return { err_t{} };
    }
};

} // namespace detail

template <typename T, typename E>
class result final
    : public detail::choose_ok<T, E>
{
public:
    template <typename... Args>
    result(Args&&... args) noexcept
        : detail::choose_ok<T, E>(std::forward<Args>(args)...)
    {}

    result(result&&) noexcept = default;
    result& operator =(result&&) noexcept = default;

    result(const result&) = delete;
    result& operator =(const result&) = delete;

    template <typename F>
    auto map(F&& f) && noexcept
        -> result<detail::mapped_t<T, F>, E>
    {
        if (this->is_ok()) {
            return std::move(*this).map_ok_impl(std::forward<F>(f));
        } else {
            assert(this->is_err());
            return std::move(*this).forward_err(std::forward<F>(f));
        }
    }

    template <typename F>
    auto map_err(F&& f) &&
        -> result<T, detail::mapped_t<E, F>>
    {
        if (this->is_ok()) {
            return std::move(*this).forward_ok(std::forward<F>(f));
        } else {
            assert(this->is_err());
            return std::move(*this).map_err_impl(std::forward<F>(f));
        }
    }
};

} // namespace nestl

