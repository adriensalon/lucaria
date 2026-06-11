#include <lucaria/core/reload_filewatch.hpp>

namespace lucaria {
namespace detail {

    namespace {

        std::optional<std::filesystem::file_time_type> _current_write_time(const std::filesystem::path& path)
        {
            std::error_code _error;
            if (!std::filesystem::exists(path, _error) || _error) {
                return std::nullopt;
            }
            const std::filesystem::file_time_type _time = std::filesystem::last_write_time(path, _error);
            if (_error) {
                return std::nullopt;
            }
            return _time;
        }

    }

    object_filewatched_path::object_filewatched_path(const std::filesystem::path& path)
        : _path(path)
    {
        refresh();
    }

    bool object_filewatched_path::exists() const
    {
        std::error_code _error;
        return std::filesystem::exists(_path, _error);
    }

    bool object_filewatched_path::peek_changed() const
    {
        return _current_write_time(_path) != _last_write_time;
    }

    bool object_filewatched_path::has_changed()
    {
        const std::optional<std::filesystem::file_time_type> _now = _current_write_time(_path);
        if (_now != _last_write_time) {
            _last_write_time = _now;
            return true;
        }
        return false;
    }

    const std::filesystem::path& object_filewatched_path::get() const
    {
        return _path;
    }

    object_filewatched_path::operator const std::filesystem::path&() const
    {
        return _path;
    }

    void object_filewatched_path::refresh()
    {
        _last_write_time = _current_write_time(_path);
    }

}
}