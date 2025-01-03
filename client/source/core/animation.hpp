#pragma once

#include <filesystem>
#include <future>
#include <memory>

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/track.h>

using animation_ref = ozz::animation::Animation;
using motion_track_ref = std::pair<ozz::animation::Float3Track, ozz::animation::QuaternionTrack>;

std::shared_future<std::shared_ptr<animation_ref>> fetch_animation(const std::filesystem::path& animation_path);
std::shared_future<std::shared_ptr<motion_track_ref>> fetch_motion_track(const std::filesystem::path& motion_track_path);
