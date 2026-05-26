#include <cstring>
#include <fstream>
#include <iostream>

#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/manager_scene.hpp>
#include <lucaria/core/utils_async.hpp>

#if defined(LUCARIA_PLATFORM_WEB)
#include <emscripten/fetch.h>
#else
#include <thread>
#endif

#if defined(LUCARIA_PLATFORM_ANDROID)
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
namespace lucaria {
extern android_app* g_app;
}
#endif

namespace lucaria {
namespace detail {

    namespace {

        static void _fetch_bytes_impl(manager_object& fetches, const std::filesystem::path& file_path, std::function<void(std::vector<char>)> callback, bool persist)
        {
            fetches.async_fetches_waiting++;
            std::filesystem::path _fetch_file_path = file_path;

#if !defined(LUCARIA_PACKAGED_ASSETS)
            _fetch_file_path = fetches.async_prefix_path / file_path;
#endif

#if defined(LUCARIA_PLATFORM_WEB) && !defined(LUCARIA_PACKAGED_ASSETS)
            emscripten_fetch_attr_t _emscripten_fetch_attr;
            emscripten_fetch_attr_init(&_emscripten_fetch_attr);
            std::strcpy(_emscripten_fetch_attr.requestMethod, "GET");
            _emscripten_fetch_attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
            if (persist) {
                _emscripten_fetch_attr.attributes |= EMSCRIPTEN_FETCH_PERSIST_FILE;
            }
            using _callback_type = std::function<void(std::vector<char>)>;
            _emscripten_fetch_attr.userData = new _callback_type(std::move(callback));
            _emscripten_fetch_attr.onsuccess = [](emscripten_fetch_t* fetch) {
                std::vector<char> _buffer(fetch->data, fetch->data + fetch->numBytes); // one copy from fetch->data
                _callback_type* _callback_ptr = static_cast<_callback_type*>(fetch->userData);
                (*_callback_ptr)(std::move(_buffer));
                delete _callback_ptr;
                fetches.async_fetches_waiting--;
                emscripten_fetch_close(fetch);
            };
            _emscripten_fetch_attr.onerror = [](emscripten_fetch_t* fetch) {
                _callback_type* _callback_ptr = static_cast<_callback_type*>(fetch->userData);
                std::fprintf(stderr, "fetch_bytes error: %s (%d)\n", fetch->statusText, fetch->status);
                delete _callback_ptr;
                emscripten_fetch_close(fetch);
                std::terminate();
            };
            emscripten_fetch(&_emscripten_fetch_attr, _fetch_file_path.c_str());
#endif

#if defined(LUCARIA_PLATFORM_WEB) && defined(LUCARIA_PACKAGED_ASSETS)
            struct _async_context {
                std::filesystem::path context_path;
                std::function<void(const std::vector<char>&)> context_callback;
            };
            _async_context* _context = new _async_context { _fetch_file_path, callback };
            emscripten_async_call(+[&fetches](void* user_data) {
                std::unique_ptr<_async_context> _context_inner(static_cast<_async_context*>(user_data));
                fetches.load_bytes(_context_inner->context_path, _context_inner->context_callback);
                fetches.async_fetches_waiting--; }, _context, 0);
#endif

#if defined(LUCARIA_PLATFORM_ANDROID) || defined(LUCARIA_PLATFORM_GLFW)
            std::thread([&fetches, _fetch_file_path, callback]() {
                fetches.load_bytes(_fetch_file_path, callback);
                fetches.async_fetches_waiting--;
            }).detach();
#endif
        }

    }

    void manager_object::load_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback)
    {
        std::string _path_str = file_path.string();

#if defined(LUCARIA_PLATFORM_ANDROID)
        AAssetManager* _asset_manager = lucaria::g_app->activity->assetManager;
        AAsset* _asset = AAssetManager_open(_asset_manager, _path_str.c_str(), AASSET_MODE_STREAMING);
        if (!_asset) {
            LUCARIA_DEBUG_ERROR("open failed: " + _path_str)
        }
        const off_t _length = AAsset_getLength(_asset);
        std::vector<char> buffer(static_cast<size_t>(_length));
        const int64_t _read = AAsset_read(_asset, buffer.data(), _length);
        AAsset_close(_asset);
        if (_read != _length) {
            LUCARIA_DEBUG_ERROR("read failed: " + _path_str)
        }
        callback(buffer);
#endif

#if !defined(LUCARIA_PLATFORM_ANDROID)
        std::ifstream _fstream(file_path, std::ios::binary);
        if (!_fstream) {
            LUCARIA_DEBUG_ERROR("open failed: " + _path_str)
        }
        _fstream.seekg(0, std::ios::end);
        const std::streamoff _size = _fstream.tellg();
        if (_size < 0) {
            LUCARIA_DEBUG_ERROR("tellg failed: " + _path_str)
        }
        std::vector<char> _bytes(static_cast<std::size_t>(_size));
        _fstream.seekg(0, std::ios::beg);
        _fstream.read(_bytes.data(), _bytes.size());
        if (!_fstream) {
            LUCARIA_DEBUG_ERROR("read failed: " + _path_str)
        }
        callback(std::move(_bytes));
#endif
    }

    void manager_object::fetch_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback, bool persist)
    {
        _fetch_bytes_impl(*this, file_path, [callback](std::vector<char> bytes) { callback(bytes); }, persist);
    }

    void manager_object::fetch_bytes(const std::vector<std::filesystem::path>& file_paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool persist)
    {
        const std::size_t _size = file_paths.size();

        if (_size == 0) {
            static const std::vector<std::vector<char>> _empty;
            callback(_empty);
            return;
        }

        std::shared_ptr<std::vector<std::vector<char>>> _shared_slots = std::make_shared<std::vector<std::vector<char>>>(_size);
        std::shared_ptr<std::atomic<std::size_t>> _shared_pending = std::make_shared<std::atomic<std::size_t>>(_size);

        for (std::size_t _index = 0; _index < _size; ++_index) {
            _fetch_bytes_impl(*this, file_paths[_index], [_index, _shared_slots, _shared_pending, callback](std::vector<char> bytes) {
                (*_shared_slots)[_index] = std::move(bytes);

                if (_shared_pending->fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    callback(*_shared_slots);
                } }, persist);
        }
    }

    void manager_object::fetch_bytes(const std::array<std::filesystem::path, 6>& file_paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool persist)
    {
        const std::size_t _size = file_paths.size();

        if (_size == 0) {
            static const std::vector<std::vector<char>> _empty;
            callback(_empty);
            return;
        }

        std::shared_ptr<std::vector<std::vector<char>>> _shared_slots = std::make_shared<std::vector<std::vector<char>>>(_size);
        std::shared_ptr<std::atomic<std::size_t>> _shared_pending = std::make_shared<std::atomic<std::size_t>>(_size);

        for (std::size_t _index = 0; _index < _size; ++_index) {
            _fetch_bytes_impl(*this, file_paths[_index], [_index, _shared_slots, _shared_pending, callback](std::vector<char> bytes) {
                (*_shared_slots)[_index] = std::move(bytes);

                if (_shared_pending->fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    callback(*_shared_slots);
                } }, persist);
        }
    }

    void manager_object::gc_unused()
    {
        animations.gc_unused();
        audios.gc_unused();
        cubemaps.gc_unused();
        event_tracks.gc_unused();
        fonts.gc_unused();
        geometries.gc_unused();
        images.gc_unused();
        meshes.gc_unused();
        motion_tracks.gc_unused();
        shapes.gc_unused();
        skeletons.gc_unused();
        sound_tracks.gc_unused();
        textures.gc_unused();
    }

    recipe_manager_object make_recipe(const manager_object& objects, mappings_manager_object_save& mappings)
    {
        recipe_manager_object _recipes;

        make_recipes_for(_recipes.images, objects.images, mappings.images);
        make_recipes_for(_recipes.textures, objects.textures, mappings.textures);
        make_recipes_for(_recipes.cubemaps, objects.cubemaps, mappings.cubemaps);
        make_recipes_for(_recipes.geometries, objects.geometries, mappings.geometries);
        make_recipes_for(_recipes.shapes, objects.shapes, mappings.shapes);
        make_recipes_for(_recipes.meshes, objects.meshes, mappings.meshes);
        make_recipes_for(_recipes.fonts, objects.fonts, mappings.fonts);
        make_recipes_for(_recipes.audios, objects.audios, mappings.audios);
        make_recipes_for(_recipes.sound_tracks, objects.sound_tracks, mappings.sound_tracks);
        make_recipes_for(_recipes.skeletons, objects.skeletons, mappings.skeletons);
        make_recipes_for(_recipes.animations, objects.animations, mappings.animations);
        make_recipes_for(_recipes.motion_tracks, objects.motion_tracks, mappings.motion_tracks);
        make_recipes_for(_recipes.event_tracks, objects.event_tracks, mappings.event_tracks);

        for (const auto& [_type_id, _callbacks] : objects.user_asset_types) {
            recipe_object_user_asset_group& _group = _recipes.user_assets.emplace_back();
            _group.type_id = _type_id;
            _group.objects = const_cast<manager_object*>(&objects);
            _group.callbacks = const_cast<user_asset_type_callbacks*>(&_callbacks);
        }

        return _recipes;
    }

}
}