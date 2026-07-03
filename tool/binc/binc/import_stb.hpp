#pragma once

#include <filesystem>

#include <lucaria/bin/data_image.hpp>

lucaria::data_image import_stb(const std::filesystem::path& stb_path);
lucaria::data_image import_stb(const std::filesystem::path& stb_path, lucaria::uint32 max_size);
void write_stb_png(const lucaria::data_image& image, const std::filesystem::path& stb_path);
