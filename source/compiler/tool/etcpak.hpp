#pragma once

#include <filesystem>

enum struct etcpak_mode {
    etc,
    s3tc
};

void execute_etcpak(const etcpak_mode mode, const std::filesystem::path& etcpak_exe, const std::filesystem::path& input_path, const std::filesystem::path& output_path);
