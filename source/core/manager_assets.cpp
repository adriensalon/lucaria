#include <cstring>
#include <deque>
#include <exception>
#include <fstream>
#include <mutex>

#include <lucaria/core/assets_async.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_scenes.hpp>

#if defined(LUCARIA_PLATFORM_WEB)
#include <emscripten/fetch.h>
#endif

#if defined(LUCARIA_PLATFORM_ANDROID) || defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
#include <thread>
#endif

#if defined(LUCARIA_PLATFORM_PSP)
#include <pspkernel.h>
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

#if defined(LUCARIA_PLATFORM_PSP)
        struct _psp_fetch_context {
            manager_assets* assets;
            std::filesystem::path file_path;
            std::function<void(std::vector<char>)> callback;
        };

        static std::deque<_psp_fetch_context*> _psp_fetch_queue;
        static std::mutex _psp_fetch_queue_mutex;
        static SceUID _psp_fetch_semaphore = -1;
        static SceUID _psp_fetch_thread_id = -1;
        static constexpr int _psp_fetch_thread_priority = 0x28;

        static int _psp_fetch_thread(SceSize, void*)
        {
            while (true) {
                sceKernelWaitSema(_psp_fetch_semaphore, 1, nullptr);

                _psp_fetch_context* _context = nullptr;
                {
                    std::scoped_lock _lock(_psp_fetch_queue_mutex);
                    if (!_psp_fetch_queue.empty()) {
                        _context = _psp_fetch_queue.front();
                        _psp_fetch_queue.pop_front();
                    }
                }
                if (_context == nullptr) {
                    continue;
                }

                try {
                    _context->assets->load_bytes(_context->file_path, std::move(_context->callback));
                }
                catch (...) {
                    std::terminate();
                }

                _context->assets->async_fetches_waiting--;
                delete _context;
            }
            return 0;
        }

        static bool _psp_ensure_fetch_worker()
        {
            if (_psp_fetch_thread_id >= 0) {
                return true;
            }

            if (_psp_fetch_semaphore < 0) {
                _psp_fetch_semaphore = sceKernelCreateSema("lucaria_fetch_sema", 0, 0, 256, nullptr);
                if (_psp_fetch_semaphore < 0) {
                    return false;
                }
            }

            _psp_fetch_thread_id = sceKernelCreateThread("lucaria_fetch", _psp_fetch_thread, _psp_fetch_thread_priority, 0x10000, PSP_THREAD_ATTR_USER, nullptr);
            if (_psp_fetch_thread_id < 0) {
                return false;
            }
            if (sceKernelStartThread(_psp_fetch_thread_id, 0, nullptr) < 0) {
                sceKernelDeleteThread(_psp_fetch_thread_id);
                _psp_fetch_thread_id = -1;
                return false;
            }
            return true;
        }

        static bool _psp_enqueue_fetch(_psp_fetch_context* context)
        {
            if (!_psp_ensure_fetch_worker()) {
                return false;
            }
            {
                std::scoped_lock _lock(_psp_fetch_queue_mutex);
                _psp_fetch_queue.push_back(context);
            }
            sceKernelSignalSema(_psp_fetch_semaphore, 1);
            return true;
        }
#endif

        static void _fetch_bytes_impl(manager_assets& assets, const std::filesystem::path& file_path, std::function<void(std::vector<char>)> callback, bool persist)
        {
            assets.async_fetches_waiting++;
            std::filesystem::path _fetch_file_path = assets.resolve_fetch_path(file_path);

#if defined(LUCARIA_PLATFORM_WEB) && !defined(LUCARIA_PACKAGED_ASSETS)
            emscripten_fetch_attr_t _emscripten_fetch_attr;
            emscripten_fetch_attr_init(&_emscripten_fetch_attr);
            std::strcpy(_emscripten_fetch_attr.requestMethod, "GET");
            _emscripten_fetch_attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
            if (persist) {
                _emscripten_fetch_attr.attributes |= EMSCRIPTEN_FETCH_PERSIST_FILE;
            }
            using _callback_type = std::function<void(std::vector<char>)>;
            struct _fetch_context {
                manager_assets* assets;
                _callback_type callback;
            };
            _emscripten_fetch_attr.userData = new _fetch_context { &assets, std::move(callback) };
            _emscripten_fetch_attr.onsuccess = [](emscripten_fetch_t* fetch) {
                _fetch_context* _context = static_cast<_fetch_context*>(fetch->userData);
                std::vector<char> _buffer(fetch->data, fetch->data + fetch->numBytes); // one copy from fetch->data
                (_context->callback)(std::move(_buffer));
                _context->assets->async_fetches_waiting--;
                delete _context;
                emscripten_fetch_close(fetch);
            };
            _emscripten_fetch_attr.onerror = [](emscripten_fetch_t* fetch) {
                _fetch_context* _context = static_cast<_fetch_context*>(fetch->userData);
                std::fprintf(stderr, "fetch_bytes error: %s (%d)\n", fetch->statusText, fetch->status);
                delete _context;
                emscripten_fetch_close(fetch);
                std::terminate();
            };
            emscripten_fetch(&_emscripten_fetch_attr, _fetch_file_path.c_str());
#endif

#if defined(LUCARIA_PLATFORM_WEB) && defined(LUCARIA_PACKAGED_ASSETS)
            struct _fetch_context {
                manager_assets* assets;
                std::filesystem::path context_path;
                std::function<void(std::vector<char>)> context_callback;
            };
            emscripten_async_call(+[](void* user_data) {
                _fetch_context* _context = static_cast<_fetch_context*>(user_data);
                _context->assets->load_bytes(_context->context_path, std::move(_context->context_callback));
                _context->assets->async_fetches_waiting--; }, new _fetch_context { &assets, _fetch_file_path, std::move(callback) }, 0);
#endif

#if defined(LUCARIA_PLATFORM_ANDROID) || defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
            std::thread([&assets, _fetch_file_path, callback = std::move(callback)]() mutable {
                assets.load_bytes(_fetch_file_path, std::move(callback));
                assets.async_fetches_waiting--;
            }).detach();
#endif

#if defined(LUCARIA_PLATFORM_PSP)
            _psp_fetch_context* _context = new _psp_fetch_context { &assets, _fetch_file_path, std::move(callback) };
            if (_psp_enqueue_fetch(_context)) {
                return;
            }

            _context->assets->load_bytes(_context->file_path, std::move(_context->callback));
            _context->assets->async_fetches_waiting--;
            delete _context;
#endif
        }

    }

    void manager_assets::load_bytes(const std::filesystem::path& file_path, std::function<void(std::vector<char>)> callback)
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
        callback(std::move(buffer));
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

    std::future<std::vector<char>> manager_assets::fetch_bytes_future(const std::filesystem::path& file_path, bool persist)
    {
        std::shared_ptr<std::promise<std::vector<char>>> _promise = std::make_shared<std::promise<std::vector<char>>>();
        std::future<std::vector<char>> _future = _promise->get_future();

        _fetch_bytes_impl(*this, file_path, [_promise](std::vector<char> bytes) mutable { _promise->set_value(std::move(bytes)); }, persist);

        return _future;
    }

    std::future<std::vector<std::vector<char>>> manager_assets::fetch_bytes_future(const std::vector<std::filesystem::path>& file_paths, bool persist)
    {
        std::shared_ptr<std::promise<std::vector<std::vector<char>>>> _promise = std::make_shared<std::promise<std::vector<std::vector<char>>>>();
        std::future<std::vector<std::vector<char>>> _future = _promise->get_future();

        fetch_bytes(file_paths, [_promise](const std::vector<std::vector<char>>& bytes) mutable { _promise->set_value(bytes); }, persist);

        return _future;
    }

    std::future<std::vector<std::vector<char>>> manager_assets::fetch_bytes_future(const std::array<std::filesystem::path, 6>& file_paths, bool persist)
    {
        std::shared_ptr<std::promise<std::vector<std::vector<char>>>> _promise = std::make_shared<std::promise<std::vector<std::vector<char>>>>();
        std::future<std::vector<std::vector<char>>> _future = _promise->get_future();

        fetch_bytes(file_paths, [_promise](const std::vector<std::vector<char>>& bytes) mutable { _promise->set_value(bytes); }, persist);

        return _future;
    }

    void manager_assets::fetch_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback, bool persist)
    {
        _fetch_bytes_impl(*this, file_path, [callback](std::vector<char> bytes) { callback(bytes); }, persist);
    }

    void manager_assets::fetch_bytes(const std::vector<std::filesystem::path>& file_paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool persist)
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

    void manager_assets::fetch_bytes(const std::array<std::filesystem::path, 6>& file_paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool persist)
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

    void manager_assets::poll_load_contexts()
    {
#if !defined(LUCARIA_DISABLE_RELOAD)
       	// std::vector<assets_filewatch_change> _changes = refetch_filewatch_changes();
#endif
        std::size_t _index = 0;
        while (_index < active_load_contexts.size()) {
            std::shared_ptr<load_storage_context_base> _context = active_load_contexts[_index];
            _context->poll();
            ++_index;
        }

        collect_finished_load_contexts();
    }

    void manager_assets::gc_unused()
    {
        assets.gc_unused();
        poll_load_contexts();
    }

}
}
