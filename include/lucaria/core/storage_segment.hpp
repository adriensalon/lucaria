#pragma once

#include <entt/entt.hpp>

#include <lucaria/core/storage_entity.hpp>
#include <lucaria/core/utils_error.hpp>

namespace lucaria {
namespace detail {

    template <typename PageType, typename Allocator>
    struct object_page_pool_cpu {
        using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<PageType>;
        using allocator_traits = std::allocator_traits<allocator_type>;

        explicit object_page_pool_cpu(const Allocator& alloc = Allocator {})
            : _allocator { alloc }
        {
        }

        ~object_page_pool_cpu()
        {
            clear_memory();
        }

        object_page_pool_cpu(const object_page_pool_cpu&) = delete;
        object_page_pool_cpu& operator=(const object_page_pool_cpu&) = delete;

        PageType* acquire()
        {
            PageType* _ptr = nullptr;

            if (!_free.empty()) {
                _ptr = _free.back();
                _free.pop_back();

            } else {
                _ptr = allocator_traits::allocate(_allocator, 1);
                allocator_traits::construct(_allocator, _ptr);
                _allocated.push_back(_ptr);
            }

            return _ptr;
        }

        void release(PageType* ptr) noexcept
        {
            if (ptr == nullptr) {
                return;
            }

            // PageType remains constructed
            // caller must have reset its runtime contents
            _free.push_back(ptr);
        }

        void clear_memory() noexcept
        {
            for (PageType* _ptr : _allocated) {
                allocator_traits::destroy(_allocator, _ptr);
                allocator_traits::deallocate(_allocator, _ptr, 1);
            }

            _allocated.clear();
            _free.clear();
        }

    private:
        allocator_type _allocator = {};
        std::vector<PageType*> _allocated = {};
        std::vector<PageType*> _free = {};
    };

    template <typename T, typename Entity, typename Allocator>
    struct object_segment_storage_cpu : public entt::basic_sparse_set<Entity, typename std::allocator_traits<Allocator>::template rebind_alloc<Entity>> {
        using alloc_traits_type = std::allocator_traits<Allocator>;
        using entity_allocator_type = typename alloc_traits_type::template rebind_alloc<Entity>;
        using value_type = T;
        using entity_type = Entity;
        using allocator_type = Allocator;
        using base_type = entt::basic_sparse_set<Entity, entity_allocator_type>;
        using size_type = std::size_t;
        using traits_type = entt::component_traits<T>;

        // static constexpr std::uint32_t page_size = 256u;
        static constexpr std::uint32_t page_size = 4u;

        struct segment {
            object_entity_scene_index scene = {};
            entity_type* entities = nullptr;
            T* components = nullptr;
            std::uint32_t count = {};
        };

        struct const_segment {
            object_entity_scene_index scene = {};
            const entity_type* entities = nullptr;
            const value_type* components = nullptr;
            std::uint32_t count = {};
        };

        object_segment_storage_cpu()
            : object_segment_storage_cpu { allocator_type {} }
        {
        }

        explicit object_segment_storage_cpu(const allocator_type& allocator)
            : base_type {
                entt::type_id<value_type>(),
                entt::deletion_policy { entt::deletion_policy::swap_and_pop },
                entity_allocator_type { allocator }
            }
            , _allocator { allocator }
            , _page_pool { allocator }
        {
        }

        object_segment_storage_cpu(const object_segment_storage_cpu&) = delete;
        object_segment_storage_cpu& operator=(const object_segment_storage_cpu&) = delete;

        ~object_segment_storage_cpu() override
        {
            clear_payload();
        }

        allocator_type get_allocator() const noexcept
        {
            return _allocator;
        }

        template <typename... Args>
        value_type& emplace(entity_type entity, Args&&... args)
        {
            const object_entity_scene_index _scene = entity_scene(entity);
            auto _iterator = base_type::try_emplace(entity, false);
            const std::size_t _base_index = static_cast<std::size_t>(_iterator.index());
            if (_locations_by_index.size() <= _base_index) {
                _locations_by_index.resize(_base_index + 1u);
            }
            partition& _partition = assure_partition(_scene);
            auto [_page_index, _page] = assure_page(_partition);
            const std::size_t _offset = _page->count++;
            _page->entities[_offset] = entity;
            T* _ptr = _page->components() + _offset;
            std::construct_at(_ptr, std::forward<Args>(args)...);
            _locations_by_index[_base_index] = location {
                .scene = _scene,
                .page = _page_index,
                .offset = static_cast<std::uint32_t>(_offset)
            };
            return *_ptr;
        }

        void clear()
        {
            _scratch_entities.clear();
            _scratch_entities.reserve(base_type::size());
            for (auto _iterator = base_type::begin(); _iterator != base_type::end(); ++_iterator) {
                _scratch_entities.push_back(*_iterator);
            }
            for (const auto _entity : _scratch_entities) {
                erase_one(_entity);
            }
            _scratch_entities.clear();
        }

        [[nodiscard]] bool contains(entity_type entity) const noexcept
        {
            return base_type::contains(entity);
        }

        [[nodiscard]] bool contains(object_entity_scene_index scene, entity_type entity) const noexcept
        {
            if (scene >= _partitions.size()) {
                return false;
            }
            if (!base_type::contains(entity)) {
                return false;
            }
            const auto _index = base_type::index(entity);
            const auto _location = _locations_by_index[_index];
            return _location.scene == scene;
        }

        value_type& get(entity_type entity) noexcept
        {
            const auto _index = base_type::index(entity);
            auto _location = _locations_by_index[_index];
            return _partitions[_location.scene].pages[_location.page]->components()[_location.offset];
        }

        const value_type& get(entity_type entity) const noexcept
        {
            const auto _index = base_type::index(entity);
            auto _location = _locations_by_index[_index];
            return _partitions[_location.scene].pages[_location.page]->components()[_location.offset];
        }

        [[nodiscard]] value_type& get(object_entity_scene_index scene, entity_type entity) noexcept
        {
            LUCARIA_DEBUG_ASSERT(contains(scene, entity), "Entity is not contained in the scene");
            const auto _index = base_type::index(entity);
            const auto _location = _locations_by_index[_index];
            return _partitions[scene].pages[_location.page]->components()[_location.offset];
        }

        [[nodiscard]] const value_type& get(object_entity_scene_index scene, entity_type entity) const noexcept
        {
            LUCARIA_DEBUG_ASSERT(contains(scene, entity), "Entity is not contained in the scene");
            const auto _index = base_type::index(entity);
            const auto _location = _locations_by_index[_index];
            return _partitions[scene].pages[_location.page]->components()[_location.offset];
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
            auto& _value = get(entity);
            (std::forward<Func>(func)(_value), ...);
            return _value;
        }

        void erase_one(entity_type entity)
        {
            const auto _erased_index = base_type::index(entity);
            const auto _last_index = base_type::size() - 1u;
            const auto _moved_entity = base_type::data()[_last_index];
            erase_payload_at_base_index(_erased_index);
            base_type::swap_and_pop(base_type::find(entity));
            if (_erased_index != _last_index) {
                _locations_by_index[_erased_index] = _locations_by_index[_last_index];
            }
            _locations_by_index.pop_back();
        }

        void pop(typename base_type::iterator first, typename base_type::iterator last) override
        {
            _scratch_entities.clear();
            for (; first != last; ++first) {
                _scratch_entities.push_back(*first);
            }
            for (auto _entity : _scratch_entities) {
                erase_one(_entity);
            }
            _scratch_entities.clear();
        }

        void erase_payload_at_base_index(std::size_t base_index)
        {
            const auto _location = _locations_by_index[base_index];
            auto& _page = *_partitions[_location.scene].pages[_location.page];
            auto* _components = _page.components();
            const auto last = _page.count - 1u;
            std::destroy_at(_components + _location.offset);
            if (_location.offset != last) {
                std::construct_at(_components + _location.offset, std::move(_components[last]));
                std::destroy_at(_components + last);
                const auto _moved_entity = _page.entities[last];
                _page.entities[_location.offset] = _moved_entity;
                const auto _moved_base_index = base_type::index(_moved_entity);
                _locations_by_index[_moved_base_index] = location {
                    .scene = _location.scene,
                    .page = _location.page,
                    .offset = _location.offset
                };
            }
            --_page.count;
        }

        template <typename CallbackType>
        void each_segment(object_entity_scene_index scene, CallbackType&& callback) noexcept
        {
            if (scene >= _partitions.size()) {
                return;
            }
            auto& _partition = _partitions[scene];
            for (page* _page : _partition.pages) {
                if (_page->count != 0u) {
                    callback(segment {
                        .scene = scene,
                        .entities = _page->entities,
                        .components = _page->components(),
                        .count = _page->count });
                }
            }
        }

        template <typename CallbackType>
        void each_segment(CallbackType&& callback) noexcept
        {
            for (object_entity_scene_index _scene = 0; _scene < _partitions.size(); ++_scene) {
                each_segment(_scene, callback);
            }
        }

        template <typename CallbackType>
        void each_segment(object_entity_scene_index scene, CallbackType&& callback) const noexcept
        {
            if (scene >= _partitions.size()) {
                return;
            }
            const auto& _partition = _partitions[scene];
            for (const auto& _page_ptr : _partition.pages) {
                const auto& _page = *_page_ptr;
                if (_page.count != 0u) {
                    callback(const_segment {
                        .scene = scene,
                        .entities = _page.entities,
                        .components = _page.components(),
                        .count = _page.count });
                }
            }
        }

        template <typename CallbackType>
        void each_segment(CallbackType&& callback) const noexcept
        {
            for (object_entity_scene_index _scene = 0; _scene < _partitions.size(); ++_scene) {
                each_segment(_scene, callback);
            }
        }

        void erase_scene(object_entity_scene_index scene)
        {
            if (scene >= _partitions.size()) {
                return;
            }
            _scratch_entities.clear();
            auto& _partition = _partitions[scene];
            for (page* pg : _partition.pages) {
                for (std::uint32_t i = 0; i < pg->count; ++i) {
                    _scratch_entities.push_back(pg->entities[i]);
                }
            }
            for (auto entity : _scratch_entities) {
                if (base_type::contains(entity)) {
                    erase_one(entity);
                }
            }
            for (page* pg : _partition.pages) {
                LUCARIA_DEBUG_ASSERT(pg->count == 0, "After erase_one all components in these pages should be gone");
                _page_pool.release(pg);
            }
            _partition.pages.clear();
        }

    private:
        struct location {
            object_entity_scene_index scene = {};
            std::uint32_t page = {};
            std::uint32_t offset = {};
        };

        struct page {
            entity_type entities[page_size];
            alignas(value_type) std::byte raw[sizeof(value_type) * page_size];
            std::uint32_t count = {};

            value_type* components() noexcept
            {
                return std::launder(reinterpret_cast<value_type*>(raw));
            }

            const value_type* components() const noexcept
            {
                return std::launder(reinterpret_cast<const value_type*>(raw));
            }
        };

        struct partition {
            std::vector<page*> pages = {};
        };

        allocator_type _allocator = {};
        std::vector<partition> _partitions = {};
        std::vector<location> _locations_by_index = {};
        std::vector<entity_type> _scratch_entities = {};
        object_page_pool_cpu<page, allocator_type> _page_pool = {};

        void clear_payload()
        {
            for (partition& _partition : _partitions) {
                for (page* _page : _partition.pages) {
                    auto* _components = _page->components();
                    for (std::uint32_t _page_index = 0; _page_index < _page->count; ++_page_index) {
                        std::destroy_at(_components + _page_index);
                    }
                    _page->count = 0;
                    _page_pool.release(_page);
                }
                _partition.pages.clear();
            }
            _partitions.clear();
            _locations_by_index.clear();
        }

        partition& assure_partition(object_entity_scene_index scene)
        {
            if (_partitions.size() <= static_cast<std::size_t>(scene)) {
                _partitions.resize(static_cast<std::size_t>(scene) + 1u);
            }
            return _partitions[scene];
        }

        std::pair<std::uint32_t, page*> assure_page(partition& part)
        {
            if (part.pages.empty() || part.pages.back()->count == page_size) {
                auto* _page = _page_pool.acquire();
                _page->count = 0;
                part.pages.push_back(_page);
            }
            const std::uint32_t _index = static_cast<std::uint32_t>(part.pages.size() - 1u);
            return { _index, part.pages.back() };
        }

        template <bool IsConst>
        struct basic_each_iterator {
            using storage_type = std::conditional_t<IsConst, const object_segment_storage_cpu, object_segment_storage_cpu>;
            using base_iterator_type = std::conditional_t<IsConst, typename base_type::const_iterator, typename base_type::iterator>;
            using component_ref = std::conditional_t<IsConst, const value_type&, value_type&>;

            using difference_type = std::ptrdiff_t;
            using value_type_tuple = std::tuple<entity_type, component_ref>;
            using value_type = value_type_tuple;
            using reference = value_type_tuple;
            using pointer = void;
            using iterator_category = std::input_iterator_tag;

            basic_each_iterator() = default;

            basic_each_iterator(storage_type* storage, base_iterator_type it)
                : _storage { storage }
                , _it { it }
            {
            }

            basic_each_iterator& operator++() noexcept
            {
                ++_it;
                return *this;
            }

            basic_each_iterator operator++(int) noexcept
            {
                auto _copy = *this;
                ++(*this);
                return _copy;
            }

            reference operator*() const noexcept
            {
                const auto _entity = *_it;
                return { _entity, _storage->get(_entity) };
            }

            bool operator==(const basic_each_iterator& other) const noexcept
            {
                return _it == other._it;
            }

            bool operator!=(const basic_each_iterator& other) const noexcept
            {
                return !(*this == other);
            }

        private:
            storage_type* _storage = nullptr;
            base_iterator_type _it = {};
        };

        template <bool IsConst>
        struct basic_each_range {
            using storage_type = std::conditional_t<IsConst, const object_segment_storage_cpu, object_segment_storage_cpu>;
            using iterator = basic_each_iterator<IsConst>;

            storage_type* storage = {};

            iterator begin() const noexcept
            {
                if constexpr (IsConst) {
                    return iterator { storage, storage->base_type::cbegin() };
                } else {
                    return iterator { storage, storage->base_type::begin() };
                }
            }

            iterator end() const noexcept
            {
                if constexpr (IsConst) {
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

}
}
