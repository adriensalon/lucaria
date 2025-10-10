#pragma once

#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

namespace lucaria {

struct event_data {
    std::string name;
    glm::float32 frame;
    glm::float32 time;
    glm::float32 time_normalized; // [0..1]

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("name", name));
        archive(cereal::make_nvp("frame", frame));
        archive(cereal::make_nvp("time", time));
        archive(cereal::make_nvp("time_normalized", time_normalized));
    }
};

struct event_track_data {
    glm::float32 frames_per_second;
    glm::float32 frame_start;
    glm::float32 frame_end;
    glm::float32 duration_seconds;
    std::vector<event_data> events;

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("frames_per_second", frames_per_second));
        archive(cereal::make_nvp("frame_start", frame_start));
        archive(cereal::make_nvp("frame_end", frame_end));
        archive(cereal::make_nvp("duration_seconds", duration_seconds));
        archive(cereal::make_nvp("events", events));
    }
};

}