#include <lucaria/core/manager_game.hpp>

PSP_MODULE_INFO("LUCARIA_GAME_NAME_HERE", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int main(int argc, char** argv)
{
    lucaria::detail::manager_game _stack_game_manager = {};
}