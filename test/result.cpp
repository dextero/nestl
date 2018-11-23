#include <nestl/result.hpp>

#include <memory>

#include <doctest.h>

#include "test_utils.hpp"

TEST_CASE("result") {
    using nestl::result;

    SUBCASE("is constructible from T if T != E") {
        auto r = result<int, const char *>{1};
    }

    SUBCASE("is constructible from E if T != W") {
        auto r = result<int, const char *>{"foo"};
    }

    SUBCASE("allows for T == E") {
        auto r = result<int, int>::ok(1);
    }

    SUBCASE("is movable in Ok state") {
        auto a = result<Movable, Mock>::ok({});
        auto b = std::move(a);
    }

    SUBCASE("is movable in Err state") {
        auto a = result<Mock, Movable>::err({});
        auto b = std::move(a);
    }

    SUBCASE("correctly handles self-move") {
        auto ok = result<Movable, Mock>::ok({});
        ok = std::move(ok);
        REQUIRE(ok.is_ok());

        auto err = result<Mock, Movable>::err({});
        err = std::move(err);
        REQUIRE(err.is_err());
    }

    SUBCASE("is true-ish in Ok state") {
        REQUIRE(static_cast<bool>(result<Movable, Mock>::ok({})));
    }

    SUBCASE("is false-ish in Err state") {
        REQUIRE(!static_cast<bool>(result<Mock, Movable>::err({})));
    }

    SUBCASE("can be safely moved-from in Ok state") {
        auto a = result<Movable, Mock>::emplace_ok();
        auto b = std::move(a).ok();
    }

    SUBCASE("can be safely moved-from in Err state") {
        auto a = result<Mock, Movable>::emplace_err();
        auto b = std::move(a).err();
    }

    SUBCASE("can be safely copied-from in Ok state") {
        auto a = result<Copyable, Mock>::emplace_ok();
        auto b = a.ok();
    }

    SUBCASE("can be safely copied-from in Err state") {
        auto a = result<Mock, Copyable>::emplace_err();
        auto b = a.err();
    }

    SUBCASE("can map Ok") {
        result<int, Mock> a =
            result<Movable, Mock>::emplace_ok()
                .map([](Movable&&) { return 0; });
    }

    SUBCASE("map is a noop in Err state") {
        result<Mock, Mock> a =
            result<Mock, Mock>::emplace_err(Mock::make().expect_moves(2))
                .map([](Mock&&) {
                         FAIL("should not be called");
                         return Mock::make();
                     });
    }

    SUBCASE("can map_err Err") {
        result<Mock, int> a =
            result<Mock, Movable>::emplace_err()
                .map_err([](Movable&&) { return 0; });
    }

    SUBCASE("map_err is a noop in Ok state") {
        result<Mock, Mock> a =
            result<Mock, Mock>::emplace_ok(Mock::make().expect_moves(2))
                .map_err([](Mock&&) {
                             FAIL("should not be called");
                             return Mock::make();
                         });
    }

#if 0
    SUBCASE("can hold void") {
        auto ok = result<void, void>::ok();
        REQUIRE(ok.is_ok());

        auto ok = result<void, void>::err();
        REQUIRE(ok.is_err());
    }
#endif
}
