#pragma once

#include <filesystem>
#include <future>
#include <memory>

#include <data/volume.hpp>

struct volume_ref {
    volume_ref() = delete;
    volume_ref(const volume_ref& other) = default;
    volume_ref& operator=(const volume_ref& other) = default;
    volume_ref(volume_ref&& other) = default;
    volume_ref& operator=(volume_ref&& other) = default;

    volume_ref(const volume_data& data);
    bool get_is_contained(const glm::vec3& position);

private:
    glm::vec3 _minimum;
    glm::vec3 _maximum;
};

volume_data load_volume_data(std::istringstream& volume_stream);
std::shared_future<std::shared_ptr<volume_ref>> fetch_volume(const std::filesystem::path& volume_path);
