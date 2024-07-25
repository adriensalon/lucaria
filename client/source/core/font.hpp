#pragma once

#include <filesystem>
#include <future>
#include <memory>

#include <glm/glm.hpp>
#include <imgui.h>

using font_ref = ImFont;

std::shared_future<std::shared_ptr<font_ref>> fetch_font(const std::filesystem::path& font_path, const glm::float32 font_size = 13.f);
