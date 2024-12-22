#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <imgui.h>

using font_data = std::vector<std::string>;

struct font_ref {
    font_ref() = delete;
    font_ref(const font_ref& other) = delete;
    font_ref& operator=(const font_ref& other) = delete;
    font_ref(font_ref&& other) = default;
    font_ref& operator=(font_ref&& other) = default;

    font_ref(const font_data& data, const glm::float32 font_size);
    ImFont* get_font(const glm::uint index = 0) const;
    glm::uint get_count() const;

private:
    std::vector<ImFont*> _ptrs = {};
};

std::shared_future<std::shared_ptr<font_ref>> fetch_font(const std::vector<std::filesystem::path>& font_paths, const glm::float32 font_size = 13.f);
void clear_font_fetches();