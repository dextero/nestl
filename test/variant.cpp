#include <nestl/variant.hpp>

#include <cstring>
#include <memory>

#include <doctest.h>

#include "test_utils.hpp"

TEST_CASE("variant") {
    using nestl::variant;

    struct Test {};

    SUBCASE("is constructible from any variant") {
        auto v1 = variant<int, const char *, Movable>{1};
        auto v2 = variant<int, const char *, Movable>{"foo"};
        auto v3 = variant<int, const char *, Movable>{Movable{}};
    }

    SUBCASE("is movable") {
        auto v1 = variant<Movable, Mock>{Movable{}};
        auto v2 = std::move(v1);
        REQUIRE(v1.get<Movable>().is_err());
        REQUIRE(v2.get<Movable>().is_ok());
    }

    SUBCASE("is copyable if all elements are copyable") {
        auto v1 = variant<Copyable, Mock>::emplace<Copyable>();
        auto v2 = v1;
        REQUIRE(v1.get<Copyable>().is_ok());
        REQUIRE(v2.get<Copyable>().is_ok());
    }

    SUBCASE("supports emplace with arguments") {
        struct Foo {
            int a;
            double b;
            Foo(int a, double b) : a(a), b(b) {}
        };
        auto v = variant<Foo, int>::emplace<Foo>(1, 2.0);
    }

    SUBCASE("can hold values of different types") {
        auto v1 = variant<int>{1};
        auto v2 = variant<int, const char *>{1};
        auto v3 = variant<int, const char *, Test>{1};
    }

    SUBCASE("allows access to any held value") {
        auto v1 = variant<int>{1};
        REQUIRE(v1.get<int>().ok() == 1);

        const char *foo = "foo";
        auto v2 = variant<int, const char *>{foo};
        REQUIRE(v2.get<const char *>().ok() == foo);

        auto v3 = variant<int, const char *, Test>{1};
        REQUIRE(v3.get<int>().ok() == 1);
    }

    SUBCASE("allows const access to held value") {
        const auto v = variant<int, const char *, Test>{1};
        REQUIRE(v.get<int>().ok() == 1);
    }

    SUBCASE("allows mutable access to held value") {
        auto v = variant<int, const char *, Test>{1};
        auto &ref = v.get<int>().ok().get();
        REQUIRE(ref == 1);
        ref = 2;
        REQUIRE(ref == 2);
        REQUIRE(v.get<int>().ok() == 2);
    }

    SUBCASE("returns error result on invalid access") {
        auto v = variant<int, const char *, Test>{Test{}};
        REQUIRE(v.get<int>().is_err());
        REQUIRE(v.get<const char *>().is_err());
        REQUIRE(v.get<Test>().is_ok());
    }

    SUBCASE("can be safely moved-from") {
        auto a = variant<Movable, Mock>{Movable{}};
        auto b = std::move(a).get<Movable>();
    }

    SUBCASE("can be safely copied-from") {
        auto a = variant<Movable, Mock>{Movable{}};
        auto b = a.get<Movable>();
    }
}
