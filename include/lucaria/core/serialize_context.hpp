#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <cereal/cereal.hpp>

#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/serialize_archives.hpp>
#include <lucaria/bin/types_containers.hpp>

namespace lucaria {
namespace detail {

    struct manager_assets;
    struct manager_window;
    struct manager_scenes;
    struct mappings_manager_game_save;
    struct mappings_manager_game_load;

}

struct context_dynamics;
struct context_game;
namespace detail {

    template <typename AssetType>
    void load_user_asset_from_bytes(manager_assets& objects, AssetType& asset, const std::vector<char>& bytes);

    template <typename AssetType, typename Callback>
    void load_user_asset_from_bytes(manager_assets& objects, AssetType& asset, const std::vector<char>& bytes, Callback&& callback);

    struct load_context_task_base {
        virtual ~load_context_task_base() = default;
        virtual bool poll() = 0;
        virtual bool finished() const = 0;
    };

    struct load_storage_context_base {
        virtual ~load_storage_context_base() = default;
        virtual bool poll() = 0;
        virtual bool finished() const = 0;
    };

    using storage_save_archive_variant = std::variant<archive_json_output*, archive_binary_output*, cereal::JSONOutputArchive*, cereal::PortableBinaryOutputArchive*>;
    using storage_load_archive_variant = std::variant<std::monostate, archive_json_input*, archive_binary_input*, cereal::JSONInputArchive*, cereal::PortableBinaryInputArchive*>;


    template <typename ValueType, typename ContextType, typename = void>
    struct has_context_save : std::false_type { };

    template <typename ValueType, typename ContextType>
    struct has_context_save<ValueType, ContextType, std::void_t<decltype(std::declval<const ValueType&>().save(std::declval<ContextType&>()))>> : std::true_type { };

    template <typename ValueType, typename ContextType>
    inline constexpr bool has_context_save_v = has_context_save<ValueType, ContextType>::value;

    template <typename ValueType, typename ContextType, typename = void>
    struct has_context_load : std::false_type { };

    template <typename ValueType, typename ContextType>
    struct has_context_load<ValueType, ContextType, std::void_t<decltype(std::declval<ValueType&>().load(std::declval<ContextType&>()))>> : std::true_type { };

    template <typename ValueType, typename ContextType>
    inline constexpr bool has_context_load_v = has_context_load<ValueType, ContextType>::value;

    template <typename ContextType, typename ValueType>
    struct context_save_field_wrapper {
        ContextType* context = nullptr;
        const ValueType* value = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType&) const
        {
            value->save(*context);
        }
    };

    template <typename ContextType, typename ValueType>
    struct context_load_field_wrapper {
        ContextType* context = nullptr;
        ValueType* value = nullptr;

        template <typename ArchiveType>
        void load(ArchiveType&)
        {
            value->load(*context);
        }
    };


    struct game_save_context {
        storage_save_archive_variant archive;
        manager_assets& objects;
        manager_scenes& scenes;
        mappings_manager_game_save& mappings;

        game_save_context(archive_json_output& archive, manager_assets& objects, manager_scenes& scenes, mappings_manager_game_save& mappings)
            : archive(&archive)
            , objects(objects)
            , scenes(scenes)
            , mappings(mappings)
        {
        }

        game_save_context(archive_binary_output& archive, manager_assets& objects, manager_scenes& scenes, mappings_manager_game_save& mappings)
            : archive(&archive)
            , objects(objects)
            , scenes(scenes)
            , mappings(mappings)
        {
        }

        game_save_context(cereal::JSONOutputArchive& archive, manager_assets& objects, manager_scenes& scenes, mappings_manager_game_save& mappings)
            : archive(&archive)
            , objects(objects)
            , scenes(scenes)
            , mappings(mappings)
        {
        }

        game_save_context(cereal::PortableBinaryOutputArchive& archive, manager_assets& objects, manager_scenes& scenes, mappings_manager_game_save& mappings)
            : archive(&archive)
            , objects(objects)
            , scenes(scenes)
            , mappings(mappings)
        {
        }

        template <typename ValueType>
        void field(std::string_view name, const ValueType& value)
        {
            const std::string _name(name);
            if constexpr (has_context_save_v<ValueType, game_save_context>) {
                context_save_field_wrapper<game_save_context, ValueType> _wrapper { this, &value };
                std::visit([&](auto* archive) {
                    (*archive)(cereal::make_nvp(_name, _wrapper));
                }, archive);
            } else {
                std::visit([&](auto* archive) {
                    (*archive)(cereal::make_nvp(_name, value));
                }, archive);
            }
        }
    };

    struct game_load_context {
        storage_load_archive_variant archive;
        manager_assets& objects;
        manager_scenes& scenes;
        context_dynamics* dynamics = nullptr;
        mappings_manager_game_load& mappings;

        game_load_context(archive_json_input& archive, manager_assets& objects, manager_scenes& scenes, context_dynamics* dynamics, mappings_manager_game_load& mappings)
            : archive(&archive)
            , objects(objects)
            , scenes(scenes)
            , dynamics(dynamics)
            , mappings(mappings)
        {
        }

        game_load_context(archive_binary_input& archive, manager_assets& objects, manager_scenes& scenes, context_dynamics* dynamics, mappings_manager_game_load& mappings)
            : archive(&archive)
            , objects(objects)
            , scenes(scenes)
            , dynamics(dynamics)
            , mappings(mappings)
        {
        }

        game_load_context(cereal::JSONInputArchive& archive, manager_assets& objects, manager_scenes& scenes, context_dynamics* dynamics, mappings_manager_game_load& mappings)
            : archive(&archive)
            , objects(objects)
            , scenes(scenes)
            , dynamics(dynamics)
            , mappings(mappings)
        {
        }

        game_load_context(cereal::PortableBinaryInputArchive& archive, manager_assets& objects, manager_scenes& scenes, context_dynamics* dynamics, mappings_manager_game_load& mappings)
            : archive(&archive)
            , objects(objects)
            , scenes(scenes)
            , dynamics(dynamics)
            , mappings(mappings)
        {
        }

        template <typename ValueType>
        void field(std::string_view name, ValueType& value)
        {
            const std::string _name(name);
            if constexpr (has_context_load_v<ValueType, game_load_context>) {
                context_load_field_wrapper<game_load_context, ValueType> _wrapper { this, &value };
                std::visit([&](auto archive_value) {
                    using archive_value_type = std::decay_t<decltype(archive_value)>;
                    if constexpr (!std::is_same_v<archive_value_type, std::monostate>) {
                        (*archive_value)(cereal::make_nvp(_name, _wrapper));
                    }
                }, archive);
            } else {
                std::visit([&](auto archive_value) {
                    using archive_value_type = std::decay_t<decltype(archive_value)>;
                    if constexpr (!std::is_same_v<archive_value_type, std::monostate>) {
                        (*archive_value)(cereal::make_nvp(_name, value));
                    }
                }, archive);
            }
        }
    };

    struct storage_save_context {
        storage_save_archive_variant archive;
        manager_assets& objects;

        storage_save_context(archive_json_output& archive, manager_assets& objects)
            : archive(&archive)
            , objects(objects)
        {
        }

        storage_save_context(archive_binary_output& archive, manager_assets& objects)
            : archive(&archive)
            , objects(objects)
        {
        }

        storage_save_context(cereal::JSONOutputArchive& archive, manager_assets& objects)
            : archive(&archive)
            , objects(objects)
        {
        }

        storage_save_context(cereal::PortableBinaryOutputArchive& archive, manager_assets& objects)
            : archive(&archive)
            , objects(objects)
        {
        }

        template <typename ValueType>
        void field(std::string_view name, const ValueType& value)
        {
            const std::string _name(name);
            if constexpr (has_context_save_v<ValueType, storage_save_context>) {
                context_save_field_wrapper<storage_save_context, ValueType> _wrapper { this, &value };
                std::visit([&](auto* archive) {
                    (*archive)(cereal::make_nvp(_name, _wrapper));
                }, archive);
            } else {
                std::visit([&](auto* archive) {
                    (*archive)(cereal::make_nvp(_name, value));
                }, archive);
            }
        }
    };

    struct asset_fetch_context : load_storage_context_base, std::enable_shared_from_this<asset_fetch_context> {
        manager_assets& objects;

        std::atomic<uint32> pending_worker_fetches = 0;
        std::atomic<bool> closed = false;
        std::atomic<bool> finish_invoked = false;
        mutable std::mutex tasks_mutex = {};
        std::vector<std::unique_ptr<load_context_task_base>> tasks = {};
        std::mutex finish_callbacks_mutex = {};
        std::vector<std::function<void()>> finish_callbacks = {};

        explicit asset_fetch_context(manager_assets& objects)
            : objects(objects)
        {
        }

        template <typename Callback>
        void fetch_worker(const std::filesystem::path& path, Callback&& callback, bool must_persist = true);

        template <typename Callback>
        void fetch_worker(const std::vector<std::filesystem::path>& paths, Callback&& callback, bool must_persist = true);

        template <typename Callback>
        void fetch_worker(const std::array<std::filesystem::path, 6>& paths, Callback&& callback, bool must_persist = true);

        template <typename Callback>
        void fetch(const std::filesystem::path& path, Callback&& callback, bool must_persist = true);

        template <typename Callback>
        void fetch(const std::vector<std::filesystem::path>& paths, Callback&& callback, bool must_persist = true);

        template <typename Callback>
        void fetch(const std::array<std::filesystem::path, 6>& paths, Callback&& callback, bool must_persist = true);

        template <typename OtherAssetType, typename Callback>
        void fetch_as(const std::filesystem::path& path, Callback&& callback, bool must_persist = true);

    protected:
        void track_fetch_path(const std::filesystem::path& path);

        template <typename Paths>
        void track_fetch_paths(const Paths& paths);

        virtual void track_resolved_fetch_path(const std::filesystem::path&)
        {
        }

    public:
        void close()
        {
            closed.store(true, std::memory_order_release);
            try_finish();
        }

        template <typename Callback>
        void on_finished(Callback&& callback)
        {
            if (finished()) {
                callback();
                return;
            }

            std::scoped_lock _lock(finish_callbacks_mutex);
            if (finished()) {
                callback();
                return;
            }
            finish_callbacks.emplace_back(std::forward<Callback>(callback));
        }

        bool poll() override
        {
            std::size_t _index = 0;
            while (true) {
                load_context_task_base* _task = nullptr;
                {
                    std::scoped_lock _lock(tasks_mutex);
                    if (_index >= tasks.size()) {
                        break;
                    }
                    _task = tasks[_index].get();
                }

                _task->poll();
                ++_index;
            }

            try_finish();
            return finished();
        }

        bool finished() const override
        {
            if (!closed.load(std::memory_order_acquire) || pending_worker_fetches.load(std::memory_order_acquire) != 0) {
                return false;
            }

            std::scoped_lock _lock(tasks_mutex);
            for (const std::unique_ptr<load_context_task_base>& _task : tasks) {
                if (!_task->finished()) {
                    return false;
                }
            }
            return true;
        }

    protected:
        virtual void notify_finished()
        {
        }

        void finish_one_worker()
        {
            if (pending_worker_fetches.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                try_finish();
            }
        }

        void try_finish()
        {
            if (!finished()) {
                return;
            }
            bool _expected = false;
            if (!finish_invoked.compare_exchange_strong(_expected, true, std::memory_order_acq_rel)) {
                return;
            }

            notify_finished();

            std::vector<std::function<void()>> _callbacks;
            {
                std::scoped_lock _lock(finish_callbacks_mutex);
                _callbacks.swap(finish_callbacks);
            }
            for (std::function<void()>& _callback : _callbacks) {
                _callback();
            }
        }
    };

    struct storage_load_context : asset_fetch_context {
        storage_load_archive_variant archive = std::monostate {};
        manager_window* loading_window = nullptr;

        storage_load_context(archive_json_input& archive, manager_assets& objects, manager_window* window = nullptr)
            : asset_fetch_context(objects)
            , archive(&archive)
            , loading_window(window)
        {
        }

        storage_load_context(archive_binary_input& archive, manager_assets& objects, manager_window* window = nullptr)
            : asset_fetch_context(objects)
            , archive(&archive)
            , loading_window(window)
        {
        }

        storage_load_context(cereal::JSONInputArchive& archive, manager_assets& objects, manager_window* window = nullptr)
            : asset_fetch_context(objects)
            , archive(&archive)
            , loading_window(window)
        {
        }

        storage_load_context(cereal::PortableBinaryInputArchive& archive, manager_assets& objects, manager_window* window = nullptr)
            : asset_fetch_context(objects)
            , archive(&archive)
            , loading_window(window)
        {
        }

        storage_load_context(manager_assets& objects, manager_window* window = nullptr)
            : asset_fetch_context(objects)
            , archive(std::monostate {})
            , loading_window(window)
        {
        }

        [[nodiscard]] bool has_archive() const
        {
            return !std::holds_alternative<std::monostate>(archive);
        }

        template <typename ValueType>
        void field(std::string_view name, ValueType& value)
        {
            if (!has_archive()) {
                return;
            }
            const std::string _name(name);
            if constexpr (has_context_load_v<ValueType, storage_load_context>) {
                context_load_field_wrapper<storage_load_context, ValueType> _wrapper { this, &value };
                std::visit([&](auto archive_value) {
                    using archive_value_type = std::decay_t<decltype(archive_value)>;
                    if constexpr (!std::is_same_v<archive_value_type, std::monostate>) {
                        (*archive_value)(cereal::make_nvp(_name, _wrapper));
                    }
                }, archive);
            } else {
                std::visit([&](auto archive_value) {
                    using archive_value_type = std::decay_t<decltype(archive_value)>;
                    if constexpr (!std::is_same_v<archive_value_type, std::monostate>) {
                        (*archive_value)(cereal::make_nvp(_name, value));
                    }
                }, archive);
            }
        }

        [[nodiscard]] manager_window* window()
        {
            return loading_window;
        }
    };

    template <typename ValueType, typename Callback>
    struct load_context_value_task final : load_context_task_base {
        container_async<ValueType> value = {};
        Callback callback;
        bool done = false;

        load_context_value_task(std::future<ValueType>&& future, Callback&& callback)
            : value(std::move(future), [](const ValueType& value) {
                return value;
            })
            , callback(std::move(callback))
        {
        }

        bool poll() override
        {
            if (done) {
                return true;
            }
            if (!value.has_value()) {
                return false;
            }
            callback(std::as_const(value.value()));
            done = true;
            return true;
        }

        bool finished() const override
        {
            return done;
        }
    };

    template <typename AssetType, typename ArchiveType>
    struct load_storage_context final : storage_load_context {
        assets_cell<AssetType>* cell;
        AssetType& self;

        load_storage_context(ArchiveType& archive, manager_assets& objects, assets_cell<AssetType>* cell, AssetType& self, manager_window* window = nullptr)
            : storage_load_context(archive, objects, window)
            , cell(cell)
            , self(self)
        {
        }

    protected:
        void track_resolved_fetch_path(const std::filesystem::path& path) override
        {
            if (cell == nullptr) {
                return;
            }

            std::vector<object_filewatched_path>& watched_paths = cell->object_filewatched_paths;
            const auto existing = std::find_if(watched_paths.begin(), watched_paths.end(), [&](const object_filewatched_path& watched_path) {
                return watched_path.get() == path;
            });
            if (existing == watched_paths.end()) {
                watched_paths.emplace_back(path);
            }
        }

        void notify_finished() override
        {
            if (cell != nullptr) {
                cell->fetched.mark_ready();
            }
        }
    };

    template <typename AssetType>
    struct runtime_storage_context final : storage_load_context {
        assets_cell<AssetType>* cell;
        AssetType& self;

        runtime_storage_context(manager_assets& objects, assets_cell<AssetType>* cell, AssetType& self, manager_window* runtime_window = nullptr)
            : storage_load_context(objects, runtime_window)
            , cell(cell)
            , self(self)
        {
        }

    protected:
        void track_resolved_fetch_path(const std::filesystem::path& path) override
        {
            if (cell == nullptr) {
                return;
            }

            std::vector<object_filewatched_path>& watched_paths = cell->object_filewatched_paths;
            const auto existing = std::find_if(watched_paths.begin(), watched_paths.end(), [&](const object_filewatched_path& watched_path) {
                return watched_path.get() == path;
            });
            if (existing == watched_paths.end()) {
                watched_paths.emplace_back(path);
            }
        }

        void notify_finished() override
        {
            if (cell != nullptr) {
                cell->fetched.mark_ready();
            }
        }
    };

    using save_storage_context = storage_save_context;

}
}

namespace lucaria {
    using game_save_context = detail::game_save_context;
    using game_load_context = detail::game_load_context;
    using storage_save_context = detail::storage_save_context;
    using storage_load_context = detail::storage_load_context;
}

