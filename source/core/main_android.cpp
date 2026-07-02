#include <lucaria/core/manager_game.hpp>

namespace lucaria {
android_app* g_app = nullptr;
}

extern "C" void android_main(struct android_app* app)
{
    lucaria::g_app = app;
    lucaria::detail::manager_game _stack_game_manager(app);
}
