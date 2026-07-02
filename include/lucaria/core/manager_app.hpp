#pragma once

#include <functional>
#include <memory>

#include <lucaria/core/manager_input.hpp>
#include <lucaria/engine/asset_texture.hpp>

#if defined(LUCARIA_PLATFORM_ANDROID)
#include <lucaria/core/platform_android.hpp>
#endif

#if defined(LUCARIA_PLATFORM_WEB)
#include <lucaria/core/platform_web.hpp>
#endif

#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
#include <lucaria/core/platform_glfw.hpp>
#endif

#if defined(LUCARIA_PLATFORM_PSP)
#include <lucaria/core/platform_psp.hpp>
#endif

namespace lucaria {
namespace detail {

    struct manager_assets;

    struct manager_window {
        manager_window() = default;
        manager_window(const manager_window& other) = delete;
        manager_window& operator=(const manager_window& other) = delete;
        manager_window(manager_window&& other) = delete;
        manager_window& operator=(manager_window&& other) = delete;
        ~manager_window();

        bool is_fullscreen = false;
        uint32x2 screen_size = uint32x2(0);
        float64 time_delta_seconds = 0.f;
        std::function<void()> stored_update_callback = nullptr;
        std::unordered_map<uint32, float32x2> pointer_accumulators = {};
        std::unique_ptr<ImFontAtlas> shared_font_atlas = nullptr;
        std::optional<asset_texture> shared_font_texture = std::nullopt;
        ImGuiContext* screen_context = nullptr;

#if defined(LUCARIA_PLATFORM_ANDROID)
        android_app* app = nullptr;
#if defined(LUCARIA_BACKEND_OPENGL)
        EGLDisplay display = EGL_NO_DISPLAY;
        EGLSurface surface = EGL_NO_SURFACE;
        EGLContext context = EGL_NO_CONTEXT;
#endif
        bool has_window = false;
        bool is_engine_initialized = false;
        bool must_install_imgui_callbacks = true;
#endif

#if defined(LUCARIA_PLATFORM_WEB)
        bool is_audio_locked = false;
        bool is_mouse_locked = false;
#endif

#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
        GLFWwindow* window = nullptr;
        bool is_mouse_locked = false;
        bool must_install_imgui_callbacks = true;
#endif

#if defined(LUCARIA_PLATFORM_PSP)
        bool is_mouse_locked = false;
        bool must_install_imgui_callbacks = true;
#endif

        [[nodiscard]] ImGuiContext* create_shared_imgui_context();
        void reupload_shared_imgui_font_texture();
        void initialize_imgui();
        void destroy_imgui();
        void initialize_openal();
        void destroy_openal();

#if defined(LUCARIA_PLATFORM_ANDROID)
        void run(
            android_app* app,
            manager_input& input,
            manager_assets& objects,
            const std::function<void()>& setup_callback,
            const std::function<void()>& update_callback);
#else
        void run(
            manager_input& input,
            manager_assets& objects,
            const std::function<void()>& setup_callback,
            const std::function<void()>& update_callback);
#endif
    };

}
}
