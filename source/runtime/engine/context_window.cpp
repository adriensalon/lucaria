#include <lucaria/core/manager_app.hpp>
#include <lucaria/engine/context_window.hpp>

namespace lucaria {

bool context_window::is_locked()
{
#if defined(LUCARIA_PLATFORM_ANDROID)
    return true;
#endif

#if defined(LUCARIA_PLATFORM_WEB)
    return _manager->is_audio_locked && _manager->is_mouse_locked;
#endif

#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
    return _manager->is_mouse_locked;
#endif

#if defined(LUCARIA_PLATFORM_PSP)
    return _manager->is_mouse_locked;
#endif

    return false;
}

float64 context_window::time_delta()
{
    return _manager->time_delta_seconds;
}

float32x2 context_window::screen_size()
{
    return float32x2(_manager->screen_size);
}

}
