#pragma once

#include <filesystem>
#include <string>

#include <lucaria/bin/data_shader.hpp>

lucaria::data_shader import_text(const std::filesystem::path& text_path);
