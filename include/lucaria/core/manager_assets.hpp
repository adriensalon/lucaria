#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/serialize_archives.hpp>
#include <lucaria/core/serialize_containers.hpp>
#include <lucaria/core/serialize_context.hpp>
#include <lucaria/core/serialize_mappings.hpp>
#include <lucaria/core/user_assets.hpp>
#include <lucaria/engine/asset_animation.hpp>
#include <lucaria/engine/asset_audio.hpp>
#include <lucaria/engine/asset_cubemap.hpp>
#include <lucaria/engine/asset_event_track.hpp>
#include <lucaria/engine/asset_font.hpp>
#include <lucaria/engine/asset_image.hpp>
#include <lucaria/engine/asset_geometry.hpp>
#include <lucaria/engine/asset_mesh.hpp>
#include <lucaria/engine/asset_motion_track.hpp>
#include <lucaria/engine/asset_shape.hpp>
#include <lucaria/engine/asset_skeleton.hpp>
#include <lucaria/engine/asset_sound_track.hpp>
#include <lucaria/engine/asset_texture.hpp>

namespace lucaria {
namespace detail {

    struct manager_assets;
    struct manager_window;

    template <typename AssetType>
    void load_user_asset_from_bytes(manager_assets& objects, AssetType& asset, const std::vector<char>& bytes);

    template <typename AssetType, typename Callback>
    void load_user_asset_from_bytes(manager_assets& objects, AssetType& asset, const std::vector<char>& bytes, Callback&& callback);

    struct user_asset_type_callbacks {
        std::function<void(manager_assets&, cereal::PortableBinaryOutputArchive&)> binary_save = nullptr;
        std::function<void(manager_assets&, cereal::PortableBinaryInputArchive&)> binary_load = nullptr;
        std::function<void(manager_assets&, cereal::JSONOutputArchive&)> json_save = nullptr;
        std::function<void(manager_assets&, cereal::JSONInputArchive&)> json_load = nullptr;
    };

    template <typename AssetType, typename ContextType, typename = void>
    struct has_storage_save : std::false_type { };

    template <typename AssetType, typename ContextType>
    struct has_storage_save<AssetType, ContextType, std::void_t<decltype(std::declval<AssetType&>().save(std::declval<ContextType&>()))>> : std::true_type { };

    template <typename AssetType, typename ContextType, typename = void>
    struct has_const_storage_save : std::false_type { };

    template <typename AssetType, typename ContextType>
    struct has_const_storage_save<AssetType, ContextType, std::void_t<decltype(std::declval<const AssetType&>().save(std::declval<ContextType&>()))>> : std::true_type { };

    template <typename AssetType, typename ContextType, typename = void>
    struct has_storage_load : std::false_type { };

    template <typename AssetType, typename ContextType>
    struct has_storage_load<AssetType, ContextType, std::void_t<decltype(std::declval<AssetType&>().load(std::declval<ContextType&>()))>> : std::true_type { };

    template <typename AssetType, typename ContextType>
    void save_user_asset_value(AssetType& asset, ContextType& context)
    {
        if constexpr (has_storage_save<AssetType, ContextType>::value) {
            asset.save(context);
        } else if constexpr (has_const_storage_save<AssetType, ContextType>::value) {
            std::as_const(asset).save(context);
        } else {
            context.field("value", asset);
        }
    }

    template <typename AssetType, typename ContextType>
    void load_user_asset_value(AssetType& asset, ContextType& context)
    {
        if constexpr (has_storage_load<AssetType, ContextType>::value) {
            asset.load(context);
        } else {
            context.field("value", asset);
        }
    }

    struct manager_assets {
        manager_assets() = default;
        manager_assets(const manager_assets& other) = delete;
        manager_assets& operator=(const manager_assets& other) = delete;
        manager_assets(manager_assets&& other) = delete;
        manager_assets& operator=(manager_assets&& other) = delete;

        bool is_etc2_supported = false;
        bool is_s3tc_supported = false;
        data_image_profile supported_image_profiles = {}; // TODO replace bools with bitfields

        std::atomic<uint32> async_fetches_waiting = 0;
        std::filesystem::path async_prefix_path = {};
        assets_registry assets = {};
        std::unordered_map<std::string, user_asset_type_callbacks> user_asset_types = {};
        std::unordered_map<std::type_index, std::string> user_asset_type_ids = {};
        std::vector<std::shared_ptr<load_storage_context_base>> active_load_contexts = {};

        // implemented in serialize_assets.hpp
        template <typename AssetType>
        void register_user_asset(std::string type_id);

        template <typename AssetType>
        assets_buffer<AssetType>& get_asset_buffer()
        {
            return assets.template get<AssetType>();
        }

        template <typename AssetType>
        const assets_buffer<AssetType>& get_asset_buffer() const
        {
            return assets.template get<AssetType>();
        }

        template <typename AssetType, typename ArchiveType>
        std::shared_ptr<load_storage_context<AssetType, ArchiveType>> make_load_storage_context(ArchiveType& archive, assets_cell<AssetType>* cell, AssetType& object)
        {
            manager_window* _window = nullptr;
            if constexpr (std::is_same_v<std::decay_t<ArchiveType>, archive_json_input> || std::is_same_v<std::decay_t<ArchiveType>, archive_binary_input>) {
                _window = cereal::get_user_data<mappings_manager_game_load>(archive).loading_window;
            }
            std::shared_ptr<load_storage_context<AssetType, ArchiveType>> _context = std::make_shared<load_storage_context<AssetType, ArchiveType>>(archive, *this, cell, object, _window);
            active_load_contexts.emplace_back(_context);
            return _context;
        }

        std::shared_ptr<asset_fetch_context> make_fetch_context()
        {
            std::shared_ptr<asset_fetch_context> _context = std::make_shared<asset_fetch_context>(*this);
            active_load_contexts.emplace_back(_context);
            return _context;
        }

        template <typename AssetType>
        std::shared_ptr<runtime_storage_context<AssetType>> make_runtime_storage_context(assets_cell<AssetType>* cell, AssetType& object, manager_window* window = nullptr)
        {
            std::shared_ptr<runtime_storage_context<AssetType>> _context = std::make_shared<runtime_storage_context<AssetType>>(*this, cell, object, window);
            active_load_contexts.emplace_back(_context);
            return _context;
        }

        void collect_finished_load_contexts()
        {
            active_load_contexts.erase(
                std::remove_if(active_load_contexts.begin(), active_load_contexts.end(), [](const std::shared_ptr<load_storage_context_base>& context) {
                    return context->finished();
                }),
                active_load_contexts.end());
        }

        void load_bytes(const std::filesystem::path& path, const std::function<void(const std::vector<char>&)>& callback);
        [[nodiscard]] std::future<std::vector<char>> fetch_bytes_future(const std::filesystem::path& path, bool must_persist = true);
        [[nodiscard]] std::future<std::vector<std::vector<char>>> fetch_bytes_future(const std::vector<std::filesystem::path>& paths, bool must_persist = true);
        [[nodiscard]] std::future<std::vector<std::vector<char>>> fetch_bytes_future(const std::array<std::filesystem::path, 6>& paths, bool must_persist = true);
        void fetch_bytes(const std::filesystem::path& path, const std::function<void(const std::vector<char>&)>& callback, bool must_persist = true);
        void fetch_bytes(const std::vector<std::filesystem::path>& paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool must_persist = true);
        void fetch_bytes(const std::array<std::filesystem::path, 6>& paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool must_persist = true);
        void poll_load_contexts();
        void save(storage_save_context& context);
        void load(storage_load_context& context);
        void gc_unused();
    };

    template <typename Asset, typename Configure>
    assets_cell<Asset>& assets_registry::fetch(
        manager_assets& objects,
        std::string cache_id,
        Configure&& configure,
        manager_window* window)
    {
        assets_buffer<Asset>& buffer = get<Asset>();

        if (assets_cell<Asset>* existing = buffer.find_by_id(cache_id)) {
            return *existing;
        }

        assets_cell<Asset>* cell = buffer.create_cell(
            container_async<Asset>::pending(Asset {}),
            std::move(cache_id));

        Asset& asset = cell->fetched.emplaced_value();
        std::forward<Configure>(configure)(asset);

        std::shared_ptr<runtime_storage_context<Asset>> context = objects.template make_runtime_storage_context<Asset>(cell, asset, window);
        load_user_asset_value(asset, *context);
        context->close();

        return *cell;
    }

    template <typename Callback>
    void asset_fetch_context::fetch_worker(const std::filesystem::path& path, Callback&& callback, bool must_persist)
    {
        pending_worker_fetches.fetch_add(1, std::memory_order_acq_rel);
        std::shared_ptr<asset_fetch_context> _context = shared_from_this();

        objects.fetch_bytes(path, [_context, callback = std::forward<Callback>(callback)](const std::vector<char>& bytes) mutable {
            callback(bytes);
            _context->finish_one_worker(); }, must_persist);
    }

    template <typename Callback>
    void asset_fetch_context::fetch_worker(const std::vector<std::filesystem::path>& paths, Callback&& callback, bool must_persist)
    {
        pending_worker_fetches.fetch_add(1, std::memory_order_acq_rel);
        std::shared_ptr<asset_fetch_context> _context = shared_from_this();

        objects.fetch_bytes(paths, [_context, callback = std::forward<Callback>(callback)](const std::vector<std::vector<char>>& bytes) mutable {
            callback(bytes);
            _context->finish_one_worker(); }, must_persist);
    }

    template <typename Callback>
    void asset_fetch_context::fetch_worker(const std::array<std::filesystem::path, 6>& paths, Callback&& callback, bool must_persist)
    {
        pending_worker_fetches.fetch_add(1, std::memory_order_acq_rel);
        std::shared_ptr<asset_fetch_context> _context = shared_from_this();

        objects.fetch_bytes(paths, [_context, callback = std::forward<Callback>(callback)](const std::vector<std::vector<char>>& bytes) mutable {
            callback(bytes);
            _context->finish_one_worker(); }, must_persist);
    }

    template <typename Callback>
    void asset_fetch_context::fetch(const std::filesystem::path& path, Callback&& callback, bool must_persist)
    {
        using _callback_type = std::decay_t<Callback>;
        using _value_type = std::vector<char>;
        std::future<_value_type> _future = objects.fetch_bytes_future(path, must_persist);
        std::unique_ptr<load_context_task_base> _task = std::make_unique<load_context_value_task<_value_type, _callback_type>>(std::move(_future), _callback_type(std::forward<Callback>(callback)));
        {
            std::scoped_lock _lock(tasks_mutex);
            tasks.emplace_back(std::move(_task));
        }
    }

    template <typename Callback>
    void asset_fetch_context::fetch(const std::vector<std::filesystem::path>& paths, Callback&& callback, bool must_persist)
    {
        using _callback_type = std::decay_t<Callback>;
        using _value_type = std::vector<std::vector<char>>;
        std::future<_value_type> _future = objects.fetch_bytes_future(paths, must_persist);
        std::unique_ptr<load_context_task_base> _task = std::make_unique<load_context_value_task<_value_type, _callback_type>>(std::move(_future), _callback_type(std::forward<Callback>(callback)));
        {
            std::scoped_lock _lock(tasks_mutex);
            tasks.emplace_back(std::move(_task));
        }
    }

    template <typename Callback>
    void asset_fetch_context::fetch(const std::array<std::filesystem::path, 6>& paths, Callback&& callback, bool must_persist)
    {
        using _callback_type = std::decay_t<Callback>;
        using _value_type = std::vector<std::vector<char>>;
        std::future<_value_type> _future = objects.fetch_bytes_future(paths, must_persist);
        std::unique_ptr<load_context_task_base> _task = std::make_unique<load_context_value_task<_value_type, _callback_type>>(std::move(_future), _callback_type(std::forward<Callback>(callback)));
        {
            std::scoped_lock _lock(tasks_mutex);
            tasks.emplace_back(std::move(_task));
        }
    }

    template <typename OtherAssetType, typename Callback>
    void asset_fetch_context::fetch_as(const std::filesystem::path& path, Callback&& callback, bool must_persist)
    {
        pending_worker_fetches.fetch_add(1, std::memory_order_acq_rel);
        std::shared_ptr<asset_fetch_context> _context = shared_from_this();

        fetch(path, [_context, callback = std::forward<Callback>(callback)](const std::vector<char>& bytes) mutable {
            std::shared_ptr<OtherAssetType> _other = std::make_shared<OtherAssetType>();
            load_user_asset_from_bytes(_context->objects, *_other, bytes, [_context, _other, callback = std::move(callback)]() mutable {
                if constexpr (std::is_invocable_v<Callback&, OtherAssetType&&>) {
                    callback(std::move(*_other));
                } else if constexpr (std::is_invocable_v<Callback&, OtherAssetType&>) {
                    callback(*_other);
                } else {
                    callback(std::as_const(*_other));
                }
                _context->finish_one_worker();
            }); }, must_persist);
    }

    template <typename AssetType>
    void load_user_asset_from_bytes(manager_assets& objects, AssetType& asset, const std::vector<char>& bytes)
    {
        load_user_asset_from_bytes(objects, asset, bytes, []() { });
    }

    template <typename AssetType, typename Callback>
    void load_user_asset_from_bytes(manager_assets& objects, AssetType& asset, const std::vector<char>& bytes, Callback&& callback)
    {
        static_assert_user_asset<AssetType>();

        bytes_stream _stream(bytes);
        mappings_manager_game_load _mappings = {};
        _mappings.loading_objects = &objects;
        archive_binary_input _archive(_mappings, _stream);

        std::shared_ptr<load_storage_context<AssetType, archive_binary_input>> _context = objects.make_load_storage_context<AssetType>(_archive, nullptr, asset);
        load_user_asset_value(asset, *_context);
        _context->on_finished(std::forward<Callback>(callback));
        _context->close();
    }

}
}
