#pragma once

#include <filesystem>

enum struct etcpak_mode {
    etc,
    s3tc,
    s3tc_psp
};

void execute_etcpak(const etcpak_mode mode, const std::filesystem::path& input_path, const std::filesystem::path& output_path);
