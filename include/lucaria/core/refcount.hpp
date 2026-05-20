#pragma once

#include <lucaria/core/math.hpp>

namespace lucaria {
namespace detail {

    struct refcount_flag {
        refcount_flag() = default;
        refcount_flag(const refcount_flag& other) noexcept;
        refcount_flag& operator=(const refcount_flag& other) noexcept;
        refcount_flag(refcount_flag&& other) noexcept;
        refcount_flag& operator=(refcount_flag&& other) noexcept;
        ~refcount_flag();

        void emplace();
        [[nodiscard]] bool owns() const noexcept;
        [[nodiscard]] bool is_last_owner() const noexcept;
        [[nodiscard]] uint32 use_count() const noexcept;

    private:
        void _retain() noexcept;
        void _release() noexcept;
        uint32* _count = nullptr;
    };
}
}