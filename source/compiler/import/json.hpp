#pragma once

#include <filesystem>
#include <string>

#include <lucaria/bin/event_track_data.hpp>

lucaria::data_event_track import_event_track(const std::filesystem::path& evtt_path);
