#pragma once

#include <lucaria/core/serialize_game.hpp>

namespace lucaria {

struct context_save_game final : detail::game_save_context {
    using detail::game_save_context::game_save_context;

    template <typename ValueType>
    void field(std::string_view name, const ValueType& value)
    {
        detail::save_context_field(*this, archive, name, value);
    }
};

struct context_load_game final : detail::game_load_context {
    using detail::game_load_context::game_load_context;

    template <typename ValueType>
    void field(std::string_view name, ValueType& value)
    {
        detail::load_context_field("context_load_game", *this, archive, name, value);
    }
};

struct context_save_storage final : detail::storage_save_context {
    using detail::storage_save_context::storage_save_context;

    template <typename ValueType>
    void field(std::string_view name, const ValueType& value)
    {
        detail::save_context_field(*this, archive, name, value);
    }
};

struct context_load_storage : detail::storage_load_context {
    using detail::storage_load_context::storage_load_context;

    template <typename ValueType>
    void field(std::string_view name, ValueType& value)
    {
        detail::load_context_field("context_load_storage", *this, archive, name, value);
    }
};

static_assert(sizeof(context_save_game) == sizeof(detail::game_save_context));
static_assert(sizeof(context_load_game) == sizeof(detail::game_load_context));
static_assert(sizeof(context_save_storage) == sizeof(detail::storage_save_context));
static_assert(sizeof(context_load_storage) == sizeof(detail::storage_load_context));

}
