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

    struct Fail {
        Fail() { FAIL("unexpected default ctor call"); }

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

    SUBCASE("is movable in Ok state") {
        auto a = nestl::result<Movable, Fail>::ok({});
        auto b = std::move(a);
    }

    SUBCASE("is movable in Err state") {
        auto a = nestl::result<Fail, Movable>::err({});
        auto b = std::move(a);
    }
}
