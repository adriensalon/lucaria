#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <lucaria/core/utils_error.hpp>
#include <lucaria/core/utils_reload.hpp>

namespace lucaria {
namespace detail {

    namespace {

        [[nodiscard]] static std::wstring _to_wstring_path(const std::filesystem::path& path)
        {
            return path.wstring();
        }

        [[nodiscard]] static std::string _last_error()
        {
            DWORD _error = GetLastError();
            if (_error == 0) {
                return {};
            }
            LPSTR _buffer = nullptr;
            DWORD _size = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                _error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&_buffer),
                0,
                nullptr);
            std::string _result = _size && _buffer ? std::string(_buffer, _size) : "Unknown Win32 error";
            if (_buffer) {
                LocalFree(_buffer);
            }
            return _result;
        }

        [[nodiscard]] static std::wstring _to_wide(const std::string& text)
        {
            if (text.empty()) {
                return std::wstring {};
            }
            const int _count = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
            std::wstring _wide;
            _wide.resize(_count);
            MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), _wide.data(), _count);
            return _wide;
        }

    }

    bool object_reload_library::load(const std::filesystem::path& path, std::string& error)
    {
        unload();
        HMODULE _module = LoadLibraryW(_to_wstring_path(path).c_str());
        if (!_module) {
            error = _last_error();
            return false;
        }
        _handle = reinterpret_cast<void*>(_module);
        _loaded_path = path;
        return true;
    }

    void object_reload_library::unload()
    {
        if (!_handle) {
            return;
        }
        HMODULE _module = reinterpret_cast<HMODULE>(_handle);
        FreeLibrary(_module);
        _handle = nullptr;
        _loaded_path.clear();
    }

    void* object_reload_library::symbol_raw(const char* name, std::string& error) const
    {
        LUCARIA_DEBUG_ASSERT(_handle, "Library is not loaded");
        FARPROC _symbol = GetProcAddress(reinterpret_cast<HMODULE>(_handle), name);
        if (!_symbol) {
            error = _last_error();
			return nullptr;
        }
        return reinterpret_cast<void*>(_symbol);
    }

    int object_reload_module::_run_process_capture_output(const std::string& command, std::string& output)
    {
        SECURITY_ATTRIBUTES _security = {};
        _security.nLength = sizeof(_security);
        _security.bInheritHandle = TRUE;
        _security.lpSecurityDescriptor = nullptr;
        HANDLE _read_pipe = nullptr;
        HANDLE _write_pipe = nullptr;
        if (!CreatePipe(&_read_pipe, &_write_pipe, &_security, 0)) {
			output = "Failed to create command pipe with error " + _last_error();
			return -1;
		}

        SetHandleInformation(_read_pipe, HANDLE_FLAG_INHERIT, 0);
        STARTUPINFOW _startup = {};
        _startup.cb = sizeof(_startup);
        _startup.dwFlags = STARTF_USESTDHANDLES;
        _startup.hStdOutput = _write_pipe;
        _startup.hStdError = _write_pipe;
        _startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        PROCESS_INFORMATION _process = {};
        std::wstring _command_line = L"cmd.exe /C " + _to_wide(command);
        if (!CreateProcessW(nullptr, _command_line.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &_startup, &_process)) {
            output = "Failed to create process with error " + _last_error();
            return -1;
        }

        char _buffer[4096];
        DWORD _bytes_read = 0;
        while (ReadFile(_read_pipe, _buffer, sizeof(_buffer), &_bytes_read, nullptr) && _bytes_read > 0) {
            output.append(_buffer, _bytes_read);
        }
        CloseHandle(_read_pipe);
        WaitForSingleObject(_process.hProcess, INFINITE);
        DWORD _exit_code = 0;
        GetExitCodeProcess(_process.hProcess, &_exit_code);
        CloseHandle(_process.hThread);
        CloseHandle(_process.hProcess);
        return static_cast<int>(_exit_code);
    }

}
}
