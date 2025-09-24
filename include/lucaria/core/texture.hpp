#pragma once

#include <filesystem>
#include <future>
#include <optional>

#include <glm/glm.hpp>

#include <lucaria/common/image.hpp>

namespace lucaria {

struct texture_ref {
    texture_ref() = delete;
    texture_ref(const texture_ref& other) = delete;
    texture_ref& operator=(const texture_ref& other) = delete;
    texture_ref(texture_ref&& other);
    texture_ref& operator=(texture_ref&& other);
    ~texture_ref();

    texture_ref(const image_data& data);
    glm::uint get_width() const;
    glm::uint get_height() const;
    glm::uint get_id() const;

private:
    bool _is_instanced;
    glm::uint _width;
    glm::uint _height;
    glm::uint _texture_id;
};

image_data load_image_data(const std::vector<char>& image_bytes);
image_data load_compressed_image_data(const std::vector<char>& image_bytes);
std::shared_future<std::shared_ptr<texture_ref>> fetch_texture(const std::filesystem::path& image_path, const std::optional<std::filesystem::path>& etc_image_path = std::nullopt, const std::optional<std::filesystem::path>& s3tc_image_path = std::nullopt);
void clear_texture_fetches();

}
