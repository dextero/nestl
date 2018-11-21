#include <nestl/variant.hpp>

#include <memory>

#include <doctest.h>

TEST_CASE("variant") {
    struct Test {};

    SUBCASE("can hold values of different types") {
        auto v1 = nestl::variant<int>{1};
        auto v2 = nestl::variant<int, const char *>{1};
        auto v3 = nestl::variant<int, const char *, Test>{1};
    }
}
