#pragma once

#include <filesystem>
#include <future>
#include <memory>

#include <ozz/animation/runtime/animation.h>

using animation_ref = ozz::animation::Animation;

std::shared_future<std::shared_ptr<animation_ref>> fetch_animation(const std::filesystem::path& animation_path);
