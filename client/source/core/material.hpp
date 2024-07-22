#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <unordered_map>

#include <core/texture.hpp>

enum struct material_texture {
    color,
    normal,
    occlusion,
    roughness,
    metallic
};

inline const std::unordered_map<material_texture, std::size_t> model_texture_channels = {
    { material_texture::color, 4 },
    { material_texture::normal, 3 },
    { material_texture::occlusion, 1 },
    { material_texture::roughness, 1 },
    { material_texture::metallic, 1 },
};

struct material_ref {
    material_ref() = delete;
    material_ref(const material_ref& other) = delete;
    material_ref& operator=(const material_ref& other) = delete;
    material_ref(material_ref&& other) = default;
    material_ref& operator=(material_ref&& other) = default;

    material_ref(const std::unordered_map<material_texture, image_data>& textures);

    bool get_has_texture(const material_texture texture);
    texture_ref& get_texture(const material_texture texture);

private:
    std::unordered_map<material_texture, std::shared_ptr<texture_ref>> _textures;
};

std::shared_future<std::shared_ptr<material_ref>> fetch_material(const std::unordered_map<material_texture, std::filesystem::path>& texture_paths);
