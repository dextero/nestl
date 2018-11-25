#pragma once

namespace {

struct Movable {
    Movable() = default;

    Movable(Movable&&) = default;
    Movable& operator=(Movable&&) = default;

    Movable(const Movable&) = delete;
    Movable& operator=(const Movable&) = delete;
};

struct Copyable {
    Copyable() = default;

    Copyable(Copyable&&) = delete;
    Copyable& operator=(Copyable&&) = delete;

    Copyable(const Copyable&) = default;
    Copyable& operator=(const Copyable&) = default;
};

struct Mock {
    struct summon_t {};
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
    static Mock make() { return {summon_t{}}; }

    Mock(Mock&& src) {
        if (src.control->expected_moves == 0) {
            FAIL("unexpected move-assign call");
        }
        *this = std::move(src);
    }

    Mock& operator=(Mock&& src) {
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

    Mock& operator=(const Mock& src) {
        control = src.control;
        if (control->expected_copies-- == 0) {
            FAIL("unexpected copy-assign call");
        }
        return *this;
    }
};

}  // namespace
