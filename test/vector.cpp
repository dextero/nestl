#include <nestl/vector.hpp>

#include <doctest.h>

#include "test_utils.hpp"

TEST_CASE("vector") {
    using nestl::vector;

    SUBCASE("push_back") {
        vector<int> v;
        REQUIRE(v.push_back(1).is_ok());
        REQUIRE(v.push_back(2).is_ok());
        REQUIRE(v.push_back(3).is_ok());
        REQUIRE(v.size() == 3);

        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
    }

    SUBCASE("pop_back") {
        vector<int> v;
        REQUIRE(v.push_back(1).is_ok());
        REQUIRE(v.push_back(2).is_ok());
        REQUIRE(v.size() == 2);
        v.pop_back();
        REQUIRE(v.size() == 1);
        v.pop_back();
        REQUIRE(v.size() == 0);
    }

    SUBCASE("empty") {
        vector<int> v;
        REQUIRE(v.empty());

        v.push_back(1);
        REQUIRE(!v.empty());

        v.pop_back();
        REQUIRE(v.empty());
    }

    SUBCASE("clear") {
        vector<int> v;
        v.push_back(1);
        REQUIRE(!v.empty());
        v.clear();
        REQUIRE(v.empty());
    }
}
