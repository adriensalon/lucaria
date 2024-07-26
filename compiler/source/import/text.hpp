#pragma once

#include <filesystem>
#include <string>

#include <data/shader.hpp>

shader_data import_text(const std::filesystem::path& text_path);
