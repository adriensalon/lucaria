#pragma once

#include <filesystem>

#include <lucaria/bin/image_data.hpp>

lucaria::image_data import_stb(const std::filesystem::path& stb_path);
