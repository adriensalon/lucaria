#pragma once

#include <lucaria/core/storage_view.hpp>

namespace lucaria {
namespace detail {

    struct object_entity_allocator {
        object_entity_local_index next_local = {};
        std::vector<object_entity_version> generations = {};
        std::vector<object_entity_local_index> free_list = {};

        object_entity create(object_entity_scene_index scene)
        {
            object_entity_local_index local {};

            if (!free_list.empty()) {
                local = free_list.back();
                free_list.pop_back();
            } else {
                assert(next_local < std::numeric_limits<object_entity_local_index>::max());

                local = next_local++;
                generations.resize(static_cast<std::size_t>(next_local));
            }

            return make_entity(scene, local, generations[local]);
        }

        void destroy(object_entity entity)
        {
            const auto local = entity_local(entity);

            assert(local < generations.size());
            assert(generations[local] == entity_version(entity));

            ++generations[local];
            free_list.push_back(local);
        }

        bool alive(object_entity entity) const
        {
            const auto local = entity_local(entity);

            return local < generations.size()
                && generations[local] == entity_version(entity);
        }

        void erase_all()
        {
            free_list.clear();

            for (object_entity_local_index local = 0; local < next_local; ++local) {
                ++generations[local];
                free_list.push_back(local);
            }
        }
    };

    struct container_segment_registry_cpu {

        using registry_type = entt::basic_registry<object_entity>;

        object_entity create(object_entity_scene_index scene)
        {
            auto& allocator = assure_scene_allocator(scene);

            auto entity = allocator.create(scene);

            auto created = registry.create(entity);
            assert(created == entity);

            return entity;
        }

        void destroy(object_entity entity)
        {
            if (!registry.valid(entity)) {
                return;
            }

            registry.destroy(entity);

            const auto scene = entity_scene(entity);
            auto& allocator = assure_scene_allocator(scene);
            allocator.destroy(entity);
        }

        [[nodiscard]] bool valid(object_entity entity) const
        {
            const auto scene = entity_scene(entity);
            const auto* allocator = find_scene_allocator(scene);

            return allocator != nullptr
                && allocator->alive(entity)
                && registry.valid(entity);
        }

        template <typename T, typename... Args>
        T& emplace(object_entity entity, Args&&... args)
        {
            assert(valid(entity));
            return registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
        T& emplace_or_replace(object_entity entity, Args&&... args)
        {
            assert(valid(entity));
            return registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
        }

        template <typename T>
        void remove(object_entity entity)
        {
            if (registry.template all_of<T>(entity)) {
                registry.template remove<T>(entity);
            }
        }

        template <typename T>
        T& get(object_entity entity)
        {
            assert(valid(entity));
            return registry.get<T>(entity);
        }

        template <typename T>
        const T& get(object_entity entity) const
        {
            assert(valid(entity));
            return registry.get<T>(entity);
        }

        template <typename T>
        T* try_get(object_entity entity)
        {
            if (!valid(entity)) {
                return nullptr;
            }

            return registry.try_get<T>(entity);
        }

        template <typename T>
        const T* try_get(object_entity entity) const
        {
            if (!valid(entity)) {
                return nullptr;
            }

            return registry.try_get<T>(entity);
        }

        template <typename... T>
        bool contains(object_entity entity) const
        {
            return valid(entity) && registry.template all_of<T...>(entity);
        }

        template <typename T>
        void register_component()
        {
            scene_erasers.push_back([this](object_entity_scene_index scene) {
                registry.storage<T>().erase_scene(scene);
            });
        }

        void erase_scene(object_entity_scene_index scene)
        {
            for (auto& erase : scene_erasers) {
                erase(scene);
            }

            auto& allocator = assure_scene_allocator(scene);
            allocator.erase_all();
        }

        template <typename Lead, typename... Rest, typename... Excluded>
        auto view(exclude_t<Excluded...> = {})
        {
            using lead_component_type = std::remove_cvref_t<Lead>;
            auto& lead = registry.storage<lead_component_type>();

            using view_type = object_registry_view_cpu<exclude_t<Excluded...>, Lead, Rest...>;
            using segment_type = typename view_type::segment_type;
            using segment_buffer = typename view_type::segment_buffer;

            auto segments = std::make_shared<segment_buffer>();

            std::uint64_t total = 0;

            lead.each_segment([&](auto seg) {
                segments->push_back(segment_type {
                    .scene = seg.scene,
                    .entities = seg.entities,
                    .components = seg.components,
                    .count = seg.count });

                total += seg.count;
            });

            const auto count = segments->size();

            return view_type {
                std::move(segments),
                0u,
                count,
                total,
                std::tuple { &registry.storage<std::remove_cvref_t<Rest>>()... },
                std::tuple { &registry.storage<std::remove_cvref_t<Excluded>>()... }
            };
        }

        template <typename Lead, typename... Rest, typename... Excluded>
        auto view(object_entity_scene_index scene, exclude_t<Excluded...> = {})
        {
            using lead_component_type = std::remove_cvref_t<Lead>;
            auto& lead = registry.storage<lead_component_type>();

            using view_type = object_registry_view_cpu<exclude_t<Excluded...>, Lead, Rest...>;
            using segment_type = typename view_type::segment_type;
            using segment_buffer = typename view_type::segment_buffer;

            auto segments = std::make_shared<segment_buffer>();

            std::uint64_t total = 0;

            lead.each_segment(scene, [&](auto seg) {
                segments->push_back(segment_type {
                    .scene = seg.scene,
                    .entities = seg.entities,
                    .components = seg.components,
                    .count = seg.count });

                total += seg.count;
            });

            const auto count = segments->size();

            return view_type {
                std::move(segments),
                0u,
                count,
                total,
                std::tuple { &registry.storage<std::remove_cvref_t<Rest>>()... },
                std::tuple { &registry.storage<std::remove_cvref_t<Excluded>>()... }
            };
        }
		
        template <typename Lead, typename... Rest>
        auto view()
        {
            return view<Lead, Rest...>(exclude<>);
        }

        template <typename Lead, typename... Rest>
        auto view(object_entity_scene_index scene)
        {
            return view<Lead, Rest...>(scene, exclude<>);
        }

        registry_type& raw_registry_for_tests() noexcept
        {
            return registry;
        }

        // private:
        registry_type registry;
        std::vector<object_entity_allocator> scene_allocators;
        std::vector<std::function<void(object_entity_scene_index)>> scene_erasers {};

        object_entity_allocator& assure_scene_allocator(object_entity_scene_index scene)
        {
            if (scene_allocators.size() <= static_cast<std::size_t>(scene)) {
                scene_allocators.resize(static_cast<std::size_t>(scene) + 1u);
            }

            return scene_allocators[scene];
        }

        const object_entity_allocator* find_scene_allocator(object_entity_scene_index scene) const
        {
            if (scene >= scene_allocators.size()) {
                return nullptr;
            }

            return &scene_allocators[scene];
        }
    };

    struct container_segment_registry_gpu {
        struct storage_base {
            virtual ~storage_base() = default;
            virtual void remove(object_entity entity) = 0;
            virtual void erase_scene(object_entity_scene_index scene) = 0;
            virtual void clear() = 0;
            virtual bool contains_entity(object_entity entity) const = 0;
        };

        template <typename T>
        struct storage_model final : storage_base {
            object_segment_storage_gpu<T> storage;

            void remove(object_entity entity) override
            {
                storage.remove(entity);
            }

            void erase_scene(object_entity_scene_index scene) override
            {
                storage.erase_scene(scene);
            }

            void clear() override
            {
                storage.clear();
            }

            bool contains_entity(object_entity entity) const override
            {
                return storage.contains(entity);
            }
        };

        object_entity create(object_entity_scene_index scene)
        {
            auto& allocator = assure_scene_allocator(scene);
            return allocator.create(scene);
        }

        void destroy(object_entity entity)
        {
            if (!valid(entity)) {
                return;
            }

            for (auto* storage_ptr : storage_order) {
                storage_ptr->remove(entity);
            }

            const auto scene = entity_scene(entity);
            auto& allocator = assure_scene_allocator(scene);
            allocator.destroy(entity);
        }

        [[nodiscard]] bool valid(object_entity entity) const
        {
            const auto scene = entity_scene(entity);
            const auto* allocator = find_scene_allocator(scene);

            return allocator != nullptr && allocator->alive(entity);
        }

        template <typename T, typename... Args>
        void emplace(object_entity entity, Args&&... args)
        {
            assert(valid(entity));
            storage<T>().emplace(entity, std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
        void emplace_or_replace(object_entity entity, Args&&... args)
        {
            assert(valid(entity));
            storage<T>().emplace_or_replace(entity, std::forward<Args>(args)...);
        }

        template <typename T>
        void remove(object_entity entity)
        {
            storage<T>().remove(entity);
        }

        template <typename T>
        [[nodiscard]] bool contains_one(object_entity entity) const
        {
            const auto* model = find_storage_model<T>();
            return model != nullptr && model->storage.contains(entity);
        }

        template <typename... T>
        [[nodiscard]] bool contains(object_entity entity) const
        {
            return valid(entity) && (contains_one<T>(entity) && ...);
        }

        template <typename T>
        void upload()
        {
            storage<T>().upload();
        }

        template <typename T, typename Command>
        void upload(Command&& command)
        {
            storage<T>().upload(std::forward<Command>(command));
        }

        template <typename T>
        void download()
        {
            storage<T>().download();
        }

        template <typename T, typename Command>
        void download(Command&& command)
        {
            storage<T>().download(std::forward<Command>(command));
        }

        template <typename T>
        [[nodiscard]] std::span<const T> readback() const
        {
            return storage<T>().readback();
        }

        template <typename T>
        [[nodiscard]] std::optional<T> readback(object_entity entity) const
        {
            return storage<T>().readback(entity);
        }

        template <typename T>
        [[nodiscard]] std::span<const object_entity> readback_entities() const
        {
            return storage<T>().downloaded_entities();
        }

        template <typename T>
        [[nodiscard]] std::span<const T> staged() const
        {
            return storage<T>().staged();
        }

        template <typename T>
        [[nodiscard]] std::span<const object_entity> staged_entities() const
        {
            return storage<T>().entities();
        }

        template <typename T>
        [[nodiscard]] std::uint32_t size() const
        {
            return storage<T>().size();
        }

        template <typename T>
        [[nodiscard]] bool has_pending_upload() const
        {
            const auto* model = find_storage_model<T>();
            return model != nullptr && model->storage.has_pending_upload();
        }

        template <typename T>
        [[nodiscard]] object_component_upload_snapshot_gpu<T> upload_snapshot() const
        {
            return storage<T>().upload_snapshot();
        }

        template <typename T>
        [[nodiscard]] object_component_download_snapshot_gpu<T> download_snapshot() const
        {
            return storage<T>().download_snapshot();
        }

        template <typename T>
        void register_component()
        {
            assure_storage_model<T>();
        }

        void erase_scene(object_entity_scene_index scene)
        {
            for (auto* storage_ptr : storage_order) {
                storage_ptr->erase_scene(scene);
            }

            auto& allocator = assure_scene_allocator(scene);
            allocator.erase_all();
        }

        template <typename T>
        object_segment_storage_gpu<T>& storage()
        {
            return assure_storage_model<T>().storage;
        }

        template <typename T>
        const object_segment_storage_gpu<T>& storage() const
        {
            const auto* model = find_storage_model<T>();
            assert(model != nullptr);
            return model->storage;
        }

        // GPU registry intentionally has no CPU object access or CPU views.
        template <typename T>
        T& get(object_entity) = delete;

        template <typename T>
        const T& get(object_entity) const = delete;

        template <typename T>
        T* try_get(object_entity) = delete;

        template <typename T>
        const T* try_get(object_entity) const = delete;

        template <typename... T>
        void view() = delete;

        std::vector<object_entity_allocator> scene_allocators;

    private:
        std::unordered_map<entt::id_type, std::unique_ptr<storage_base>> storages;
        std::vector<storage_base*> storage_order;

        object_entity_allocator& assure_scene_allocator(object_entity_scene_index scene)
        {
            if (scene_allocators.size() <= static_cast<std::size_t>(scene)) {
                scene_allocators.resize(static_cast<std::size_t>(scene) + 1u);
            }

            return scene_allocators[scene];
        }

        const object_entity_allocator* find_scene_allocator(object_entity_scene_index scene) const
        {
            if (scene >= scene_allocators.size()) {
                return nullptr;
            }

            return &scene_allocators[scene];
        }

        template <typename T>
        static entt::id_type storage_key() noexcept
        {
            return entt::type_hash<T>::value();
        }

        template <typename T>
        storage_model<T>& assure_storage_model()
        {
            const auto key = storage_key<T>();
            auto it = storages.find(key);

            if (it == storages.end()) {
                auto model = std::make_unique<storage_model<T>>();
                auto* raw = model.get();
                it = storages.emplace(key, std::move(model)).first;
                storage_order.push_back(raw);
            }

            return static_cast<storage_model<T>&>(*it->second);
        }

        template <typename T>
        const storage_model<T>* find_storage_model() const
        {
            const auto it = storages.find(storage_key<T>());
            if (it == storages.end()) {
                return nullptr;
            }

            return static_cast<const storage_model<T>*>(it->second.get());
        }
    };

}
}
