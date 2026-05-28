#pragma once

#include <type_traits>
#include <utility>
#include <vector>

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <entt/entt.hpp>
#include <pfr.hpp>

namespace lucaria {

struct context_game;

namespace detail {

    // construct from const std::vector<char>&

    template <typename T>
    struct is_bytes_compatible : std::is_constructible<T, const std::vector<char>&> { };

    template <typename T>
    constexpr bool is_bytes_compatible_v = is_bytes_compatible<T>::value;

    // std::any traits

    template <typename T>
    inline constexpr bool is_any_compatible_v = std::is_copy_constructible_v<std::decay_t<T>>;

    // cereal traits

    template <typename T, typename InputArchive, typename OutputArchive>
    struct is_cereal_compatible_archives : std::bool_constant<cereal::traits::is_input_serializable<T, InputArchive>::value && cereal::traits::is_output_serializable<T, OutputArchive>::value> { };

    template <typename T, typename InputArchive, typename OutputArchive>
    inline constexpr bool is_cereal_compatible_archives_v = is_cereal_compatible_archives<T, InputArchive, OutputArchive>::value;

    template <typename T>
    inline constexpr bool is_cereal_compatible_v = is_cereal_compatible_archives_v<T, cereal::JSONInputArchive, cereal::JSONOutputArchive> && is_cereal_compatible_archives_v<T, cereal::PortableBinaryInputArchive, cereal::PortableBinaryOutputArchive>;

    // entt traits

    struct entt_emplace_factory_default_tag { };

    template <typename ComponentType>
    struct entt_emplace_factory : entt_emplace_factory_default_tag {

        template <typename ArchiveType>
        static ComponentType& emplace(ArchiveType&, entt::registry& registry, entt::entity entity)
        {
            static_assert(std::is_default_constructible_v<ComponentType>, "Component is not default constructible. Provide entt_emplace_factory specialization");
            return registry.emplace_or_replace<ComponentType>(entity);
        }
    };

    template <typename T>
    struct has_default_entt_emplace_factory : std::is_base_of<detail::entt_emplace_factory_default_tag, entt_emplace_factory<T>> { };

    template <typename T>
    inline constexpr bool has_default_entt_emplace_factory_v = has_default_entt_emplace_factory<T>::value;

    template <typename T>
    struct has_specialized_entt_emplace_factory : std::integral_constant<bool, !has_default_entt_emplace_factory_v<T>> { };

    template <typename T>
    inline constexpr bool has_specialized_entt_emplace_factory_v = has_specialized_entt_emplace_factory<T>::value;

    template <typename T>
    struct is_entt_constructible : std::bool_constant<has_specialized_entt_emplace_factory_v<T> || std::is_default_constructible_v<T>> { };

    template <typename T>
    inline constexpr bool is_entt_constructible_v = is_entt_constructible<T>::value;

    template <typename T>
    struct is_entt_compatible : std::bool_constant<is_entt_constructible_v<T> && std::is_object_v<T> && !std::is_array_v<T> && std::is_move_constructible_v<T> && std::is_move_assignable_v<T>> { };

    template <typename T>
    inline constexpr bool is_entt_compatible_v = is_entt_compatible<T>::value;

    // pfr traits

    template <typename T>
    using clean_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    template <typename, typename = void>
    struct is_pfr_compatible : std::false_type { };

    template <typename T>
    struct is_pfr_compatible<T, std::void_t<decltype(pfr::tuple_size<clean_t<T>>::value)>> : std::bool_constant<std::is_aggregate_v<clean_t<T>> && std::is_class_v<clean_t<T>> && !std::is_union_v<clean_t<T>>> { };

    template <typename T>
    inline constexpr bool is_pfr_compatible_v = is_pfr_compatible<T>::value;

}
}