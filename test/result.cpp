#include <nestl/result.hpp>

#include <doctest.h>

TEST_CASE("result") {
    struct Movable {
        Movable() = default;

        Movable(Movable&&) = default;
        Movable& operator =(Movable&&) = default;

        Movable(const Movable&) = delete;
        Movable& operator =(const Movable&) = delete;
    };

    struct Copyable {
        Copyable() = default;

        Copyable(Copyable&&) = delete;
        Copyable& operator =(Copyable&&) = delete;

        Copyable(const Copyable&) = default;
        Copyable& operator =(const Copyable&) = default;
    };

    struct Fail {
        struct summon_t {};

        Fail() { FAIL("unexpected default ctor call"); }
        Fail(summon_t) {}

        Fail(Fail&&) { FAIL("unexpected move ctor call"); }
        Fail& operator =(Fail&&) { FAIL("unexpected move-assign call"); }

        Fail(const Fail&) { FAIL("unexpected copy ctor call"); }
        const Fail& operator =(Fail&) { FAIL("unexpected copy-assign call"); }
    };

    SUBCASE("is constructible from T if T != E") {
        auto r = nestl::result<int, const char *>{1};
    }

    SUBCASE("is constructible from E if T != W") {
        auto r = nestl::result<int, const char *>{"foo"};
    }

    SUBCASE("allows for T == E") {
        auto r = nestl::result<int, int>::ok(1);
    }

    SUBCASE("is movable in Ok state") {
        auto a = nestl::result<Movable, Fail>::ok({});
        auto b = std::move(a);
    }

    SUBCASE("is movable in Err state") {
        auto a = nestl::result<Fail, Movable>::err({});
        auto b = std::move(a);
    }

    SUBCASE("is true-ish in Ok state") {
        REQUIRE(static_cast<bool>(nestl::result<Movable, Fail>::ok({})));
    }

    SUBCASE("is false-ish in Err state") {
        REQUIRE(!static_cast<bool>(nestl::result<Fail, Movable>::err({})));
    }

    SUBCASE("can be safely moved-from in Ok state") {
        auto a = nestl::result<Movable, Fail>::emplace_ok();
        auto b = std::move(a).ok();
    }

    SUBCASE("can be safely moved-from in Err state") {
        auto a = nestl::result<Fail, Movable>::emplace_err();
        auto b = std::move(a).err();
    }

    SUBCASE("can be safely copied-from in Ok state") {
        auto a = nestl::result<Copyable, Fail>::emplace_ok();
        auto b = a.ok();
    }

    SUBCASE("can be safely copied-from in Err state") {
        auto a = nestl::result<Fail, Copyable>::emplace_err();
        auto b = a.err();
    }

    SUBCASE("can map Ok") {
        nestl::result<Copyable, Fail> a =
            nestl::result<Movable, Fail>::emplace_ok()
                .map([](Movable&) { return Copyable{}; });
    }

    SUBCASE("map is a noop in Err state") {
        nestl::result<Fail, Fail> a =
            nestl::result<Fail, Fail>::emplace_err(Fail::summon_t{})
                .map([](Fail&) { return Fail{}; });
    }

    SUBCASE("can map_err Err") {
        nestl::result<Fail, Copyable> a =
            nestl::result<Fail, Movable>::emplace_err()
                .map_err([](Movable&) { return Copyable{}; });
    }

    SUBCASE("map_err is a noop in Ok state") {
        nestl::result<Fail, Fail> a =
            nestl::result<Fail, Fail>::emplace_ok(Fail::summon_t{})
                .map_err([](Fail&) { return Fail{}; });
    }
}
