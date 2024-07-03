#pragma once

#include <functional>
#include <vector>

#define REGISTER_FOR_UPDATE(typename_m) \
    inline static bool _register()      \
    {                                   \
        register_updater<typename_m>(); \
        return true;                    \
    }                                   \
    inline static bool _is_registered = _register();

namespace detail {
inline std::vector<std::function<void()>> updaters = {};
}

/// @brief
/// @tparam updater_t
template <typename updater_t>
void register_updater()
{
    detail::updaters.emplace_back([=]() {
        updater_t::update();
    });
}

/// @brief
void update();