#pragma once

#include <filesystem>
#include <string>

#include <lucaria/common/shader.hpp>

lucaria::shader_data import_text(const std::filesystem::path& text_path);
