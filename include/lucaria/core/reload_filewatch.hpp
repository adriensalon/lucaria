#pragma once

#include <filesystem>
#include <optional>
#include <system_error>

namespace lucaria {
namespace detail {

    struct object_filewatched_path {
        object_filewatched_path() = default;
        object_filewatched_path(const std::filesystem::path& path);
        object_filewatched_path(const object_filewatched_path&) = default;
        object_filewatched_path& operator=(const object_filewatched_path&) = default;
        object_filewatched_path(object_filewatched_path&&) = default;
        object_filewatched_path& operator=(object_filewatched_path&&) = default;
		
        [[nodiscard]] bool exists() const;
        [[nodiscard]] bool peek_changed() const;
        [[nodiscard]] bool has_changed(); 
        [[nodiscard]] const std::filesystem::path& get() const;
        [[nodiscard]] operator const std::filesystem::path&() const;
        void refresh();

    private:
        std::filesystem::path _path = {};
        std::optional<std::filesystem::file_time_type> _last_write_time = std::nullopt;
    };

}
}
