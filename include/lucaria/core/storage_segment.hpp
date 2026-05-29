#pragma once

#include <lucaria/core/storage_page.hpp>

namespace lucaria {
namespace detail {

    template <typename T, typename Entity, typename Allocator>
    struct object_segment_storage_cpu : public entt::basic_sparse_set<Entity, typename std::allocator_traits<Allocator>::template rebind_alloc<Entity>> {
        using alloc_traits = std::allocator_traits<Allocator>;
        using entity_allocator_type = typename alloc_traits::template rebind_alloc<Entity>;
        using value_type = T;
        using entity_type = Entity;
        using allocator_type = Allocator;
        using base_type = entt::basic_sparse_set<Entity, entity_allocator_type>;
        using size_type = std::size_t;
        using traits_type = entt::component_traits<T>;

        // static constexpr std::uint32_t page_size = 256u;
        static constexpr std::uint32_t page_size = 4u;

        struct segment {
            object_entity_scene_index scene;
            entity_type* entities;
            T* components;
            std::uint32_t count;
        };

        struct const_segment {
            object_entity_scene_index scene {};
            const entity_type* entities {};
            const value_type* components {};
            std::uint32_t count {};
        };

        template <typename Fn>
        void each_segment(object_entity_scene_index scene, Fn&& fn) noexcept
        {
            if (scene >= partitions.size()) {
                return;
            }

            auto& part = partitions[scene];

            for (page* pg : part.pages) {
                if (pg->count != 0u) {
                    fn(segment {
                        .scene = scene,
                        .entities = pg->entities,
                        .components = pg->components(),
                        .count = pg->count });
                }
            }
        }

        template <typename Fn>
        void each_segment(Fn&& fn) noexcept
        {
            for (object_entity_scene_index scene = 0; scene < partitions.size(); ++scene) {
                each_segment(scene, fn);
            }
        }

        template <typename Fn>
        void each_segment(object_entity_scene_index scene, Fn&& fn) const noexcept
        {
            if (scene >= partitions.size()) {
                return;
            }

            const auto& part = partitions[scene];

            for (const auto& pg_ptr : part.pages) {
                const auto& pg = *pg_ptr;

                if (pg.count != 0u) {
                    fn(const_segment {
                        .scene = scene,
                        .entities = pg.entities,
                        .components = pg.components(),
                        .count = pg.count });
                }
            }
        }

        template <typename Fn>
        void each_segment(Fn&& fn) const noexcept
        {
            for (object_entity_scene_index scene = 0; scene < partitions.size(); ++scene) {
                each_segment(scene, fn);
            }
        }

    private:
        struct location {
            object_entity_scene_index scene {};
            std::uint32_t page {};
            std::uint32_t offset {};
        };

        struct page {
            entity_type entities[page_size];
            alignas(value_type) std::byte raw[sizeof(value_type) * page_size];
            std::uint32_t count {};

            value_type* components() noexcept
            {
                return std::launder(reinterpret_cast<value_type*>(raw));
            }

            const value_type* components() const noexcept
            {
                return std::launder(reinterpret_cast<const value_type*>(raw));
            }
        };

        using page_pool_type = object_page_pool_cpu<page, allocator_type>;

        page_pool_type page_pool;

        struct partition {
            std::vector<page*> pages;
        };

    public:
        object_segment_storage_cpu()
            : object_segment_storage_cpu { allocator_type {} }
        {
        }

        explicit object_segment_storage_cpu(const allocator_type& alloc)
            : base_type {
                entt::type_id<value_type>(),
                entt::deletion_policy { entt::deletion_policy::swap_and_pop },
                entity_allocator_type { alloc }
            }
            , allocator { alloc }
            , page_pool { alloc }
        {
        }

        ~object_segment_storage_cpu() override
        {
            clear_payload();
        }

        object_segment_storage_cpu(const object_segment_storage_cpu&) = delete;
        object_segment_storage_cpu& operator=(const object_segment_storage_cpu&) = delete;

        allocator_type get_allocator() const noexcept
        {
            return allocator;
        }

        template <typename... Args>
        value_type& emplace(entity_type entity, Args&&... args)
        {
            const auto scene = entity_scene(entity);

            auto it = base_type::try_emplace(entity, false);
            const auto base_index = static_cast<std::size_t>(it.index());

            if (locations_by_index.size() <= base_index) {
                locations_by_index.resize(base_index + 1u);
            }

            auto& part = assure_partition(scene);
            auto [page_index, pg] = assure_page(part);

            const auto offset = pg->count++;
            pg->entities[offset] = entity;

            T* ptr = pg->components() + offset;
            std::construct_at(ptr, std::forward<Args>(args)...);

            locations_by_index[base_index] = location {
                .scene = scene,
                .page = page_index,
                .offset = offset
            };

            return *ptr;
        }

        void clear()
        {
            scratch_entities_.clear();
            scratch_entities_.reserve(base_type::size());

            for (auto it = base_type::begin(); it != base_type::end(); ++it) {
                scratch_entities_.push_back(*it);
            }

            for (const auto entity : scratch_entities_) {
                erase_one(entity);
            }
            scratch_entities_.clear();
        }

        [[nodiscard]] bool contains(entity_type entity) const noexcept
        {
            return base_type::contains(entity);
        }

        [[nodiscard]] bool contains(object_entity_scene_index scene, entity_type entity) const noexcept
        {
            if (scene >= partitions.size()) {
                return false;
            }

            if (!base_type::contains(entity)) {
                return false;
            }

            const auto index = base_type::index(entity);
            const auto loc = locations_by_index[index];

            return loc.scene == scene;
        }

        value_type& get(entity_type entity) noexcept
        {
            const auto index = base_type::index(entity);
            auto loc = locations_by_index[index];

            return partitions[loc.scene].pages[loc.page]->components()[loc.offset];
        }

        const value_type& get(entity_type entity) const noexcept
        {
            const auto index = base_type::index(entity);
            auto loc = locations_by_index[index];

            return partitions[loc.scene].pages[loc.page]->components()[loc.offset];
        }

        [[nodiscard]] value_type& get(object_entity_scene_index scene, entity_type entity) noexcept
        {
            assert(contains(scene, entity));

            const auto index = base_type::index(entity);
            const auto loc = locations_by_index[index];

            return partitions[scene].pages[loc.page]->components()[loc.offset];
        }

        [[nodiscard]] const value_type& get(object_entity_scene_index scene, entity_type entity) const noexcept
        {
            assert(contains(scene, entity));

            const auto index = base_type::index(entity);
            const auto loc = locations_by_index[index];

            return partitions[scene].pages[loc.page]->components()[loc.offset];
        }

        std::tuple<value_type&> get_as_tuple(entity_type entity) noexcept
        {
            return std::forward_as_tuple(get(entity));
        }

        std::tuple<const value_type&> get_as_tuple(entity_type entity) const noexcept
        {
            return std::forward_as_tuple(get(entity));
        }

        value_type* try_get(entity_type entity) noexcept
        {
            return base_type::contains(entity) ? std::addressof(get(entity)) : nullptr;
        }

        template <typename... Func>
        value_type& patch(entity_type entity, Func&&... func)
        {
            auto& value = get(entity);

            (std::forward<Func>(func)(value), ...);

            return value;
        }

        void erase_one(entity_type entity)
        {
            const auto erased_index = base_type::index(entity);
            const auto last_index = base_type::size() - 1u;
            const auto moved_entity = base_type::data()[last_index];

            erase_payload_at_base_index(erased_index);

            base_type::swap_and_pop(base_type::find(entity));

            if (erased_index != last_index) {
                locations_by_index[erased_index] = locations_by_index[last_index];
            }

            locations_by_index.pop_back();
        }

        void pop(typename base_type::iterator first, typename base_type::iterator last) override
        {
            scratch_entities_.clear();

            for (; first != last; ++first) {
                scratch_entities_.push_back(*first);
            }

            for (auto entity : scratch_entities_) {
                erase_one(entity);
            }

            scratch_entities_.clear();
        }

        void erase_payload_at_base_index(std::size_t base_index)
        {
            const auto loc = locations_by_index[base_index];

            auto& pg = *partitions[loc.scene].pages[loc.page];
            auto* comps = pg.components();

            const auto last = pg.count - 1u;

            std::destroy_at(comps + loc.offset);

            if (loc.offset != last) {
                std::construct_at(comps + loc.offset, std::move(comps[last]));
                std::destroy_at(comps + last);

                const auto moved_entity = pg.entities[last];
                pg.entities[loc.offset] = moved_entity;

                const auto moved_base_index = base_type::index(moved_entity);
                locations_by_index[moved_base_index] = location {
                    .scene = loc.scene,
                    .page = loc.page,
                    .offset = loc.offset
                };
            }

            --pg.count;
        }

    private:
        allocator_type allocator;
        std::vector<partition> partitions;
        std::vector<location> locations_by_index;
        std::vector<entity_type> scratch_entities_;

        void clear_payload()
        {
            for (auto& part : partitions) {
                for (page* pg : part.pages) {
                    auto* comps = pg->components();

                    for (std::uint32_t i = 0; i < pg->count; ++i) {
                        std::destroy_at(comps + i);
                    }

                    pg->count = 0;
                    page_pool.release(pg);
                }

                part.pages.clear();
            }

            partitions.clear();
            locations_by_index.clear();
        }

        partition& assure_partition(object_entity_scene_index scene)
        {
            if (partitions.size() <= static_cast<std::size_t>(scene)) {
                partitions.resize(static_cast<std::size_t>(scene) + 1u);
            }

            return partitions[scene];
        }

        std::pair<std::uint32_t, page*> assure_page(partition& part)
        {
            if (part.pages.empty() || part.pages.back()->count == page_size) {
                auto* pg = page_pool.acquire();
                pg->count = 0;
                part.pages.push_back(pg);
            }

            const auto index = static_cast<std::uint32_t>(part.pages.size() - 1u);
            return { index, part.pages.back() };
        }

        template <bool Const>
        class basic_each_iterator {
            using storage_type = std::conditional_t<
                Const,
                const object_segment_storage_cpu,
                object_segment_storage_cpu>;

            using base_iterator_type = std::conditional_t<
                Const,
                typename base_type::const_iterator,
                typename base_type::iterator>;

            using component_ref = std::conditional_t<
                Const,
                const value_type&,
                value_type&>;

        public:
            using difference_type = std::ptrdiff_t;
            using value_type_tuple = std::tuple<entity_type, component_ref>;
            using value_type = value_type_tuple;
            using reference = value_type_tuple;
            using pointer = void;
            using iterator_category = std::input_iterator_tag;

            basic_each_iterator() = default;

            basic_each_iterator(storage_type* storage, base_iterator_type it)
                : storage { storage }
                , it { it }
            {
            }

            basic_each_iterator& operator++() noexcept
            {
                ++it;
                return *this;
            }

            basic_each_iterator operator++(int) noexcept
            {
                auto copy = *this;
                ++(*this);
                return copy;
            }

            reference operator*() const noexcept
            {
                const auto entity = *it;
                return { entity, storage->get(entity) };
            }

            bool operator==(const basic_each_iterator& other) const noexcept
            {
                return it == other.it;
            }

            bool operator!=(const basic_each_iterator& other) const noexcept
            {
                return !(*this == other);
            }

        private:
            storage_type* storage {};
            base_iterator_type it {};
        };

        template <bool Const>
        struct basic_each_range {
            using storage_type = std::conditional_t<
                Const,
                const object_segment_storage_cpu,
                object_segment_storage_cpu>;

            using iterator = basic_each_iterator<Const>;

            storage_type* storage {};

            iterator begin() const noexcept
            {
                if constexpr (Const) {
                    return iterator { storage, storage->base_type::cbegin() };
                } else {
                    return iterator { storage, storage->base_type::begin() };
                }
            }

            iterator end() const noexcept
            {
                if constexpr (Const) {
                    return iterator { storage, storage->base_type::cend() };
                } else {
                    return iterator { storage, storage->base_type::end() };
                }
            }

        public:
            using iterable = basic_each_range<false>;
            using const_iterable = basic_each_range<true>;

            [[nodiscard]] iterable each() noexcept
            {
                return iterable { this };
            }

            [[nodiscard]] const_iterable each() const noexcept
            {
                return const_iterable { this };
            }
        };

    public:
        using iterable = basic_each_range<false>;
        using const_iterable = basic_each_range<true>;

        [[nodiscard]] iterable each() noexcept
        {
            return iterable { this };
        }

        [[nodiscard]] const_iterable each() const noexcept
        {
            return const_iterable { this };
        }

        void erase_scene(object_entity_scene_index scene)
        {
            if (scene >= partitions.size()) {
                return;
            }

            scratch_entities_.clear();

            auto& part = partitions[scene];

            for (page* pg : part.pages) {
                for (std::uint32_t i = 0; i < pg->count; ++i) {
                    scratch_entities_.push_back(pg->entities[i]);
                }
            }

            for (auto entity : scratch_entities_) {
                if (base_type::contains(entity)) {
                    erase_one(entity);
                }
            }

            // After erase_one, all components in these pages should be gone.
            for (page* pg : part.pages) {
                assert(pg->count == 0);
                page_pool.release(pg);
            }

            part.pages.clear();
        }
    };

    template <typename T>
    struct object_segment_storage_gpu {
        using value_type = T;
        using entity_type = object_entity;
        using size_type = std::uint32_t;

        template <typename... Args>
        void emplace(entity_type entity, Args&&... args)
        {
            assert(!contains(entity));

            const auto dense = dense_count++;

            dense_entities.push_back(entity);
            staging_components.emplace_back(std::forward<Args>(args)...);

            locations[entity_raw(entity)] = location {
                .dense_index = dense,
                .version = entity_version(entity)
            };

            entity_dirty.include(dense);
            component_dirty.include(dense);
            location_dirty = true;
        }

        template <typename... Args>
        void emplace_or_replace(entity_type entity, Args&&... args)
        {
            if (auto* ptr = try_stage(entity)) {
                *ptr = value_type { std::forward<Args>(args)... };
                const auto dense = locations[entity_raw(entity)].dense_index;
                component_dirty.include(dense);
                return;
            }

            emplace(entity, std::forward<Args>(args)...);
        }

        void remove(entity_type entity)
        {
            const auto it = locations.find(entity_raw(entity));
            if (it == locations.end()) {
                return;
            }

            const auto dense = it->second.dense_index;
            const auto last = dense_count - 1u;

            locations.erase(it);

            if (dense != last) {
                dense_entities[dense] = dense_entities[last];
                staging_components[dense] = std::move(staging_components[last]);

                auto moved_it = locations.find(entity_raw(dense_entities[dense]));
                assert(moved_it != locations.end());
                moved_it->second.dense_index = dense;

                entity_dirty.include(dense);
                component_dirty.include(dense);
            }

            --dense_count;
            dense_entities.pop_back();
            staging_components.pop_back();

            // Removing a dense item changes the valid entity set. Conservatively
            // re-upload the compact arrays and location table on the next upload().
            entity_dirty.include_all(dense_count);
            component_dirty.include_all(dense_count);
            location_dirty = true;
        }

        [[nodiscard]] bool contains(entity_type entity) const noexcept
        {
            const auto it = locations.find(entity_raw(entity));
            if (it == locations.end()) {
                return false;
            }

            const auto dense = it->second.dense_index;

            return dense < dense_count
                && it->second.version == entity_version(entity)
                && dense_entities[dense] == entity;
        }

        [[nodiscard]] std::span<const entity_type> entities() const noexcept
        {
            return { dense_entities.data(), dense_count };
        }

        [[nodiscard]] std::span<const value_type> staged() const noexcept
        {
            return { staging_components.data(), dense_count };
        }

        [[nodiscard]] std::span<const entity_type> uploaded_entities() const noexcept
        {
            return { uploaded_entities_.data(), uploaded_entities_.size() };
        }

        [[nodiscard]] std::span<const value_type> uploaded() const noexcept
        {
            return { uploaded_components.data(), uploaded_components.size() };
        }

        [[nodiscard]] std::span<const entity_type> downloaded_entities() const noexcept
        {
            return { downloaded_entities_.data(), downloaded_entities_.size() };
        }

        [[nodiscard]] std::span<const value_type> readback() const noexcept
        {
            return { downloaded_components.data(), downloaded_components.size() };
        }

        [[nodiscard]] std::optional<value_type> readback(entity_type entity) const
        {
            for (std::size_t i = 0; i < downloaded_entities_.size(); ++i) {
                if (downloaded_entities_[i] == entity) {
                    return downloaded_components[i];
                }
            }

            return std::nullopt;
        }

        [[nodiscard]] size_type size() const noexcept
        {
            return dense_count;
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return dense_count == 0u;
        }

        void clear()
        {
            dense_entities.clear();
            staging_components.clear();
            uploaded_entities_.clear();
            uploaded_components.clear();
            downloaded_entities_.clear();
            downloaded_components.clear();
            locations.clear();
            dense_count = 0u;
            entity_dirty.clear();
            component_dirty.clear();
            location_dirty = false;
        }

        void erase_scene(object_entity_scene_index scene)
        {
            scratch_entities.clear();

            for (std::uint32_t i = 0; i < dense_count; ++i) {
                if (entity_scene(dense_entities[i]) == scene) {
                    scratch_entities.push_back(dense_entities[i]);
                }
            }

            for (auto entity : scratch_entities) {
                remove(entity);
            }

            scratch_entities.clear();
        }

        void upload()
        {
            uploaded_entities_.assign(dense_entities.begin(), dense_entities.end());
            uploaded_components.assign(staging_components.begin(), staging_components.end());
            entity_dirty.clear();
            component_dirty.clear();
            location_dirty = false;
        }

        template <typename Command>
        void upload(Command&&)
        {
            upload();
        }

        void download()
        {
            downloaded_entities_.assign(uploaded_entities_.begin(), uploaded_entities_.end());
            downloaded_components.assign(uploaded_components.begin(), uploaded_components.end());
        }

        template <typename Command>
        void download(Command&&)
        {
            download();
        }

        [[nodiscard]] bool has_pending_upload() const noexcept
        {
            return entity_dirty.any() || component_dirty.any() || location_dirty;
        }

        [[nodiscard]] object_component_upload_snapshot_gpu<T> upload_snapshot() const noexcept
        {
            return {
                .entities = entities(),
                .components = staged()
            };
        }

        [[nodiscard]] object_component_download_snapshot_gpu<T> download_snapshot() const noexcept
        {
            return {
                .entities = uploaded_entities(),
                .components = uploaded()
            };
        }

    private:
        struct location {
            std::uint32_t dense_index {};
            object_entity_version version {};
        };

        value_type* try_stage(entity_type entity)
        {
            const auto it = locations.find(entity_raw(entity));
            if (it == locations.end()) {
                return nullptr;
            }

            const auto dense = it->second.dense_index;
            if (dense >= dense_count || it->second.version != entity_version(entity)) {
                return nullptr;
            }

            return std::addressof(staging_components[dense]);
        }

        std::vector<entity_type> dense_entities;
        std::vector<value_type> staging_components;

        std::vector<entity_type> uploaded_entities_;
        std::vector<value_type> uploaded_components;

        std::vector<entity_type> downloaded_entities_;
        std::vector<value_type> downloaded_components;

        std::unordered_map<std::uint64_t, location> locations;
        std::vector<entity_type> scratch_entities;

        std::uint32_t dense_count {};
        object_dirty_range_gpu entity_dirty {};
        object_dirty_range_gpu component_dirty {};
        bool location_dirty {};
    };

}
}
