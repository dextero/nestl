#include <nestl/result.hpp>

#include <memory>

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

    struct Mock {
        struct summon_t{};
        struct Control {
            size_t expected_moves = 0;
            size_t expected_copies = 0;
        };

        std::shared_ptr<Control> control;

        Mock&& expect_moves(size_t n) && {
            control->expected_moves = n;
            return std::move(*this);
        }

        Mock&& expect_copies(size_t n) && {
            control->expected_copies = n;
            return std::move(*this);
        }

        Mock() { FAIL("unexpected default ctor call"); }
        ~Mock() {
            if (control.use_count() == 1) {
                REQUIRE(control->expected_moves == 0);
                REQUIRE(control->expected_copies == 0);
            }
        }

        Mock(summon_t) : control(std::make_shared<Control>()) {}
        static Mock make() { return { summon_t{} }; }

        Mock(Mock&& src) {
            if (src.control->expected_moves == 0) {
                FAIL("unexpected move-assign call");
            }
            *this = std::move(src);
        }

        Mock& operator =(Mock&& src) {
            control = src.control;
            if (control->expected_moves-- == 0) {
                FAIL("unexpected move-assign call");
            }
            return *this;
        }

        Mock(const Mock& src) {
            if (src.control->expected_copies == 0) {
                FAIL("unexpected copy ctor call");
            }
            *this = src;
        }

        Mock& operator =(const Mock& src) {
            control = src.control;
            if (control->expected_copies-- == 0) {
                FAIL("unexpected copy-assign call");
            }
            return *this;
        }
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
        auto a = nestl::result<Movable, Mock>::ok({});
        auto b = std::move(a);
    }

    SUBCASE("is movable in Err state") {
        auto a = nestl::result<Mock, Movable>::err({});
        auto b = std::move(a);
    }

    SUBCASE("is true-ish in Ok state") {
        REQUIRE(static_cast<bool>(nestl::result<Movable, Mock>::ok({})));
    }

    SUBCASE("is false-ish in Err state") {
        REQUIRE(!static_cast<bool>(nestl::result<Mock, Movable>::err({})));
    }

    SUBCASE("can be safely moved-from in Ok state") {
        auto a = nestl::result<Movable, Mock>::emplace_ok();
        auto b = std::move(a).ok();
    }

    SUBCASE("can be safely moved-from in Err state") {
        auto a = nestl::result<Mock, Movable>::emplace_err();
        auto b = std::move(a).err();
    }

    SUBCASE("can be safely copied-from in Ok state") {
        auto a = nestl::result<Copyable, Mock>::emplace_ok();
        auto b = a.ok();
    }

    SUBCASE("can be safely copied-from in Err state") {
        auto a = nestl::result<Mock, Copyable>::emplace_err();
        auto b = a.err();
    }

    SUBCASE("can map Ok") {
        nestl::result<int, Mock> a =
            nestl::result<Movable, Mock>::emplace_ok()
                .map([](Movable&) { return 0; });
    }

    SUBCASE("map is a noop in Err state") {
        nestl::result<Mock, Mock> a =
            nestl::result<Mock, Mock>::emplace_err(Mock::make().expect_moves(2))
                .map([](Mock&) {
                         FAIL("should not be called");
                         return Mock::make();
                     });
    }

    SUBCASE("can map_err Err") {
        nestl::result<Mock, int> a =
            nestl::result<Mock, Movable>::emplace_err()
                .map_err([](Movable&) { return 0; });
    }

    SUBCASE("map_err is a noop in Ok state") {
        nestl::result<Mock, Mock> a =
            nestl::result<Mock, Mock>::emplace_ok(Mock::make().expect_moves(2))
                .map_err([](Mock&) {
                             FAIL("should not be called");
                             return Mock::make();
                         });
    }
}
