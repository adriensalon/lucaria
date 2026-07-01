#include <chrono>
#include <cstring>
#include <exception>
#include <pspctrl.h>
#include <pspiofilemgr.h>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_app.hpp>

#define BUFFER_WIDTH 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

static unsigned int __attribute__((aligned(16))) _gu_command_list[262144]; // size ? lol
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

        void _psp_log(const char* message)
        {
            const SceUID _file = sceIoOpen("ms0:/lucaria_psp.log", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
            if (_file >= 0) {
                sceIoWrite(_file, message, std::strlen(message));
                sceIoWrite(_file, "\n", 1);
                sceIoClose(_file);
            }
        }

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
            _psp_log("lucaria: exit callback");
			sceKernelExitGame();
			return 0;
		}

		static int _callback_thread(SceSize args, void* argp)
		{
            _psp_log("lucaria: callback thread begin");
			const int _cbid = sceKernelCreateCallback("exit_callback", _exit_callback, nullptr);
			sceKernelRegisterExitCallback(_cbid);
			sceKernelSleepThreadCB();
            _psp_log("lucaria: callback thread returned");
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
                _psp_log("lucaria: psp input locked");
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
            static bool _first_frame = true;
            if (_first_frame) {
                _psp_log("lucaria: frame begin");
            }

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

            if (_first_frame) {
                _psp_log("lucaria: update callback begin");
            }
            try {
                window.stored_update_callback();
            }
            catch (const std::exception& exception) {
                _psp_log("lucaria: frame exception");
                _psp_log(exception.what());
                _psp_present_error(true);
            }
            catch (...) {
                _psp_log("lucaria: frame unknown exception");
                _psp_present_error(true);
            }
            if (_first_frame) {
                _psp_log("lucaria: update callback end");
            }

            if (_first_frame) {
                _psp_log("lucaria: gu finish begin");
            }
			sceGuFinish();
			sceGuSync(0, 0);
			sceDisplayWaitVblankStart();
			sceGuSwapBuffers();
            _psp_current_draw_buffer = _psp_current_draw_buffer == (void*)0 ? _psp_display_buffer : (void*)0;

            if (_first_frame) {
                _psp_log("lucaria: frame end");
            }
            _first_frame = false;
        }
    }

    void manager_window::run(
        manager_input& input,
        manager_assets& objects,
        const std::function<void()>& setup_callback,
        const std::function<void()>& update_callback)
    {
        _psp_log("lucaria: window run begin");
		screen_size = uint32x2(SCREEN_WIDTH, SCREEN_HEIGHT);
        objects.is_etc2_supported = false;
        objects.is_s3tc_supported = true;
        input.is_mouse_supported = false;
        input.is_keyboard_supported = true;
        input.is_touch_supported = false;

		pspDebugScreenInit();
        _psp_log("lucaria: psp debug initialized");
		const int _thread_id = sceKernelCreateThread("update_thread", _callback_thread, 0x11, 0xFA0, 0, 0);
		if (_thread_id >= 0) {
			sceKernelStartThread(_thread_id, 0, nullptr);
		}
        _psp_log("lucaria: callback thread started");
		sceGuInit();
		sceGuStart(GU_DIRECT, _gu_command_list);
		sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUFFER_WIDTH);
		sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, _psp_display_buffer, BUFFER_WIDTH);
		sceGuDepthBuffer(_psp_default_depth_buffer, BUFFER_WIDTH);
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
        _psp_log("lucaria: gu initialized");

        try {
            _psp_log("lucaria: imgui init begin");
            initialize_imgui();
            _psp_log("lucaria: imgui init end");
            _psp_log("lucaria: openal init begin");
            initialize_openal();
            _psp_log("lucaria: openal init end");

            if (setup_callback) {
                _psp_log("lucaria: setup begin");
                setup_callback();
                _psp_log("lucaria: setup end");
            }
        }
        catch (const std::exception& exception) {
            _psp_log("lucaria: setup exception");
            _psp_log(exception.what());
            _psp_present_error(false);
        }
        catch (...) {
            _psp_log("lucaria: setup unknown exception");
            _psp_present_error(false);
        }
        stored_update_callback = update_callback;
        _psp_log("lucaria: loop begin");

		while (true) {
			_update_loop(*this, input);
		}
    }

}
}
