#pragma once

#include <span>
#include <type_traits>

#include <lucaria/core/storage_segment.hpp>
#include <lucaria/public/handle_entity.hpp>

namespace entt {

template <typename T, typename Allocator>
    requires(!std::is_same_v<T, ::lucaria::detail::object_entity>)
struct storage_type<T, ::lucaria::detail::object_entity, Allocator> {
    using type = sigh_mixin<
        ::lucaria::detail::object_segment_storage_cpu<T, ::lucaria::detail::object_entity, Allocator>>;
};

}

namespace lucaria {
namespace detail {

    template <typename... T>
    struct exclude_t {
    };

    template <typename... T>
    inline constexpr exclude_t<T...> exclude {};

    template <typename ExcludedComponentTypesList, typename LeadComponentType, typename... RestComponentTypes>
    struct object_registry_view_cpu;

    template <typename... ExcludedComponentTypes, typename LeadComponentType, typename... RestComponentTypes>
    struct object_registry_view_cpu<exclude_t<ExcludedComponentTypes...>, LeadComponentType, RestComponentTypes...> {

        template <typename T>
        using storage_component_type = std::remove_cvref_t<T>;
        using lead_component_type = storage_component_type<LeadComponentType>;
        using lead_storage_type = object_segment_storage_cpu<lead_component_type, object_entity, std::allocator<lead_component_type>>;

        template <typename T>
        using storage_type = object_segment_storage_cpu<storage_component_type<T>, object_entity, std::allocator<storage_component_type<T>>>;
        using segment_type = typename lead_storage_type::segment;
        using segment_buffer_type = std::vector<segment_type>;

        template <typename T>
        using view_reference_type = std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, const storage_component_type<T>&, storage_component_type<T>&>;

        object_registry_view_cpu(
            std::shared_ptr<const segment_buffer_type> segments,
            std::size_t begin,
            std::size_t end,
            std::uint64_t entity_count,
            std::tuple<storage_type<RestComponentTypes>*...> rest,
            std::tuple<storage_type<ExcludedComponentTypes>*...> excluded)
            : _segments { std::move(segments) }
            , _begin { begin }
            , _end { end }
            , _rest_storage { rest }
            , _excluded_storage { excluded }
            , _entity_count { entity_count }
        {
        }

        template <typename CallbackType>
        void each(CallbackType&& callback) const
        {
            const auto& _segments_reference = *_segments;
            for (std::size_t _segment_index = _begin; _segment_index < _end; ++_segment_index) {
                const auto& _current_segment = _segments_reference[_segment_index];
                for (std::uint32_t i = 0; i < _current_segment.count; ++i) {
                    const auto _entity = _current_segment.entities[i];
                    if (!_has_required(_current_segment.scene, _entity)) {
                        continue;
                    }
                    if (_has_excluded(_current_segment.scene, _entity)) {
                        continue;
                    }
                    _call(std::forward<CallbackType>(callback), _current_segment.scene, handle_entity { _entity }, _current_segment.components[i]);
                }
            }
        }

        [[nodiscard]] std::vector<object_registry_view_cpu> split(std::size_t requested_count) const
        {
            std::vector<object_registry_view_cpu> _result = {};
            if (requested_count == 0 || empty()) {
                return _result;
            }
            if (requested_count == 1) {
                _result.push_back(*this);
                return _result;
            }
            const std::uint64_t _total = _entity_count;
            const std::uint64_t _target = (_total + requested_count - 1u) / requested_count;
            _result.reserve(requested_count);
            std::size_t _part_begin = _begin;
            std::uint64_t _current_count = 0;
            for (std::size_t _segment_index = _begin; _segment_index < _end; ++_segment_index) {
                _current_count += (*_segments)[_segment_index].count;
                const bool _should_cut = (_current_count >= _target) && (_result.size() + 1u < requested_count) && (_segment_index + 1u < _end);
                if (_should_cut) {
                    _result.emplace_back(_segments, _part_begin, _segment_index + 1u, _current_count, _rest_storage, _excluded_storage);
                    _part_begin = _segment_index + 1u;
                    _current_count = 0;
                }
            }
            if (_part_begin < _end) {
                _result.emplace_back(_segments, _part_begin, _end, _current_count, _rest_storage, _excluded_storage);
            }
            return _result;
        }

        [[nodiscard]] std::uint64_t total_count() const noexcept
        {
            return _entity_count;
        }

        [[nodiscard]] std::uint64_t size() const noexcept
        {
            return _entity_count;
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return _entity_count == 0;
        }

    private:
        std::shared_ptr<const segment_buffer_type> _segments = {};
        std::size_t _begin = {};
        std::size_t _end = {};
        std::tuple<storage_type<RestComponentTypes>*...> _rest_storage = {};
        std::tuple<storage_type<ExcludedComponentTypes>*...> _excluded_storage = {};
        std::uint64_t _entity_count = {};

        [[nodiscard]] bool _has_required(object_entity_scene_index scene, object_entity entity) const
        {
            if constexpr (sizeof...(RestComponentTypes) == 0) {
                return true;
            } else {
                return (std::get<storage_type<RestComponentTypes>*>(_rest_storage)->contains(scene, entity) && ...);
            }
        }

        [[nodiscard]] bool _has_excluded(object_entity_scene_index scene, object_entity entity) const
        {
            if constexpr (sizeof...(ExcludedComponentTypes) == 0) {
                return false;
            } else {
                return (std::get<storage_type<ExcludedComponentTypes>*>(_excluded_storage)->contains(scene, entity) || ...);
            }
        }

        template <typename CallbackType>
        void _call(CallbackType&& callback, object_entity_scene_index scene, handle_entity entity, lead_component_type& lead) const
        {
            view_reference_type<LeadComponentType> _lead_reference = lead;
            if constexpr (sizeof...(RestComponentTypes) == 0) {
                _call_with_rest(
                    std::forward<CallbackType>(callback),
                    scene,
                    entity,
                    _lead_reference);
            } else {
                _call_with_rest(
                    std::forward<CallbackType>(callback),
                    scene,
                    entity,
                    _lead_reference,
                    static_cast<view_reference_type<RestComponentTypes>>(
                        std::get<storage_type<RestComponentTypes>*>(_rest_storage)->get(scene, entity._entity))...);
            }
        }

        template <typename CallbackType, typename... Components>
        void _call_with_rest(CallbackType&& callback, object_entity_scene_index scene, handle_entity entity, Components&&... components) const
        {
            if constexpr (std::is_invocable_v<CallbackType, object_entity, Components...>) { // for detail::
                std::forward<CallbackType>(callback)(entity._entity, std::forward<Components>(components)...);
            } else if constexpr (std::is_invocable_v<CallbackType, handle_entity, Components...>) {
                std::forward<CallbackType>(callback)(entity, std::forward<Components>(components)...);
            } else if constexpr (std::is_invocable_v<CallbackType, Components...>) {
                std::forward<CallbackType>(callback)(std::forward<Components>(components)...);
            } else {
                static_assert(false, "Invalid view callback signature.");
            }
        }
    };

}
}
