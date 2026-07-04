#include <chrono>
#include <exception>
#include <pspctrl.h>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_app.hpp>

#define BUFFER_WIDTH 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

static unsigned int __attribute__((aligned(16))) _gu_command_list[262144];
static void* _psp_current_draw_buffer = (void*)0;
static void* _psp_display_buffer = (void*)(BUFFER_WIDTH * SCREEN_HEIGHT * 4);
static void* _psp_default_depth_buffer = (void*)(2 * BUFFER_WIDTH * SCREEN_HEIGHT * 4);

namespace lucaria {
namespace detail {

    void* psp_current_draw_buffer()
    {
        return _psp_current_draw_buffer;
    }

    void* psp_default_depth_buffer()
    {
        return _psp_default_depth_buffer;
    }

    int psp_default_buffer_width()
    {
        return BUFFER_WIDTH;
    }

    namespace {

        void _psp_present_error(const bool has_active_list)
        {
            if (!has_active_list) {
                sceGuStart(GU_DIRECT, _gu_command_list);
            }
            sceGuClearColor(0xff0000ff);
            sceGuClear(GU_COLOR_BUFFER_BIT);
            sceGuFinish();
            sceGuSync(0, 0);
            sceDisplayWaitVblankStart();
            sceGuSwapBuffers();
            sceKernelSleepThread();
        }

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

        void _set_key(manager_input& input, const input_key key, const bool state)
        {
            key_event& _event = input.key_events[key];
            _event.is_down = state && !_event.state;
            _event.is_up = !state && _event.state;
            _event.state = state;
        }

        void _update_input(manager_window& window, manager_input& input)
        {
            SceCtrlData _pad {};
            sceCtrlReadBufferPositive(&_pad, 1);

            const bool _any_button = _pad.Buttons != 0;
            if (!window.is_mouse_locked && _any_button) {
                window.is_mouse_locked = true;
            }

            _set_key(input, input_key::keyboard_z, (_pad.Buttons & PSP_CTRL_CROSS) != 0);
            _set_key(input, input_key::keyboard_w, (_pad.Buttons & PSP_CTRL_UP) != 0);
            _set_key(input, input_key::keyboard_s, (_pad.Buttons & PSP_CTRL_DOWN) != 0);
            _set_key(input, input_key::keyboard_a, (_pad.Buttons & PSP_CTRL_LEFT) != 0);
            _set_key(input, input_key::keyboard_d, (_pad.Buttons & PSP_CTRL_RIGHT) != 0);
            _set_key(input, input_key::keyboard_e, (_pad.Buttons & PSP_CTRL_TRIANGLE) != 0);
            _set_key(input, input_key::keyboard_g, (_pad.Buttons & PSP_CTRL_SQUARE) != 0);
        }
		

        void _update_loop(manager_window& window, manager_input& input)
        {
            static std::chrono::high_resolution_clock::time_point _last_render_time = std::chrono::high_resolution_clock::now();
            const std::chrono::high_resolution_clock::time_point _render_time = std::chrono::high_resolution_clock::now();
            window.time_delta_seconds = std::chrono::duration<float64>(_render_time - _last_render_time).count();
            _last_render_time = _render_time;

            _update_input(window, input);

            for (std::pair<const uint32, float32x2>& _accumulator : window.pointer_accumulators) {
                input.pointer_events[_accumulator.first].delta = _accumulator.second;
                _accumulator.second = float32x2(0);
            }

            ImGui_ImplPSP_NewFrame();
            ImGui::GetIO().DisplaySize = convert_imgui(window.screen_size);

            sceGuStart(GU_DIRECT, _gu_command_list);
            sceGuClearColor(0xff000000);
            sceGuClear(GU_COLOR_BUFFER_BIT);

            try {
                window.stored_update_callback();
            }
            catch (const std::exception&) {
                _psp_present_error(true);
            }
            catch (...) {
                _psp_present_error(true);
            }

			sceGuFinish();
			sceGuSync(0, 0);
			sceDisplayWaitVblankStart();
			sceGuSwapBuffers();
            _psp_current_draw_buffer = _psp_current_draw_buffer == (void*)0 ? _psp_display_buffer : (void*)0;
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
        objects.is_s3tc_supported = true;
        input.is_mouse_supported = false;
        input.is_keyboard_supported = true;
        input.is_touch_supported = false;

		const int _thread_id = sceKernelCreateThread("update_thread", _callback_thread, 0x11, 0xFA0, 0, 0);
		if (_thread_id >= 0) {
			sceKernelStartThread(_thread_id, 0, nullptr);
		}
		sceGuInit();
		sceGuStart(GU_DIRECT, _gu_command_list);
		sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUFFER_WIDTH);
		sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, _psp_display_buffer, BUFFER_WIDTH);
		sceGuDepthBuffer(_psp_default_depth_buffer, BUFFER_WIDTH);
		sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
		sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
		sceGuDepthRange(65535, 0);
		sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		sceGuEnable(GU_SCISSOR_TEST);
		sceGuDepthFunc(GU_GEQUAL);
		sceGuDepthMask(GU_TRUE);
		sceGuDisable(GU_DEPTH_TEST);
		sceGuEnable(GU_CLIP_PLANES);
		sceGuFinish();
		sceGuSync(0, 0);
		sceDisplayWaitVblankStart();
		sceGuDisplay(GU_TRUE);

        try {
            initialize_imgui();
            initialize_openal();

            if (setup_callback) {
                setup_callback();
            }
        }
        catch (const std::exception&) {
            _psp_present_error(false);
        }
        catch (...) {
            _psp_present_error(false);
        }
        stored_update_callback = update_callback;

		while (true) {
			_update_loop(*this, input);
		}
    }

}
}
