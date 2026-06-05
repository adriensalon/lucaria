#pragma once

#include <lucaria/bin/types_math.hpp>

namespace lucaria {
namespace detail {

    struct manager_scenes;

    /// @brief 
    enum struct object_entity : uint64 { };
}

struct context_object;
struct context_scene;

/// @brief
struct handle_entity {

    /// @brief
    [[nodiscard]] bool has_value() const;

    // private:
    detail::object_entity _entity = {};

    template <typename ArchiveType>
    void save(ArchiveType& archive) const;

    template <typename ArchiveType>
    void load(ArchiveType& archive);

    friend struct context_object;
    friend struct context_scene;
    friend struct detail::manager_scenes;
    friend class cereal::access;
};

}