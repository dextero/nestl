//
// Copyright 2018 Marcin Radomski. All rights reserved.
//
// Licensed under the MIT license. See LICENSE file in the project root for
// details.
//
#include <doctest.h>

#include <functional>
#include <initializer_list>
#include <utility>
#include <iostream>

#include <nestl/result.hpp>
#include <nestl/vector.hpp>

namespace {

template <typename T = int>
struct V : public nestl::vector<T> {
    V(std::initializer_list<T>&& ilist) {
        this->assign(std::forward<std::initializer_list<T>>(ilist));
    }
};

}  // namespace

namespace nestl {

template <typename T>
std::ostream& operator<<(std::ostream& os, const nestl::vector<T>& v) {
    if (v.empty()) {
        return os << "{}";
    }
    os << "{ ";
    for (auto it = v.begin(); it != v.end() - 1; ++it) {
        os << *it << ", ";
    }
    return os << v.back() << " }";
}

}  // namespace nestl

TEST_CASE("vector") {
    using nestl::vector;

    SUBCASE("assign") {
        SUBCASE("count + element, same size") {
            vector<int> v;
            REQUIRE(v.push_back(1).is_ok());

            REQUIRE(v.assign(1u, 4).is_ok());
            REQUIRE(v.size() == 1);
            REQUIRE(v == V{4});
        }

        SUBCASE("count + element, smaller") {
            vector<int> v;
            REQUIRE(v.push_back(1).is_ok());

            REQUIRE(v.assign(0u, 4).is_ok());
            REQUIRE(v == V{});
        }

        SUBCASE("count + element, bigger") {
            vector<int> v;
            REQUIRE(v.assign(1u, 4).is_ok());
            REQUIRE(v.size() == 1);
            REQUIRE(v == V{4});
        }

        SUBCASE("initializer_list, same size") {
            vector<int> v;
            REQUIRE(v.push_back(1).is_ok());

            REQUIRE(v.assign({4}).is_ok());
            REQUIRE(v.size() == 1);
            REQUIRE(v == V{4});
        }

        SUBCASE("initializer_list, smaller") {
            vector<int> v;
            REQUIRE(v.push_back(1).is_ok());

            REQUIRE(v.assign({}).is_ok());
            REQUIRE(v == V{});
        }

        SUBCASE("initializer_list, bigger") {
            vector<int> v;
            REQUIRE(v.assign({4}).is_ok());
            REQUIRE(v.size() == 1);
            REQUIRE(v == V{4});
        }
    }

    SUBCASE("at") {
        SUBCASE("in bounds") {
            vector<int> v;
            v.assign({1, 2, 3});
            REQUIRE(v.at(0).ok() == 1);
            REQUIRE(v.at(1).ok() == 2);
            REQUIRE(v.at(2).ok() == 3);
        }

        SUBCASE("out of bounds") {
            vector<int> v;
            v.assign({1, 2, 3});
            REQUIRE(v.at(3).is_err());
            REQUIRE(v.at(-1).is_err());
        }
    }

    SUBCASE("empty") {
        vector<int> v;
        v.assign({1});
        REQUIRE(!v.empty());

        v.pop_back();
        REQUIRE(v.empty());
    }

    SUBCASE("clear") {
        SUBCASE("empty") {
            vector<int> v;
            v.clear();
            REQUIRE(v.empty());
        }

        SUBCASE("not empty") {
            vector<int> v;
            v.assign({1});
            v.clear();
            REQUIRE(v.empty());
        }
    }

    SUBCASE("erase") {
        SUBCASE("element at begin") {
            vector<int> v;
            v.assign({1, 2});
            auto it = v.erase(v.begin());
            REQUIRE(it == v.begin());
            REQUIRE(v == V{2});
        }

        SUBCASE("element in the middle") {
            vector<int> v;
            v.assign({1, 2, 3});
            auto it = v.erase(v.begin() + 1);
            REQUIRE(it == v.begin() + 1);
            REQUIRE(v == V{1, 3});
        }

        SUBCASE("range at begin") {
            vector<int> v;
            v.assign({1, 2, 3});
            auto it = v.erase(v.begin(), v.begin() + 2);
            REQUIRE(it == v.begin());
            REQUIRE(v == V{3});
        }

        SUBCASE("range in the middle") {
            vector<int> v;
            v.assign({1, 2, 3, 4});
            auto it = v.erase(v.begin() + 1, v.begin() + 3);
            REQUIRE(it == v.begin() + 1);
            REQUIRE(v == V{1, 4});
        }

        SUBCASE("range at end") {
            vector<int> v;
            v.assign({1});
            auto it = v.erase(v.end(), v.end());
            REQUIRE(it == v.end());
            REQUIRE(v == V{1});
        }
    }

    SUBCASE("emplace") {
        SUBCASE("at begin") {
            vector<int> v;
            v.assign({2});
            auto it = v.emplace(v.begin(), 1).ok();
            REQUIRE(it == v.begin());
            REQUIRE(v == V{1, 2});
        }

        SUBCASE("in the middle") {
            vector<int> v;
            v.assign({1, 3});
            auto it = v.emplace(v.begin() + 1, 2).ok();
            REQUIRE(it == v.begin() + 1);
            REQUIRE(v == V{1, 2, 3});
        }

        SUBCASE("at end") {
            vector<int> v;
            v.assign({1});
            auto it = v.emplace(v.end(), 2).ok();
            REQUIRE(it == v.begin() + 1);
            REQUIRE(v == V{1, 2});
        }
    }

    SUBCASE("insert") {
        SUBCASE("value at begin") {
            vector<int> v;
            v.assign({2});
            auto it = v.insert(v.begin(), 1).ok();
            REQUIRE(it == v.begin());
            REQUIRE(v == V{1, 2});
        }

        SUBCASE("value in the middle") {
            vector<int> v;
            v.assign({1, 3});
            auto it = v.insert(v.begin() + 1, 2).ok();
            REQUIRE(it == v.begin() + 1);
            REQUIRE(v == V{1, 2, 3});
        }

        SUBCASE("value at end") {
            vector<int> v;
            v.assign({1});
            auto it = v.insert(v.end(), 2).ok();
            REQUIRE(it == v.begin() + 1);
            REQUIRE(v == V{1, 2});
        }

        SUBCASE("range at begin") {
            vector<int> v;
            v.assign({3});
            auto it = v.insert(v.begin(), {1, 2}).ok();
            REQUIRE(it == v.begin());
            REQUIRE(v == V{1, 2, 3});
        }

        SUBCASE("range in the middle") {
            vector<int> v;
            v.assign({1, 4});
            auto it = v.insert(v.begin() + 1, {2, 3}).ok();
            REQUIRE(it == v.begin() + 1);
            REQUIRE(v == V{1, 2, 3, 4});
        }

        SUBCASE("range at end") {
            vector<int> v;
            v.assign({1});
            auto it = v.insert(v.end(), {2, 3}).ok();
            REQUIRE(it == v.begin() + 1);
            REQUIRE(v == V{1, 2, 3});
        }
    }

    SUBCASE("push_back") {
        vector<int> v;
        REQUIRE(v.push_back(1).is_ok());
        REQUIRE(v == V{1});
    }

    SUBCASE("emplace_back") {
        vector<int> v;
        REQUIRE(v.emplace_back(1).is_ok());
        REQUIRE(v == V{1});
    }

    SUBCASE("pop_back") {
        vector<int> v;
        v.assign({1, 2});
        v.pop_back();
        REQUIRE(v == V{1});
        v.pop_back();
        REQUIRE(v == V{});
    }

    SUBCASE("resize") {
        SUBCASE("same size") {
            vector<int> v;
            v.assign({1, 2});
            REQUIRE(v.resize(2).is_ok());
            REQUIRE(v == V{1, 2});
        }

        SUBCASE("shrink") {
            vector<int> v;
            v.assign({1, 2});
            REQUIRE(v.resize(1).is_ok());
            REQUIRE(v == V{1});
        }

        SUBCASE("grow") {
            vector<int> v;
            v.assign({1});
            REQUIRE(v.resize(2).is_ok());
            REQUIRE(v == V{1, 0});
        }
    }

    SUBCASE("swap") {
        vector<int> v1;
        v1.assign({1, 2});

        vector<int> v2;
        v2.assign({3, 4, 5, 6});

        v1.swap(v2);

        REQUIRE(v1 == V{3, 4, 5, 6});
        REQUIRE(v2 == V{1, 2});
    }
}
