#pragma once

#include <lucaria/core/platform/web/key_web.hpp>

namespace lucaria {
namespace detail {

    struct window_implementation_web {
        bool is_audio_locked = false;
        bool is_mouse_locked = false;
    };

}
}