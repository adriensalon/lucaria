#pragma once

#include <filesystem>
#include <string>

#include <lucaria/common/event_track_data.hpp>

lucaria::event_track_data import_event_track(const std::filesystem::path& evtt_path);
