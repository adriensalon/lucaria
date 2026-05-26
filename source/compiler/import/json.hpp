#pragma once

#include <filesystem>
#include <string>

#include <lucaria/bin/data_event_track.hpp>

lucaria::data_event_track import_event_track(const std::filesystem::path& evtt_path);
