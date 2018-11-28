//
// Copyright 2018 Marcin Radomski. All rights reserved.
//
// Licensed under the MIT license. See LICENSE file in the project root for
// details.
//
#pragma once

#include <nestl/result.hpp>

#include <cassert>
#include <cstdlib>

namespace nestl {

class out_of_memory {};

class system_allocator {
public:
    result<void*, out_of_memory> allocate(size_t size) noexcept {
        assert(size > 0);

        void* p = ::malloc(size);
        if (p) {
            return {p};
        } else {
            return {out_of_memory{}};
        }
    }

    result<void*, out_of_memory> reallocate(void* p, size_t new_size) noexcept {
        assert(new_size > 0);

        p = ::realloc(p, new_size);
        if (p) {
            return {p};
        } else {
            return {out_of_memory{}};
        }
    }

    void free(void* p) noexcept { ::free(p); }
};

}  // namespace nestl
