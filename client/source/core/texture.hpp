#pragma once

#include <filesystem>
#include <future>

#include <glm/glm.hpp>

#include <data/image.hpp>

struct texture_ref {
    texture_ref() = delete;
    texture_ref(const texture_ref& other) = delete;
    texture_ref& operator=(const texture_ref& other) = delete;
    texture_ref(texture_ref&& other);
    texture_ref& operator=(texture_ref&& other);
    ~texture_ref();

    texture_ref(const image_data& data);
    glm::uint get_id() const;

private:
    bool _is_instanced;
    glm::uint _texture_id;
};

image_data load_image_data(std::istringstream& image_stream);
std::shared_future<std::shared_ptr<texture_ref>> fetch_texture(const std::filesystem::path& texture_path);
