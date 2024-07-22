#pragma once

#include <filesystem>
#include <string>

#include <data/shader.hpp>

struct imported_text_data {
    shader_data shader;
    std::string version;
};

imported_text_data import_text(const std::filesystem::path& text_path);
