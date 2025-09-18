#pragma once

#include <filesystem>

#include <lucaria/common/image.hpp>

image_data import_stb(const std::filesystem::path& stb_path);
