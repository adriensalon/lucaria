#pragma once

#include <filesystem>

#include <data/image.hpp>

struct imported_stb_data {
    image_data image;
    bool is_hdr;
};

imported_stb_data import_stb(const std::filesystem::path& stb_path);
