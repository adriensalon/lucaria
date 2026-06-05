#pragma once

#include <lucaria/core/manager_app.hpp>

namespace lucaria {

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