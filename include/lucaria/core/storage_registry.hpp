#pragma once

#include <lucaria/core/storage_view.hpp>
#include <lucaria/core/utils_error.hpp>

namespace lucaria {
namespace detail {

    struct object_entity_allocator {
        object_entity_local_index next_local = {};
        std::vector<object_entity_version> generations = {};
        std::vector<object_entity_local_index> free_list = {};

        [[nodiscard]] object_entity create(object_entity_scene_index scene)
        {
            object_entity_local_index _local = {};
            if (!free_list.empty()) {
                _local = free_list.back();
                free_list.pop_back();
            } else {
                LUCARIA_DEBUG_ASSERT(next_local < std::numeric_limits<object_entity_local_index>::max(), "Next local > numeric limits");
                _local = next_local++;
                generations.resize(static_cast<std::size_t>(next_local));
            }
            return make_entity(scene, _local, generations[_local]);
        }

        void destroy(object_entity entity)
        {
            const object_entity_local_index _local = entity_local(entity);
            LUCARIA_DEBUG_ASSERT(_local < generations.size(), "Local >= generations size");
            LUCARIA_DEBUG_ASSERT(generations[_local] == entity_version(entity), "Generation different than entity version");
            ++generations[_local];
            free_list.push_back(_local);
        }

        [[nodiscard]] bool alive(object_entity entity) const
        {
            const object_entity_local_index _local = entity_local(entity);
            return (_local < generations.size()) && (generations[_local] == entity_version(entity));
        }

        void erase_all()
        {
            free_list.clear();
            for (object_entity_local_index _local = 0; _local < next_local; ++_local) {
                ++generations[_local];
                free_list.push_back(_local);
            }
        }
    };

    struct container_segment_registry_cpu {
        std::vector<object_entity_allocator> scene_allocators = {};
        std::vector<std::function<void(object_entity_scene_index)>> scene_erasers {};

        [[nodiscard]] object_entity create(object_entity_scene_index scene)
        {
            object_entity_allocator& _allocator = _assure_scene_allocator(scene);
            object_entity _entity = _allocator.create(scene);
            object_entity _created = _registry.create(_entity);
            LUCARIA_DEBUG_ASSERT(_created == _entity, "Invalid new entity handle");
            return _entity;
        }

        void destroy(object_entity entity)
        {
            if (!_registry.valid(entity)) {
                return;
            }
            _registry.destroy(entity);
            const object_entity_scene_index _scene = entity_scene(entity);
            object_entity_allocator& _allocator = _assure_scene_allocator(_scene);
            _allocator.destroy(entity);
        }

        [[nodiscard]] bool valid(object_entity entity) const
        {
            const object_entity_scene_index _scene = entity_scene(entity);
            const object_entity_allocator* _allocator = _find_scene_allocator(_scene);
            return (_allocator != nullptr) && (_allocator->alive(entity)) && (_registry.valid(entity));
        }

        template <typename T, typename... Args>
        T& emplace(object_entity entity, Args&&... args)
        {
            LUCARIA_DEBUG_ASSERT(valid(entity), "Invalid entity");
            return _registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
        T& emplace_or_replace(object_entity entity, Args&&... args)
        {
            LUCARIA_DEBUG_ASSERT(valid(entity), "Invalid entity");
            return _registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
        }

        template <typename T>
        void remove(object_entity entity)
        {
            if (_registry.template all_of<T>(entity)) {
                _registry.template remove<T>(entity);
            }
        }

        template <typename T>
        [[nodiscard]] T& get(object_entity entity)
        {
            LUCARIA_DEBUG_ASSERT(valid(entity), "Invalid entity");
            return _registry.get<T>(entity);
        }

        template <typename T>
        [[nodiscard]] const T& get(object_entity entity) const
        {
            LUCARIA_DEBUG_ASSERT(valid(entity), "Invalid entity");
            return _registry.get<T>(entity);
        }

        template <typename T>
        [[nodiscard]] T* try_get(object_entity entity)
        {
            if (!valid(entity)) {
                return nullptr;
            }
            return _registry.try_get<T>(entity);
        }

        template <typename T>
        [[nodiscard]] const T* try_get(object_entity entity) const
        {
            if (!valid(entity)) {
                return nullptr;
            }
            return _registry.try_get<T>(entity);
        }

        template <typename... T>
        [[nodiscard]] bool contains(object_entity entity) const
        {
            return valid(entity) && _registry.template all_of<T...>(entity);
        }

        template <typename T>
        void register_component()
        {
            scene_erasers.push_back([this](object_entity_scene_index scene) {
                _registry.storage<T>().erase_scene(scene);
            });
        }

        void erase_scene(object_entity_scene_index scene)
        {
            for (std::function<void(object_entity_scene_index)>& _erase : scene_erasers) {
                _erase(scene);
            }
            object_entity_allocator& _allocator = _assure_scene_allocator(scene);
            _allocator.erase_all();
        }

        template <typename Lead, typename... Rest, typename... Excluded>
        [[nodiscard]] auto view(exclude_t<Excluded...> = {})
        {
            using view_type = object_registry_view_cpu<exclude_t<Excluded...>, Lead, Rest...>;
            using segment_type = typename view_type::segment_type;
            using segment_buffer_type = typename view_type::segment_buffer_type;
            using lead_component_type = std::remove_cvref_t<Lead>;
            auto& _lead = _registry.storage<lead_component_type>();
            std::shared_ptr<segment_buffer_type> _segments = std::make_shared<segment_buffer_type>();
            std::uint64_t _total = 0;
            _lead.each_segment([&](auto seg) {
                _segments->push_back(segment_type {
                    .scene = seg.scene,
                    .entities = seg.entities,
                    .components = seg.components,
                    .count = seg.count });
                _total += seg.count;
            });
            const std::size_t _count = _segments->size();
            return view_type { std::move(_segments), 0u, _count, _total,
                std::tuple { &_registry.storage<std::remove_cvref_t<Rest>>()... },
                std::tuple { &_registry.storage<std::remove_cvref_t<Excluded>>()... } };
        }

        template <typename Lead, typename... Rest, typename... Excluded>
        [[nodiscard]] auto view(object_entity_scene_index scene, exclude_t<Excluded...> = {})
        {
            using view_type = object_registry_view_cpu<exclude_t<Excluded...>, Lead, Rest...>;
            using segment_type = typename view_type::segment_type;
            using segment_buffer_type = typename view_type::segment_buffer_type;
            using lead_component_type = std::remove_cvref_t<Lead>;
            auto& _lead = _registry.storage<lead_component_type>();
            std::shared_ptr<segment_buffer_type> _segments = std::make_shared<segment_buffer_type>();
            std::uint64_t _total = 0;
            _lead.each_segment(scene, [&](auto seg) {
                _segments->push_back(segment_type {
                    .scene = seg.scene,
                    .entities = seg.entities,
                    .components = seg.components,
                    .count = seg.count });
                _total += seg.count;
            });
            const std::size_t _count = _segments->size();
            return view_type {
                std::move(_segments),
                0u,
                _count,
                _total,
                std::tuple { &_registry.storage<std::remove_cvref_t<Rest>>()... },
                std::tuple { &_registry.storage<std::remove_cvref_t<Excluded>>()... }
            };
        }

        template <typename Lead, typename... Rest>
        [[nodiscard]] auto view()
        {
            return view<Lead, Rest...>(exclude<>);
        }

        template <typename Lead, typename... Rest>
        [[nodiscard]] auto view(object_entity_scene_index scene)
        {
            return view<Lead, Rest...>(scene, exclude<>);
        }

    private:
        entt::basic_registry<object_entity> _registry = {};

        [[nodiscard]] object_entity_allocator& _assure_scene_allocator(object_entity_scene_index scene)
        {
            if (scene_allocators.size() <= static_cast<std::size_t>(scene)) {
                scene_allocators.resize(static_cast<std::size_t>(scene) + 1u);
            }
            return scene_allocators[scene];
        }

        [[nodiscard]] const object_entity_allocator* _find_scene_allocator(object_entity_scene_index scene) const
        {
            if (scene >= scene_allocators.size()) {
                return nullptr;
            }
            return &scene_allocators[scene];
        }
    };

}
}
