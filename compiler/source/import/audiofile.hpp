#pragma once

#include <filesystem>

#include <data/audio.hpp>

struct imported_audiofile_data {
    glm::uint sample_rate;
    glm::uint bit_depth;
    audio_data song_audio;
};

imported_audiofile_data import_audiofile(const std::filesystem::path& audiofile_path);
