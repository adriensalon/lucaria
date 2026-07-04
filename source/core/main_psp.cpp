#include <lucaria/core/manager_game.hpp>

#include <exception>
#include <pspkernel.h>

PSP_MODULE_INFO("777reroll", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(256);
PSP_HEAP_SIZE_KB(-1024);

int main(int argc, char** argv)
{
    try {
        lucaria::detail::manager_game* _game_manager = new lucaria::detail::manager_game();
        delete _game_manager;
    }
    catch (const std::exception&) {
        sceKernelSleepThread();
    }
    catch (...) {
        sceKernelSleepThread();
    }

    sceKernelExitGame();
    return 0;
}
