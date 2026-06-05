#pragma once

#include <span>
#include <type_traits>

#include <lucaria/core/scenes_entity.hpp>
#include <lucaria/core/scenes_buffer.hpp>
#include <lucaria/engine/handle_entity.hpp>

namespace lucaria {
namespace detail {

    template <typename... T>
    struct exclude_t {
    };

    template <typename... T>
    inline constexpr exclude_t<T...> exclude {};

    template <typename ExcludedComponentTypesList, typename LeadComponentType, typename... RestComponentTypes>
    struct storage_view;

    template <typename... ExcludedComponentTypes, typename LeadComponentType, typename... RestComponentTypes>
    struct storage_view<exclude_t<ExcludedComponentTypes...>, LeadComponentType, RestComponentTypes...> {

        template <typename T>
        using storage_component_type = std::remove_cvref_t<T>;
        using lead_component_type = storage_component_type<LeadComponentType>;
        using lead_storage_type = storage_buffer<lead_component_type, object_entity, std::allocator<lead_component_type>>;
        using lead_segment_type = typename lead_storage_type::segment;

        template <typename T>
        using storage_type = storage_buffer<storage_component_type<T>, object_entity, std::allocator<storage_component_type<T>>>;
        using segment_type = typename lead_storage_type::segment;
        using segment_buffer_type = std::vector<segment_type>;

        template <typename T>
        using view_reference_type = std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, const storage_component_type<T>&, storage_component_type<T>&>;

        storage_view(
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

        struct memory_segment {
            const storage_view* view = nullptr;
            const lead_segment_type* segment = nullptr;

            [[nodiscard]] object_entity_scene_index scene() const noexcept
            {
                return segment->scene;
            }

            [[nodiscard]] object_entity* entities() const noexcept
            {
                return segment->entities;
            }

            [[nodiscard]] lead_component_type* components() const noexcept
            {
                return segment->components;
            }

            [[nodiscard]] std::uint32_t count() const noexcept
            {
                return segment->count;
            }

            [[nodiscard]] const auto& rest_storage() const noexcept
            {
                return view->_rest_storage;
            }

            [[nodiscard]] const auto& excluded_storage() const noexcept
            {
                return view->_excluded_storage;
            }

            [[nodiscard]] std::uint64_t entity_count() const noexcept
            {
                return segment->count;
            }
        };

        template <typename CallbackType>
        void each_segment(CallbackType&& callback) const
        {
            const auto& segments = *_segments;
            for (std::size_t segment_index = _begin; segment_index < _end; ++segment_index) {
                const auto& current = segments[segment_index];
                if (current.count == 0) {
                    continue;
                }
                callback(memory_segment { this, &current });
            }
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

        [[nodiscard]] std::vector<storage_view> split(std::size_t requested_count) const
        {
            std::vector<storage_view> _result = {};
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

    template <typename ExcludedComponentTypesList, typename LeadComponentType, typename... RestComponentTypes>
    struct storage_view_compute;

    template <typename... ExcludedComponentTypes, typename LeadComponentType, typename... RestComponentTypes>
    struct storage_view_compute<exclude_t<ExcludedComponentTypes...>, LeadComponentType, RestComponentTypes...> {
        template <typename T>
        using storage_component_type = std::remove_cvref_t<T>;

        using lead_component_type = storage_component_type<LeadComponentType>;

        template <typename T>
        using storage_type = storage_buffer_compute<
            storage_component_type<T>,
            object_entity,
            std::allocator<storage_component_type<T>>>;

        using lead_storage_type = storage_type<lead_component_type>;
        using lead_segment_type = typename lead_storage_type::compute_segment;
        using segment_buffer_type = std::vector<lead_segment_type>;

        storage_view_compute(
            std::shared_ptr<const segment_buffer_type> segments,
            std::size_t begin,
            std::size_t end,
            std::uint64_t entity_count,
            lead_storage_type* lead_storage,
            std::tuple<storage_type<RestComponentTypes>*...> rest,
            std::tuple<storage_type<ExcludedComponentTypes>*...> excluded)
            : _segments { std::move(segments) }
            , _begin { begin }
            , _end { end }
            , _entity_count { entity_count }
            , _lead_storage { lead_storage }
            , _rest_storage { rest }
            , _excluded_storage { excluded }
        {
        }

        struct segment {
            const storage_view_compute* view = nullptr;
            const lead_segment_type* lead = nullptr;

            [[nodiscard]] object_entity_scene_index scene() const noexcept
            {
                return lead->scene;
            }

            [[nodiscard]] std::uint32_t count() const noexcept
            {
                return lead->count;
            }

            [[nodiscard]] object_entity entity(std::uint32_t index) const noexcept
            {
                return lead->page->entity(index);
            }

            template <typename ComponentType>
            [[nodiscard]] auto* page(std::uint32_t index) const noexcept
            {
                using component_type = storage_component_type<ComponentType>;

                if constexpr (std::is_same_v<component_type, lead_component_type>) {
                    return lead->page;
                } else {
                    auto entity_value = entity(index);
                    auto* storage = view->template _storage_for<component_type>();
                    auto location = storage->locate(lead->scene, entity_value);
                    return storage->segment_at(location.scene, location.segment).page;
                }
            }

            template <typename ComponentType>
            [[nodiscard]] std::uint32_t index(std::uint32_t index) const noexcept
            {
                using component_type = storage_component_type<ComponentType>;

                if constexpr (std::is_same_v<component_type, lead_component_type>) {
                    return lead->page->backend_index(index);
                } else {
                    auto entity_value = entity(index);
                    auto* storage = view->template _storage_for<component_type>();
                    return storage->backend_index(lead->scene, entity_value);
                }
            }
        };

        template <typename CallbackType>
        void each_segment(CallbackType&& callback) const
        {
            const auto& segments_ref = *_segments;

            for (std::size_t segment_index = _begin; segment_index < _end; ++segment_index) {
                const auto& current = segments_ref[segment_index];

                if (current.count == 0) {
                    continue;
                }

                segment compute_segment {
                    .view = this,
                    .lead = &current
                };

                std::forward<CallbackType>(callback)(compute_segment);
            }
        }

        [[nodiscard]] std::vector<storage_view_compute> split(std::size_t requested_count) const
        {
            std::vector<storage_view_compute> result = {};

            if (requested_count == 0 || empty()) {
                return result;
            }

            if (requested_count == 1) {
                result.push_back(*this);
                return result;
            }

            const std::uint64_t total = _entity_count;
            const std::uint64_t target = (total + requested_count - 1u) / requested_count;

            result.reserve(requested_count);

            std::size_t part_begin = _begin;
            std::uint64_t current_count = 0;

            for (std::size_t segment_index = _begin; segment_index < _end; ++segment_index) {
                current_count += (*_segments)[segment_index].count;

                const bool should_cut = current_count >= target && result.size() + 1u < requested_count && segment_index + 1u < _end;

                if (should_cut) {
                    result.emplace_back(
                        _segments,
                        part_begin,
                        segment_index + 1u,
                        current_count,
                        _lead_storage,
                        _rest_storage,
                        _excluded_storage);

                    part_begin = segment_index + 1u;
                    current_count = 0;
                }
            }

            if (part_begin < _end) {
                result.emplace_back(
                    _segments,
                    part_begin,
                    _end,
                    current_count,
                    _lead_storage,
                    _rest_storage,
                    _excluded_storage);
            }

            return result;
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
        std::uint64_t _entity_count = {};

        lead_storage_type* _lead_storage = nullptr;
        std::tuple<storage_type<RestComponentTypes>*...> _rest_storage = {};
        std::tuple<storage_type<ExcludedComponentTypes>*...> _excluded_storage = {};

        template <typename ComponentType>
        [[nodiscard]] auto* _storage_for() const noexcept
        {
            using component_type = storage_component_type<ComponentType>;

            if constexpr (std::is_same_v<component_type, lead_component_type>) {
                return _lead_storage;
            } else {
                return _storage_for_rest<component_type>();
            }
        }

        template <typename ComponentType>
        [[nodiscard]] auto* _storage_for_rest() const noexcept
        {
            return _storage_for_rest_impl<ComponentType, 0, RestComponentTypes...>();
        }

        template <typename ComponentType, std::size_t Index>
        [[nodiscard]] auto* _storage_for_rest_impl() const noexcept
        {
            static_assert(Index != Index, "Component type is not part of this compute view.");
            return nullptr;
        }

        template <typename ComponentType, std::size_t Index, typename Current, typename... Tail>
        [[nodiscard]] auto* _storage_for_rest_impl() const noexcept
        {
            using current_type = storage_component_type<Current>;

            if constexpr (std::is_same_v<ComponentType, current_type>) {
                return std::get<Index>(_rest_storage);
            } else {
                return _storage_for_rest_impl<ComponentType, Index + 1u, Tail...>();
            }
        }
    };

}
}
