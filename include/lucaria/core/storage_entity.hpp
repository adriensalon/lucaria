#pragma once

#include <ostream>

#include <lucaria/bin/types_math.hpp>

namespace lucaria {
namespace detail {

    // uint64 entity:
    // [ 32-bit version ][ 16-bit scene ][ 16-bit local ]

    using object_entity_scene_index = uint16;
    using object_entity_local_index = uint16;
    using object_entity_version = uint32;

    struct object_entity_layout {

        static constexpr uint32 local_bits = 16;
        static constexpr uint32 scene_bits = 16;
        static constexpr uint32 entity_bits = local_bits + scene_bits;
        static constexpr uint32 version_bits = 32;

        static constexpr uint64 local_mask = (uint64 { 1 } << local_bits) - 1u;
        static constexpr uint64 scene_mask = (uint64 { 1 } << scene_bits) - 1u;
        static constexpr uint64 entity_mask = (uint64 { 1 } << entity_bits) - 1u;
        static constexpr uint64 version_mask = (uint64 { 1 } << version_bits) - 1u;

        static constexpr uint32 scene_shift = local_bits;
        static constexpr uint32 version_shift = entity_bits;
    };

    enum struct object_entity : uint64 { };

    [[nodiscard]] static constexpr uint64 entity_raw(object_entity entity) noexcept
    {
        return static_cast<uint64>(entity);
    }

    [[nodiscard]] static constexpr uint32 make_entity_part(object_entity_scene_index scene, object_entity_local_index local) noexcept
    {
        return (uint32 { scene } << object_entity_layout::scene_shift) | uint32 { local };
    }

    [[nodiscard]] static constexpr object_entity make_entity(object_entity_scene_index scene, object_entity_local_index local, object_entity_version version) noexcept
    {
        return static_cast<object_entity>((uint64 { version } << object_entity_layout::version_shift) | uint64 { make_entity_part(scene, local) });
    }

    [[nodiscard]] static constexpr object_entity_scene_index entity_scene(object_entity entity) noexcept
    {
        return object_entity_scene_index((entity_raw(entity) & object_entity_layout::entity_mask) >> object_entity_layout::scene_shift);
    }

    [[nodiscard]] static constexpr object_entity_local_index entity_local(object_entity entity) noexcept
    {
        return object_entity_local_index(entity_raw(entity) & object_entity_layout::local_mask);
    }

    [[nodiscard]] static constexpr object_entity_version entity_version(object_entity entity) noexcept
    {
        return object_entity_version(entity_raw(entity) >> object_entity_layout::version_shift);
    }

    inline std::ostream& operator<<(std::ostream& os, object_entity entity)
    {
        return os << entity_raw(entity);
    }

}
}
