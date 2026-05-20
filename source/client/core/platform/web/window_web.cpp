#include <lucaria/core/error.hpp>
#include <lucaria/core/window.hpp>

extern "C" void __lucaria_main_scene();

namespace lucaria {
namespace detail {

    namespace {

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
                    LUCARIA_RUNTIME_ERROR("Failed emscripten operation with result '" + _brief + "'")
                }
            }
#endif
        }

        EM_BOOL _key_callback(int event_type, const EmscriptenKeyboardEvent* event, void* user_data)
        {
            window_implementation* _window = static_cast<window_implementation*>(user_data);

            if (!_window->implementation_web.is_audio_locked) {
                _window->initialize_openal();
                _window->implementation_web.is_audio_locked = true;
            }

            EmscriptenPointerlockChangeEvent _pointer_lock;
            _emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
            _window->implementation_web.is_mouse_locked = _pointer_lock.isActive;
            if (!_window->implementation_web.is_mouse_locked) {
                _emscripten_assert(emscripten_request_pointerlock("#canvas", 1));
                _window->implementation_web.is_mouse_locked = true;
            }

            const input_key _key(emscripten_keyboard_mappings.at(std::string(event->key)));
            if (event_type == EMSCRIPTEN_EVENT_KEYDOWN) {
                _window->key_events[_key].state = true;
            } else if (event_type == EMSCRIPTEN_EVENT_KEYUP) {
                _window->key_events[_key].state = false;
            }

            return 0;
        }

        EM_BOOL _mouse_callback(int event_type, const EmscriptenMouseEvent* event, void* user_data)
        {
            window_implementation* _window = static_cast<window_implementation*>(user_data);

            if (event_type == EMSCRIPTEN_EVENT_MOUSEMOVE) {
                const float32 _dpr = _window_get_dpr();
                const float32x2 _new_position = _dpr * float32x2(event->clientX, event->clientY);
                _window->pointer_accumulators[0] += _dpr * float32x2(event->movementX, event->movementY);
                _window->pointer_events[0].position = _new_position;
                ImGui::GetIO().AddMousePosEvent(_new_position.x, _new_position.y);

            } else if (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) {
                const uint32 _button = event->button;
                switch (_button) {
                case 0:
                    _window->key_events[input_key::mouse_left].state = true;
                    break;
                case 1:
                    _window->key_events[input_key::mouse_right].state = true;
                    break;
                case 2:
                    _window->key_events[input_key::mouse_middle].state = true;
                    break;
                default:
                    break;
                }
                ImGui::GetIO().AddMouseButtonEvent(_button, true);

                if (!_window->implementation_web.is_audio_locked) {
                    _window->initialize_openal();
                    _window->implementation_web.is_audio_locked = true;
                }

                EmscriptenPointerlockChangeEvent _pointer_lock;
                _emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
                _window->implementation_web.is_mouse_locked = _pointer_lock.isActive;
                if (!_window->implementation_web.is_mouse_locked) {
                    _emscripten_assert(emscripten_request_pointerlock("#canvas", 1));
                    _window->implementation_web.is_mouse_locked = true;
                }

            } else if (event_type == EMSCRIPTEN_EVENT_MOUSEUP) {
                const uint32 _button = event->button;
                switch (_button) {
                case 0:
                    _window->key_events[input_key::mouse_left].state = false;
                    break;
                case 1:
                    _window->key_events[input_key::mouse_right].state = false;
                    break;
                case 2:
                    _window->key_events[input_key::mouse_middle].state = false;
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
            window_implementation* _window = static_cast<window_implementation*>(user_data);

            if (!_window->implementation_web.is_audio_locked) {
                _window->initialize_openal();
                _window->implementation_web.is_audio_locked = true;
            }

            // mobile web goes fullscreen on first touch
            EmscriptenFullscreenChangeEvent _fullscreen;
            _emscripten_assert(emscripten_get_fullscreen_status(&_fullscreen));
            _window->implementation_web.is_mouse_locked = _fullscreen.isFullscreen;
            if (!_window->implementation_web.is_mouse_locked) {
                EmscriptenFullscreenStrategy _strategy;
                std::memset(&_strategy, 0, sizeof(_strategy));
                _strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
                _strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
                _strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
                _strategy.canvasResizedCallback = [](int event_type, const void* reserved, void* user_data) -> EM_BOOL { return EM_TRUE; };
                _emscripten_assert(emscripten_request_fullscreen_strategy("#canvas", EM_TRUE, &_strategy));
                _window->implementation_web.is_mouse_locked = true;
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
                    _window->pointer_accumulators[_event_id] = float32x2(0);
                    _window->pointer_events[_event_id].position = _new_position;
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
                    _window->pointer_accumulators[_event_id] += _delta_position;
                    _window->pointer_events[_event_id].position = _new_position;
                }

            } else if (event_type == EMSCRIPTEN_EVENT_TOUCHEND || event_type == EMSCRIPTEN_EVENT_TOUCHCANCEL) {
                for (int _pointer_index = 0; _pointer_index < event->numTouches; ++_pointer_index) {
                    const EmscriptenTouchPoint& _touch_point = event->touches[_pointer_index];
                    if (!_touch_point.isChanged) {
                        continue;
                    }

                    const uint32 _event_id = static_cast<uint32>(_touch_point.identifier);
                    _last_positions.erase(_event_id);
                    _window->pointer_accumulators.erase(_event_id);
                    _window->pointer_events.erase(_event_id);
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
                LUCARIA_RUNTIME_ERROR("Failed to create WebGL2 context on this device")
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
            window_implementation* _window = static_cast<window_implementation*>(user_data);

            static float64 _last_render_time = 0;
            const float64 _render_time = emscripten_get_now();
            _window->time_delta_seconds = (_render_time - _last_render_time) / 1000.0;
            _last_render_time = _render_time;

            int32 _screen_width, _screen_height;
            _screen_width = _canvas_get_width();
            _screen_height = _canvas_get_height();
            _window->screen_size = uint32x2(_screen_width, _screen_height);
            if (_window->screen_size == uint32x2(0)) {
                return;
            }

            for (std::pair<const uint32, float32x2>& _accumulator : _window->pointer_accumulators) {
                _window->pointer_events[_accumulator.first].delta = _accumulator.second;
                _accumulator.second = float32x2(0);
            }

            ImGui::GetIO().DisplaySize = convert_imgui(_window->screen_size);

            _window->stored_update_callback();
        }

    }

    window_implementation::window_implementation(
        const std::function<void()>& update_callback,
        const std::function<void()>& initialize_callback,
        const std::function<void()>& destroy_callback)
    {
		set_engine_window(this);

        is_touch_supported = _navigator_get_touch_points() > 1;
        is_mouse_supported = !is_touch_supported;
        is_keyboard_supported = !is_touch_supported;

        void* _user_data = static_cast<void*>(this);

        if (is_touch_supported) {
            _emscripten_assert(emscripten_set_touchstart_callback("#canvas", _user_data, 1, _touch_callback));
            _emscripten_assert(emscripten_set_touchend_callback("#canvas", _user_data, 1, _touch_callback));
            _emscripten_assert(emscripten_set_touchmove_callback("#canvas", _user_data, 1, _touch_callback));
            _emscripten_assert(emscripten_set_touchcancel_callback("#canvas", _user_data, 1, _touch_callback)); // EMSCRIPTEN_EVENT_TARGET_WINDOW doesnt seem to work on safari
        }

        if (is_mouse_supported) {
            _emscripten_assert(emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_dblclick_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseenter_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseleave_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseover_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _mouse_callback));
            _emscripten_assert(emscripten_set_mouseout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _mouse_callback));
        }

        if (is_keyboard_supported) {
            _emscripten_assert(emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _key_callback));
            _emscripten_assert(emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _key_callback));
            _emscripten_assert(emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, _user_data, 1, _key_callback));
        }

        _initialize_backend(is_etc2_supported, is_s3tc_supported);
        initialize_imgui();
		
        if (initialize_callback) {
            initialize_callback();
        }
        stored_update_callback = update_callback;
		
		__lucaria_main_scene();

        emscripten_set_main_loop_arg(_update_loop, _user_data, 0, EM_TRUE);
        emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);

        if (destroy_callback) {
            destroy_callback();
        }
    }

}
}