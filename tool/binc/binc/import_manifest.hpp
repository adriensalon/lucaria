#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace detail {

struct asset_profiles {
    bool raw = true;
    bool etc = false;
    bool s3tc = false;
    std::uint32_t texture_size = 1024;
};

struct asset_manifest_entry {
    std::int64_t timestamp = 0;
    std::string profile;
    std::vector<std::string> outputs;
};

using asset_manifest = std::unordered_map<std::string, asset_manifest_entry>;

std::string manifest_path(const std::filesystem::path& path);
std::int64_t file_timestamp(const std::filesystem::path& path);
std::string profile_key(asset_profiles profiles);

std::vector<std::filesystem::path> expected_outputs(
    asset_profiles profiles,
    const std::filesystem::path& input_file,
    const std::filesystem::path& output_file);

std::vector<std::string> manifest_outputs(const std::vector<std::filesystem::path>& outputs);

bool is_up_to_date(
    const asset_manifest& manifest,
    const std::string& input,
    std::int64_t timestamp,
    const std::string& profile,
    const std::vector<std::filesystem::path>& outputs);

asset_manifest load_manifest(const std::optional<std::filesystem::path>& path);
void write_manifest(const std::optional<std::filesystem::path>& path, const asset_manifest& manifest);

}
