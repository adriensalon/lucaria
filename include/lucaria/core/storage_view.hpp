#pragma once

#include <span>
#include <type_traits>

#include <lucaria/core/storage_segment.hpp>

namespace entt {

template <typename T, typename Allocator>
    requires(!std::is_same_v<T, ::lucaria::detail::object_entity>)
struct storage_type<T, ::lucaria::detail::object_entity, Allocator> {
    using type = sigh_mixin<
        ::lucaria::detail::object_segment_storage_cpu<
            T,
            ::lucaria::detail::object_entity,
            Allocator>>;
};

}

namespace lucaria {
namespace detail {

    template <typename... T>
    struct exclude_t {
    };

    template <typename... T>
    inline constexpr exclude_t<T...> exclude {};

    template <typename ExcludeList, typename Lead, typename... Rest>
    struct object_registry_view_cpu;

    template <typename... Excluded, typename Lead, typename... Rest>
    struct object_registry_view_cpu<exclude_t<Excluded...>, Lead, Rest...> {

        template <typename T>
        using storage_component_type = std::remove_cvref_t<T>;

        using lead_component_type = storage_component_type<Lead>;

        using lead_storage_type = object_segment_storage_cpu<
            lead_component_type,
            object_entity,
            std::allocator<lead_component_type>>;

        template <typename T>
        using storage_type = object_segment_storage_cpu<
            storage_component_type<T>,
            object_entity,
            std::allocator<storage_component_type<T>>>;

        using segment_type = typename lead_storage_type::segment;
        using segment_buffer = std::vector<segment_type>;

        template <typename T>
        using view_ref_t = std::conditional_t<
            std::is_const_v<std::remove_reference_t<T>>,
            const storage_component_type<T>&,
            storage_component_type<T>&>;

        object_registry_view_cpu(
            std::shared_ptr<const segment_buffer> segments,
            std::size_t begin,
            std::size_t end,
            std::uint64_t entity_count,
            std::tuple<storage_type<Rest>*...> rest,
            std::tuple<storage_type<Excluded>*...> excluded)
            : segments_ { std::move(segments) }
            , begin_ { begin }
            , end_ { end }
            , rest_ { rest }
            , excluded_ { excluded }
            , entity_count_ { entity_count }
        {
        }

        template <typename Fn>
        void each(Fn&& fn) const
        {
            const auto& segments = *segments_;

            for (std::size_t s = begin_; s < end_; ++s) {
                const auto& seg = segments[s];

                for (std::uint32_t i = 0; i < seg.count; ++i) {
                    const auto entity = seg.entities[i];

                    if (!has_required(seg.scene, entity)) {
                        continue;
                    }

                    if (has_excluded(seg.scene, entity)) {
                        continue;
                    }

                    call(
                        std::forward<Fn>(fn),
                        seg.scene,
                        entity,
                        seg.components[i]);
                }
            }
        }

        [[nodiscard]] std::vector<object_registry_view_cpu> split(std::size_t requested_count) const
        {
            std::vector<object_registry_view_cpu> result;

            if (requested_count == 0 || empty()) {
                return result;
            }

            if (requested_count == 1) {
                result.push_back(*this);
                return result;
            }

            const std::uint64_t total = entity_count_;
            const std::uint64_t target = (total + requested_count - 1u) / requested_count;

            result.reserve(requested_count);

            std::size_t part_begin = begin_;
            std::uint64_t current_count = 0;

            for (std::size_t s = begin_; s < end_; ++s) {
                current_count += (*segments_)[s].count;

                const bool should_cut = current_count >= target
                    && result.size() + 1u < requested_count
                    && s + 1u < end_;

                if (should_cut) {
                    result.emplace_back(
                        segments_,
                        part_begin,
                        s + 1u,
                        current_count,
                        rest_,
                        excluded_);

                    part_begin = s + 1u;
                    current_count = 0;
                }
            }

            if (part_begin < end_) {
                result.emplace_back(
                    segments_,
                    part_begin,
                    end_,
                    current_count,
                    rest_,
                    excluded_);
            }

            return result;
        }

        [[nodiscard]] std::uint64_t total_count() const noexcept
        {
            return entity_count_;
        }

        [[nodiscard]] std::uint64_t size() const noexcept
        {
            return entity_count_;
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return entity_count_ == 0;
        }

    private:
        [[nodiscard]] bool has_required(
            object_entity_scene_index scene,
            object_entity entity) const
        {
            if constexpr (sizeof...(Rest) == 0) {
                return true;
            } else {
                return (std::get<storage_type<Rest>*>(rest_)->contains(scene, entity) && ...);
            }
        }

        [[nodiscard]] bool has_excluded(
            object_entity_scene_index scene,
            object_entity entity) const
        {
            if constexpr (sizeof...(Excluded) == 0) {
                return false;
            } else {
                return (std::get<storage_type<Excluded>*>(excluded_)->contains(scene, entity) || ...);
            }
        }

        template <typename Fn>
        void call(
            Fn&& fn,
            object_entity_scene_index scene,
            object_entity entity,
            lead_component_type& lead) const
        {
            view_ref_t<Lead> lead_ref = lead;

            if constexpr (sizeof...(Rest) == 0) {
                call_with_rest(
                    std::forward<Fn>(fn),
                    scene,
                    entity,
                    lead_ref);
            } else {
                call_with_rest(
                    std::forward<Fn>(fn),
                    scene,
                    entity,
                    lead_ref,
                    static_cast<view_ref_t<Rest>>(
                        std::get<storage_type<Rest>*>(rest_)->get(scene, entity))...);
            }
        }

        template <typename Fn, typename... Components>
        void call_with_rest(
            Fn&& fn,
            object_entity_scene_index scene,
            object_entity entity,
            Components&&... components) const
        {
            if constexpr (std::is_invocable_v<Fn, object_entity_scene_index, object_entity, Components...>) {
                std::forward<Fn>(fn)(scene, entity, std::forward<Components>(components)...);
            } else if constexpr (std::is_invocable_v<Fn, object_entity, Components...>) {
                std::forward<Fn>(fn)(entity, std::forward<Components>(components)...);
            } else if constexpr (std::is_invocable_v<Fn, Components...>) {
                std::forward<Fn>(fn)(std::forward<Components>(components)...);
            } else {
                static_assert(
                    std::is_invocable_v<Fn, object_entity_scene_index, object_entity, Components...>
                        || std::is_invocable_v<Fn, object_entity, Components...>
                        || std::is_invocable_v<Fn, Components...>,
                    "Invalid view callback signature.");
            }
        }

    private:
        std::shared_ptr<const segment_buffer> segments_;
        std::size_t begin_ {};
        std::size_t end_ {};
        std::tuple<storage_type<Rest>*...> rest_;
        std::tuple<storage_type<Excluded>*...> excluded_;
        std::uint64_t entity_count_ {};
    };

    template <typename T>
    struct object_component_readback_gpu {
        object_entity entity {};
        T component {};
    };

    template <typename T>
    struct object_component_upload_snapshot_gpu {
        std::span<const object_entity> entities;
        std::span<const T> components;
    };

    template <typename T>
    struct object_component_download_snapshot_gpu {
        std::span<const object_entity> entities;
        std::span<const T> components;
    };

    struct object_dirty_range_gpu {
        std::uint32_t begin {};
        std::uint32_t end {};

        [[nodiscard]] bool any() const noexcept
        {
            return begin < end;
        }

        [[nodiscard]] std::uint32_t count() const noexcept
        {
            return end - begin;
        }

        void include(std::uint32_t index) noexcept
        {
            if (!any()) {
                begin = index;
                end = index + 1u;
                return;
            }

            if (index < begin) {
                begin = index;
            }

            if (index + 1u > end) {
                end = index + 1u;
            }
        }

        void include_all(std::uint32_t count_) noexcept
        {
            begin = 0u;
            end = count_;
        }

        void clear() noexcept
        {
            begin = 0u;
            end = 0u;
        }
    };

}
}
