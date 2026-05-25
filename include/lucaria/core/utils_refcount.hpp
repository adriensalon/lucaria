#pragma once

#include <atomic>

#include <lucaria/bin/types_math.hpp>

namespace lucaria {
namespace detail {

    struct flag_refcount_control {
        std::atomic<uint32> count = { 0 };
    };

    struct flag_refcount {
        flag_refcount() noexcept = default;
        flag_refcount(const flag_refcount& other) noexcept;
        flag_refcount& operator=(const flag_refcount& other) noexcept;
        flag_refcount(flag_refcount&& other) noexcept;
        flag_refcount& operator=(flag_refcount&& other) noexcept;
        ~flag_refcount();

        explicit flag_refcount(flag_refcount_control* control) noexcept;
        void reset() noexcept;
        bool owns() const noexcept;
        bool is_last_owner() const noexcept;
        uint32 use_count() const noexcept;

    private:
        void _retain() noexcept;
        void _release() noexcept;

    private:
        flag_refcount_control* _control = nullptr;
    };

}
}