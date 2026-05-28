#pragma once

#include <lucaria/core/object_animation.hpp>
#include <lucaria/core/object_cubemap.hpp>
#include <lucaria/core/object_event_track.hpp>
#include <lucaria/core/object_font.hpp>
#include <lucaria/core/object_mesh.hpp>
#include <lucaria/core/object_motion_track.hpp>
#include <lucaria/core/object_shape.hpp>
#include <lucaria/core/object_skeleton.hpp>
#include <lucaria/core/object_sound_track.hpp>
#include <lucaria/core/object_texture.hpp>
#include <lucaria/core/serialize_mappings.hpp>
#include <lucaria/core/serialize_containers.hpp>
#include <lucaria/core/user_asset.hpp>

namespace lucaria {
namespace detail {

    struct manager_assets;

    struct user_asset_type_callbacks {
        std::function<void(manager_assets&, cereal::PortableBinaryOutputArchive&)> binary_save = nullptr;
        std::function<void(manager_assets&, cereal::PortableBinaryInputArchive&)> binary_load = nullptr;
        std::function<void(manager_assets&, cereal::JSONOutputArchive&)> json_save = nullptr;
        std::function<void(manager_assets&, cereal::JSONInputArchive&)> json_load = nullptr;
    };

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
        container_cache_vector<object_animation> animations = {};
        container_cache_vector<object_audio> audios = {};
        container_cache_vector<object_cubemap> cubemaps = {};
        container_cache_vector<object_event_track> event_tracks = {};
        container_cache_vector<object_font> fonts = {};
        container_cache_vector<object_geometry> geometries = {};
        container_cache_vector<object_image> images = {};
        container_cache_vector<object_mesh> meshes = {};
        container_cache_vector<object_motion_track> motion_tracks = {};
        container_cache_vector<object_shape> shapes = {};
        container_cache_vector<object_skeleton> skeletons = {};
        container_cache_vector<object_sound_track> sound_tracks = {};
        container_cache_vector<object_texture> textures = {};
        std::unordered_map<std::type_index, std::unique_ptr<storage_user_asset_base>> user_assets = {};
        std::unordered_map<std::string, user_asset_type_callbacks> user_asset_types = {};
        std::unordered_map<std::type_index, std::string> user_asset_type_ids = {};

		// implemented in serialize_assets.hpp
        template <typename AssetType>
        void register_user_asset(std::string type_id);

        template <typename AssetType>
        storage_user_asset<AssetType>& get_user_asset_storage()
        {
            const std::type_index _type = std::type_index(typeid(AssetType));
            typename std::unordered_map<std::type_index, std::unique_ptr<storage_user_asset_base>>::const_iterator _iterator = user_assets.find(_type);
            if (_iterator == user_assets.end()) {
                std::unique_ptr<storage_user_asset<AssetType>> storage = std::make_unique<storage_user_asset<AssetType>>();
                storage_user_asset<AssetType>* raw = storage.get();
                user_assets.emplace(_type, std::move(storage));
                return *raw;
            }
            return *static_cast<storage_user_asset<AssetType>*>(_iterator->second.get());
        }

        void load_bytes(const std::filesystem::path& path, const std::function<void(const std::vector<char>&)>& callback);
        void fetch_bytes(const std::filesystem::path& path, const std::function<void(const std::vector<char>&)>& callback, bool must_persist = true);
        void fetch_bytes(const std::vector<std::filesystem::path>& paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool must_persist = true);
        void fetch_bytes(const std::array<std::filesystem::path, 6>& paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool must_persist = true);
        void gc_unused();
    };

	// defined in user_asset.hpp
    template <typename AssetType>
    [[nodiscard]] container_cache<object_user_asset<AssetType>>& fetch(
        manager_assets& objects,
        container_cache_vector<object_user_asset<AssetType>>& cached_vector,
        const std::filesystem::path& path)
    {
        return *cached_vector.get_or_create_by_path(path, [&objects, path] {
            std::shared_ptr<std::promise<object_user_asset<AssetType>>> _promise = std::make_shared<std::promise<object_user_asset<AssetType>>>();
            objects.fetch_bytes(path, [_promise](const std::vector<char>& _bytes) {
				object_user_asset<AssetType> _asset(_bytes);
				_promise->set_value(std::move(_asset)); }, true);

            return container_async<object_user_asset<AssetType>>(_promise->get_future());
        });
    }

}
}
