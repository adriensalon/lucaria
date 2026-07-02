
#include <lucaria/core/manager_app.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_input.hpp>
#include <lucaria/core/utils_math.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static const std::unordered_map<std::string, input_key> _emscripten_keyboard_mappings = {
            { "a", input_key::keyboard_a },
            { "z", input_key::keyboard_z },
            { "e", input_key::keyboard_e },
            { "r", input_key::keyboard_r },
            { "t", input_key::keyboard_t },
            { "y", input_key::keyboard_y },
            { "u", input_key::keyboard_u },
            { "i", input_key::keyboard_i },
            { "o", input_key::keyboard_o },
            { "p", input_key::keyboard_p },
            { "q", input_key::keyboard_q },
            { "s", input_key::keyboard_s },
            { "d", input_key::keyboard_d },
            { "f", input_key::keyboard_f },
            { "g", input_key::keyboard_g },
            { "h", input_key::keyboard_h },
            { "j", input_key::keyboard_j },
            { "k", input_key::keyboard_k },
            { "l", input_key::keyboard_l },
            { "m", input_key::keyboard_m },
            { "w", input_key::keyboard_w },
            { "x", input_key::keyboard_x },
            { "c", input_key::keyboard_c },
            { "v", input_key::keyboard_v },
            { "b", input_key::keyboard_b },
            { "n", input_key::keyboard_n },
        };

        struct _emscripten_context {
            manager_window* window = nullptr;
            manager_input* input = nullptr;
        };

        EM_JS(int, _browser_get_samplerate, (), {
            var AudioContext = window.AudioContext || window.webkitAudioContext;
            var ctx = new AudioContext();
            var sr = ctx.sampleRate;
            ctx.close();
            return sr;
        });

        EM_JS(float, _window_get_dpr, (), {
            return window.devicePixelRatio || 1.0;
        });

        EM_JS(int, _canvas_get_width, (), {
            var _canvas = document.getElementById('canvas');
            var _dpr = window.devicePixelRatio;
            var _width = _canvas.getBoundingClientRect().width * _dpr;
            _canvas.width = _width;
            return _width;
        });

        EM_JS(int, _canvas_get_height, (), {
            var _canvas = document.getElementById('canvas');
            var _dpr = window.devicePixelRatio;
            var _height = _canvas.getBoundingClientRect().height * _dpr;
            _canvas.height = _height;
            return _height;
        });

        EM_JS(int, _navigator_get_touch_points, (), {
            return navigator.maxTouchPoints;
        });

        void _emscripten_assert(EMSCRIPTEN_RESULT result)
        {
#if defined(LUCARIA_DEBUG)
            if (result != EMSCRIPTEN_RESULT_SUCCESS) {
                std::string _brief;
                bool _is_fatal = true;
                switch (result) {
                case EMSCRIPTEN_RESULT_DEFERRED:
                    _brief = "EMSCRIPTEN_RESULT_DEFERRED";
                    _is_fatal = false;
                    break;
                case EMSCRIPTEN_RESULT_NOT_SUPPORTED:
                    _brief = "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
                    break;
                case EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED:
                    _brief = "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
                    break;
                case EMSCRIPTEN_RESULT_INVALID_TARGET:
                    _brief = "EMSCRIPTEN_RESULT_INVALID_TARGET";
                    break;
                case EMSCRIPTEN_RESULT_UNKNOWN_TARGET:
                    _brief = "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
                    break;
                case EMSCRIPTEN_RESULT_INVALID_PARAM:
                    _brief = "EMSCRIPTEN_RESULT_INVALID_PARAM";
                    break;
                case EMSCRIPTEN_RESULT_FAILED:
                    _brief = "EMSCRIPTEN_RESULT_FAILED";
                    break;
                case EMSCRIPTEN_RESULT_NO_DATA:
                    _brief = "EMSCRIPTEN_RESULT_NO_DATA";
                    break;
                default:
                    _brief = "Unknown emscripten result";
                    break;
                }
                if (_is_fatal) {
                    LUCARIA_DEBUG_ERROR("Failed emscripten operation with result '" + _brief + "'")
                }
            }
#endif
        }

        EM_BOOL _key_callback(int event_type, const EmscriptenKeyboardEvent* event, void* user_data)
        {
            _emscripten_context* _user_context = static_cast<_emscripten_context*>(user_data);

            if (!_user_context->window->is_audio_locked) {
                _user_context->window->initialize_openal();
                _user_context->window->is_audio_locked = true;
            }

            EmscriptenPointerlockChangeEvent _pointer_lock;
            _emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
            _user_context->window->is_mouse_locked = _pointer_lock.isActive;
            if (!_user_context->window->is_mouse_locked) {
                _emscripten_assert(emscripten_request_pointerlock("#canvas", 1));
                _user_context->window->is_mouse_locked = true;
            }

            std::string _event_key(event->key);
            if (_event_key.size() == 1 && _event_key[0] >= 'A' && _event_key[0] <= 'Z') {
                _event_key[0] = static_cast<char>(_event_key[0] - 'A' + 'a');
            }
            const std::unordered_map<std::string, input_key>::const_iterator _key_iterator = _emscripten_keyboard_mappings.find(_event_key);
            if (_key_iterator == _emscripten_keyboard_mappings.end()) {
                return 0;
            }

            const input_key _key(_key_iterator->second);
            if (event_type == EMSCRIPTEN_EVENT_KEYDOWN) {
                _user_context->input->key_events[_key].state = true;
            } else if (event_type == EMSCRIPTEN_EVENT_KEYUP) {
                _user_context->input->key_events[_key].state = false;
            }

            return 0;
        }

        EM_BOOL _mouse_callback(int event_type, const EmscriptenMouseEvent* event, void* user_data)
        {
            _emscripten_context* _user_context = static_cast<_emscripten_context*>(user_data);

            if (event_type == EMSCRIPTEN_EVENT_MOUSEMOVE) {
                const float32 _dpr = _window_get_dpr();
                const float32x2 _new_position = _dpr * float32x2(event->clientX, event->clientY);
                _user_context->window->pointer_accumulators[0] += _dpr * float32x2(event->movementX, event->movementY);
                _user_context->input->pointer_events[0].position = _new_position;
                ImGui::GetIO().AddMousePosEvent(_new_position.x, _new_position.y);

            } else if (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) {
                const uint32 _button = event->button;
                switch (_button) {
                case 0:
                    _user_context->input->key_events[input_key::mouse_left].state = true;
                    break;
                case 1:
                    _user_context->input->key_events[input_key::mouse_right].state = true;
                    break;
                case 2:
                    _user_context->input->key_events[input_key::mouse_middle].state = true;
                    break;
                default:
                    break;
                }
                ImGui::GetIO().AddMouseButtonEvent(_button, true);

                if (!_user_context->window->is_audio_locked) {
                    _user_context->window->initialize_openal();
                    _user_context->window->is_audio_locked = true;
                }

                EmscriptenPointerlockChangeEvent _pointer_lock;
                _emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
                _user_context->window->is_mouse_locked = _pointer_lock.isActive;
                if (!_user_context->window->is_mouse_locked) {
                    _emscripten_assert(emscripten_request_pointerlock("#canvas", 1));
                    _user_context->window->is_mouse_locked = true;
                }

            } else if (event_type == EMSCRIPTEN_EVENT_MOUSEUP) {
                const uint32 _button = event->button;
                switch (_button) {
                case 0:
                    _user_context->input->key_events[input_key::mouse_left].state = false;
                    break;
                case 1:
                    _user_context->input->key_events[input_key::mouse_right].state = false;
                    break;
                case 2:
                    _user_context->input->key_events[input_key::mouse_middle].state = false;
                    break;
                default:
                    break;
                }
                ImGui::GetIO().AddMouseButtonEvent(_button, false);

            } else if (event_type == EMSCRIPTEN_EVENT_CLICK) {
            } else if (event_type == EMSCRIPTEN_EVENT_MOUSEOVER) {
            } else if (event_type == EMSCRIPTEN_EVENT_MOUSEOUT) {
            }

            return 0;
        }

        EM_BOOL _touch_callback(int event_type, const EmscriptenTouchEvent* event, void* user_data)
        {
            _emscripten_context* _user_context = static_cast<_emscripten_context*>(user_data);

            if (!_user_context->window->is_audio_locked) {
                _user_context->window->initialize_openal();
                _user_context->window->is_audio_locked = true;
            }

            // mobile web goes fullscreen on first touch
            EmscriptenFullscreenChangeEvent _fullscreen;
            _emscripten_assert(emscripten_get_fullscreen_status(&_fullscreen));
            _user_context->window->is_mouse_locked = _fullscreen.isFullscreen;
            if (!_user_context->window->is_mouse_locked) {
                EmscriptenFullscreenStrategy _strategy;
                std::memset(&_strategy, 0, sizeof(_strategy));
                _strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
                _strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
                _strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
                _strategy.canvasResizedCallback = [](int event_type, const void* reserved, void* user_data) -> EM_BOOL { return EM_TRUE; };
                _emscripten_assert(emscripten_request_fullscreen_strategy("#canvas", EM_TRUE, &_strategy));
                _user_context->window->is_mouse_locked = true;
            }

            static std::unordered_map<uint32, float32x2> _last_positions = {};
            const float32 _dpr = _window_get_dpr();

            if (event_type == EMSCRIPTEN_EVENT_TOUCHSTART) {
                for (int _pointer_index = 0; _pointer_index < event->numTouches; ++_pointer_index) {
                    const EmscriptenTouchPoint& _touch_point = event->touches[_pointer_index];
                    if (!_touch_point.isChanged) {
                        continue;
                    }

                    const uint32 _event_id = static_cast<uint32>(_touch_point.identifier);
                    const float32x2 _new_position = _dpr * float32x2(_touch_point.clientX, _touch_point.clientY);
                    _last_positions[_event_id] = _new_position;
                    _user_context->window->pointer_accumulators[_event_id] = float32x2(0);
                    _user_context->input->pointer_events[_event_id].position = _new_position;
                }

            } else if (event_type == EMSCRIPTEN_EVENT_TOUCHMOVE) {
                for (int _pointer_index = 0; _pointer_index < event->numTouches; ++_pointer_index) {
                    const EmscriptenTouchPoint& _touch_point = event->touches[_pointer_index];
                    if (!_touch_point.isChanged) {
                        continue;
                    }

                    const uint32 _event_id = static_cast<uint32>(_touch_point.identifier);
                    const float32x2 _new_position = _dpr * float32x2(_touch_point.clientX, _touch_point.clientY);
                    const float32x2 _delta_position = _new_position - _last_positions[_event_id];
                    _last_positions[_event_id] = _new_position;
                    _user_context->window->pointer_accumulators[_event_id] += _delta_position;
                    _user_context->input->pointer_events[_event_id].position = _new_position;
                }

            } else if (event_type == EMSCRIPTEN_EVENT_TOUCHEND || event_type == EMSCRIPTEN_EVENT_TOUCHCANCEL) {
                for (int _pointer_index = 0; _pointer_index < event->numTouches; ++_pointer_index) {
                    const EmscriptenTouchPoint& _touch_point = event->touches[_pointer_index];
                    if (!_touch_point.isChanged) {
                        continue;
                    }

                    const uint32 _event_id = static_cast<uint32>(_touch_point.identifier);
                    _last_positions.erase(_event_id);
                    _user_context->window->pointer_accumulators.erase(_event_id);
                    _user_context->input->pointer_events.erase(_event_id);
                }
            }

            return 0;
        }

        void _initialize_backend(bool& is_etc2_supported, bool& is_s3tc_supported)
        {
            EmscriptenWebGLContextAttributes _webgl_attributes;
            emscripten_webgl_init_context_attributes(&_webgl_attributes);
            _webgl_attributes.explicitSwapControl = 0;
            _webgl_attributes.depth = 1;
            _webgl_attributes.stencil = 1;
            _webgl_attributes.antialias = 1;
            _webgl_attributes.majorVersion = 2;
            _webgl_attributes.minorVersion = 0;

            EMSCRIPTEN_WEBGL_CONTEXT_HANDLE _webgl_context = emscripten_webgl_create_context("#canvas", &_webgl_attributes);
            if (_webgl_context < 0) {
                LUCARIA_DEBUG_ERROR("Failed to create WebGL2 context on this device")
            }

            _emscripten_assert(emscripten_webgl_make_context_current(_webgl_context));

            if (emscripten_webgl_enable_extension(_webgl_context, "WEBGL_compressed_texture_etc")) {
                is_etc2_supported = true;
            }
            if (emscripten_webgl_enable_extension(_webgl_context, "WEBGL_compressed_texture_s3tc")) {
                is_s3tc_supported = true;
            }
        }

        static void _update_loop(void* user_data)
        {
            _emscripten_context* _user_context = static_cast<_emscripten_context*>(user_data);

            static float64 _last_render_time = 0;
            const float64 _render_time = emscripten_get_now();
            _user_context->window->time_delta_seconds = (_render_time - _last_render_time) / 1000.0;
            _last_render_time = _render_time;

            int32 _screen_width, _screen_height;
            _screen_width = _canvas_get_width();
            _screen_height = _canvas_get_height();
            _user_context->window->screen_size = uint32x2(_screen_width, _screen_height);
            if (_user_context->window->screen_size == uint32x2(0)) {
                return;
            }

            for (std::pair<const uint32, float32x2>& _accumulator : _user_context->window->pointer_accumulators) {
                const float32x2 _delta = _accumulator.second;
                _user_context->input->pointer_events[_accumulator.first].delta = _delta;
                if (_accumulator.first == 0) {
                    _user_context->input->mouse_position_delta = _delta;
                    _user_context->input->mouse_position = _user_context->input->pointer_events[0].position;
                }
                _accumulator.second = float32x2(0);
            }

            ImGui::GetIO().DisplaySize = convert_imgui(_user_context->window->screen_size);

            _user_context->window->stored_update_callback();
        }

    }

    void manager_window::run(
        manager_input& input,
        manager_assets& objects,
        const std::function<void()>& setup_callback,
        const std::function<void()>& update_callback)
    {
        input.is_touch_supported = _navigator_get_touch_points() > 1;
        input.is_mouse_supported = !input.is_touch_supported;
        input.is_keyboard_supported = !input.is_touch_supported;

        _emscripten_context _user_context = { this, &input };
        if (input.is_touch_supported) {
            _emscripten_assert(emscripten_set_touchstart_callback("#canvas", &_user_context, 1, _touch_callback));
            _emscripten_assert(emscripten_set_touchend_callback("#canvas", &_user_context, 1, _touch_callback));
            _emscripten_assert(emscripten_set_touchmove_callback("#canvas", &_user_context, 1, _touch_callback));
            _emscripten_assert(emscripten_set_touchcancel_callback("#canvas", &_user_context, 1, _touch_callback)); // EMSCRIPTEN_EVENT_TARGET_WINDOW doesnt seem to work on safari
        }
        if (input.is_mouse_supported) {
            _emscripten_assert(emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_dblclick_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseenter_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseleave_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseover_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _mouse_callback));
        }
        if (input.is_keyboard_supported) {
            _emscripten_assert(emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _key_callback));
            _emscripten_assert(emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _key_callback));
            _emscripten_assert(emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &_user_context, 1, _key_callback));
        }

        _initialize_backend(objects.is_etc2_supported, objects.is_s3tc_supported);
        initialize_imgui();

        setup_callback();
        stored_update_callback = update_callback;
        emscripten_set_main_loop_arg(_update_loop, &_user_context, 0, EM_TRUE);
        emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
    }

}
}
