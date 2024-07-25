#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <imgui.h>

struct font_ref {
    std::vector<ImFont*> ptrs = {};

    font_ref(std::vector<ImFont*> _ptr) : ptrs(_ptr) {}
};

std::shared_future<std::shared_ptr<font_ref>> fetch_font(const std::vector<std::filesystem::path>& font_paths, const glm::float32 font_size = 13.f);
