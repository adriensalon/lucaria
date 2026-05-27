#pragma once

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

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

namespace lucaria {
namespace detail {

    struct manager_window;

    enum struct object_user_asset_origin {
        path,
        data
    };

    template <typename AssetType>
    struct object_user_asset {

        object_user_asset(const std::vector<char>& bytes)
            : origin(object_user_asset_origin::path)
            , data(AssetType(bytes))
        {
        }

        object_user_asset(AssetType&& data)
            : origin(object_user_asset_origin::data)
            , data(std::move(data))
        {
        }

        object_user_asset_origin origin;
        AssetType data;
    };

    template <typename AssetType>
    struct recipe_object_user_asset_path {
        std::filesystem::path path;

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    template <typename AssetType>
    struct recipe_object_user_asset_data {
        AssetType value;

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(cereal::make_nvp("value", value));
        }
    };

    template <typename AssetType>
    using recipe_object_user_asset = std::variant<
        recipe_object_user_asset_path<AssetType>,
        recipe_object_user_asset_data<AssetType>>;

    template <typename AssetType>
    [[nodiscard]] recipe_object_user_asset<AssetType> make_recipe(const container_cache<object_user_asset<AssetType>>& cache)
    {
        const object_user_asset<AssetType>& _asset = cache.fetched.value();

        if (_asset.origin == object_user_asset_origin::path) {
            return recipe_object_user_asset_path<AssetType> { cache.origin_path.value() };

        } else if (_asset.origin == object_user_asset_origin::data) {
            return recipe_object_user_asset_data<AssetType> { _asset.data };

        } else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

    template <typename AssetType>
    container_cache<object_user_asset<AssetType>>* apply_recipe(
        manager_object& objects,
        container_cache_vector<object_user_asset<AssetType>>& cached_vector,
        recipe_object_user_asset<AssetType>& recipe)
    {
        return std::visit([&](auto& value) -> container_cache<object_user_asset<AssetType>>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_user_asset_path<AssetType>>) {
                return &fetch(objects, cached_vector, value.path);

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_user_asset_data<AssetType>>) {
                return cached_vector.create_cell(
                    container_async<object_user_asset<AssetType>>(
                        object_user_asset<AssetType>(std::move(value.value))));

            } else {
                LUCARIA_DEBUG_ERROR("Unhandled user asset recipe type");
                return nullptr;
            }
        },
            recipe);
    }

    struct storage_user_asset_base {
        virtual ~storage_user_asset_base() = default;
    };

    template <typename AssetType>
    struct storage_user_asset final : storage_user_asset_base {
        container_cache_vector<object_user_asset<AssetType>> assets;
    };

    struct user_asset_type_callbacks {
        std::function<void(manager_object&, cereal::PortableBinaryOutputArchive&)> binary_save = nullptr;
        std::function<void(manager_object&, cereal::PortableBinaryInputArchive&)> binary_load = nullptr;
        std::function<void(manager_object&, cereal::JSONOutputArchive&)> json_save = nullptr;
        std::function<void(manager_object&, cereal::JSONInputArchive&)> json_load = nullptr;
    };

    template <typename AssetType, typename Archive>
    void save_user_asset_group(manager_object& objects, Archive& archive);

    template <typename AssetType, typename Archive>
    void load_user_asset_group(manager_object& objects, Archive& archive);

    struct manager_object {
        manager_object() = default;
        manager_object(const manager_object& other) = delete;
        manager_object& operator=(const manager_object& other) = delete;
        manager_object(manager_object&& other) = default;
        manager_object& operator=(manager_object&& other) = default;

        bool is_etc2_supported = false;
        bool is_s3tc_supported = false;
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

        template <typename AssetType>
        void register_user_asset(std::string type_id)
        {
            user_asset_type_callbacks callbacks = {};

            callbacks.binary_save = [](manager_object& objects, cereal::PortableBinaryOutputArchive& archive) {
                save_user_asset_group<AssetType>(objects, archive);
            };

            callbacks.binary_load = [](manager_object& objects, cereal::PortableBinaryInputArchive& archive) {
                load_user_asset_group<AssetType>(objects, archive);
            };

            callbacks.json_save = [](manager_object& objects, cereal::JSONOutputArchive& archive) {
                save_user_asset_group<AssetType>(objects, archive);
            };

            callbacks.json_load = [](manager_object& objects, cereal::JSONInputArchive& archive) {
                load_user_asset_group<AssetType>(objects, archive);
            };

            user_asset_type_ids[std::type_index(typeid(AssetType))] = type_id;
            user_asset_types[std::move(type_id)] = std::move(callbacks);

            get_user_asset_storage<AssetType>();
        }

        template <typename AssetType>
        storage_user_asset<AssetType>& get_user_asset_storage()
        {
            const std::type_index type = std::type_index(typeid(AssetType));
            auto it = user_assets.find(type);
            if (it == user_assets.end()) {
                auto storage = std::make_unique<storage_user_asset<AssetType>>();
                storage_user_asset<AssetType>* raw = storage.get();
                user_assets.emplace(type, std::move(storage));
                return *raw;
            }
            return *static_cast<storage_user_asset<AssetType>*>(it->second.get());
        }

        void load_bytes(const std::filesystem::path& path, const std::function<void(const std::vector<char>&)>& callback);
        void fetch_bytes(const std::filesystem::path& path, const std::function<void(const std::vector<char>&)>& callback, bool must_persist = true);
        void fetch_bytes(const std::vector<std::filesystem::path>& paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool must_persist = true);
        void fetch_bytes(const std::array<std::filesystem::path, 6>& paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool must_persist = true);
        void gc_unused();
    };

    template <typename AssetType>
    [[nodiscard]] container_cache<object_user_asset<AssetType>>& fetch(
        manager_object& objects,
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

    // mappings

    struct mappings_user_asset_base_save {
        virtual ~mappings_user_asset_base_save() = default;
    };

    template <typename AssetType>
    struct mappings_user_asset_save final : mappings_user_asset_base_save {
        mappings_container_cache_vector_save<object_user_asset<AssetType>> typed_mapping = {};
    };

    struct mappings_user_assets_save {

        template <typename AssetType>
        [[nodiscard]] mappings_container_cache_vector_save<object_user_asset<AssetType>>& get_mapping()
        {
            const std::type_index _type = std::type_index(typeid(AssetType));
            typename std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_save>>::const_iterator _iterator = _mappings.find(_type);
            if (_iterator == _mappings.end()) {
                std::unique_ptr<mappings_user_asset_save<AssetType>> _mapping_ptr = std::make_unique<mappings_user_asset_save<AssetType>>();
                mappings_user_asset_save<AssetType>* _mapping_raw = _mapping_ptr.get();
                _mappings.emplace(_type, std::move(_mapping_ptr));
                return _mapping_raw->typed_mapping;
            }
            return static_cast<mappings_user_asset_save<AssetType>*>(_iterator->second.get())->typed_mapping;
        }

        template <typename AssetType>
        [[nodiscard]] uint32 get_or_create(const container_cache<object_user_asset<AssetType>>* cell)
        {
            return get_mapping<AssetType>().get(cell);
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_save>> _mappings = {};
    };

    struct mappings_user_asset_base_load {
        virtual ~mappings_user_asset_base_load() = default;
    };

    template <typename AssetType>
    struct mappings_user_asset_load final : mappings_user_asset_base_load {
        mappings_container_cache_vector_load<object_user_asset<AssetType>> typed_mapping = {};
    };

    struct mappings_user_assets_load {

        template <typename AssetType>
        [[nodiscard]] mappings_container_cache_vector_load<object_user_asset<AssetType>>& get_mapping()
        {
            const std::type_index _type = std::type_index(typeid(AssetType));
            typename std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_load>>::const_iterator _iterator = _mappings.find(_type);
            if (_iterator == _mappings.end()) {
                std::unique_ptr<mappings_user_asset_load<AssetType>> _mapping_ptr = std::make_unique<mappings_user_asset_load<AssetType>>();
                mappings_user_asset_load<AssetType>* _mapping_raw = _mapping_ptr.get();
                _mappings.emplace(_type, std::move(_mapping_ptr));
                return _mapping_raw->typed_mapping;
            }
            return static_cast<mappings_user_asset_load<AssetType>*>(_iterator->second.get())->typed_mapping;
        }

        template <typename AssetType>
        void set(uint32 id, container_cache<object_user_asset<AssetType>>* cell)
        {
            get_mapping<AssetType>().set(id, cell);
        }

        template <typename AssetType>
        [[nodiscard]] container_cache<object_user_asset<AssetType>>* get(uint32 id)
        {
            return get_mapping<AssetType>().get(id);
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_load>> _mappings = {};
    };

    struct mappings_manager_object_save {
        mappings_container_cache_vector_save<object_animation> animations = {};
        mappings_container_cache_vector_save<object_audio> audios = {};
        mappings_container_cache_vector_save<object_cubemap> cubemaps = {};
        mappings_container_cache_vector_save<object_event_track> event_tracks = {};
        mappings_container_cache_vector_save<object_font> fonts = {};
        mappings_container_cache_vector_save<object_geometry> geometries = {};
        mappings_container_cache_vector_save<object_image> images = {};
        mappings_container_cache_vector_save<object_mesh> meshes = {};
        mappings_container_cache_vector_save<object_motion_track> motion_tracks = {};
        mappings_container_cache_vector_save<object_shape> shapes = {};
        mappings_container_cache_vector_save<object_skeleton> skeletons = {};
        mappings_container_cache_vector_save<object_sound_track> sound_tracks = {};
        mappings_container_cache_vector_save<object_texture> textures = {};

        mappings_user_assets_save user_assets = {};
    };

    struct mappings_manager_object_load {
        mappings_container_cache_vector_load<object_image> images = {};
        mappings_container_cache_vector_load<object_texture> textures = {};
        mappings_container_cache_vector_load<object_cubemap> cubemaps = {};
        mappings_container_cache_vector_load<object_geometry> geometries = {};
        mappings_container_cache_vector_load<object_shape> shapes = {};
        mappings_container_cache_vector_load<object_mesh> meshes = {};
        mappings_container_cache_vector_load<object_font> fonts = {};
        mappings_container_cache_vector_load<object_audio> audios = {};
        mappings_container_cache_vector_load<object_sound_track> sound_tracks = {};
        mappings_container_cache_vector_load<object_skeleton> skeletons = {};
        mappings_container_cache_vector_load<object_animation> animations = {};
        mappings_container_cache_vector_load<object_motion_track> motion_tracks = {};
        mappings_container_cache_vector_load<object_event_track> event_tracks = {};

        mappings_user_assets_load user_assets = {};
    };

    // recipes

    template <typename RecipeType>
    struct recipe_object_entry {
        uint32 save_id;
        RecipeType recipe;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("object_save_id", save_id));
            archive(cereal::make_nvp("recipe", recipe));
        }
    };

    struct recipe_object_user_asset_group {
        std::string type_id = {};
        manager_object* objects = nullptr;
        user_asset_type_callbacks* callbacks = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("type_id", type_id));

            if constexpr (std::is_base_of_v<cereal::JSONOutputArchive, ArchiveType>) {
                callbacks->json_save(*objects, archive);
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryOutputArchive, ArchiveType>) {
                callbacks->binary_save(*objects, archive);
            }
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive);
    };

    struct recipe_manager_object {
        std::vector<recipe_object_entry<recipe_object_animation>> animations = {};
        std::vector<recipe_object_entry<recipe_object_audio>> audios = {};
        std::vector<recipe_object_entry<recipe_object_cubemap>> cubemaps = {};
        std::vector<recipe_object_entry<recipe_object_event_track>> event_tracks = {};
        std::vector<recipe_object_entry<recipe_object_font>> fonts = {};
        std::vector<recipe_object_entry<recipe_object_geometry>> geometries = {};
        std::vector<recipe_object_entry<recipe_object_image>> images = {};
        std::vector<recipe_object_entry<recipe_object_mesh>> meshes = {};
        std::vector<recipe_object_entry<recipe_object_motion_track>> motion_tracks = {};
        std::vector<recipe_object_entry<recipe_object_shape>> shapes = {};
        std::vector<recipe_object_entry<recipe_object_skeleton>> skeletons = {};
        std::vector<recipe_object_entry<recipe_object_sound_track>> sound_tracks = {};
        std::vector<recipe_object_entry<recipe_object_texture>> textures = {};
        std::vector<recipe_object_user_asset_group> user_assets = {};

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("images", images));
            archive(cereal::make_nvp("textures", textures));
            archive(cereal::make_nvp("cubemaps", cubemaps));
            archive(cereal::make_nvp("geometries", geometries));
            archive(cereal::make_nvp("shapes", shapes));
            archive(cereal::make_nvp("meshes", meshes));
            archive(cereal::make_nvp("fonts", fonts));
            archive(cereal::make_nvp("audios", audios));
            archive(cereal::make_nvp("sound_tracks", sound_tracks));
            archive(cereal::make_nvp("skeletons", skeletons));
            archive(cereal::make_nvp("animations", animations));
            archive(cereal::make_nvp("motion_tracks", motion_tracks));
            archive(cereal::make_nvp("event_tracks", event_tracks));
            archive(cereal::make_nvp("user_assets", user_assets));
        }
    };

    [[nodiscard]] recipe_manager_object make_recipe(const manager_object& objects, mappings_manager_object_save& mappings);
    void apply_recipe(manager_window& window, manager_object& objects, mappings_manager_object_load& mappings, recipe_manager_object& recipe);

}
}
