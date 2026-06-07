#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/game_access.hpp>
#include <lucaria/engine/handle_entity.hpp>

namespace lucaria {

struct component_transform;

namespace detail {

	struct manager_scenes;

    struct system_mixer {
        system_mixer() = default;
        system_mixer(const system_mixer& other) = delete;
        system_mixer& operator=(const system_mixer& other) = delete;
        system_mixer(system_mixer&& other) = default;
        system_mixer& operator=(system_mixer&& other) = default;

        std::optional<handle_entity> listener_entity = std::nullopt;
        component_transform* listener_transform = nullptr;

        void use_listener_transform(handle_entity entity);
        void resolve_runtime_references(manager_scenes& scenes);
        void update_apply_speaker_transforms(manager_scenes& scenes);
        void update_apply_listener_transform(manager_scenes& scenes);

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("listener_entity", listener_entity));
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("listener_entity", listener_entity));
            listener_transform = nullptr;
        }
    };

}

}
