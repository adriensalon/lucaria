#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>

namespace lucaria {

struct data_event {
    std::string name = {};
    float32 frame = 0.f;
    float32 time = 0.f;
    float32 time_normalized = 0.f;

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("name", name));
        archive(cereal::make_nvp("frame", frame));
        archive(cereal::make_nvp("time", time));
        archive(cereal::make_nvp("time_normalized", time_normalized));
    }
};

struct data_event_track {
    float32 frames_per_second = 0.f;
    float32 frame_start = 0.f;
    float32 frame_end = 0.f;
    float32 duration_seconds = 0.f;
    std::vector<data_event> events = {};

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