#pragma once

namespace lucaria {

    struct event_track_data {
        glm::float32 frames_per_second;
        std::vector<std::string> animation_names;
        std::vector<std::optional<std::string>> animation_events;
    };

}