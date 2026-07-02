#include <array>
#include <cstdio>

#include <dlfcn.h>
#include <sys/wait.h>

#include <lucaria/core/utils_error.hpp>
#include <lucaria/core/utils_reload.hpp>

namespace lucaria {
namespace detail {

    bool object_reload_library::load(const std::filesystem::path& path, std::string& error)
    {
        unload();
        dlerror();
        void* _module = dlopen(path.string().c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!_module) {
            error += "Unknown dlopen error";
            return false;
        }
        _handle = _module;
        _loaded_path = path;
        return true;
    }

    void object_reload_library::unload()
    {
        if (!_handle) {
            return;
        }
        dlclose(_handle);
        _handle = nullptr;
        _loaded_path.clear();
    }

    void* object_reload_library::symbol_raw(const char* name, std::string& error) const
    {
        LUCARIA_DEBUG_ASSERT(_handle, "Library is not loaded");
        dlerror();
        void* _symbol = dlsym(_handle, name);
        const char* _error = dlerror();
        if (_error) {
            error = _error;
            return nullptr;
        }
        return _symbol;
    }

    int object_reload_module::_run_process_capture_output(const std::string& command, std::string& output)
    {
        const std::string _command = command + " 2>&1";
        FILE* _pipe = popen(_command.c_str(), "r");
        if (!_pipe) {
            output += "Failed to run command from command " + command;
            return -1;
        }

        std::array<char, 4096> _buffer = {};
        while (fgets(_buffer.data(), static_cast<int>(_buffer.size()), _pipe)) {
            output += _buffer.data();
        }
        const int _status = pclose(_pipe);
        if (_status == -1) {
            output += "Failed to close command pipe from command " + command;
            return -1;
        }

        if (WIFEXITED(_status)) {
            return WEXITSTATUS(_status);
        }
        return _status;
    }

}
}