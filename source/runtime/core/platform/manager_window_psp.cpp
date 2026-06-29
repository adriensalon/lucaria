#include <chrono>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_app.hpp>

#define BUFFER_WIDTH 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

static unsigned int __attribute__((aligned(16))) _gu_command_list[262144]; // size ? lol

namespace lucaria {
namespace detail {

    namespace {

		static int _exit_callback(int arg1, int arg2, void* common)
		{
			sceKernelExitGame();
			return 0;
		}

		static int _callback_thread(SceSize args, void* argp)
		{
			const int _cbid = sceKernelCreateCallback("exit_callback", _exit_callback, nullptr);
			sceKernelRegisterExitCallback(_cbid);
			sceKernelSleepThreadCB();
			return 0;
		}
		

        void _update_loop(manager_window& window, manager_input& input)
        {
            static std::chrono::high_resolution_clock::time_point _last_render_time = std::chrono::high_resolution_clock::now();
            const std::chrono::high_resolution_clock::time_point _render_time = std::chrono::high_resolution_clock::now();
            window.time_delta_seconds = std::chrono::duration<float64>(_render_time - _last_render_time).count();
            _last_render_time = _render_time;

            for (std::pair<const uint32, float32x2>& _accumulator : window.pointer_accumulators) {
                input.pointer_events[_accumulator.first].delta = _accumulator.second;
                _accumulator.second = float32x2(0);
            }

            ImGui_ImplPSP_NewFrame();
            ImGui::GetIO().DisplaySize = convert_imgui(window.screen_size);

            window.stored_update_callback();

			sceGuFinish();
			sceGuSync(0, 0);
			sceDisplayWaitVblankStart();
			sceGuSwapBuffers();
        }
    }

    void manager_window::run(
        manager_input& input,
        manager_assets& objects,
        const std::function<void()>& setup_callback,
        const std::function<void()>& update_callback)
    {
		screen_size = uint32x2(SCREEN_WIDTH, SCREEN_HEIGHT);
        objects.is_etc2_supported = false;
        objects.is_s3tc_supported = false;
        input.is_mouse_supported = false;
        input.is_keyboard_supported = true;
        input.is_touch_supported = false;

		pspDebugScreenInit();
		const int _thread_id = sceKernelCreateThread("update_thread", _callback_thread, 0x11, 0xFA0, 0, 0);
		if (_thread_id >= 0) {
			sceKernelStartThread(_thread_id, 0, nullptr);
		}
		sceGuInit();
		sceGuStart(GU_DIRECT, _gu_command_list);
		sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUFFER_WIDTH);
		sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, (void*)(BUFFER_WIDTH * SCREEN_HEIGHT * 4), BUFFER_WIDTH);
		sceGuDepthBuffer((void*)(2 * BUFFER_WIDTH * SCREEN_HEIGHT * 4), BUFFER_WIDTH);
		sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
		sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
		sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		sceGuEnable(GU_SCISSOR_TEST);
		sceGuDepthMask(GU_TRUE);
		sceGuDisable(GU_DEPTH_TEST);
		sceGuFinish();
		sceGuSync(0, 0);
		sceDisplayWaitVblankStart();
		sceGuDisplay(GU_TRUE);

        initialize_imgui();
        initialize_openal();
		
        if (setup_callback) {
            setup_callback();
        }
        stored_update_callback = update_callback;

		while (true) {
			_update_loop(*this, input);
		}
    }

}
}
