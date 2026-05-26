#pragma once

#include <filesystem>

#include <lucaria/bin/data_image.hpp>

lucaria::data_image import_stb(const std::filesystem::path& stb_path);
