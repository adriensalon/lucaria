#pragma once

#include <lucaria/bin/types_math.hpp>

namespace lucaria {
namespace detail {
    struct manager_window;
}

struct context_window {

    /// @brief
    /// @return
    bool is_locked();

    /// @brief
    /// @return
    float64 time_delta();

    /// @brief
    /// @return
    float32x2 screen_size();

private:
    detail::manager_window* _manager;
    friend struct access_context;
    friend struct context_object;
};

}