#include <lucaria/core/manager_game.hpp>

#if defined(LUCARIA_PLATFORM_WIN32) && defined(LUCARIA_HIDE_CONSOLE)
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main(int argc, char** argv)
#endif
{
    lucaria::detail::manager_game _stack_game_manager = {};
}