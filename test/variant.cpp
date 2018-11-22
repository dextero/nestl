#include <nestl/variant.hpp>

#include <memory>
#include <cstring>

#include <doctest.h>

TEST_CASE("variant") {
    struct Test {};

    SUBCASE("can hold values of different types") {
        auto v1 = nestl::variant<int>{1};
        auto v2 = nestl::variant<int, const char *>{1};
        auto v3 = nestl::variant<int, const char *, Test>{1};
    }

    SUBCASE("allows access to any held value") {
        auto v1 = nestl::variant<int>{1};
        REQUIRE(v1.get<int>().ok() == 1);

        auto v2 = nestl::variant<int, const char *>{"foo"};
        REQUIRE(strcmp(v2.get<const char*>().ok(), "foo") == 0);

        auto v3 = nestl::variant<int, const char *, Test>{1};
        REQUIRE(v3.get<int>().ok() == 1);
    }

    SUBCASE("allows const access to held value") {
        const auto v = nestl::variant<int, const char *, Test>{1};
        REQUIRE(v.get<int>().ok() == 1);
    }

    SUBCASE("allows mutable access to held value") {
        auto v = nestl::variant<int, const char *, Test>{1};
        auto &ref = v.get<int>().ok().get();
        REQUIRE(ref == 1);
        ref = 2;
        REQUIRE(ref == 2);
        REQUIRE(v.get<int>().ok() == 2);
    }

    SUBCASE("returns error result on invalid access") {
        auto v = nestl::variant<int, const char *, Test>{Test{}};
        REQUIRE(v.get<int>().is_err());
        REQUIRE(v.get<const char *>().is_err());
        REQUIRE(v.get<Test>().is_ok());
    }
}
