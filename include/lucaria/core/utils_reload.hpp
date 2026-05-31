#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <lucaria/bin/types_math.hpp>
#include <lucaria/core/utils_filewatch.hpp>

#if defined(_WIN32)
#define LUCARIA_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define LUCARIA_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#define LUCARIA_RELOAD_MODULE_IMPLEMENTATION                                                    \
    LUCARIA_PLUGIN_EXPORT bool __lucaria_plugin_register(::lucaria::detail::manager_game* game) \
    {                                                                                           \
        if (!game) {                                                                            \
            return false;                                                                       \
        }                                                                                       \
        ::lucaria::detail::apply_lgsl_system_registrations(game->scenes);                       \
        ::lucaria::detail::apply_user_asset_registrations(game->objects);                       \
        ::lucaria::detail::apply_component_registrations(game->scenes);                         \
        ::lucaria::detail::apply_scene_registrations(game->scenes);                             \
        ::lucaria::detail::clear_pending_type_registrations();                                  \
        return true;                                                                            \
    }                                                                                           \
    LUCARIA_PLUGIN_EXPORT bool __lucaria_plugin_start(::lucaria::context_game* game)            \
    {                                                                                           \
        if (!game) {                                                                            \
            return false;                                                                       \
        }                                                                                       \
        ::lucaria::detail::apply_main_scene(*game);                                             \
        ::lucaria::detail::clear_pending_main_scene_registration();                             \
        return true;                                                                            \
    }

namespace lucaria {

struct context_game;

namespace detail {

    struct manager_game;

    struct object_reload_library {
        object_reload_library() = default;
        object_reload_library(const object_reload_library&) = delete;
        object_reload_library& operator=(const object_reload_library&) = delete;
        object_reload_library(object_reload_library&& other);
        object_reload_library& operator=(object_reload_library&& other);
        ~object_reload_library();

        [[nodiscard]] bool load(const std::filesystem::path& path, std::string& error);
        void unload();
        [[nodiscard]] void* symbol_raw(const char* name, std::string& error) const;
        [[nodiscard]] bool has_value() const;
        [[nodiscard]] explicit operator bool() const;

        template <typename SymbolType>
		[[nodiscard]] SymbolType symbol(const char* name, std::string& error) const
        {
            return reinterpret_cast<SymbolType>(symbol_raw(name, error));
        }

    private:
        void* _handle = nullptr;
        std::filesystem::path _loaded_path = {};
    };

    enum struct object_reload_module_status {
        idle,
		compilation_started,
        compiling,
		compilation_finished
    };

    using object_reload_module_register_function = bool (*)(manager_game*);
    using object_reload_module_start_function = bool (*)(context_game*);

    struct object_reload_module {
        object_reload_module();
        object_reload_module(const object_reload_module&) = delete;
        object_reload_module& operator=(const object_reload_module&) = delete;
        object_reload_module(object_reload_module&& other) = delete;
        object_reload_module& operator=(object_reload_module&& other) = delete;
        ~object_reload_module();

        uint32 version = 0;
        object_reload_module_register_function module_register = nullptr;
        object_reload_module_start_function module_start = nullptr;

		[[nodiscard]] object_reload_module_status poll_sources_and_recompile_library(int& cmake_return, std::string& cmake_output);
		[[nodiscard]] bool reload_library(std::string& load_error);

    private:
		std::thread _compile_thread = {};
        object_reload_library _shared_library = {};
        std::atomic<object_reload_module_status> _status = object_reload_module_status::idle;
        std::vector<object_filewatched_path> _watched_source_paths = {};
        std::filesystem::path _current_library_path = {};
        int _cmake_result = {};
        std::string _cmake_output = {};

		[[nodiscard]] int _run_process_capture_output(const std::string& command, std::string& output);
    };

}
}
