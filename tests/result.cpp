//
// Copyright 2018 Marcin Radomski. All rights reserved.
//
// Licensed under the MIT license. See LICENSE file in the project root for
// details.
//
#include <nestl/result.hpp>

#include <doctest.h>

#include <stdexcept>

#include "test_utils.hpp"

TEST_SUITE("result") {
    using nestl::result;

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("is constructible from T if T != E") {
        auto r = result<int, const char*>{1};
        REQUIRE(r.is_ok());
        REQUIRE(r.ok() == 1);
        REQUIRE(!r.is_err());
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("is constructible from E if T != W") {
        const char* foo = "foo";
        auto r = result<int, const char*>{foo};
        REQUIRE(!r.is_ok());
        REQUIRE(r.is_err());
        REQUIRE(r.err() == foo);
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("allows for T == E") {
        auto r = result<int, int>::ok(1);
        REQUIRE(r.is_ok());
        REQUIRE(r.ok() == 1);
        REQUIRE(!r.is_err());
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("is movable in Ok state") {
        auto a = result<Movable, Mock>::ok({});
        auto b = std::move(a);
        REQUIRE(b.is_ok());
        REQUIRE(!b.is_err());
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("is movable in Err state") {
        auto a = result<Mock, Movable>::err({});
        auto b = std::move(a);
        REQUIRE(!b.is_ok());
        REQUIRE(b.is_err());
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("correctly handles self-move") {
        auto ok = result<Movable, Mock>::ok({});
        ok = std::move(ok);
        REQUIRE(ok.is_ok());  // NOLINT (bugprone-user-after-move)

        auto err = result<Mock, Movable>::err({});
        err = std::move(err);
        REQUIRE(err.is_err());  // NOLINT (bugprone-user-after-move)
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("is true-ish in Ok state") {
        REQUIRE(static_cast<bool>(result<Movable, Mock>::ok({})));
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("is false-ish in Err state") {
        REQUIRE(!static_cast<bool>(result<Mock, Movable>::err({})));
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can be safely moved-from in Ok state") {
        auto a = result<Movable, Mock>::emplace_ok();
        [[maybe_unused]] auto b = std::move(a).ok();
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can be safely moved-from in Err state") {
        auto a = result<Mock, Movable>::emplace_err();
        [[maybe_unused]] auto b = std::move(a).err();
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can be safely copied-from in Ok state") {
        auto a = result<Copyable, Mock>::emplace_ok();
        [[maybe_unused]] auto b = a.ok();
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can be safely copied-from in Err state") {
        auto a = result<Mock, Copyable>::emplace_err();
        [[maybe_unused]] auto b = a.err();
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can map Ok") {
        result<int, Mock> a = result<Movable, Mock>::emplace_ok().map(
            [](Movable&&) { return 0; });
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("map is a noop in Err state") {
        result<Mock, Mock> a =
            result<Mock, Mock>::emplace_err(Mock::make().expect_moves(3))
                .map([](Mock&&) {
                    FAIL("should not be called");
                    return Mock::make();
                });
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can map_err Err") {
        result<Mock, int> a = result<Mock, Movable>::emplace_err().map_err(
            [](Movable&&) { return 0; });
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("map_err is a noop in Ok state") {
        result<Mock, Mock> a =
            result<Mock, Mock>::emplace_ok(Mock::make().expect_moves(3))
                .map_err([](Mock&&) {
                    FAIL("should not be called");
                    return Mock::make();
                });
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can hold void") {
        auto ok = result<void, void>::ok();
        REQUIRE(ok.is_ok());

        auto err = result<void, void>::err();
        REQUIRE(err.is_err());
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can map void in Ok state") {
        result<int, void> o = result<void, void>::ok().map([]() { return 1; });
        REQUIRE(o.is_ok());
        REQUIRE(o.ok() == 1);
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can map void in Err state") {
        result<void, int> e1 =
            result<void, void>::err().map_err([]() { return 1; });
        REQUIRE(e1.is_err());
        REQUIRE(e1.err() == 1);
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("forwards Ok void during map") {
        result<void, int> e2 =
            result<void, void>::ok().map_err([]() { return 1; });
        REQUIRE(e2.is_ok());
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("forwards Err void during map") {
        result<int, void> o2 =
            result<void, void>::err().map([]() { return 1; });
        REQUIRE(o2.is_err());
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("can map into void") {
        result<void, int> ok = result<int, int>::ok(1).map([](int&&) {});
        REQUIRE(ok.is_ok());

        result<int, void> err = result<int, int>::err(1).map_err([](int&&) {});
        REQUIRE(err.is_err());
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("does not introduce unnecessary memory overhead") {
        REQUIRE(sizeof(result<void, void>) == sizeof(bool));
        REQUIRE(sizeof(result<int, int>) <= sizeof(int) + alignof(int));
        REQUIRE(sizeof(result<double, int>)
                <= sizeof(double) + alignof(double));
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("map keeps noexceptness") {
        auto void_except = []() { throw std::runtime_error(""); };
        auto void_no_except = []() noexcept {};
        auto int_except = [](int) { throw std::runtime_error(""); };
        auto int_no_except = [](int) noexcept {};

        REQUIRE(!noexcept(result<void, void>::ok().map(void_except)));
        REQUIRE(noexcept(result<void, void>::ok().map(void_no_except)));
        REQUIRE(!noexcept(result<int, void>::ok(1).map(int_except)));
        REQUIRE(noexcept(result<int, void>::ok(1).map(int_no_except)));
    }

    // NOLINTNEXTLINE (cert-err58-cpp)
    TEST_CASE("map_err keeps noexceptness") {
        auto void_except = []() { throw std::runtime_error(""); };
        auto void_no_except = []() noexcept {};
        auto int_except = [](int) { throw std::runtime_error(""); };
        auto int_no_except = [](int) noexcept {};

        REQUIRE(!noexcept(result<void, void>::err().map_err(void_except)));
        REQUIRE(noexcept(result<void, void>::err().map_err(void_no_except)));
        REQUIRE(!noexcept(result<void, int>::err(1).map_err(int_except)));
        REQUIRE(noexcept(result<void, int>::err(1).map_err(int_no_except)));
    }
}
