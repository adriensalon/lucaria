#include <lucaria/core/manager_game.hpp>

#include <cstring>
#include <exception>
#include <pspiofilemgr.h>
#include <pspkernel.h>

PSP_MODULE_INFO("777reroll", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(256);
PSP_HEAP_SIZE_KB(-1024);

namespace {

    void _psp_log(const char* message, const bool reset = false)
    {
        const int _flags = PSP_O_WRONLY | PSP_O_CREAT | (reset ? PSP_O_TRUNC : PSP_O_APPEND);
        const SceUID _file = sceIoOpen("ms0:/lucaria_psp.log", _flags, 0777);
        if (_file >= 0) {
            sceIoWrite(_file, message, std::strlen(message));
            sceIoWrite(_file, "\n", 1);
            sceIoClose(_file);
        }
    }

}

int main(int argc, char** argv)
{
    _psp_log("lucaria: main begin", true);

    try {
        lucaria::detail::manager_game* _game_manager = new lucaria::detail::manager_game();

        _psp_log("lucaria: main returned");
        delete _game_manager;
    }
    catch (const std::exception& exception) {
        _psp_log("lucaria: exception");
        _psp_log(exception.what());
        sceKernelSleepThread();
    }
    catch (...) {
        _psp_log("lucaria: unknown exception");
        sceKernelSleepThread();
    }

    sceKernelExitGame();
    return 0;
}
