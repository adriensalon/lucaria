#pragma once

#include <filesystem>
#include <string>

#include <lucaria/bin/shader_data.hpp>

lucaria::data_shader import_text(const std::filesystem::path& text_path);
