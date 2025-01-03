#pragma once

#include <filesystem>
#include <future>
#include <memory>

#include <ozz/animation/runtime/skeleton.h>

#include <core/animation.hpp>

using skeleton_ref = ozz::animation::Skeleton;

std::shared_future<std::shared_ptr<skeleton_ref>> fetch_skeleton(const std::filesystem::path& skeleton_path);
