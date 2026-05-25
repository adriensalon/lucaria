#pragma once

#include <utility>

namespace lucaria {
namespace detail {

    struct flag_owning {
        flag_owning() = default;
        flag_owning(const flag_owning&) = delete;
        flag_owning& operator=(const flag_owning&) = delete;
        flag_owning(flag_owning&& other) noexcept;
        flag_owning& operator=(flag_owning&& other) noexcept;

        void emplace() noexcept;
        [[nodiscard]] bool owns() const noexcept;

    private:
        bool _is_owning = false;
    };

}
}