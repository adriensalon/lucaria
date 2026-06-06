#include <future>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/reload_config.hpp>
#include <lucaria/core/reload_module.hpp>

namespace lucaria {
namespace detail {

    namespace {

#if defined(_WIN32)
        constexpr const char* _extension = ".dll";
#else
        constexpr const char* _extension = ".so";
#endif

        [[nodiscard]] static std::filesystem::path _copy_current_build_to_cache(const uint32 version)
        {
            const std::filesystem::path _source_path = std::filesystem::path(__lucaria_reload_shared_library_path);
            const std::filesystem::path _cache_root = std::filesystem::path(__lucaria_reload_cache_directory);
            const std::filesystem::path _cache_directory = _cache_root / std::to_string(version);
            std::filesystem::create_directories(_cache_directory);
            const std::filesystem::path _target_path = _cache_directory / _source_path.filename();
            std::filesystem::copy_file(_source_path, _target_path, std::filesystem::copy_options::overwrite_existing);
#if defined(_WIN32)
            std::filesystem::path _source_pdb = _source_path;
            _source_pdb.replace_extension(".pdb");
            if (std::filesystem::exists(_source_pdb)) {
                const std::filesystem::path _target_pdb = _cache_directory / _source_pdb.filename();
                std::filesystem::rename(_source_pdb, _target_pdb);
            }
#endif
            return _target_path;
        }
    }

    object_reload_library::object_reload_library(object_reload_library&& other)
        : _handle(other._handle)
        , _loaded_path(std::move(other._loaded_path))
    {
        other._handle = nullptr;
    }

    object_reload_library& object_reload_library::operator=(object_reload_library&& other)
    {
        if (this != &other) {
            unload();
            _handle = other._handle;
            _loaded_path = std::move(other._loaded_path);
            other._handle = nullptr;
        }
        return *this;
    }

    object_reload_library::~object_reload_library()
    {
        unload();
    }

    bool object_reload_library::has_value() const
    {
        return _handle != nullptr;
    }

    object_reload_library::operator bool() const
    {
        return has_value();
    }

    object_reload_module::object_reload_module()
    {
        std::string _error = {};
        _current_library_path = _copy_current_build_to_cache(version);
        LUCARIA_DEBUG_ASSERT(_shared_library.load(_current_library_path, _error), _error);

        module_register = _shared_library.symbol<object_reload_module_register_function>("__lucaria_plugin_register", _error);
        LUCARIA_DEBUG_ASSERT(module_register, _error);

        module_start = _shared_library.symbol<object_reload_module_start_function>("__lucaria_plugin_start", _error);
        LUCARIA_DEBUG_ASSERT(module_start, _error);

        for (uint32 _index = 0; _index < __lucaria_reload_watched_paths_count; ++_index) {
            _watched_source_paths.emplace_back(__lucaria_reload_watched_paths[_index]);
        }
    }

    object_reload_module::~object_reload_module()
    {
        if (_compile_thread.joinable()) {
            _compile_thread.join();
        }
    }

    object_reload_module_status object_reload_module::poll_sources_and_recompile_library(int& cmake_result, std::string& cmake_output)
    {
        cmake_result = 0;
        cmake_output.clear();

        const auto current_status = _status.load(std::memory_order_acquire);

        object_reload_module_status _expected = object_reload_module_status::compilation_finished;
        if (_status.compare_exchange_strong(_expected, object_reload_module_status::idle, std::memory_order_acq_rel)) {
            if (_compile_thread.joinable()) {
                _compile_thread.join();
            }
            cmake_result = _cmake_result;
            cmake_output = std::move(_cmake_output);
            _cmake_result = 0;
            _cmake_output.clear();
            return object_reload_module_status::compilation_finished;
        }

        if (_status.load(std::memory_order_acquire) == object_reload_module_status::compiling) {
            return object_reload_module_status::compiling;
        }

        if (_status.load(std::memory_order_acquire) == object_reload_module_status::idle) {
            bool _has_changed = false;
            for (object_filewatched_path& watched_path : _watched_source_paths) {
                if (watched_path.has_changed()) {
                    _has_changed = true;
                    break;
                }
            }
            if (!_has_changed) {
                return object_reload_module_status::idle;
            }
            object_reload_module_status _expected = object_reload_module_status::idle;
            if (!_status.compare_exchange_strong(_expected, object_reload_module_status::compilation_started, std::memory_order_acq_rel)) {
                return _expected;
            }
            if (_compile_thread.joinable()) {
                _compile_thread.join();
            }

            _compile_thread = std::thread([this] {
                std::string _configure_output = {};
                _status.store(object_reload_module_status::compiling, std::memory_order_release);
                int _result = _run_process_capture_output(__lucaria_reload_cmake_configure_command, _configure_output);
                if (_result != 0) {
                    _cmake_result = _result;
                    _cmake_output += _configure_output;
                    _status.store(object_reload_module_status::compilation_finished, std::memory_order_release);
                    return;
                }

                std::string _build_output = {};
                _result = _run_process_capture_output(__lucaria_reload_cmake_build_command, _build_output);
                _cmake_result = _result;
                _cmake_output += _configure_output + _build_output;
                _status.store(object_reload_module_status::compilation_finished, std::memory_order_release);
            });

            return object_reload_module_status::compilation_started;
        }

        return _status.load(std::memory_order_acquire);
    }

    bool object_reload_module::reload_library(std::string& load_error)
    {
        object_reload_library _next_library = {};
        std::filesystem::path _next_library_path = _copy_current_build_to_cache(version + 1);
        if (!_next_library.load(_next_library_path, load_error)) {
            return false;
        }
        object_reload_module_register_function _register_symbol = _next_library.symbol<object_reload_module_register_function>("__lucaria_plugin_register", load_error);
        if (!_register_symbol) {
            return false;
        }
        object_reload_module_start_function _start_symbol = _next_library.symbol<object_reload_module_start_function>("__lucaria_plugin_start", load_error);
        if (!_start_symbol) {
            return false;
        }
        _shared_library.unload();
        _shared_library = std::move(_next_library);
        _current_library_path = _next_library_path;
        module_register = _register_symbol;
        module_start = _start_symbol;
        ++version;
        return true;
    }

}
}
