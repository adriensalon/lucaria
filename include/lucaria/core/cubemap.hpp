#pragma once

#include <array>
#include <filesystem>
#include <future>
#include <memory>
#include <optional>
#include <sstream>
#include <unordered_map>

#include <glm/glm.hpp>

#include <lucaria/common/image.hpp>

enum struct cubemap_side : glm::uint {
    positive_x = 0,
    positive_y = 1,
    positive_z = 2,
    negative_x = 3,
    negative_y = 4,
    negative_z = 5
};

using cubemap_data = std::array<image_data, 6>;

struct cubemap_ref {
    cubemap_ref() = delete;
    cubemap_ref(const cubemap_ref& other) = delete;
    cubemap_ref& operator=(const cubemap_ref& other) = delete;
    cubemap_ref(cubemap_ref&& other);
    cubemap_ref& operator=(cubemap_ref&& other);
    ~cubemap_ref();

    cubemap_ref(const cubemap_data& data);
    glm::uint get_id() const;

private:
    bool _is_instanced;
    glm::uint _cubemap_id;
};

std::shared_future<std::shared_ptr<cubemap_ref>> fetch_cubemap(const std::array<std::filesystem::path, 6>& image_paths, const std::optional<std::array<std::filesystem::path, 6>>& etc_image_paths = std::nullopt, const std::optional<std::array<std::filesystem::path, 6>>& s3tc_image_paths = std::nullopt);
void clear_cubemap_fetches();